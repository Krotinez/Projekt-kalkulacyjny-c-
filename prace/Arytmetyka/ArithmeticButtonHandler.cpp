#include "ArithmeticButtonHandler.h"
#include "Expression.h"
#include "ExpressionCalculator.h"
#include "BaseConversion.h"
#include "CalculationSteps.h"
#include "ArithmeticUI.h"
#include "NumberSystem.h"
#include "../GUI/PowerVisual.h"
#include "../GUI/FractionModel.h"
#include "../GUI/RootModel.h"
#include "../GUI/LogModel.h"
#include <SFML/System/String.hpp>
#include <algorithm>

ButtonActionResult handleArithmeticButton(const std::string& action,
                                         const std::string& lbl,
                                         ArithmeticButtonContext context) {
    auto& expression = context.expression;
    auto& expressionPanel = context.expressionPanel;
    auto& mathFont = context.mathFont;
    auto& currentSystem = context.currentSystem;
    auto& resultText = context.resultText;
    auto& expressionAtResult = context.expressionAtResult;
    auto& stepHighlights = context.stepHighlights;
    auto& resultFocused = context.interaction.resultFocused;
    auto& stepsMode = context.stepsMode;
    const bool showBlue = context.showBlue;
    const bool showGreen = context.showGreen;
    auto displayedExpression = [&]() {
        return ArithmeticUI::displayedText(expression.get(), resultText);
    };

                
                if (action == "ESC") return ButtonActionResult::Exit;
                else if (action == "SYSTEM_BIN" || action == "SYSTEM_OCT" ||
                         action == "SYSTEM_DEC" || action == "SYSTEM_HEX") {
                    const int oldBase=NumberSystems::base(currentSystem);
                    const int newBase=action=="SYSTEM_BIN"?2:action=="SYSTEM_OCT"?8:
                        action=="SYSTEM_HEX"?16:10;
                    if(oldBase!=newBase) {
                        std::string converted;
                        std::size_t caret=expression.cursor();
                        std::size_t anchor=expression.hasSelection()?expression.selectionStart():caret;
                        if(BaseConversion::convert(expression.get(),oldBase,newBase,converted,caret,anchor)) {
                            expression.replaceForBaseConversion(converted,caret,anchor);
                            currentSystem=NumberSystems::fromBase(newBase);
                            resultText.clear(); expressionAtResult.clear(); stepHighlights.clear();
                            resultFocused=false;
                        }
                    }
                }
                else if (action == "CLEAR")
                {
                    expression.clear();
                    resultText.clear();
                    expressionAtResult.clear();
                    stepHighlights.clear();
                    resultFocused=false;
                    expressionPanel.resetScroll();
                }
                else if (action == "^" || lbl == "^") {
                    // Ten sam mechanizm dla przycisku i klawiatury.
                    const auto raw = expression.get();
                    const auto chars = sf::String::fromUtf8(raw.begin(), raw.end());
                    const auto at = expression.cursor();
                    if (!expression.hasSelection() && at > 0 && chars[at - 1] == U'^') {
                        expression.removeLast();
                    } else {
                        const auto layout = makePowerVisual(raw, mathFont, 28);
                        int depth = 0;
                        for (const auto& exponent : layout.exponents)
                            if (at >= exponent.start && at <= exponent.end)
                                depth = std::max(depth, exponent.level);
                        if (depth < 2) {
                            if (at == 0 || chars[at-1] == U'+' || chars[at-1] == U'-' ||
                                chars[at-1] == U'*' || chars[at-1] == U'/' ||
                                chars[at-1] == U'(' || chars[at-1] == U'[' || chars[at-1] == U'{')
                                expression.add("1");
                            expression.add("^");
                        }
                    }
                    // Przyciski nie aktywuja tekstowego kursora.
                    resultFocused = false;
                }
                else if (action == "FRACTION" || lbl == "1/2") {
                    // Wstawiamy znaczniki strukturalne; kursor trafia do licznika.
                    // Maksymalnie dwa poziomy zagniezdzenia.
                    if (FractionModel::nestingAt(expression.get(), expression.cursor()) < 2) {
                        expression.add(FractionModel::emptyFraction());
                        expression.setCursor(expression.cursor() - 2);
                        resultFocused = false;
                    }
                }
                else if (action == "ROOT" || lbl == "√") {
                    expression.add(RootModel::emptyRoot());
                    // Begin, Separator, End: cursor directly after Separator.
                    expression.setCursor(expression.cursor() - 1);
                    resultFocused = false;
                }
                else if (action == "LOG" || lbl == "log") {
                    expression.add(LogModel::emptyLog());
                    // Start in the argument, immediately after the separator.
                    expression.setCursor(expression.cursor() - 1);
                    resultFocused = false;
                }
                else if (action == "BACKSPACE") expression.removeLast();
                else if (action == "ABS_OPEN") {
                    // Tworzymy pare kresek, kursor ustawiamy pomiedzy nimi.
                    expression.add("<");
                    resultFocused = false;
                }

                else if (action == "ABS_CLOSE")
                    expression.add(">");
                else if (
                    action == "HEX_A" ||
                    action == "HEX_B" ||
                    action == "HEX_C" ||
                    action == "HEX_D" ||
                    action == "HEX_E" ||
                    action == "HEX_F"
                )
                {
                    expression.add(action.substr(4));
                }
                else if (action == "W") stepsMode = false;
                else if (action == "K") stepsMode = true;
                else if (action == "=") {
                    const int base = NumberSystems::base(currentSystem);
                    stepHighlights.clear();
                    resultFocused=false;
                    // Znaczniki graficznych ulamkow nie sa operatorami parsera.
                    // Przed obliczeniem zamieniamy je na nawiasowane dzielenie.
                    std::string logArithmetic;
                    if (!LogModel::toArithmetic(expression.get(), logArithmetic)) {
                        resultText = "Blad: Nieprawidlowa struktura logarytmu";
                        expressionAtResult = expression.get();
                        expressionPanel.scrollToBottom(displayedExpression());
                        return ButtonActionResult::SkipFrame;
                    }
                    std::string rootArithmetic;
                    if (!RootModel::toArithmetic(logArithmetic, rootArithmetic)) {
                        resultText = "Blad: Nieprawidlowa struktura pierwiastka";
                        expressionAtResult = expression.get();
                        expressionPanel.scrollToBottom(displayedExpression());
                        return ButtonActionResult::SkipFrame;
                    }
                    std::string arithmeticInput;
                    const bool fractionInputValid = FractionModel::toArithmetic(
                        rootArithmetic, arithmeticInput);
                    if (!fractionInputValid) {
                        resultText = "Blad: Nieprawidlowa struktura ulamka";
                        expressionAtResult = expression.get();
                        expressionPanel.scrollToBottom(displayedExpression());
                        return ButtonActionResult::SkipFrame;
                    }
                    // Only graphical fractions need their own source-to-parser mapping.
                    // Root/log conversion also changes the parser text, but must
                    // not disable all K-mode blue/green highlights.
                    const bool containsFractions =
                        expression.get().find(FractionModel::emptyFraction().substr(0,3))
                        != std::string::npos;
                    if (stepsMode) {
                        const auto result = calculateSteps(arithmeticInput, base);
                        if (!result.success) resultText = "= Blad: " + result.error;
                        else {
                            // Indeksy podswietlenia odnosza sie do tekstu parsera,
                            // a nie do graficznych znacznikow ulamka.
                            if (!containsFractions) ArithmeticUI::makeHighlights(expression.get(),result,stepHighlights,showBlue,showGreen);
                            resultText.clear();
                            for (const auto& step : result.steps) {
                                if (!resultText.empty()) resultText += "\n";
                                resultText += "= " + step;
                            }
                            if (!result.error.empty()) {
                                const std::string last = result.steps.empty() ? expression.get() : result.steps.back();
                                const std::size_t offset = result.steps.empty() ? 0 :
                                    ArithmeticUI::codepoints(expression.get())+1+ArithmeticUI::codepoints(resultText)-ArithmeticUI::codepoints(last);
                                if (!containsFractions)
                                    stepHighlights.push_back({offset+result.errorStart,offset+result.errorEnd,false,true});
                                if (!resultText.empty()) resultText += "\n";
                                resultText += "= Blad: " + result.error;
                            }
                        }
                    } else {
                        const auto result = calculateExpression(arithmeticInput, base);
                        resultText = result.success ? "= " + result.text : "Blad: " + result.error;
                    }
                    expressionAtResult = expression.get();
                    expressionPanel.scrollToBottom(displayedExpression());
                }
                else if (
                action == "WYNIKI v" ||
                action == "SYSTEM_BIN" ||
                action == "SYSTEM_OCT" ||
                action == "SYSTEM_DEC" ||
                action == "SYSTEM_HEX" ||
                action == "NWD" ||
                action == "NWW" ||
                action == "1/x" ||
                action == "-x"
            ){
                    // Te przyciski nie wpisują tekstu do wyrażenia.
                    // Ich właściwe działanie dodamy w kolejnych etapach.
                }
                else {
                    expression.add(lbl);
                }

    return ButtonActionResult::Handled;
}
