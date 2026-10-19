#include "Button.h"

Button::Button(
    const sf::Font& font,
    const std::string& label,
    sf::Vector2f position
)
    : Button(font, font, label, position, ButtonType::Large)
{
}

Button::Button(
    const sf::Font& font,
    const sf::Font& mathFont,
    const std::string& label,
    sf::Vector2f position,
    ButtonType buttonType,
    std::optional<sf::Keyboard::Key> buttonShortcut
)
    : text(
        (buttonType == ButtonType::Large) ? font : mathFont,
        sf::String::fromUtf8(label.begin(), label.end()),
        buttonType == ButtonType::Large ? 30 : (buttonType == ButtonType::Small ? 22 : 20)
      ),
      type(buttonType),
      shortcut(buttonShortcut)
{
    sf::Vector2f size;

    switch (type)
    {
        case ButtonType::Large:
            size = {250.f, 70.f};
            break;

        case ButtonType::Medium:
            size = {90.f, 40.f};
            break;

        case ButtonType::Small:
            size = {60.f, 45.f};
            break;

        case ButtonType::Dropdown:
            size = {100.f, 40.f};
            break;
    }

    shape.setSize(size);
    shape.setPosition(position);

    // Dawna kolorystyka przycisków.
    shape.setFillColor(sf::Color(70, 70, 70));
    shape.setOutlineColor(sf::Color::White);
    shape.setOutlineThickness(0.f);

    text.setFillColor(sf::Color::White);

    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin({
        textBounds.position.x + textBounds.size.x / 2.f,
        textBounds.position.y + textBounds.size.y / 2.f
    });

    text.setPosition({
        position.x + size.x / 2.f,
        position.y + size.y / 2.f
    });
}

void Button::updateColors()
{
    if (!enabled)
    {
        shape.setFillColor(sf::Color(40, 40, 40));
        shape.setOutlineThickness(0.f);
        text.setFillColor(sf::Color(100, 100, 100));
        return;
    }

    text.setFillColor(sf::Color::White);

    if ((mousePressed && isHoveredState) || enterPressed)
    {
        shape.setFillColor(sf::Color(160, 160, 160));
    }
        else if (isHoveredState)
    {
        shape.setFillColor(sf::Color(100, 100, 100));
    }
    else
    {
        shape.setFillColor(sf::Color(70, 70, 70));
    }

    if (isSelectedState)
    {
        shape.setOutlineColor(sf::Color::White);
        shape.setOutlineThickness(3.f);
    }
    else
    {
        shape.setOutlineThickness(0.f);
    }
}

void Button::setSelected(bool selected)
{
    isSelectedState = selected;
    updateColors();
}

void Button::setEnabled(bool value)
{
    enabled = value;

    if (!enabled)
    {
        activated = false;
        mousePressed = false;
        enterPressed = false;
        isHoveredState = false;
        isSelectedState = false;
    }

    updateColors();
}

bool Button::isEnabled() const
{
    return enabled;
}

bool Button::isSelected() const
{
    return isSelectedState;
}

void Button::setFocused(bool focused)
{
    setSelected(focused);
}

bool Button::isFocused() const
{
    return isSelectedState;
}

bool Button::isHovered() const
{
    return isHoveredState;
}

void Button::setTabActivation(bool value)
{
    tabActivates = value;
}

void Button::handleEvent(const sf::Event& event)
{
    if (!enabled)
    {
        return;
    }

    if (event.is<sf::Event::MouseButtonPressed>())
    {
        const auto* mouseEvent =
            event.getIf<sf::Event::MouseButtonPressed>();

        if (mouseEvent->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mousePosition(
                static_cast<float>(mouseEvent->position.x),
                static_cast<float>(mouseEvent->position.y)
            );

            if (shape.getGlobalBounds().contains(mousePosition))
            {
                mousePressed = true;
                updateColors();
            }
        }
    }

    if (event.is<sf::Event::MouseButtonReleased>())
    {
        const auto* mouseEvent =
            event.getIf<sf::Event::MouseButtonReleased>();

        if (mouseEvent->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mousePosition(
                static_cast<float>(mouseEvent->position.x),
                static_cast<float>(mouseEvent->position.y)
            );

            if (mousePressed &&
                shape.getGlobalBounds().contains(mousePosition))
            {
                activated = true;
            }

            mousePressed = false;
            updateColors();
        }
    }

    if (event.is<sf::Event::KeyPressed>())
    {
        const auto* keyEvent =
            event.getIf<sf::Event::KeyPressed>();

        if (shortcut && keyEvent->code == *shortcut)
        {
            activated = true;
        }

        if (isSelectedState &&
            (keyEvent->code == sf::Keyboard::Key::Enter ||
            keyEvent->code == sf::Keyboard::Key::Space ||
            (tabActivates && keyEvent->code == sf::Keyboard::Key::Tab)))
        {
            activated = true;
            enterPressed = true;
            updateColors();
        }
    }

    if (event.is<sf::Event::KeyReleased>())
    {
        const auto* keyEvent =
            event.getIf<sf::Event::KeyReleased>();

        if (keyEvent->code == sf::Keyboard::Key::Enter ||
            keyEvent->code == sf::Keyboard::Key::Space ||
            (tabActivates && keyEvent->code == sf::Keyboard::Key::Tab))
        {
            enterPressed = false;
            updateColors();
        }
    }
}

void Button::update(sf::Vector2f mousePos)
{
    bool hovered = enabled && shape.getGlobalBounds().contains(mousePos);

    if (hovered != isHoveredState)
    {
        isHoveredState = hovered;
        updateColors();
    }
    else if (mousePressed)
    {
        updateColors();
    }
}

void Button::draw(sf::RenderWindow& window)
{
    window.draw(shape);
    window.draw(text);
}

bool Button::isActivated()
{
    if (activated)
    {
        activated = false;
        return true;
    }

    return false;
}

sf::Vector2f Button::getPosition() const
{
    return shape.getPosition();
}

sf::Vector2f Button::getSize() const
{
    return shape.getSize();
}

sf::FloatRect Button::getGlobalBounds() const
{
    return shape.getGlobalBounds();
}
