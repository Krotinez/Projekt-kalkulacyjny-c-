#pragma once
#include <string>

struct CalculationResult {
    bool success = false;
    std::string text;
    std::string error;
};

// First stage: arithmetic, parentheses and decimal numbers.
CalculationResult calculateExpression(const std::string& expression, int base = 10);
