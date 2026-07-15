#include "texttests/ScriptRunner.h"

#include "engine/GameEngine.h"
#include "input/BoardMapper.h"
#include "input/Controller.h"
#include "io/BoardParser.h"
#include "io/IBoardPrinter.h"
#include "texttests/IScriptSource.h"
#include "texttests/ScriptCommand.h"

#include <iostream>
#include <optional>
#include <ostream>
#include <utility>

namespace {
constexpr int kCellSize = 100;
}

ScriptRunner::ScriptRunner(const IScriptSource& source, const IBoardPrinter& printer)
    : source_(source), printer_(printer) {}

bool ScriptRunner::run(std::istream& input, std::ostream& output) const {
    try {
        if (!input) {
            throw std::runtime_error("Input stream is not readable");
        }
        if (!output) {
            throw std::runtime_error("Output stream is not writable");
        }

        ScriptParseResult parsed = source_.parse(input);

        if (!parsed.board.has_value()) {
            output << toReasonCode(parsed.boardStatus) << '\n';
            return false;
        }

        GameEngine engine;
        engine.setup(std::move(*parsed.board));
        Controller controller(engine);
        const BoardMapper mapper(kCellSize, engine.columnCount(), engine.rowCount());

        for (const ScriptCommand& cmd : parsed.commands) {
            switch (cmd.kind) {
                case ScriptCommandKind::Click: {
                    const std::optional<Position> cell = mapper.pixelToCell(cmd.x, cmd.y);
                    if (cell) {
                        controller.click(*cell);
                    } else {
                        controller.clearSelection();
                    }
                    break;
                }
                case ScriptCommandKind::Jump: {
                    const std::optional<Position> cell = mapper.pixelToCell(cmd.x, cmd.y);
                    if (cell) {
                        controller.jump(*cell);
                    }
                    break;
                }
                case ScriptCommandKind::Wait:
                    engine.wait(cmd.ms);
                    break;
                case ScriptCommandKind::PrintBoard:
                    printer_.print(Board(engine.snapshot().cells), output);
                    break;
                case ScriptCommandKind::Unknown:
                    break;
            }
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error in ScriptRunner::run: " << e.what() << std::endl;
        output << "ERROR RUNTIME_FAILURE\n";
        return false;
    }
}
