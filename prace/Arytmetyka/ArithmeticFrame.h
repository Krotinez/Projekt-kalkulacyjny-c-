#pragma once
#include "ArithmeticButtons.h"
#include "ArithmeticUI.h"
#include "Expression.h"
#include "../GUI/ExpressionPanel.h"
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <vector>

// Frame-level UI state is passed explicitly; the rendering code never owns it.
struct ArithmeticFrameContext {
    Expression& expression;
    ExpressionPanel& panel;
    std::string& resultText;
    std::string& expressionAtResult;
    std::vector<ExpressionHighlight>& highlights;
    ArithmeticUI::EditorInteractionState& interaction;
    bool stepsMode;
    std::vector<ButtonDef>& defs;
    std::vector<Button>& buttons;
    std::vector<Button>& tableButtons;
    std::optional<std::size_t>& tableFocus;
    std::size_t fractionCursorBefore;
    const std::string& fractionBefore;
    std::size_t& lastVisibleCursor;
    std::string& lastVisibleExpression;
    sf::Vector2f mousePos;
};

void updateArithmeticFrame(ArithmeticFrameContext context);

struct ArithmeticDrawContext {
    sf::RenderWindow& window;
    sf::Font& font;
    sf::Text& title;
    Expression& expression;
    ExpressionPanel& panel;
    const std::string& displayed;
    const std::vector<ExpressionHighlight>& highlights;
    std::vector<Button>& buttons;
    std::vector<Button>& tableButtons;
    const std::optional<std::size_t>& tableFocus;
    bool showBlue;
    bool showGreen;
    const ArithmeticUI::EditorInteractionState& interaction;
};

void drawArithmeticFrame(ArithmeticDrawContext context);
