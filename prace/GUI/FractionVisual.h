#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// Geometry for a single graphical fraction. This module does not change the
// calculator expression or its evaluation; it is a rendering building block.
struct FractionGeometry {
    float width = 0.f;
    float height = 0.f;
    float numeratorX = 0.f;
    float numeratorY = 0.f;
    float denominatorX = 0.f;
    float denominatorY = 0.f;
    float barY = 0.f;
};

FractionGeometry layoutFraction(float numeratorWidth, float denominatorWidth,
                                float fontLineHeight, float padding = 6.f);
void drawFractionBar(sf::RenderTarget& target, sf::Vector2f origin,
                     const FractionGeometry& geometry, sf::Color color);
