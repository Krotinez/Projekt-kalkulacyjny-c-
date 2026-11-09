#include "ArithmeticUI.h"
#include <SFML/System/String.hpp>
#include <algorithm>
#include <vector>
#include <utility>

namespace ArithmeticUI {
std::string displayedText(const std::string& expression, const std::string& resultText) {
    return expression + (resultText.empty() ? "" : "\n" + resultText);
}

std::size_t codepoints(const std::string& s) {
        return sf::String::fromUtf8(s.begin(),s.end()).getSize();
}

// Align a normalized K-step rendering with the actual displayed editor row.
// This is especially important for the first row: implicit multiplication,
// graphical roots and omitted default degrees change the character offsets.
// If alignment cannot identify the selected subtree reliably, omit that
// blue selection instead of highlighting unrelated glyphs or the '=' label.
static bool mapSourceRange(const std::string& displayed, const StepMark& mark,
                           std::size_t& start, std::size_t& end) {
    if (mark.sourceRendering.empty()) {
        start = mark.sourceStart; end = mark.sourceEnd;
        return end <= codepoints(displayed);
    }
    const sf::String actual = sf::String::fromUtf8(displayed.begin(), displayed.end());
    const sf::String rendered = sf::String::fromUtf8(
        mark.sourceRendering.begin(), mark.sourceRendering.end());
    const std::size_t n=rendered.getSize(), m=actual.getSize();
    if (mark.renderedEnd > n || mark.renderedStart >= mark.renderedEnd) return false;
    if (rendered == actual) {
        start=mark.renderedStart; end=mark.renderedEnd;
        return true;
    }
    if (n*m > 250000) return false;
    // Longest-common-subsequence alignment preserves the order of nested
    // structure markers while tolerating extra parentheses or implicit '×'.
    std::vector<std::vector<unsigned>> dp(n+1,std::vector<unsigned>(m+1,0));
    for(std::size_t i=n;i-- > 0;) for(std::size_t j=m;j-- > 0;)
        dp[i][j] = rendered[i]==actual[j] ? 1+dp[i+1][j+1] :
            std::max(dp[i+1][j],dp[i][j+1]);
    std::vector<std::pair<std::size_t,std::size_t>> matches;
    for(std::size_t i=0,j=0;i<n && j<m;) {
        if(rendered[i]==actual[j]) {matches.push_back({i,j});++i;++j;}
        else if(dp[i+1][j]>=dp[i][j+1]) ++i;
        else ++j;
    }
    std::size_t matched=0, lo=m, hi=0;
    for(const auto& [i,j]:matches) if(i>=mark.renderedStart && i<mark.renderedEnd) {
        ++matched;lo=std::min(lo,j);hi=std::max(hi,j+1);
    }
    const std::size_t selected=mark.renderedEnd-mark.renderedStart;
    // Require meaningful correspondence; never paint an arbitrary range.
    if(!matched || matched*2 < selected || hi<=lo) return false;
    start=lo;end=hi;
    return true;
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
            std::size_t blueStart=0, blueEnd=0;
            if(showBlue && mapSourceRange(previous,mark,blueStart,blueEnd))
                stepHighlights.push_back({previousOffset+blueStart,previousOffset+blueEnd,false});
            if(showGreen && mark.resultEnd>mark.resultStart)
                stepHighlights.push_back({nextOffset+mark.resultStart,nextOffset+mark.resultEnd,true});
            previousOffset=nextOffset;
            previous=next;
        }
}
}
