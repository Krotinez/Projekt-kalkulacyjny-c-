#pragma once
#include <SFML/Graphics.hpp>

// Geometry of log with an optional small base and a normal-size argument.
// Positions are relative to the top-left corner of the whole expression.
struct LogGeometry {
    float width = 0.f;
    float height = 0.f;
    sf::Vector2f labelPosition{};
    sf::Vector2f basePosition{};
    sf::Vector2f argumentPosition{};
    float labelBaseline = 0.f;
};

// The base field is reserved even when empty, but no "10" is drawn.
LogGeometry layoutLog(const sf::Font& font, unsigned textSize,
                      float baseWidth, float argumentWidth,
                      float argumentHeight = 0.f, float padding = 4.f);

// Draw only the "log" label; the editor renders the base and argument
// independently so their carets can have different sizes.
void drawLogLabel(sf::RenderTarget& target, const sf::Font& font,
                  unsigned textSize, sf::Vector2f origin,
                  const LogGeometry& geometry, sf::Color color);
