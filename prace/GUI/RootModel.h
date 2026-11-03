#pragma once
#include <string>
#include <cstddef>
namespace RootModel {
constexpr char32_t Begin = U'\uE110';
constexpr char32_t Separator = U'\uE111';
constexpr char32_t End = U'\uE112';
std::string emptyRoot();
// Navigation between the small degree field and the radicand.
// Cursor offsets are Unicode code point offsets, not UTF-8 bytes.
bool horizontalFieldCursor(const std::string& expression, std::size_t cursor,
                           bool right, std::size_t& target);
bool inDegree(const std::string& expression, std::size_t cursor);
// Converts graphical root markers to parenthesized arithmetic expressions.
// Empty degree is treated as 2; malformed roots return false.
bool toArithmetic(const std::string& expression, std::string& arithmetic);
}
