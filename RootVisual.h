#pragma once
#include <SFML/Graphics.hpp>

// Geometry of a radical with an optional, smaller index on the left.
// Coordinates are relative to the upper-left corner of its bounding box.
struct RootGeometry {
    float width = 0.f;
    float height = 0.f;
    float indexX = 0.f;
    float indexY = 0.f;
    float radicandX = 0.f;
    float radicandY = 0.f;
    float overbarY = 0.f;
    float overbarStartX = 0.f;
};

RootGeometry layoutRoot(float radicandWidth, float radicandHeight,
                        float indexWidth, float indexHeight,
                        float padding = 5.f);

// Draws the radical sign and its horizontal overbar. Content is drawn separately
// at the positions returned by layoutRoot; the index is omitted when empty.
void drawRootSymbol(sf::RenderTarget& target, sf::Vector2f origin,
                    const RootGeometry& geometry, sf::Color color,
                    float thickness = 2.f);
