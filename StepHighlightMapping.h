#pragma once
#include "CalculationSteps.h"
#include <cstddef>
#include <string>

namespace StepHighlightMapping {
struct SourceRange {
    std::string text;
    std::size_t start = 0;
    std::size_t end = 0;
};

// Map a reduced subtree onto the source row and the replacement onto the next row.
StepMark mapReduction(const std::string& previous, const std::string& current,
                      const SourceRange& source, const std::string& replacementLiteral);
}
