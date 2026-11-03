#include "ExpressionKeyHandler.h"
#include "Expression.h"
#include "ArithmeticUI.h"
#include "../GUI/ExpressionPanel.h"
#include "../GUI/FractionModel.h"
#include "../GUI/RootModel.h"
#include "../GUI/LogModel.h"
#include <algorithm>

void handleExpressionKey(sf::Keyboard::Key key, bool ctrl, bool shift,
                         ExpressionKeyContext context)
{
    auto& expression = context.expression;
    auto& expressionPanel = context.panel;
    const auto& resultText = context.resultText;
    auto& editing = context.interaction.editing;
    auto& panelFocused = context.interaction.panelFocused;
    auto& resultFocused = context.interaction.resultFocused;
    auto& resultCaret = context.interaction.resultCaret;
    auto& resultAnchor = context.interaction.resultAnchor;
    const auto displayedExpression = [&]() {
        return ArithmeticUI::displayedText(expression.get(), resultText);
    };
    if (ctrl && key == sf::Keyboard::Key::A) expression.selectAll();
    else if (ctrl && key == sf::Keyboard::Key::C) expression.copy();
    else if (ctrl && key == sf::Keyboard::Key::X) expression.cut();
    else if (ctrl && key == sf::Keyboard::Key::V) expression.paste();
    else if (ctrl && key == sf::Keyboard::Key::Z) expression.undo();
    else if (ctrl && key == sf::Keyboard::Key::Y) expression.redo();
    else if (key == sf::Keyboard::Key::Left || key == sf::Keyboard::Key::Right) {
        const bool right = key == sf::Keyboard::Key::Right;
        std::size_t rootTarget = 0;
        if (!ctrl && !shift && LogModel::horizontalFieldCursor(
                expression.get(), expression.cursor(), right, rootTarget)) {
            expression.setCursor(rootTarget);
            expressionPanel.setWrapSide(right);
            expressionPanel.ensureCursorVisible(expression.get(), expression.cursor());
            return;
        }
        if (!ctrl && !shift && RootModel::horizontalFieldCursor(
                expression.get(), expression.cursor(), right, rootTarget)) {
            expression.setCursor(rootTarget);
            expressionPanel.setWrapSide(right);
            expressionPanel.ensureCursorVisible(expression.get(), expression.cursor());
            return;
        }
        const bool boundary = expressionPanel.isWrapBoundary(expression.get(), expression.cursor());
        // Pierwsza strzalka na granicy zawijania zmienia tylko wizualna strone kursora.
        if (!ctrl && boundary && ((right && expressionPanel.wrapSideAtEnd()) || (!right && !expressionPanel.wrapSideAtEnd()))) {
            expressionPanel.setWrapSide(!right);
        } else {
            // Na koncu wykladnika sa dwie pozycje kursora:
            // wewnatrz indeksu i poza cala potega.
            const bool atPowerEnd = !ctrl && !shift &&
                expressionPanel.isPowerEnd(expression.get(), expression.cursor());
            if (atPowerEnd && right && expressionPanel.powerExitSteps() <
                expressionPanel.powerEndDepth(expression.get(), expression.cursor())) {
                expressionPanel.setPowerExitSteps(expressionPanel.powerExitSteps() + 1);
            } else if (atPowerEnd && !right && expressionPanel.powerExitSteps() > 0) {
                expressionPanel.setPowerExitSteps(expressionPanel.powerExitSteps() - 1);
            } else {
                expression.moveHorizontal(right ? 1 : -1, ctrl, shift);
                expressionPanel.setPowerOutsideAtEnd(false);
            }
            expressionPanel.setWrapSide(right);
        }
        expressionPanel.ensureCursorVisible(expression.get(), expression.cursor());
    }
    else if (key == sf::Keyboard::Key::Up) {
        if (ctrl) expression.setCursor(expressionPanel.lineBoundary(expression.get(), expression.cursor(), false), shift);
        else {
            std::size_t fractionTarget = 0;
            if (FractionModel::verticalFieldCursor(expression.get(), expression.cursor(), false, fractionTarget))
                expression.setCursor(fractionTarget, shift);
            else expression.setCursor(expressionPanel.verticalCursor(expression.get(), expression.cursor(), -1), shift);
        }
        expressionPanel.setWrapSide(!ctrl);
    }
    else if (key == sf::Keyboard::Key::Down) {
        std::size_t fractionDownTarget = 0;
        const bool fractionDown = !ctrl && FractionModel::verticalFieldCursor(
            expression.get(), expression.cursor(), true, fractionDownTarget);
        if (!fractionDown && !ctrl && !shift && !resultText.empty()) {
            const auto target=expressionPanel.verticalCursor(displayedExpression(),expression.cursor(),1);
            if(target>ArithmeticUI::codepoints(expression.get())) {
                resultFocused=true;
                editing=false;
                resultCaret=resultAnchor=target;
                expressionPanel.ensureCursorVisible(displayedExpression(),resultCaret);
                return;
            }
        }
        if (ctrl) expression.setCursor(expressionPanel.lineBoundary(expression.get(), expression.cursor(), true), shift);
        else {
            std::size_t fractionTarget = 0;
            if (FractionModel::verticalFieldCursor(expression.get(), expression.cursor(), true, fractionTarget))
                expression.setCursor(fractionTarget, shift);
            else expression.setCursor(expressionPanel.verticalCursor(expression.get(), expression.cursor(), 1), shift);
        }
        expressionPanel.setWrapSide(ctrl);
    }
    else if (key == sf::Keyboard::Key::Backspace) expression.removeLast();
    else if (key == sf::Keyboard::Key::Delete) expression.deleteForward();
    else if (key == sf::Keyboard::Key::Enter) expression.add("\n");
    else if (key == sf::Keyboard::Key::Escape) { editing = false; panelFocused = true; }
}
