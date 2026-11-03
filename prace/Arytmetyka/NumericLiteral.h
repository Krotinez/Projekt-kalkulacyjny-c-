#pragma once
#include <cstddef>
#include <string>

// One source of truth for numeric digits and fractional literals in BIN/OCT/DEC/HEX.
namespace NumericLiteral {
double parse(const std::string& text, std::size_t& position, int base);
}
