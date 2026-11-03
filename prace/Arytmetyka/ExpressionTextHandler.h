#pragma once
#include "NumberSystem.h"
#include "ArithmeticUI.h"
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Font.hpp>

class Expression;

struct ExpressionTextContext {
    Expression& expression;
    sf::Font& mathFont;
    NumberSystem currentSystem;
    ArithmeticUI::EditorInteractionState& interaction;
};

// Inserts a character, applying numeric-base and exponent-depth rules.
void handleExpressionText(const sf::Event::TextEntered& entered,
                          ExpressionTextContext context);
