#pragma once
#include <string>
#include <vector>

struct StepMark {
    std::string sourceRendering;
    std::size_t renderedStart = 0, renderedEnd = 0;
    std::size_t sourceStart = 0, sourceEnd = 0;
    std::size_t resultStart = 0, resultEnd = 0;
};

struct StepCalculation {
    bool success = false;
    std::vector<std::string> steps;
    std::vector<StepMark> marks;
    std::size_t errorStart = 0, errorEnd = 0;
    std::string error;
};

// Returns successive equivalent expressions (without the original input).
StepCalculation calculateSteps(const std::string& expression, int base);
