#pragma once
#include <cstddef>
#include <string>
#include <vector>

// Fractions are represented by structural markers, not a textual "1/2".
// Each fraction is encoded as: {numerator|denominator} using private-use
// Unicode code points to avoid collisions with user-entered parentheses.
namespace FractionModel {
constexpr char32_t Begin = U'\uE100';
constexpr char32_t Separator = U'\uE101';
constexpr char32_t End = U'\uE102';
struct Span {
    std::size_t begin = 0, separator = 0, end = 0;
    int level = 0;
};
std::string emptyFraction();
std::vector<Span> spans(const std::string& expression);
int nestingAt(const std::string& expression, std::size_t cursor);
// Move between numerator and denominator of the innermost fraction.
// Returns false when no matching fraction field is active.
bool verticalFieldCursor(const std::string& expression, std::size_t cursor,
                         bool down, std::size_t& destination);
// Converts a complete structured fraction into arithmetic division.
// Empty denominators become 1. Returns false for incomplete structures.
bool toArithmetic(const std::string& expression, std::string& arithmetic);
}
