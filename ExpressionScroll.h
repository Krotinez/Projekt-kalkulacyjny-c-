#pragma once
#include <SFML/Graphics.hpp>
#include <cstddef>
#include <string>
#include <vector>

struct ExpressionScroll
{
    // Aktualne przesunięcie zawartości.
    float offsetX = 0.f;
    float offsetY = 0.f;

    // Przeciąganie pionowego suwaka.
    bool draggingY = false;
    float dragOffsetY = 0.f;

    // Przeciąganie poziomego suwaka.
    bool draggingX = false;
    float dragOffsetX = 0.f;
};


void setExpressionScroll(
    ExpressionScroll& scroll,
    float x,
    float y,
    float maxX,
    float maxY
);

float calculateMaxScrollY(
    std::size_t lineCount,
    float lineHeight,
    float visibleHeight
);

void drawVerticalScrollbar(
    sf::RenderWindow& window,
    float x,
    float y,
    float width,
    float height,
    float offsetY,
    float maxScrollY
);

bool isVerticalScrollbarClicked(
    sf::Vector2f mousePosition,
    float x,
    float y,
    float width,
    float height,
    float offsetY,
    float maxScrollY
);

void beginVerticalScrollbarDrag(
    ExpressionScroll& scroll,
    sf::Vector2f mousePosition,
    float x,
    float y,
    float width,
    float height,
    float maxScrollY
);

void updateVerticalScrollbarDrag(
    ExpressionScroll& scroll,
    float mouseY,
    float trackY,
    float trackHeight,
    float maxScrollY
);

void endVerticalScrollbarDrag(
    ExpressionScroll& scroll
);

float calculateMaxScrollX(
    float contentWidth,
    float visibleWidth
);

void drawHorizontalScrollbar(
    sf::RenderWindow& window,
    float x,
    float y,
    float width,
    float height,
    float offsetX,
    float maxScrollX
);

bool isHorizontalScrollbarClicked(
    sf::Vector2f mousePosition,
    float x,
    float y,
    float width,
    float height,
    float offsetX,
    float maxScrollX
);

void beginHorizontalScrollbarDrag(
    ExpressionScroll& scroll,
    sf::Vector2f mousePosition,
    float x,
    float y,
    float width,
    float height,
    float maxScrollX
);

void updateHorizontalScrollbarDrag(
    ExpressionScroll& scroll,
    float mouseX,
    float trackX,
    float trackWidth,
    float maxScrollX
);

void endHorizontalScrollbarDrag(
    ExpressionScroll& scroll
);

void scrollExpressionWithWheel(
    ExpressionScroll& scroll,
    float wheelDelta,
    float scrollSpeed,
    float maxScrollY
);

float calculateContentWidth(
    const std::vector<std::string>& lines,
    const sf::Font& font
);

void stopExpressionScrollbarDrag(ExpressionScroll& scroll);

void beginExpressionScrollbarDrag(
    ExpressionScroll& scroll,
    sf::Vector2f mousePos,
    sf::FloatRect expressionArea,
    float scrollbarPadding,
    float scrollbarWidth,
    float horizontalScrollbarHeight,
    float maxScrollY,
    float maxScrollX
);

void updateExpressionVerticalDrag(
    ExpressionScroll& scroll,
    float mouseY,
    sf::FloatRect expressionArea,
    float maxScrollY
);
