#include "ArithmeticFrame.h"
#include "../GUI/FractionModel.h"
#include <algorithm>

void updateArithmeticFrame(ArithmeticFrameContext context)
{
    auto& expression = context.expression;
    auto& expressionPanel = context.panel;
    auto& resultText = context.resultText;
    auto& expressionAtResult = context.expressionAtResult;
    auto& stepHighlights = context.highlights;
    auto& resultFocused = context.interaction.resultFocused;
    const bool stepsMode = context.stepsMode;
    auto& defs = context.defs;
    auto& buttons = context.buttons;
    auto& tableButtons = context.tableButtons;
    auto& tableFocus = context.tableFocus;
    const auto fractionCursorBefore = context.fractionCursorBefore;
    const auto& fractionBefore = context.fractionBefore;
    auto& lastVisibleCursor = context.lastVisibleCursor;
    auto& lastVisibleExpression = context.lastVisibleExpression;
    const auto mousePos = context.mousePos;
// Po zmianie wyrazenia wynik staje sie nieaktualny.
    if (!resultText.empty() && expression.get() != expressionAtResult) {
        resultText.clear();
        expressionAtResult.clear();
        stepHighlights.clear();
        resultFocused=false;
    }

    for (std::size_t i = 0; i < buttons.size(); ++i) {
        if (defs[i].action == "W" || defs[i].action == "K")
            buttons[i].setModeActive(defs[i].action == (stepsMode ? "K" : "W"));
    }

    for (std::size_t i = 0; i < tableButtons.size(); ++i) {
        tableButtons[i].update(mousePos);
        tableButtons[i].setSelected(tableFocus && *tableFocus == i);
    }

    // Fill an empty denominator only when the user actually leaves its
    // fraction. Do not interfere with edits, nested fields or calculations.
    if (fractionBefore == expression.get() &&
        fractionCursorBefore != expression.cursor()) {
        const auto allFractions = FractionModel::spans(fractionBefore);
        const auto currentCursor = expression.cursor();
        // Process inner fractions first. Insertions are made right-to-left
        // so earlier structural offsets remain valid.
        for (auto it = allFractions.rbegin(); it != allFractions.rend(); ++it) {
            const auto& fraction = *it;
            const bool wasInside = fractionCursorBefore > fraction.begin &&
                                   fractionCursorBefore <= fraction.end;
            const bool isOutside = currentCursor <= fraction.begin ||
                                   currentCursor > fraction.end;
            const bool emptyDenominator = fraction.separator + 1 == fraction.end;
            if (!wasInside || !isOutside || !emptyDenominator) continue;
            // Insert at the start of the denominator without changing
            // the user's final cursor location.
            const auto returnCursor = expression.cursor();
            expression.setCursor(fraction.separator + 1);
            expression.add("1");
            expression.setCursor(returnCursor > fraction.separator ?
                                 returnCursor + 1 : returnCursor);
        }
    }

    if (lastVisibleCursor != expression.cursor() ||
        lastVisibleExpression != expression.get())
        expressionPanel.ensureCursorVisible(expression.get(), expression.cursor());
    lastVisibleCursor = expression.cursor();
    lastVisibleExpression = expression.get();

}

void drawArithmeticFrame(ArithmeticDrawContext context)
{
    auto& window = context.window;
    auto& font = context.font;
    auto& title = context.title;
    auto& expression = context.expression;
    auto& expressionPanel = context.panel;
    const auto& stepHighlights = context.highlights;
    auto& buttons = context.buttons;
    auto& tableButtons = context.tableButtons;
    const auto& tableFocus = context.tableFocus;
    const auto showBlue = context.showBlue;
    const auto showGreen = context.showGreen;
    const auto panelFocused = context.interaction.panelFocused;
    const auto editing = context.interaction.editing;
    const auto resultFocused = context.interaction.resultFocused;
    const auto resultCaret = context.interaction.resultCaret;
    const auto resultAnchor = context.interaction.resultAnchor;
    const auto& displayed = context.displayed;
    window.clear(sf::Color(30, 30, 30));
    window.draw(title);
    sf::Text overlayHint(font,
        std::string("K: F6 niebieski ")+(showBlue?"ON":"OFF")+
        " | F7 zielony "+(showGreen?"ON":"OFF"), 14);
    overlayHint.setFillColor(sf::Color(175,175,175));
    overlayHint.setPosition({690.f,94.f});
    window.draw(overlayHint);
    expressionPanel.setKeyboardFocus(panelFocused && !editing);
    expressionPanel.draw(window, displayed);
    expressionPanel.drawHighlights(window, displayed,stepHighlights);
    if(resultFocused) expressionPanel.drawEditor(window,displayed,resultCaret,
        std::min(resultCaret,resultAnchor),std::max(resultCaret,resultAnchor),true);
    expressionPanel.drawEditor(window, expression.get(), expression.cursor(),
        expression.selectionStart(), expression.selectionEnd(), true, editing);

    for (auto& btn : buttons) btn.draw(window);
    for (std::size_t i = 0; i < tableButtons.size(); ++i)
        if (!tableFocus || *tableFocus != i) tableButtons[i].draw(window);
    if (tableFocus && *tableFocus < tableButtons.size())
        tableButtons[*tableFocus].draw(window);

    window.display();
}
