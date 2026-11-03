#pragma once

#include "ArithmeticButtons.h"
#include "ArithmeticUI.h"
#include "../GUI/ButtonNavigation.h"
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cstddef>
#include <optional>
#include <vector>

// Keyboard focus between calculator buttons, the expression panel and results table.
// References preserve the original focus state without copying UI controls.
struct ButtonFocusContext {
    const std::vector<ButtonDef>& defs;
    std::vector<Button>& buttons;
    std::vector<Button>& tableButtons;
    ButtonNavigation& navigation;
    std::optional<std::size_t>& tableFocus;
    ArithmeticUI::EditorInteractionState& interaction;
};

void handleButtonFocusKey(sf::Keyboard::Key direction,
                          sf::Vector2f mousePos,
                          ButtonFocusContext context);
