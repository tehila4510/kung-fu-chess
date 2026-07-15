# Kung Fu Chess — Agent Instructions

> **Cursor:** Primary instructions live in [`.cursor/rules/agents.mdc`](.cursor/rules/agents.mdc) (`alwaysApply: true`).
> Cursor Agent loads that rule automatically — keep both files in sync when editing.

C++17 console game that reads board setup and commands from stdin.

## Layout

```
include/
├── model/       Board, Piece, Position, GameState
├── rules/       IPieceRules strategies (one *Rules.{h,cpp} per piece), RuleEngine
├── realtime/    Motion, RealTimeArbiter
├── engine/      GameEngine
├── io/          BoardParser, BoardPrinter (IBoardPrinter)
├── input/       Controller, BoardMapper
├── graphics/    Animation, AnimationLoader, AnimationCache (AnimationSpec), BoardLayout
├── view/        Img (OpenCV wrapper), Renderer
├── texttests/   ScriptParser (IScriptSource), ScriptRunner, ScriptCommand
└── App.h        Composition root

src/             mirrors include/ paths + main.cpp + graphics_main.cpp
build.bat        primary Windows build script
CMakeLists.txt   optional CMake build
.cursor/rules/   Cursor MDC rules (agents.mdc = entry point)
```

> Legacy `Board.h`/`Board.cpp`, `Game.h`/`Game.cpp`, and `MoveRules.h`/`MoveRules.cpp`
> have been removed. `src/main.cpp` is the composition root; it constructs the concrete `ScriptParser`
> (an `IScriptSource`) and `BoardPrinter` (an `IBoardPrinter`), injects them into `ScriptRunner` by const
> reference, then calls `ScriptRunner::run(std::cin, std::cout)`, which parses the `Board:`/`Commands:`
> protocol and replays it against the `model/rules/realtime/engine/input/io` layers.

## Build & verify

```powershell
.\build.bat        # build and run the app (src/main.cpp composition root)
.\build.bat test   # build and run the engine test suite
.\build.bat graphics  # animated board window (OpenCV via Img only; requires MSVC + OpenCV_451)
```

`build.bat graphics` builds `src/graphics_main.cpp` — the single graphics entry point. It reads the
initial layout from `assets/pieces/board.csv`, draws `assets/board.png`, and plays each piece's looping
`idle` animation. No game logic yet; the window closes on ESC/Q or when the user closes it.

Compiler flags: `-std=c++17 -Iinclude -Wall -Wextra -Wpedantic`

## Graphics dependencies

- Engine, console app, and tests: **STL only** — no third-party libraries
- Graphics: use the Qama Tech `Img` wrapper (`include/view/Img.h`, `src/view/Img.cpp`) from the course repo
- OpenCV (`OpenCV_451`) is allowed **only** inside `Img.h` / `Img.cpp` — never `#include <opencv2/...>` elsewhere
- No other graphics or rendering libraries (SDL, SFML, GLFW, etc.); all graphics code goes through the `Img` API
- `graphics/` (animation + asset loading) and `Renderer` depend on `Img` only; they never touch OpenCV types directly

## Architecture

Dependencies point inward only:

```
main/App → input/view/io → engine → rules/realtime → model
```

| Layer | Key types | Responsibility |
|-------|-----------|----------------|
| Model | `Board`, `Piece`, `Position`, `GameState` | Grid data, bounds, cell access — no I/O or rules |
| Rules | `IPieceRules`, `RuleEngine` | Per-piece move legality via Strategy pattern |
| Realtime | `Motion`, `RealTimeArbiter` | Timed movement, arrival and capture resolution |
| Engine | `GameEngine` | Orchestrates model + rules + realtime; exposes `MoveResult`, `GameSnapshot` |
| I/O | `BoardParser`, `BoardPrinter` | Parse and serialize board text |
| Input | `Controller`, `BoardMapper` | Map clicks/pixels to engine calls |
| Graphics | `Animation`, `AnimationCache`, `AnimationLoader`, `BoardLayout` | Load/scale sprite frames, play animations, read asset board layout — depends on `Img` only |
| View | `Img`, `Renderer` | `Img` wraps OpenCV; `Renderer` composites sprites over the board and drives the window — never mutate `Board` |
| Text tests | `ScriptParser`, `ScriptRunner` | Scripted stdin integration |
| App | `App` | Wire layers; `main` dispatches commands |

### Cross-boundary DTOs

- `MoveValidation` — rule check result (`is_valid`, `reason`)
- `MoveResult` — engine accept/reject (`is_accepted`, `reason`)
- `GameSnapshot` — read-only board + state for view
- `ArrivalEvent` — piece landing and capture info from arbiter

