#include "texttests/ScriptRunner.h"

#include "engine/GameEngine.h"
#include "input/Controller.h"
#include "io/BoardParser.h"
#include "io/IBoardPrinter.h"
#include "texttests/IScriptSource.h"
#include "texttests/ScriptCommand.h"

#include <ostream>
#include <utility>

namespace {
constexpr int kCellSize = 100;
}

ScriptRunner::ScriptRunner(const IScriptSource& source, const IBoardPrinter& printer)
    : source_(source), printer_(printer) {}

bool ScriptRunner::run(std::istream& input, std::ostream& output) const {
    ScriptParseResult parsed = source_.parse(input);

    if (!parsed.board.has_value()) {
        output << toReasonCode(parsed.boardStatus) << '\n';
        return false;
    }

    GameEngine engine;
    engine.setup(std::move(*parsed.board));
    Controller controller(engine, kCellSize);

    for (const ScriptCommand& cmd : parsed.commands) {
        switch (cmd.kind) {
            case ScriptCommandKind::Click:
                controller.click(cmd.x, cmd.y);
                break;
            case ScriptCommandKind::Jump:
                controller.jump(cmd.x, cmd.y);
                break;
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
}
