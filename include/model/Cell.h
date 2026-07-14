#ifndef MODEL_CELL_H
#define MODEL_CELL_H

#include <string>

class Cell {
    std::string value;
public:
    explicit Cell(std::string value);

    const std::string& getContent() const;
    bool isEmpty() const;
    char getColor() const;
};

#endif
