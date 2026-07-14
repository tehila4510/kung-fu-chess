#include "model/Piece.h"

#include <optional>
#include <stdexcept>
#include <string>

namespace {

bool isValidActiveColor(char color) {
    return color == 'w' || color == 'b';
}

bool isValidActiveKind(char kind) {
    static const std::string pieces = "KQRBNP";
    return pieces.find(kind) != std::string::npos;
}

bool isEmptyFields(char color, char kind) {
    return color == Piece::NoColor && kind == Piece::NoKind;
}

void validatePieceFields(char color, char kind) {
    if (isEmptyFields(color, kind)) {
        return;
    }

    if (color == Piece::NoColor || kind == Piece::NoKind) {
        throw std::invalid_argument("Piece color and kind must both be set or both empty");
    }
    if (!isValidActiveColor(color)) {
        throw std::invalid_argument("Invalid piece color");
    }
    if (!isValidActiveKind(kind)) {
        throw std::invalid_argument("Invalid piece kind");
    }
}

void validateTokenOrThrow(const std::string& token) {
    if (token == ".") {
        return;
    }
    if (token.size() != 2) {
        throw std::invalid_argument("Token must be '.' or exactly two characters");
    }
    if (!isValidActiveColor(token[0])) {
        throw std::invalid_argument("Invalid token color");
    }
    if (!isValidActiveKind(token[1])) {
        throw std::invalid_argument("Invalid token kind");
    }
}

void validatePositionOrThrow(const Position& at) {
    if (!at.isValid()) {
        throw std::out_of_range("Position must have non-negative row and col");
    }
}

} // namespace

Piece::Piece(char color, char kind, Position position)
    : kind(kind), color(color), position(position) {
    validatePieceFields(color, kind);
    if (!isEmptyFields(color, kind)) {
        validatePositionOrThrow(position);
    }
}

std::optional<Piece> Piece::tryFromToken(const std::string& token, Position at) {
    if (token == ".") {
        return Piece();
    }
    if (token.size() != 2) {
        return std::nullopt;
    }
    if (!isValidActiveColor(token[0]) || !isValidActiveKind(token[1])) {
        return std::nullopt;
    }
    if (!at.isValid()) {
        return std::nullopt;
    }
    return Piece(token[0], token[1], at);
}

Piece Piece::fromToken(const std::string& token, Position at) {
    const std::optional<Piece> piece = tryFromToken(token, at);
    if (!piece) {
        validateTokenOrThrow(token);
        validatePositionOrThrow(at);
    }
    return *piece;
}

std::string Piece::toToken() const {
    if (isEmpty()) {
        return std::string(1, NoKind);
    }

    validatePieceFields(color, kind);
    return std::string{ color, kind };
}

bool Piece::isEmpty() const { return color == NoColor || kind == NoKind; }
bool Piece::isWhite() const { return color == 'w'; }
bool Piece::isBlack() const { return color == 'b'; }

bool Piece::sameColorAs(const Piece& other) const {
    return !isEmpty() && !other.isEmpty() && color == other.color;
}

bool Piece::isEnemyOf(const Piece& other) const {
    return !isEmpty() && !other.isEmpty() && color != other.color;
}

char Piece::opponentColor() const {
    if (isEmpty()) {
        throw std::runtime_error("Cannot determine opponent color for an empty piece");
    }
    return isWhite() ? 'b' : 'w';
}

bool Piece::operator==(const Piece& o) const {
    return kind == o.kind && color == o.color
        && position.row == o.position.row
        && position.col == o.position.col;
}

bool Piece::operator!=(const Piece& o) const { return !(*this == o); }
