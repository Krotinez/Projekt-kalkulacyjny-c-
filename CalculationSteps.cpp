#include "CalculationSteps.h"
#include "StepExpressionParser.h"
#include "StepReduction.h"
#include "ExpressionCalculator.h"
#include "StepHighlightMapping.h"
#include <exception>
#include <string>
#include <SFML/System/String.hpp>

using StepExpressionParser::Node;
using StepExpressionParser::Parser;
using StepReduction::simplify;
using StepReduction::render;
using StepReduction::markedRender;

StepCalculation calculateSteps(const std::string& expression,int base) {
    StepCalculation output;
    try {
        Parser p{expression,0,base}; auto tree=p.sum();p.skip();
        if(p.i!=expression.size()) {output.error="Nieobslugiwany znak lub operator";return output;}
        output.success=true;
        std::string previous=expression;
        for(int i=0;i<256;++i) {
            simplify(tree);
            const Node* oldTarget=nullptr;
            const Node* newTarget=nullptr;
            std::string replacementLiteral;
            // Before reducing, find the next reducible node using the same
            // depth-first order, without computing its value yet.
            const Node* selected=StepReduction::nextCandidate(tree.get());
            if(!selected) break;
            const auto before=markedRender(*tree,selected);
            try {
                if(!StepReduction::reduce(tree,base,oldTarget,newTarget,replacementLiteral)) break;
            } catch(const std::exception& e) {
                output.success=false;
                output.error=e.what();
                // Error positions refer to the last displayed step, or input.
                output.errorStart=before.second.first;
                output.errorEnd=before.second.second;
                return output;
            }
            // Keep a render with the replacement node marked, before removing
            // redundant grouping. The resulting text is rendered without those
            // groups, so locate the replacement by its full token there.
            simplify(tree);
            const std::string current=render(*tree);
            if(current==previous || current==expression ||
               (!output.steps.empty() && output.steps.back()==current)) continue;
            auto mark = StepHighlightMapping::mapReduction(
                previous, current,
                {before.first, before.second.first, before.second.second},
                replacementLiteral);
            // The replacement node is unique even if its displayed number
            // appears elsewhere (e.g. 2+2×2). Mark that node directly,
            // instead of searching for the first matching number.
            if (newTarget) {
                const auto after = markedRender(*tree, newTarget);
                if (after.first == current) {
                    mark.resultStart = after.second.first;
                    mark.resultEnd = after.second.second;
                }
            }
            output.steps.push_back(current);
            output.marks.push_back(mark);
            previous=current;
        }
        const auto final=calculateExpression(expression,base);
        if(!final.success) {output.success=false;output.error=final.error;return output;}
        if(output.steps.empty() || output.steps.back()!=final.text) {
            StepMark mark;
            mark.sourceStart=0;
            mark.sourceEnd=sf::String::fromUtf8(previous.begin(),previous.end()).getSize();
            mark.resultStart=0;
            mark.resultEnd=sf::String::fromUtf8(final.text.begin(),final.text.end()).getSize();
            output.steps.push_back(final.text);
            output.marks.push_back(mark);
        }
        return output;
    } catch(const std::exception& e) {output.success=false;output.error=e.what();return output;}
}
