#include "graphics/FileBoardSource.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace graphics {

namespace {

std::string trim(const std::string& value) {
    std::size_t begin = 0;
    std::size_t end = value.size();
    while (begin < end && std::isspace(static_cast<unsigned char>(value[begin]))) {
        ++begin;
    }
    while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }
    return value.substr(begin, end - begin);
}

}  // namespace

BoardLayout FileBoardSource::load(const std::string& path) const {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot open board file: " + path);
    }

    BoardLayout layout;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();  // tolerate Windows CRLF
        }
        if (line.empty()) {
            continue;
        }

        std::vector<std::string> row;
        std::stringstream ss(line);
        std::string cell;
        while (std::getline(ss, cell, ',')) {
            row.push_back(trim(cell));
        }
        // A trailing comma denotes a final empty field (e.g. ",,,,,,," → 8 columns).
        if (!line.empty() && line.back() == ',') {
            row.push_back("");
        }
        layout.cells.push_back(std::move(row));
    }

    if (layout.cells.empty()) {
        throw std::runtime_error("Board file is empty: " + path);
    }

    const std::size_t expectedCols = layout.cells.front().size();
    for (std::size_t i = 0; i < layout.cells.size(); ++i) {
        if (layout.cells[i].size() != expectedCols) {
            throw std::runtime_error(
                "Board file column count mismatch at line " + std::to_string(i + 1) +
                ": expected " + std::to_string(expectedCols) +
                " columns, got " + std::to_string(layout.cells[i].size()));
        }
    }

    layout.rows = static_cast<int>(layout.cells.size());
    layout.cols = static_cast<int>(expectedCols);
    return layout;
}

}  // namespace graphics
