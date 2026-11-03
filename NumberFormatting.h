#pragma once

#include <string>

namespace NumberFormatting {
// Converts a calculated value to a display string in the selected base.
std::string formatBase(double value, int base);
// Human-readable intermediate value; the calculation keeps its original double.
std::string formatStep(double value, int base);
}
