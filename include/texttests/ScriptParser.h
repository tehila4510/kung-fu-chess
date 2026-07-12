#ifndef SCRIPT_PARSER_H
#define SCRIPT_PARSER_H

#include "texttests/IScriptSource.h"
#include "texttests/ScriptCommand.h"

#include <iosfwd>

class ScriptParser : public IScriptSource {
public:
    ScriptParseResult parse(std::istream& input) const override;
};

#endif
