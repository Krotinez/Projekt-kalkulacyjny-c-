#pragma once
#include "../GUI/Button.h"
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <vector>

struct ButtonDef {
    std::string label;
    sf::Vector2f pos;
    ButtonType type;
    std::optional<sf::Keyboard::Key> shortcut = std::nullopt;
    std::string action;
};
struct ArithmeticButtons {
    std::vector<ButtonDef> defs;
    std::vector<Button> buttons;
    std::vector<Button> tableButtons;
};
ArithmeticButtons createArithmeticButtons(sf::Font& font, sf::Font& mathFont);
