#include "server/WebSocketServer.h"

#include "protocol/StateSerializer.h"

#include <iostream>
#include <vector>

namespace {

bool sameConnection(ConnectionHdl a, ConnectionHdl b) {
    return !a.owner_before(b) && !b.owner_before(a);
}

}  // namespace

WebSocketServer::WebSocketServer(const std::string& dbPath)
    : usersRepo_(dbPath),
      users_(usersRepo_),
      auth_(users_),
      moveLog_(std::cout),
      sound_("assets/sounds", std::cout) {
    server_.init_asio();
    server_.set_reuse_addr(true);
    server_.clear_access_channels(websocketpp::log::alevel::all);
    server_.set_access_channels(websocketpp::log::alevel::connect |
                                websocketpp::log::alevel::disconnect |
                                websocketpp::log::alevel::app);

    server_.set_open_handler([this](ConnectionHdl hdl) { onOpen(hdl); });
    server_.set_close_handler([this](ConnectionHdl hdl) { onClose(hdl); });
    server_.set_message_handler(
        [this](ConnectionHdl hdl, WsServer::message_ptr msg) { onMessage(hdl, msg); });
}

WebSocketServer::~WebSocketServer() {
    running_ = false;
    rooms_.clear();
}

void WebSocketServer::sendJson(ConnectionHdl hdl, const std::string& payload) {
    websocketpp::lib::error_code ec;
    server_.send(hdl, payload, websocketpp::frame::opcode::text, ec);
    if (ec) {
        std::cerr << "send failed: " << ec.message() << '\n';
    }
}

void WebSocketServer::broadcastToRoom(const MatchRoom& room,
                                      const std::string& payload) {
    if (room.hasWhite()) {
        sendJson(room.whiteHdl(), payload);
    }
    if (room.hasBlack()) {
        sendJson(room.blackHdl(), payload);
    }
    // TODO: full viewer support (Rooms feature)
}

void WebSocketServer::sendSeatWelcome(ConnectionHdl hdl, MatchRoom& room,
                                      char color,
                                      const std::vector<GameEvent>& events) {
    sendJson(hdl, protocol::serializeWelcomeJson(color));
    sendJson(hdl, protocol::serializeGameStateJson(room.session().engine(), events,
                                                   "ok", "welcome"));
}

int WebSocketServer::seatedPlayerCount() const {
    int count = 0;
    for (const auto& entry : clients_) {
        if (entry.second.state == ClientState::Seated) {
            ++count;
        }
    }
    return count;
}

bool WebSocketServer::usernameAlreadyConnected(const std::string& username) const {
    for (const auto& entry : clients_) {
        if (entry.second.state == ClientState::PendingAuth) {
            continue;
        }
        if (entry.second.username == username) {
            return true;
        }
    }
    return false;
}

void WebSocketServer::onOpen(ConnectionHdl hdl) {
    // Accept all connections. Players AUTH then PLAY; extra non-play clients
    // use Viewer (stub until Rooms). Do not reject with server_full.
    ClientConn client;
    client.playerId = nextPlayerId_++;
    clients_[hdl] = client;
    playerToHdl_[client.playerId] = hdl;
    sendJson(hdl, protocol::serializeAuthRequiredJson());
    std::cout << "Client connected — AUTH required (" << clients_.size()
              << " connected)\n";
}

void WebSocketServer::tearDownMatch(int matchId, ConnectionHdl exceptHdl) {
    auto roomIt = rooms_.find(matchId);
    if (roomIt == rooms_.end()) {
        return;
    }
    MatchRoom& room = *roomIt->second;

    auto resetOpponent = [this, &exceptHdl](ConnectionHdl other) {
        if (sameConnection(other, exceptHdl)) {
            return;
        }
        auto it = clients_.find(other);
        if (it == clients_.end()) {
            return;
        }
        sendJson(other, protocol::serializeErrorJson("opponent_disconnected"));
        it->second.state = ClientState::Authenticated;
        it->second.matchId = -1;
        it->second.color = '?';
    };

    // TODO: implement 20s auto-resign with countdown per spec
    if (room.hasWhite()) {
        resetOpponent(room.whiteHdl());
    }
    if (room.hasBlack()) {
        resetOpponent(room.blackHdl());
    }
    rooms_.erase(roomIt);
}

void WebSocketServer::onClose(ConnectionHdl hdl) {
    auto clientIt = clients_.find(hdl);
    if (clientIt == clients_.end()) {
        return;
    }
    ClientConn client = clientIt->second;

    matchQueue_.remove(client.playerId);
    playerToHdl_.erase(client.playerId);

    if (client.state == ClientState::Seated && client.matchId >= 0) {
        // TODO: implement 20s auto-resign with countdown per spec
        tearDownMatch(client.matchId, hdl);
        std::cout << "Player " << client.username << " disconnected from match "
                  << client.matchId << '\n';
    }

    clients_.erase(clientIt);
}