## Input format

1. Board rows until `Board:` marker (tokens like `.`, `wK`, `bQ`)
2. `Commands:` section — `click X Y`, `jump X Y`, `wait MS`, `print board`

Errors go to stdout (e.g. `ERROR UNKNOWN_TOKEN`, `ERROR ROW_WIDTH_MISMATCH`).

`jump X Y` lifts the piece at that cell airborne in place (no destination — it always returns to the same
square) for a fixed 1s duration, bypassing normal per-cell travel time. It lands back down and captures
whatever occupies the square at that point (an enemy that moved in while it was airborne); it is rejected
if the cell is empty or that color already has a motion in flight.

## Language

- All code comments, error strings, and reason codes must be in **English**
- Never write Hebrew comments in source files

## Header vs implementation (strict)

- `.h` files are **headers only**: include guards, includes, declarations, `static constexpr` constants, and defaulted special members (`= default`)
- `.cpp` files hold **all implementations** — every function and method body lives in the matching source file, never inline in the header

## Exception handling (throw up, catch at boundaries)

**Goal:** validate early, throw from inner layers, catch only where errors must be translated or recovered — never log-and-rethrow the same exception at every layer.

```
model / realtime / view(Img)  →  throw only (no try-catch)
         ↓ propagates
rules / engine / input        →  catch & translate to DTO status codes
         ↓ propagates (unexpected)
ScriptRunner / main           →  single catch + log + graceful exit
```

### Model & low-level layers — validation only

Applies to: `model/`, `realtime/`, low-level helpers (`BoardMapper` constructor, `Img`).

- Perform defensive checks; **throw** from `<stdexcept>` when preconditions fail:
  - `std::invalid_argument` — bad arguments, malformed tokens, inconsistent state
  - `std::out_of_range` — grid/cell bounds, invalid coordinates
  - `std::runtime_error` — I/O preconditions (e.g. unreadable stream, missing image)
- **Do not** wrap throws in try-catch here. Let exceptions propagate upward.
- **Do not** log in model/low-level code.

### No redundant catch-and-rethrow

- **Never** add `try { … } catch (const std::exception& e) { std::cerr << …; throw; }` unless local cleanup is required (RAII should handle resources).
- One exception → one log line, at the top boundary only.

### Boundary catchers — translate, don't rethrow

| Layer | Catch when | Return / action |
|-------|------------|-----------------|
| `RuleEngine::validateMove` | `Piece::fromToken` or board access fails | `MoveValidation{ false, "invalid_piece" }` |
| `PieceRules` (`pieceAt`) | token parse fails | `std::nullopt` → `isValidMove` returns `false` |
| `GameEngine::requestMove` / `requestJump` | unexpected runtime error | `MoveResult{ false, "runtime_error" }` |
| `Controller::click` / `jump` | unexpected error during input handling | `ClickResult` / `MoveResult` with `runtime_error` |
| `BoardParser` | — | prefer `BoardParseStatus` enum; throw only for unreadable streams |
| `ScriptRunner::run` | any uncaught `std::exception` | log once to `stderr`, print `ERROR RUNTIME_FAILURE` to stdout, return `false` |
| `main` / `graphics_main` | startup or top-level failure | log once, non-zero exit |

`GameEngine::wait` and `setup` **propagate** exceptions — `ScriptRunner` is the recovery boundary.

### Keep it simple

- No try-catch on operations that cannot realistically fail: `toString()`, basic integer math, trivial getters.
- Catch by `const std::exception&` at boundaries; do not catch `...` unless rethrowing immediately.
- Prefer `#include <stdexcept>` over custom exception types.

## When changing code

- If a change affects layout, architecture, layers, build setup, input protocol, or project conventions — update the instruction files accordingly:
  - `.cursor/rules/agents.mdc` (primary for Cursor)
  - `AGENTS.md` (keep in sync with agents.mdc)
  - Relevant `.cursor/rules/*.mdc` layer files as needed
- Keep changes minimal and focused on the requested task
- Place logic in the correct layer — not in `main`
- Match naming: PascalCase classes, camelCase methods
- Register piece rules by kind in the `RuleEngine` constructor map — no piece-specific branches in `validateMove`
- Do not add external libraries to the engine/console/tests — STL only there
- Graphics: Qama Tech `Img` only; OpenCV confined to `Img.h` / `Img.cpp` — remove any other graphics libs immediately
- Do not commit `build/` artifacts unless explicitly asked
- Register new `.cpp` files in both `build.bat` and `CMakeLists.txt`
- Do not change the stdin command protocol without explicit request
