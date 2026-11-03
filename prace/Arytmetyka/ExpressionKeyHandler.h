#pragma once
#include "ArithmeticUI.h"
#include <SFML/Window/Keyboard.hpp>
#include <cstddef>
#include <string>

class Expression;
class ExpressionPanel;

// State that can change while editing an expression with the keyboard.
struct ExpressionKeyContext {
    Expression& expression;
    ExpressionPanel& panel;
    const std::string& resultText;
    ArithmeticUI::EditorInteractionState& interaction;
};

void handleExpressionKey(sf::Keyboard::Key key, bool ctrl, bool shift,
                         ExpressionKeyContext context);