void WebSocketServer::handleAuth(ConnectionHdl hdl, const std::string& line) {
    try {
        const AuthCommandResult auth = auth_.handle(line);
        if (!auth.ok) {
            sendJson(hdl, protocol::serializeErrorJson(auth.reason));
            return;
        }

        auto clientIt = clients_.find(hdl);
        if (clientIt == clients_.end()) {
            return;
        }
        ClientConn& client = clientIt->second;
        if (client.state != ClientState::PendingAuth) {
            sendJson(hdl, protocol::serializeErrorJson("already_authenticated"));
            return;
        }
        if (usernameAlreadyConnected(auth.username)) {
            sendJson(hdl, protocol::serializeErrorJson("already_logged_in"));
            return;
        }

        client.username = auth.username;
        client.rating = auth.rating;

        // When a match already has both seats filled, further AUTHs are viewers
        // (not rejected). Full spectator broadcast is Rooms TODO.
        // TODO: full viewer support (Rooms feature)
        if (seatedPlayerCount() >= 2) {
            client.state = ClientState::Viewer;
            sendJson(hdl, protocol::serializeAuthOkJson(auth.username, auth.rating,
                                                        "viewer"));
            std::cout << "Viewer " << auth.username << " authenticated (rating "
                      << auth.rating << ")\n";
            return;
        }

        client.state = ClientState::Authenticated;
        sendJson(hdl, protocol::serializeAuthOkJson(auth.username, auth.rating,
                                                    "player"));
        std::cout << "Player " << auth.username << " authenticated (rating "
                  << auth.rating << ")\n";
    } catch (const std::exception& ex) {
        std::cerr << "AUTH failed: " << ex.what() << '\n';
        sendJson(hdl, protocol::serializeErrorJson("runtime_error"));
    }
}

void WebSocketServer::createMatch(ConnectionHdl white, ConnectionHdl black) {
    auto whiteIt = clients_.find(white);
    auto blackIt = clients_.find(black);
    if (whiteIt == clients_.end() || blackIt == clients_.end()) {
        return;
    }

    const int matchId = nextMatchId_++;
    auto room = std::make_unique<MatchRoom>(matchId, users_, &moveLog_, &sound_);
    room->seatPlayers(white, whiteIt->second.username, black,
                      blackIt->second.username);

    whiteIt->second.state = ClientState::Seated;
    whiteIt->second.matchId = matchId;
    whiteIt->second.color = 'W';

    blackIt->second.state = ClientState::Seated;
    blackIt->second.matchId = matchId;
    blackIt->second.color = 'B';

    const auto events = room->session().drainEvents();
    sendSeatWelcome(white, *room, 'W', events);
    sendSeatWelcome(black, *room, 'B', events);

    std::cout << "Match " << matchId << ": " << whiteIt->second.username
              << " (W) vs " << blackIt->second.username << " (B)\n";

    rooms_[matchId] = std::move(room);
}

void WebSocketServer::tryMatch(ConnectionHdl joiner) {
    auto joinerIt = clients_.find(joiner);
    if (joinerIt == clients_.end() || joinerIt->second.state != ClientState::Queued) {
        return;
    }

    const auto matched = matchQueue_.tryMatch(joinerIt->second.playerId);
    if (!matched) {
        sendJson(joiner, protocol::serializeSearchingJson());
        return;
    }

    auto whiteHdlIt = playerToHdl_.find(matched->white);
    auto blackHdlIt = playerToHdl_.find(matched->black);
    if (whiteHdlIt == playerToHdl_.end() || blackHdlIt == playerToHdl_.end()) {
        return;
    }

    // Earlier waiter is White; joiner is Black (assigned inside MatchQueue).
    createMatch(whiteHdlIt->second, blackHdlIt->second);
}

void WebSocketServer::handlePlay(ConnectionHdl hdl) {
    auto clientIt = clients_.find(hdl);
    if (clientIt == clients_.end()) {
        return;
    }
    ClientConn& client = clientIt->second;

    if (client.state == ClientState::Viewer) {
        // TODO: full viewer support (Rooms feature)
        sendJson(hdl, protocol::serializeErrorJson("viewer_cannot_play"));
        return;
    }
    if (client.state != ClientState::Authenticated) {
        sendJson(hdl, protocol::serializeErrorJson("not_authenticated"));
        return;
    }

    client.state = ClientState::Queued;
    matchQueue_.enqueue(client.playerId, client.rating,
                        std::chrono::steady_clock::now());
    std::cout << "Player " << client.username << " entered queue (rating "
              << client.rating << ")\n";
    tryMatch(hdl);
}

