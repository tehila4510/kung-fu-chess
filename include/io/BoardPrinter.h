#ifndef BOARD_PRINTER_H
#define BOARD_PRINTER_H

#include "io/IBoardPrinter.h"
#include "model/Board.h"

#include <iosfwd>

class BoardPrinter : public IBoardPrinter {
public:
    void print(const Board& board, std::ostream& out) const override;

    void print(const Board& board) const;
};

#endif
