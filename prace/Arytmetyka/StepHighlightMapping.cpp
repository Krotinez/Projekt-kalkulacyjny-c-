#include "StepHighlightMapping.h"
#include <SFML/System/String.hpp>

namespace StepHighlightMapping {
StepMark mapReduction(const std::string& previous, const std::string& current,
                      const SourceRange& source, const std::string& replacementLiteral) {
    StepMark mark;
    // First row may be written with implicit multiplication and
    // differently shaped brackets. Locate the exact operation text.
    const auto& sourceText=source.text;
    const auto begin=source.start,end=source.end;
    const sf::String src=sf::String::fromUtf8(sourceText.begin(),sourceText.end());
    std::size_t sourceBegin=begin;
    std::size_t sourceEnd=sourceBegin+(end-begin);
    if(sourceText!=previous) {
        // Repeated operations can have identical text. Select the occurrence
        // whose surrounding context best matches the marked source subtree.
        // Compare Unicode codepoints, not UTF-8 byte positions.
        const sf::String previousUnicode=sf::String::fromUtf8(previous.begin(),previous.end());
        const sf::String selectedUnicode=src.substring(begin,end-begin);
        std::size_t best=std::string::npos;
        std::size_t bestScore=0;
        bool hasMatch=false;
        for(std::size_t atChar=0;atChar+selectedUnicode.getSize()<=previousUnicode.getSize();++atChar) {
            if(previousUnicode.substring(atChar,selectedUnicode.getSize())!=selectedUnicode) continue;
            std::size_t score=0;
            // Compare the text immediately before and after the occurrence.
            // Exact neighboring context disambiguates e.g. 2×3+2×3.
            for(std::size_t d=1;d<=begin && d<=atChar;++d) {
                if(src[begin-d]!=previousUnicode[atChar-d]) break;
                ++score;
            }
            for(std::size_t d=0;end+d<src.getSize() && atChar+selectedUnicode.getSize()+d<previousUnicode.getSize();++d) {
                if(src[end+d]!=previousUnicode[atChar+selectedUnicode.getSize()+d]) break;
                ++score;
            }
            // Prefer the position implied by the source offset on ties.
            if(!hasMatch || score>bestScore ||
               (score==bestScore &&
                (atChar>begin?atChar-begin:begin-atChar)<(best>begin?best-begin:begin-best))) {
                best=atChar;bestScore=score;hasMatch=true;
            }
        }
        if(hasMatch) {
            sourceBegin=best;
            sourceEnd=best+selectedUnicode.getSize();
        } else {
            // A power may be rendered as a raised index with grouping around
            // the exponent: 2^3 in the reduction tree, 2^(3) in the row.
            // Match the exact base and exponent first, then allow a single
            // balanced wrapper around the exponent. Avoid guessing by merely
            // searching for the nearest '^' (which can mark another power).
            bool mappedPower = false;
            std::size_t bestPowerScore = 0;
            std::size_t bestPowerDistance = previousUnicode.getSize();
            // Try every caret: a nested power can have more than one '^'.
            // Preserve the original exponent, and only add the wrapper
            // parentheses which are present in the displayed expression.
            for (std::size_t caret = 0; caret < selectedUnicode.getSize(); ++caret) {
                if (selectedUnicode[caret] != '^') continue;
                const sf::String base = selectedUnicode.substring(0, caret);
                const sf::String exponent = selectedUnicode.substring(caret + 1);
                if (base.getSize() == 0 || exponent.getSize() == 0) continue;
                const sf::String wrapped = base + sf::String("^(") + exponent + sf::String(")");
                for (std::size_t at = 0; at + wrapped.getSize() <= previousUnicode.getSize(); ++at) {
                    if (previousUnicode.substring(at, wrapped.getSize()) != wrapped) continue;
                    std::size_t score = 0;
                    for (std::size_t d = 1; d <= begin && d <= at; ++d) {
                        if (src[begin-d] != previousUnicode[at-d]) break;
                        ++score;
                    }
                    for (std::size_t d = 0; end+d < src.getSize() &&
                         at+wrapped.getSize()+d < previousUnicode.getSize(); ++d) {
                        if (src[end+d] != previousUnicode[at+wrapped.getSize()+d]) break;
                        ++score;
                    }
                    const std::size_t distance = at > begin ? at-begin : begin-at;
                    if (!mappedPower || score > bestPowerScore ||
                        (score == bestPowerScore && distance < bestPowerDistance)) {
                        sourceBegin = at;
                        sourceEnd = at + wrapped.getSize();
                        bestPowerScore = score;
                        bestPowerDistance = distance;
                        mappedPower = true;
                    }
                }
            }
            // A graphical log/root can contain extra parentheses around both
            // arguments. When the exact rendered subtree cannot be found,
            // select the complete balanced function call in the original row.
            // This fallback is restricted to a selected function node; it does
            // not interfere with reductions inside its arguments.
            const bool isLog = selectedUnicode.getSize() >= 4 &&
                selectedUnicode.substring(0,4) == sf::String("log(");
            const bool isRoot = selectedUnicode.getSize() >= 5 &&
                selectedUnicode.substring(0,5) == sf::String("root(");
            bool mappedFunction = false;
            if (!mappedPower && (isLog || isRoot)) {
                const sf::String prefix(isLog ? "log(" : "root(");
                std::size_t closest = previousUnicode.getSize();
                for (std::size_t pos=0; pos+prefix.getSize()<=previousUnicode.getSize(); ++pos) {
                    if (previousUnicode.substring(pos,prefix.getSize()) != prefix) continue;
                    std::size_t depth=1, finish=pos+prefix.getSize();
                    for (;finish<previousUnicode.getSize();++finish) {
                        if (previousUnicode[finish]=='(') ++depth;
                        else if (previousUnicode[finish]==')' && --depth==0) {++finish;break;}
                    }
                    if (depth!=0) continue;
                    const std::size_t distance=pos>begin?pos-begin:begin-pos;
                    if (!mappedFunction || distance<closest) {
                        sourceBegin=pos; sourceEnd=finish;
                        closest=distance; mappedFunction=true;
                    }
                }
            }
            if (!mappedPower && !mappedFunction) {
                // Simplifying brackets around single numbers changes the
                // rendered expression before the first arithmetic step.
                // Map the reduction back to the original bracketed source:
                // e.g. 2+3{(4)+[1]} -> highlight (4)+[1], not (4)+.
                // The braces themselves belong to the containing group,
                // not to the addition being evaluated.
                const auto opening=previous.find('{');
                if(opening!=std::string::npos) {
                    std::size_t depth=0, closing=std::string::npos;
                    for(std::size_t j=opening;j<previous.size();++j) {
                        if(previous[j]=='{') ++depth;
                        else if(previous[j]=='}' && --depth==0) {closing=j;break;}
                    }
                    if(closing!=std::string::npos && closing>opening+1) {
                        sourceBegin=sf::String::fromUtf8(previous.begin(),previous.begin()+opening+1).getSize();
                        sourceEnd=sf::String::fromUtf8(previous.begin(),previous.begin()+closing).getSize();
                    }
                }
            }
        }
    }
    mark.sourceStart=sourceBegin;
    mark.sourceEnd=sourceEnd;
    // Find the full replacement token, not the common text prefix/suffix.
    const std::string& replacement=replacementLiteral;
    std::size_t byteAt=current.find(replacement);
    if(byteAt==std::string::npos) byteAt=0;
    mark.resultStart=sf::String::fromUtf8(current.begin(),current.begin()+byteAt).getSize();
    mark.resultEnd=mark.resultStart+sf::String::fromUtf8(replacement.begin(),replacement.end()).getSize();
    return mark;
}
}
