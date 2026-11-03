#include "MouseSelectionHandler.h"
#include "ArithmeticUI.h"
#include "Expression.h"
#include "../GUI/ButtonNavigation.h"
#include "../GUI/ExpressionPanel.h"
#include <algorithm>

bool handleMouseSelectionEvent(const sf::Event& event,
                               sf::Vector2f mousePos,
                               MouseSelectionContext context)
{
    auto& expression = context.expression;
    auto& panel = context.panel;
    auto& navigation = context.navigation;
    auto& resultFocused = context.interaction.resultFocused;
    auto& resultCaret = context.interaction.resultCaret;
    auto& resultAnchor = context.interaction.resultAnchor;
    auto& editing = context.interaction.editing;
    auto& selectingWithMouse = context.interaction.selectingWithMouse;
    auto& panelFocused = context.interaction.panelFocused;

    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouse->button == sf::Mouse::Button::Left) {
            if (panel.contains(mousePos)) {
                const std::string all = ArithmeticUI::displayedText(expression.get(), context.resultText);
                const std::size_t clicked = panel.cursorAt(mousePos, all);
                const std::size_t resultBegin = ArithmeticUI::codepoints(expression.get()) + 1;
                if (!context.resultText.empty() && clicked >= resultBegin) {
                    resultFocused = true;
                    resultCaret = resultAnchor = clicked;
                    selectingWithMouse = true;
                    editing = false;
                    navigation.clearFocus();
                    panelFocused = false;
                    return true;
                }
                resultFocused = false;
                editing = true;
                selectingWithMouse = true;
                expression.setCursor(panel.cursorAt(mousePos, expression.get()));
                panel.setWrapSideFromClick(mousePos, expression.get(), expression.cursor());
                navigation.clearFocus();
                panelFocused = false;
            } else {
                panelFocused = false;
                selectingWithMouse = false;
                editing = false;
                resultFocused = false;
            }
        }
    }
    if (const auto* mouse = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouse->button == sf::Mouse::Button::Left) selectingWithMouse = false;
    }
    if (event.is<sf::Event::MouseMoved>() && selectingWithMouse && resultFocused) {
        const std::string all = ArithmeticUI::displayedText(expression.get(), context.resultText);
        resultCaret = std::max(ArithmeticUI::codepoints(expression.get()) + 1,
                               panel.cursorAt(mousePos, all));
    } else if (event.is<sf::Event::MouseMoved>() && selectingWithMouse) {
        expression.setCursor(panel.cursorAt(mousePos, expression.get()), true);
        panel.setWrapSideFromClick(mousePos, expression.get(), expression.cursor());
    }
    return false;
}
