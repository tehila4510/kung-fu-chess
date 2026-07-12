#ifndef SCRIPT_RUNNER_H
#define SCRIPT_RUNNER_H

#include <iosfwd>

class IScriptSource;
class IBoardPrinter;

class ScriptRunner {
public:
    ScriptRunner(const IScriptSource& source, const IBoardPrinter& printer);

    bool run(std::istream& input, std::ostream& output) const;

private:
    const IScriptSource& source_;
    const IBoardPrinter& printer_;
};

#endif
