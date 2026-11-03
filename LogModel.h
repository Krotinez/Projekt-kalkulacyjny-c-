#pragma once
#include <string>
#include <cstddef>
namespace LogModel {
constexpr char32_t Begin = U'\uE120';
constexpr char32_t Separator = U'\uE121';
constexpr char32_t End = U'\uE122';
std::string emptyLog();
bool toArithmetic(const std::string& expression, std::string& arithmetic);
// Cursor offsets count Unicode code points.
bool horizontalFieldCursor(const std::string& expression, std::size_t cursor,
                           bool right, std::size_t& target);
bool inBase(const std::string& expression, std::size_t cursor);
}
