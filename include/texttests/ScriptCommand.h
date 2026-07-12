#ifndef SCRIPT_COMMAND_H
#define SCRIPT_COMMAND_H

#include "io/BoardParser.h"
#include "model/Board.h"

#include <optional>
#include <vector>

enum class ScriptCommandKind {
    Click,
    Jump,
    Wait,
    PrintBoard,
    Unknown
};

struct ScriptCommand {
    ScriptCommandKind kind = ScriptCommandKind::Unknown;
    int x = 0;
    int y = 0;
    int ms = 0;
};

struct ScriptParseResult {
    std::optional<Board> board;
    BoardParseStatus boardStatus = BoardParseStatus::Ok;
    std::vector<ScriptCommand> commands;
};

#endif
