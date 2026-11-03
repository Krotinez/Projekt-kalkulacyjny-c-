#pragma once
#include "NumberSystem.h"
#include "ArithmeticUI.h"
#include "../GUI/ExpressionPanel.h"
#include <SFML/Graphics/Font.hpp>
#include <cstddef>
#include <string>
#include <vector>

class Expression;
struct ArithmeticButtonDef;

enum class ButtonActionResult { Handled, SkipFrame, Exit };

struct ArithmeticButtonContext {
    Expression& expression;
    ExpressionPanel& expressionPanel;
    sf::Font& mathFont;
    NumberSystem& currentSystem;
    std::string& resultText;
    std::string& expressionAtResult;
    std::vector<ExpressionHighlight>& stepHighlights;
    ArithmeticUI::EditorInteractionState& interaction;
    bool& stepsMode;
    bool showBlue;
    bool showGreen;
};

ButtonActionResult handleArithmeticButton(const std::string& action,
                                         const std::string& lbl,
                                         ArithmeticButtonContext context);
