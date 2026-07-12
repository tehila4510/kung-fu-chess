#include "io/BoardPrinter.h"
#include "texttests/ScriptParser.h"
#include "texttests/ScriptRunner.h"

#include <iostream>

int main() {
    ScriptParser parser;
    BoardPrinter printer;
    ScriptRunner runner(parser, printer);
    runner.run(std::cin, std::cout);
    return 0;
}
