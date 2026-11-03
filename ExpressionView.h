#pragma once

#include <SFML/Graphics.hpp>
#include "ExpressionScroll.h"
#include <string>

void drawExpressionLine(
    sf::RenderWindow& window,
    sf::Text& expressionText,
    const std::string& line,
    float x,
    float y
);

#include "ExpressionLayout.h"

void drawExpressionLines(
    sf::RenderWindow& window,
    sf::Text& expressionText,
    const std::vector<ExpressionLine>& preparedLines,
    float startX,
    float startY,
    float lineHeight,
    float maxY,
    const ExpressionScroll& scroll
);

sf::View createExpressionView(
    const sf::RenderWindow& window,
    const sf::FloatRect& area
);

// Pozycje kursora w pikselach wzgledem poczatku wiersza.
// Y to wysokosc gornego brzegu kursora (wzgledem y wiersza).
std::vector<sf::Vector2f> fractionCaretPositions(
    const std::string& line, const sf::Font& font, unsigned size = 28);
