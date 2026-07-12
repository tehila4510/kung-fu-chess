#ifndef ISCRIPT_SOURCE_H
#define ISCRIPT_SOURCE_H

#include "texttests/ScriptCommand.h"

#include <iosfwd>

class IScriptSource {
public:
    virtual ~IScriptSource() = default;
    virtual ScriptParseResult parse(std::istream& input) const = 0;
};

#endif
