#include "FractionVisual.h"
#include <algorithm>

FractionGeometry layoutFraction(float numeratorWidth, float denominatorWidth,
                                float fontLineHeight, float padding) {
    FractionGeometry result;
    const float content = std::max({numeratorWidth, denominatorWidth, 18.f});
    result.width = content + 2.f * padding;
    result.height = 1.4f * fontLineHeight + 1.f;
    result.numeratorX = (result.width - numeratorWidth) * 0.5f;
    result.numeratorY = 0.f;
    result.barY = fontLineHeight + 1.f;
    result.denominatorX = (result.width - denominatorWidth) * 0.5f;
    result.denominatorY = fontLineHeight + 2.f;
    return result;
}

void drawFractionBar(sf::RenderTarget& target, sf::Vector2f origin,
                     const FractionGeometry& geometry, sf::Color color) {
    sf::RectangleShape bar({geometry.width, 2.f});
    bar.setPosition({origin.x, origin.y + geometry.barY});
    bar.setFillColor(color);
    target.draw(bar);
}
