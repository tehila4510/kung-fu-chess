# Kung Fu Chess

C++17 real-time chess game with a console (scripted stdin) mode and an interactive graphics window.

## Requirements

- **Console / engine tests:** C++17 compiler (`g++` from MSYS2/MinGW, or MSVC)
- **Graphics mode / graphics tests:** MSVC + bundled `OpenCV_451` in the project root (course starter pack)

## Build & Run

```powershell
.\build.bat              # build and run the console app
.\build.bat test         # engine + STL graphics helper tests
.\build.bat graphics     # interactive board window (requires MSVC + OpenCV_451)
.\build.bat graphics-test  # OpenCV graphics unit tests
```

Console app with input from a file:

```powershell
Get-Content input.txt | .\build\KungFuChess.exe
```

## Input protocol (console)

1. Board rows until a `Board:` marker (tokens: `.`, `wK`, `bQ`, …)
2. `Commands:` section — `click X Y`, `jump X Y`, `wait MS`, `print board`

Errors are printed to stdout (e.g. `ERROR UNKNOWN_TOKEN`, `ERROR ROW_WIDTH_MISMATCH`).

## Project structure

```
include/
├── model/       Board, Piece, Position, GameState
├── rules/       IPieceRules strategies, RuleEngine
├── realtime/    Motion, Rest, RealTimeArbiter
├── engine/      GameEngine
├── io/          BoardParser, BoardPrinter
├── input/       Controller, BoardMapper
├── graphics/    Animation, PieceVisual, BoardLayout, …
├── view/        Img (OpenCV wrapper), Renderer
├── texttests/   ScriptParser, ScriptRunner
├── App.h
└── GraphicsApplication.h

src/             mirrors include/ + main.cpp + graphics_main.cpp
assets/          board.png (background), piece sprites, configs, board.csv
tests/           unit tests
build.bat        primary Windows build script
CMakeLists.txt   optional CMake build
```

## Assets

Game art lives under `assets/`:

- `assets/board.png` — board background
- `assets/pieces/` — per-piece state sprites (`idle`, `move`, `jump`, `long_rest`, `short_rest`), `config.json`, and `board.csv`

These files are tracked in git. OpenCV itself (`OpenCV_451/`) is **not** committed — download it separately from the course starter.

## Architecture

Dependencies point inward only:

```
main/App → input/view/io → engine → rules/realtime → model
```

Graphics stack:

```
graphics_main → GraphicsApplication → engine/input + graphics + view(Renderer/Img)
```

OpenCV is used **only** inside `view/Img`. All other graphics code goes through the `Img` API.

## Graphics controls

- Left click: select / move (via `Controller`)
- Jump: as wired in the graphics app
- ESC / Q or close the window: quit

After a move, pieces take a long rest; after a jump, a short rest (visual overlays + rest animations).
