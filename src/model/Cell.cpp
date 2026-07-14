#include "model/Cell.h"

Cell::Cell(std::string value) : value(std::move(value)) {}

const std::string& Cell::getContent() const {
    return value;
}

bool Cell::isEmpty() const {
    return value == ".";
}

char Cell::getColor() const {
    if (isEmpty()) {
        return '\0';
    }
    return value[0];
}
