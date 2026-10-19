#pragma once
#include "Button.h"
#include <optional>
#include <vector>
#include <cstddef>

class ButtonNavigation {
public:
    explicit ButtonNavigation(std::vector<Button>& buttons);
    void clearFocus();
    void setFocus(std::size_t newIndex);
    std::optional<std::size_t> nearestInDirection(std::size_t fromIndex, sf::Keyboard::Key direction) const;
    std::optional<std::size_t> wrapInDirection(std::size_t fromIndex, sf::Keyboard::Key direction) const;
    std::size_t nearestToMouse(sf::Vector2f mousePos) const;
    const std::optional<std::size_t>& focus() const { return focusedIndex; }
private:
    sf::Vector2f buttonCenter(std::size_t index) const;
    std::vector<Button>& buttons;
    std::optional<std::size_t> focusedIndex;
};
