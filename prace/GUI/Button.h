#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <optional>

enum class ButtonType
{
    Large,
    Medium,
    Small,
    Dropdown
};

class Button
{
public:
    Button(
        const sf::Font& font,
        const std::string& label,
        sf::Vector2f position
    );

    Button(
        const sf::Font& font,
        const sf::Font& mathFont,
        const std::string& label,
        sf::Vector2f position,
        ButtonType type,
        std::optional<sf::Keyboard::Key> shortcut = std::nullopt
    );

    void handleEvent(const sf::Event& event);
    void setTabActivation(bool value);
    void update(sf::Vector2f mousePos);
    void draw(sf::RenderWindow& window);

    bool isActivated();

    void setEnabled(bool enabled);
    bool isEnabled() const;

    // Zaznaczenie używane wyłącznie do nawigacji klawiaturą.
    void setSelected(bool selected);
    bool isSelected() const;

    // Zachowane jako alias dla kodu, który używał wcześniej nazwy "Focused".
    void setFocused(bool focused);
    bool isFocused() const;

    bool isHovered() const;

    sf::Vector2f getPosition() const;
    sf::Vector2f getSize() const;
    sf::FloatRect getGlobalBounds() const;

private:
    sf::RectangleShape shape;
    sf::Text text;

    ButtonType type;
    std::optional<sf::Keyboard::Key> shortcut;

    bool isHoveredState = false;
    bool isSelectedState = false;
    bool activated = false;
    bool mousePressed = false;
    bool enterPressed = false;
    bool enabled = true;
    bool tabActivates = false;

    void updateColors();
};
