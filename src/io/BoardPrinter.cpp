#include "io/BoardPrinter.h"

#include <iostream>
#include <ostream>
#include <stdexcept>

void BoardPrinter::print(const Board& board, std::ostream& out) const {
    if (!out) {
        throw std::runtime_error("Output stream is not writable");
    }

    const int rows = board.getRowCount();
    const int cols = board.getColCount();
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (c) out << ' ';
            out << board.getCell(Position{ r, c }).getContent();
        }
        out << '\n';
    }
}

void BoardPrinter::print(const Board& board) const {
    print(board, std::cout);
}
