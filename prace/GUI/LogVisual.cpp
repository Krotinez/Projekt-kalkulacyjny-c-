#include "LogVisual.h"
#include <algorithm>

LogGeometry layoutLog(const sf::Font& font, unsigned textSize,
                      float baseWidth, float argumentWidth,
                      float argumentHeight, float padding) {
    const float p = std::max(0.f, padding);
    const float labelWidth = sf::Text(font, "log", textSize).getLocalBounds().size.x;
    const float normalHeight = static_cast<float>(textSize) * 1.35f;
    const float smallHeight = static_cast<float>(textSize) * 0.72f;
    const float baseFieldWidth = std::max(10.f, baseWidth);
    const float argumentFieldWidth = std::max(16.f, argumentWidth);
    const float argumentFieldHeight = std::max(normalHeight, argumentHeight);

    LogGeometry g;
    g.labelPosition = {0.f, 0.f};
    g.labelBaseline = normalHeight;
    g.basePosition = {labelWidth + 1.f, normalHeight - smallHeight * 0.45f};
    g.argumentPosition = {labelWidth + baseFieldWidth + p * 2.f, 0.f};
    g.width = g.argumentPosition.x + argumentFieldWidth;
    g.height = std::max(argumentFieldHeight, g.basePosition.y + smallHeight);
    return g;
}

void drawLogLabel(sf::RenderTarget& target, const sf::Font& font,
                  unsigned textSize, sf::Vector2f origin,
                  const LogGeometry& geometry, sf::Color color) {
    sf::Text label(font, "log", textSize);
    label.setFillColor(color);
    label.setPosition(origin + geometry.labelPosition);
    target.draw(label);
}
