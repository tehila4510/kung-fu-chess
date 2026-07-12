#ifndef IBOARD_PRINTER_H
#define IBOARD_PRINTER_H

#include "model/Board.h"

#include <iosfwd>


class IBoardPrinter {
public:
    virtual ~IBoardPrinter() = default;
    virtual void print(const Board& board, std::ostream& out) const = 0;
};

#endif