void WebSocketServer::onMessage(ConnectionHdl hdl, WsServer::message_ptr msg) {
    const std::string line = msg->get_payload();
    auto clientIt = clients_.find(hdl);
    if (clientIt == clients_.end()) {
        return;
    }
    ClientConn& client = clientIt->second;

    if (client.state == ClientState::PendingAuth) {
        handleAuth(hdl, line);
        return;
    }

    if (client.state == ClientState::Viewer) {
        // TODO: full viewer support (Rooms feature)
        if (line == "PLAY") {
            sendJson(hdl, protocol::serializeErrorJson("viewer_cannot_play"));
        } else {
            sendJson(hdl, protocol::serializeErrorJson("viewer_readonly"));
        }
        return;
    }

    if (client.state == ClientState::Authenticated ||
        client.state == ClientState::Queued) {
        if (line == "PLAY") {
            if (client.state == ClientState::Queued) {
                sendJson(hdl, protocol::serializeErrorJson("already_searching"));
                return;
            }
            handlePlay(hdl);
            return;
        }
        sendJson(hdl, protocol::serializeErrorJson("not_seated"));
        return;
    }

    if (client.state != ClientState::Seated || client.matchId < 0) {
        sendJson(hdl, protocol::serializeErrorJson("not_seated"));
        return;
    }

    auto roomIt = rooms_.find(client.matchId);
    if (roomIt == rooms_.end()) {
        sendJson(hdl, protocol::serializeErrorJson("match_gone"));
        return;
    }

    MatchRoom& room = *roomIt->second;
    const SessionResult result = room.session().handleCommand(client.color, line);
    const std::string status = result.accepted ? "ok" : "rejected";
    broadcastToRoom(room, protocol::serializeGameStateJson(
                              room.session().engine(), result.events, status,
                              result.reason));
}

void WebSocketServer::expireQueueEntries() {
    const auto now = std::chrono::steady_clock::now();
    const std::vector<MatchQueue::PlayerId> expired = matchQueue_.expire(now);

    for (MatchQueue::PlayerId playerId : expired) {
        auto hdlIt = playerToHdl_.find(playerId);
        if (hdlIt == playerToHdl_.end()) {
            continue;
        }
        auto clientIt = clients_.find(hdlIt->second);
        if (clientIt == clients_.end()) {
            continue;
        }
        clientIt->second.state = ClientState::Authenticated;
        sendJson(hdlIt->second, protocol::serializeErrorJson("match_not_found"));
        std::cout << "Matchmaking timeout for " << clientIt->second.username
                  << '\n';
    }
}

void WebSocketServer::scheduleTick() {
    if (!tickTimer_) {
        return;
    }
    tickTimer_->expires_after(std::chrono::milliseconds(kTickMs));
    tickTimer_->async_wait([this](const asio::error_code& ec) { onTick(ec); });
}

void WebSocketServer::onTick(const asio::error_code& ec) {
    if (ec || !running_) {
        return;
    }

    expireQueueEntries();

    for (auto& entry : rooms_) {
        MatchRoom& room = *entry.second;
        GameSession& session = room.session();
        const bool hadMotion = !session.engine().activeMotions().empty();
        const bool hadRest = !session.engine().activeRests().empty();
        const SessionResult result = session.tick(kTickMs);
        const bool hasMotion = !session.engine().activeMotions().empty();
        const bool hasRest = !session.engine().activeRests().empty();

        if (!result.events.empty() || hadMotion || hadRest || hasMotion ||
            hasRest) {
            broadcastToRoom(room, protocol::serializeGameStateJson(
                                      session.engine(), result.events, "ok",
                                      "tick"));
        }
    }

    scheduleTick();
}

int WebSocketServer::run() {
    try {
        server_.listen(kPort);
        server_.start_accept();

        tickTimer_ = std::make_unique<asio::steady_timer>(server_.get_io_service());
        running_ = true;
        scheduleTick();

        std::cout << "Kung Fu Chess WebSocket server listening on port " << kPort
                  << '\n';
        std::cout << "Flow: AUTH <user> <pass> then PLAY to matchmake\n";
        std::cout << "Matchmaking: ELO +/-" << MatchQueue::kEloWindow
                  << ", timeout " << (MatchQueue::kQueueTimeoutMs / 1000)
                  << "s\n";
        std::cout << "Commands after match: WMe2e4 | WJe2 | WAIT 100 | STATE\n";

        server_.run();
        running_ = false;
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Server failed: " << ex.what() << '\n';
        running_ = false;
        return 1;
    }
}
