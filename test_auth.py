import asyncio
import json
import time
import uuid
import websockets

SERVER = "ws://127.0.0.1:9002"
QUEUE_TIMEOUT_S = 60
TIMEOUT_SLACK_S = 5


def uniq(prefix: str) -> str:
    return f"{prefix}_{uuid.uuid4().hex[:8]}"


async def recv_json(ws, timeout=10.0):
    raw = await asyncio.wait_for(ws.recv(), timeout=timeout)
    return json.loads(raw)


async def connect_and_auth(username: str, password: str = "1234"):
    ws = await websockets.connect(SERVER)
    msg = await recv_json(ws)
    assert msg.get("type") == "auth_required", msg
    await ws.send(f"AUTH {username} {password}")
    auth = await recv_json(ws)
    return ws, auth


async def test_auth_no_auto_seat():
    print("=== 1. AUTH does not auto-seat (no welcome/state) ===")
    user = uniq("auth_only")
    async with websockets.connect(SERVER) as ws:
        assert (await recv_json(ws))["type"] == "auth_required"
        await ws.send(f"AUTH {user} 1234")
        auth = await recv_json(ws)
        assert auth["type"] == "auth_ok", auth
        assert auth["username"] == user
        assert "rating" in auth
        assert auth.get("role") == "player", auth
        assert "color" not in auth or auth.get("color") in ("", None)

        # No welcome/state should arrive without PLAY + match
        try:
            extra = await asyncio.wait_for(ws.recv(), timeout=0.5)
            raise AssertionError(f"unexpected message after AUTH: {extra}")
        except asyncio.TimeoutError:
            pass
    print("OK: auth_ok only, no welcome/state")


async def test_play_searching_alone():
    print("\n=== 2. AUTH then PLAY alone -> searching (still no seat) ===")
    user = uniq("search")
    ws, auth = await connect_and_auth(user)
    try:
        assert auth["type"] == "auth_ok" and auth.get("role") == "player", auth
        await ws.send("PLAY")
        msg = await recv_json(ws)
        assert msg["type"] == "searching", msg
        try:
            extra = await asyncio.wait_for(ws.recv(), timeout=0.5)
            raise AssertionError(f"unexpected message after searching: {extra}")
        except asyncio.TimeoutError:
            pass
        print("OK: searching, no welcome yet")
    finally:
        await ws.close()


async def test_match_two_players():
    print("\n=== 3. Two close-ELO clients match and get W/B seats ===")
    u1, u2 = uniq("w"), uniq("b")
    ws1, auth1 = await connect_and_auth(u1)
    ws2, auth2 = await connect_and_auth(u2)
    try:
        assert auth1["type"] == "auth_ok" and auth2["type"] == "auth_ok"
        assert abs(auth1["rating"] - auth2["rating"]) <= 100

        await ws1.send("PLAY")
        s1 = await recv_json(ws1)
        assert s1["type"] == "searching", s1

        await ws2.send("PLAY")

        # Both should get welcome + state (order may interleave per socket)
        async def expect_seat(ws):
            welcome = await recv_json(ws)
            assert welcome["type"] == "welcome", welcome
            assert welcome["color"] in ("W", "B"), welcome
            state = await recv_json(ws)
            assert state["type"] == "state", state
            assert state.get("reason") == "welcome"
            return welcome["color"]

        c1 = await expect_seat(ws1)
        c2 = await expect_seat(ws2)
        assert {c1, c2} == {"W", "B"}, (c1, c2)
        print(f"OK: matched seats {c1}/{c2}")
        return ws1, ws2, c1, c2
    except Exception:
        await ws1.close()
        await ws2.close()
        raise


async def test_viewer_third_client(ws1, ws2):
    print("\n=== 4. Third client -> viewer (not rejected, no seat) ===")
    user = uniq("viewer")
    ws3, auth = await connect_and_auth(user)
    try:
        assert auth["type"] == "auth_ok", auth
        assert auth.get("role") == "viewer", auth
        assert "color" not in auth or auth.get("color") in ("", None)

        await ws3.send("PLAY")
        err = await recv_json(ws3)
        assert err["type"] == "error", err
        assert err["reason"] == "viewer_cannot_play", err

        try:
            extra = await asyncio.wait_for(ws3.recv(), timeout=0.5)
            raise AssertionError(f"viewer got unexpected seat/welcome: {extra}")
        except asyncio.TimeoutError:
            pass
        print("OK: third client is viewer, PLAY rejected, no seat")
    finally:
        await ws3.close()
        await ws1.close()
        await ws2.close()


async def test_match_timeout():
    print("\n=== 5. PLAY alone -> match_not_found after ~60s ===")
    user = uniq("timeout")
    ws, auth = await connect_and_auth(user)
    try:
        assert auth["type"] == "auth_ok" and auth.get("role") == "player", auth
        await ws.send("PLAY")
        searching = await recv_json(ws)
        assert searching["type"] == "searching", searching

        t0 = time.monotonic()
        msg = await recv_json(ws, timeout=QUEUE_TIMEOUT_S + TIMEOUT_SLACK_S)
        elapsed = time.monotonic() - t0
        assert msg["type"] == "error", msg
        assert msg["reason"] == "match_not_found", msg
        assert elapsed >= QUEUE_TIMEOUT_S - 2, f"too early: {elapsed:.1f}s"
        assert elapsed <= QUEUE_TIMEOUT_S + TIMEOUT_SLACK_S, f"too late: {elapsed:.1f}s"
        print(f"OK: match_not_found after {elapsed:.1f}s")
    finally:
        await ws.close()


async def test_wrong_password():
    print("\n=== 6. Wrong password still rejected ===")
    user = uniq("pwd")
    ws, auth = await connect_and_auth(user)
    await ws.close()
    async with websockets.connect(SERVER) as ws2:
        await recv_json(ws2)
        await ws2.send(f"AUTH {user} WRONG")
        err = await recv_json(ws2)
        assert err == {"type": "error", "reason": "bad_password"}, err
    print("OK: bad_password")


async def main():
    await test_auth_no_auto_seat()
    await test_play_searching_alone()
    ws1, ws2, _, _ = await test_match_two_players()
    await test_viewer_third_client(ws1, ws2)
    await test_wrong_password()
    await test_match_timeout()
    print("\nAll tests passed.")


if __name__ == "__main__":
    asyncio.run(main())
