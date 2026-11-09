#pragma once
#include "ExpressionScroll.h"
#include "ExpressionLayout.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

struct ExpressionHighlight {
    std::size_t start = 0, end = 0;
    bool result = false;
    bool error = false;
};

class ExpressionPanel {
public:
    explicit ExpressionPanel(const sf::Font& font);
    void handleEvent(const sf::Event& event, sf::Vector2f mousePos, const std::string& expression);
    void draw(sf::RenderWindow& window, const std::string& expression);
    void resetScroll();
    void drawHighlights(sf::RenderWindow& window, const std::string& displayed,
        const std::vector<ExpressionHighlight>& highlights);
    void scrollToBottom(const std::string& expression);
    void setKeyboardFocus(bool focused) { keyboardFocused = focused; }
    // Rozroznia dwie wizualne pozycje tego samego indeksu na granicy zawijania.
    void setWrapSide(bool endOfPrevious) { endOfPreviousLine = endOfPrevious; }
    bool wrapSideAtEnd() const { return endOfPreviousLine; }
    bool isWrapBoundary(const std::string& expression, std::size_t cursor) const;
    void setWrapSideFromClick(sf::Vector2f pos, const std::string& expression, std::size_t cursor);
    // Druga wizualna pozycja na koncu wykladnika: poza indeksem.
    bool powerOutsideAtEnd() const { return powerExitLevel > 0; }
    int powerExitSteps() const { return powerExitLevel; }
    void setPowerExitSteps(int steps) { powerExitLevel = steps; }
    int powerEndDepth(const std::string& expression, std::size_t cursor) const;
    void setPowerOutsideAtEnd(bool outside) { powerExitLevel = outside ? 1 : 0; }
    bool isPowerEnd(const std::string& expression, std::size_t cursor) const;
    void ensureCursorVisible(const std::string& expression, std::size_t cursor);
    std::size_t verticalCursor(const std::string& expression, std::size_t cursor, int direction) const;
    std::size_t lineBoundary(const std::string& expression, std::size_t cursor, bool end) const;
    bool contains(sf::Vector2f pos) const;
    std::size_t cursorAt(sf::Vector2f pos, const std::string& expression) const;
    void drawEditor(sf::RenderWindow& window, const std::string& expression,
                    std::size_t cursor, std::size_t selectionStart,
                    std::size_t selectionEnd, bool active, bool drawCaret = true);
private:
    struct VisualLine { std::string text; std::size_t start, end; };
    std::vector<VisualLine> visualLines(const std::string& expression) const;
    std::size_t rowForCursor(const std::vector<VisualLine>& rows, std::size_t cursor) const;
    float rowHeight(const VisualLine& row) const;
    float rowTop(const VisualLine& row) const;
    float rowOffset(const std::vector<VisualLine>& rows, std::size_t index) const;
    float contentHeight(const std::vector<VisualLine>& rows) const;
    std::size_t rowAtY(const std::vector<VisualLine>& rows, float y) const;
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
    int powerExitLevel = 0;
    static constexpr float padding = 20.f;
    static constexpr float lineHeight = 36.f;
    static constexpr float scrollbarWidth = 12.f;
    static constexpr float scrollbarPadding = 4.f;
    static constexpr float horizontalHeight = 12.f;
    static constexpr float wheelSpeed = 36.f;
};
