#pragma once
#include "ArithmeticUI.h"
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <cstddef>
#include <string>

class Expression;
class ExpressionPanel;
class ButtonNavigation;

// References the editor state owned by runArytmetyka; does not own it.
struct MouseSelectionContext {
    Expression& expression;
    ExpressionPanel& panel;
    ButtonNavigation& navigation;
    const std::string& resultText;
    ArithmeticUI::EditorInteractionState& interaction;
};

// Returns true only when the event has been fully consumed.
bool handleMouseSelectionEvent(const sf::Event& event,
                               sf::Vector2f mousePos,
                               MouseSelectionContext context);
