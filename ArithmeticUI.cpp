#include "ArithmeticUI.h"
#include <SFML/System/String.hpp>

namespace ArithmeticUI {
std::string displayedText(const std::string& expression, const std::string& resultText) {
    return expression + (resultText.empty() ? "" : "\n" + resultText);
}

std::size_t codepoints(const std::string& s) {
        return sf::String::fromUtf8(s.begin(),s.end()).getSize();
}

void makeHighlights(const std::string& source, const StepCalculation& calculation,
                    std::vector<ExpressionHighlight>& stepHighlights, bool showBlue, bool showGreen) {
        stepHighlights.clear();
        std::string previous=source;
        std::size_t previousOffset=0;
        for(std::size_t i=0;i<calculation.steps.size();++i) {
            const auto& next=calculation.steps[i];
            const auto& mark=calculation.marks[i];
            const std::size_t nextOffset=previousOffset+codepoints(previous)+3;
            if(showBlue && mark.sourceEnd>mark.sourceStart)
                stepHighlights.push_back({previousOffset+mark.sourceStart,previousOffset+mark.sourceEnd,false});
            if(showGreen && mark.resultEnd>mark.resultStart)
                stepHighlights.push_back({nextOffset+mark.resultStart,nextOffset+mark.resultEnd,true});
            previousOffset=nextOffset;
            previous=next;
        }
}
}
