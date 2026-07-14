#include "model/Position.h"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>

namespace {

void validatePositionOrThrow(const Position& pos) {
    if (!pos.isValid()) {
        throw std::out_of_range("Position must have non-negative row and col");
    }
}

} // namespace

int Position::rowDistanceTo(const Position& o) const {
    validatePositionOrThrow(*this);
    validatePositionOrThrow(o);
    return std::abs(row - o.row);
}

int Position::colDistanceTo(const Position& o) const {
    validatePositionOrThrow(*this);
    validatePositionOrThrow(o);
    return std::abs(col - o.col);
}

int Position::chebyshevDistanceTo(const Position& o) const {
    return std::max(rowDistanceTo(o), colDistanceTo(o));
}

int Position::manhattanDistanceTo(const Position& o) const {
    return rowDistanceTo(o) + colDistanceTo(o);
}

std::string Position::toString() const {
    return "(" + std::to_string(row) + ", " + std::to_string(col) + ")";
}
