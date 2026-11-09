#include "RootVisual.h"
#include <algorithm>
#include <array>
#include <cmath>

RootGeometry layoutRoot(float radicandWidth, float radicandHeight,
                        float indexWidth, float indexHeight, float padding) {
    RootGeometry g;
    radicandWidth = std::max(18.f, radicandWidth);
    radicandHeight = std::max(22.f, radicandHeight);
    indexWidth = std::max(0.f, indexWidth);
    indexHeight = std::max(0.f, indexHeight);
    padding = std::max(0.f, padding);

    // Reserve a little space for the small index even if it is currently empty.
    const float left = std::max(11.f, indexWidth + 2.f);
    const float hookWidth = 21.f;
    const float upper = std::max(12.f, indexHeight * 0.6f);
    g.overbarY = upper + padding;
    g.overbarStartX = left + hookWidth;
    g.radicandX = g.overbarStartX + padding;
    g.radicandY = g.overbarY + padding;
    g.indexX = std::max(0.f, left - indexWidth + 6.f);
    // Keep the index above the short left hook, close to the diagonal.
    g.indexY = std::max(0.f, g.overbarY + 19.f - indexHeight);
    g.width = g.radicandX + radicandWidth + padding;
    g.height = g.radicandY + radicandHeight + padding;
    return g;
}

void drawRootSymbol(sf::RenderTarget& target, sf::Vector2f origin,
                    const RootGeometry& g, sf::Color color, float thickness) {
    const float x = origin.x + g.overbarStartX;
    const float y = origin.y + g.overbarY;
    // The hook must not grow with the radicand or nested structures.
    // Keep the check-shaped hook at a fixed size. Nested radicands can
    // increase g.height, but must not stretch the hook across multiple lines.
    const float bottom = y + 38.f;
    const float t = std::max(1.f, thickness);
    // Four connected segments form the check-shaped radical and top bar.
    const std::array<sf::Vector2f, 5> points = {{
        {x - 21.f, y + 30.f},
        {x - 15.f, y + 27.f},
        {x - 9.f, bottom},
        {x, y},
        {origin.x + g.width - 2.f, y}
    }};
    // A small downstroke marks the right boundary of the radical's overbar.
    // Nested radicals remain visually distinct without changing the hook height.
    sf::RectangleShape endMark({t, 5.f});
    endMark.setFillColor(color);
    endMark.setPosition({origin.x + g.width - 2.f - t * .5f, y});
    target.draw(endMark);
    for (std::size_t i = 1; i < points.size(); ++i) {
        const sf::Vector2f d = points[i] - points[i-1];
        const float length = std::sqrt(d.x*d.x + d.y*d.y);
        if (length < 0.001f) continue;
        sf::RectangleShape segment({length, t});
        segment.setFillColor(color);
        segment.setPosition(points[i-1]);
        segment.setRotation(sf::radians(std::atan2(d.y, d.x)));
        target.draw(segment);
    }
}
