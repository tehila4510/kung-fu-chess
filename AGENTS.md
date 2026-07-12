# Kung Fu Chess — Agent Instructions

> **Cursor:** Primary instructions live in [`.cursor/rules/agents.mdc`](.cursor/rules/agents.mdc) (`alwaysApply: true`).
> Cursor Agent loads that rule automatically — keep both files in sync when editing.

C++17 console game that reads board setup and commands from stdin.

## Layout

```
include/
├── model/       Board, Piece, Position, GameState
├── rules/       IPieceRule strategies, RuleEngine
├── realtime/    Motion, RealTimeArbiter
├── engine/      GameEngine
├── io/          BoardParser, BoardPrinter (IBoardPrinter)
├── input/       Controller, BoardMapper
├── view/        Renderer, ImageView
├── texttests/   ScriptParser (IScriptSource), ScriptRunner, ScriptCommand
└── App.h        Composition root

src/             mirrors include/ paths + main.cpp
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
```

Compiler flags: `-std=c++17 -Iinclude -Wall -Wextra -Wpedantic`

## Architecture

Dependencies point inward only:

```
main/App → input/view/io → engine → rules/realtime → model
```

| Layer | Key types | Responsibility |
|-------|-----------|----------------|
| Model | `Board`, `Piece`, `Position`, `GameState` | Grid data, bounds, cell access — no I/O or rules |
| Rules | `IPieceRule`, `RuleEngine` | Per-piece move legality via Strategy pattern |
| Realtime | `Motion`, `RealTimeArbiter` | Timed movement, arrival and capture resolution |
| Engine | `GameEngine` | Orchestrates model + rules + realtime; exposes `MoveResult`, `GameSnapshot` |
| I/O | `BoardParser`, `BoardPrinter` | Parse and serialize board text |
| Input | `Controller`, `BoardMapper` | Map clicks/pixels to engine calls |
| View | `Renderer`, `ImageView` | Render `GameSnapshot` — never mutate `Board` |
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

## When changing code

- If a change affects layout, architecture, layers, build setup, input protocol, or project conventions — update the instruction files accordingly:
  - `.cursor/rules/agents.mdc` (primary for Cursor)
  - `AGENTS.md` (keep in sync with agents.mdc)
  - Relevant `.cursor/rules/*.mdc` layer files as needed
- Keep changes minimal and focused on the requested task
- Place logic in the correct layer — not in `main`
- Match naming: PascalCase classes, camelCase methods
- Use `ruleFor(kind)` factory — no piece-specific branches in `RuleEngine`
- Do not add external libraries — use STL only
- Do not commit `build/` artifacts unless explicitly asked
- Register new `.cpp` files in both `build.bat` and `CMakeLists.txt`
- Do not change the stdin command protocol without explicit request
