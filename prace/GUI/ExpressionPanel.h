#pragma once
#include "ExpressionScroll.h"
#include "ExpressionLayout.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class ExpressionPanel {
public:
    explicit ExpressionPanel(const sf::Font& font);
    void handleEvent(const sf::Event& event, sf::Vector2f mousePos, const std::string& expression);
    void draw(sf::RenderWindow& window, const std::string& expression);
    void resetScroll();
    void setKeyboardFocus(bool focused) { keyboardFocused = focused; }
    // Rozroznia dwie wizualne pozycje tego samego indeksu na granicy zawijania.
    void setWrapSide(bool endOfPrevious) { endOfPreviousLine = endOfPrevious; }
    bool wrapSideAtEnd() const { return endOfPreviousLine; }
    bool isWrapBoundary(const std::string& expression, std::size_t cursor) const;
    void setWrapSideFromClick(sf::Vector2f pos, const std::string& expression, std::size_t cursor);
    void ensureCursorVisible(const std::string& expression, std::size_t cursor);
    std::size_t verticalCursor(const std::string& expression, std::size_t cursor, int direction) const;
    std::size_t lineBoundary(const std::string& expression, std::size_t cursor, bool end) const;
    bool contains(sf::Vector2f pos) const;
    std::size_t cursorAt(sf::Vector2f pos, const std::string& expression) const;
    void drawEditor(sf::RenderWindow& window, const std::string& expression,
                    std::size_t cursor, std::size_t selectionStart,
                    std::size_t selectionEnd, bool active);
private:
    struct VisualLine { std::string text; std::size_t start, end; };
    std::vector<VisualLine> visualLines(const std::string& expression) const;
    std::size_t rowForCursor(const std::vector<VisualLine>& rows, std::size_t cursor) const;
    float cursorX(const VisualLine& row, std::size_t cursor) const;
    std::size_t cursorAtX(const VisualLine& row, float x) const;
    std::vector<std::string> lines(const std::string& expression) const;
    float maxY(std::size_t count) const;
    float maxX(const std::vector<std::string>& wrapped) const;
    const sf::Font& font;
    sf::RectangleShape box;
    ExpressionScroll scroll;
    bool endOfPreviousLine = false;
    bool keyboardFocused = false;
    static constexpr float padding = 20.f;
    static constexpr float lineHeight = 36.f;
    static constexpr float scrollbarWidth = 12.f;
    static constexpr float scrollbarPadding = 4.f;
    static constexpr float horizontalHeight = 12.f;
    static constexpr float wheelSpeed = 36.f;
};
