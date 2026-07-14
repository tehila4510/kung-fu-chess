#include "io/BoardPrinter.h"
#include "texttests/ScriptParser.h"
#include "texttests/ScriptRunner.h"

#include <iostream>

int main() {
    try {
        ScriptParser parser;
        BoardPrinter printer;
        ScriptRunner runner(parser, printer);
        return runner.run(std::cin, std::cout) ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
