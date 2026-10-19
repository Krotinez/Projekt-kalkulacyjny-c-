#include "ExpressionPanel.h"
#include "ExpressionView.h"
#include <algorithm>
#include <cmath>

ExpressionPanel::ExpressionPanel(const sf::Font& font) : font(font), box({780.f, 210.f})
{
    box.setFillColor(sf::Color(45, 45, 45));
    box.setOutlineColor(sf::Color(100, 100, 100));
    box.setOutlineThickness(2.f);
    box.setPosition({220.f, 180.f});
}

std::vector<ExpressionPanel::VisualLine> ExpressionPanel::visualLines(const std::string& expression) const
{
    std::vector<VisualLine> result;
    sf::String all = sf::String::fromUtf8(expression.begin(), expression.end());
    std::size_t segmentStart = 0;
    for (std::size_t i = 0; i <= all.getSize(); ++i)
    {
        if (i != all.getSize() && all[i] != U'\n') continue;
        const sf::String segment = all.substring(segmentStart, i - segmentStart);
        auto bytes = segment.toUtf8();
        const std::string content(bytes.begin(), bytes.end());
        auto wrapped = buildExpressionLines(splitExpression(content),
            box.getSize().x - 2.f * padding, font, 28);
        if (wrapped.empty()) wrapped.push_back("");
        std::size_t offset = segmentStart;
        for (const auto& part : wrapped)
        {
            const std::size_t count = sf::String::fromUtf8(part.begin(), part.end()).getSize();
            result.push_back({part, offset, offset + count});
            offset += count;
        }
        segmentStart = i + 1; // Explicit newline occupies one character.
    }
    return result;
}
std::vector<std::string> ExpressionPanel::lines(const std::string& expression) const
{
    std::vector<std::string> result;
    for (const auto& row : visualLines(expression)) result.push_back(row.text);
    return result;
}
std::size_t ExpressionPanel::rowForCursor(const std::vector<VisualLine>& rows, std::size_t cursor) const
{
    if (rows.empty()) return 0;
    // Soft wraps have two valid visual caret positions for the same text index.
    for (std::size_t i = 0; i + 1 < rows.size(); ++i)
        if (endOfPreviousLine && cursor == rows[i].end && rows[i+1].start == cursor) return i;
    for (std::size_t i = 0; i < rows.size(); ++i)
        if (cursor < rows[i].end || (cursor == rows[i].end &&
            (i + 1 == rows.size() || rows[i+1].start > cursor))) return i;
    return rows.size() - 1;
}

bool ExpressionPanel::isWrapBoundary(const std::string& expression, std::size_t cursor) const
{
    const auto rows = visualLines(expression);
    for (std::size_t i = 0; i + 1 < rows.size(); ++i)
        if (rows[i].end == cursor && rows[i+1].start == cursor) return true;
    return false;
}
void ExpressionPanel::setWrapSideFromClick(sf::Vector2f pos, const std::string& expression, std::size_t cursor)
{
    const auto rows = visualLines(expression);
    const int index = std::clamp(static_cast<int>(std::floor(
        (pos.y - box.getPosition().y - padding + scroll.offsetY) / lineHeight)),
        0, static_cast<int>(rows.size()) - 1);
    endOfPreviousLine = index + 1 < static_cast<int>(rows.size()) &&
        rows[index].end == cursor && rows[index+1].start == cursor;
}
float ExpressionPanel::cursorX(const VisualLine& row, std::size_t cursor) const
{
    sf::Text probe(font, sf::String::fromUtf8(row.text.begin(), row.text.end()), 28);
    const auto len = row.end - row.start;
    return probe.findCharacterPos(std::min(len, cursor > row.start ? cursor - row.start : std::size_t{0})).x;
}
std::size_t ExpressionPanel::cursorAtX(const VisualLine& row, float x) const
{
    sf::Text probe(font, sf::String::fromUtf8(row.text.begin(), row.text.end()), 28);
    float best = 1e30f;
    std::size_t nearest = 0;
    for (std::size_t i = 0; i <= row.end - row.start; ++i)
    {
        const float d = std::abs(probe.findCharacterPos(i).x - x);
        if (d < best) { best = d; nearest = i; }
    }
    return row.start + nearest;
}
std::size_t ExpressionPanel::verticalCursor(const std::string& expression, std::size_t cursor, int direction) const
{
    const auto rows = visualLines(expression);
    const auto row = rowForCursor(rows, cursor);
    const auto next = std::clamp(static_cast<int>(row) + direction, 0, static_cast<int>(rows.size()) - 1);
    if (next == static_cast<int>(row)) return cursor;
    return cursorAtX(rows[next], cursorX(rows[row], cursor));
}
std::size_t ExpressionPanel::lineBoundary(const std::string& expression, std::size_t cursor, bool end) const
{
    const auto rows = visualLines(expression);
    const auto& row = rows[rowForCursor(rows, cursor)];
    return end ? row.end : row.start;
}
void ExpressionPanel::ensureCursorVisible(const std::string& expression, std::size_t cursor)
{
    const auto rows = visualLines(expression);
    const auto index = rowForCursor(rows, cursor);
    const float availableHeight = box.getSize().y - 2.f * padding;
    const float availableWidth = box.getSize().x - 2.f * padding;
    const float y = static_cast<float>(index) * lineHeight;
    const float x = cursorX(rows[index], cursor);
    if (y < scroll.offsetY) scroll.offsetY = y;
    if (y + lineHeight > scroll.offsetY + availableHeight)
        scroll.offsetY = y + lineHeight - availableHeight;
    if (x < scroll.offsetX) scroll.offsetX = x;
    if (x + 5.f > scroll.offsetX + availableWidth)
        scroll.offsetX = x + 5.f - availableWidth;
    float widest = 0.f;
    for (const auto& row : rows) widest = std::max(widest, getTextWidth(row.text, font, 28));
    setExpressionScroll(scroll, scroll.offsetX, scroll.offsetY,
        calculateMaxScrollX(widest, availableWidth), maxY(rows.size()));
}
float ExpressionPanel::maxY(std::size_t count) const
{
    return calculateMaxScrollY(count, lineHeight, box.getSize().y - 2.f * padding);
}
float ExpressionPanel::maxX(const std::vector<std::string>& wrapped) const
{
    return calculateMaxScrollX(calculateContentWidth(wrapped, font), box.getSize().x - 2.f * padding);
}
void ExpressionPanel::resetScroll()
{
    scroll.offsetX = scroll.offsetY = 0.f;
    stopExpressionScrollbarDrag(scroll);
}
void ExpressionPanel::handleEvent(const sf::Event& event, sf::Vector2f mousePos, const std::string& expression)
{
    if (const auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>())
    {
        if (wheel->wheel == sf::Mouse::Wheel::Vertical)
            scrollExpressionWithWheel(scroll, wheel->delta, wheelSpeed, maxY(lines(expression).size()));
    }
    if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (pressed->button == sf::Mouse::Button::Left)
        {
            const auto wrapped = lines(expression);
            beginExpressionScrollbarDrag(scroll, mousePos, {box.getPosition(), box.getSize()},
                scrollbarPadding, scrollbarWidth, horizontalHeight, maxY(wrapped.size()), maxX(wrapped));
        }
    }
    if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>())
    {
        if (released->button == sf::Mouse::Button::Left)
            stopExpressionScrollbarDrag(scroll);
    }
    if (const auto* moved = event.getIf<sf::Event::MouseMoved>())
    {
        if (scroll.draggingY || scroll.draggingX)
        {
            const auto wrapped = lines(expression);
            if (scroll.draggingY)
                updateExpressionVerticalDrag(scroll, static_cast<float>(moved->position.y),
                    {box.getPosition(), box.getSize()}, maxY(wrapped.size()));
            if (scroll.draggingX)
                updateHorizontalScrollbarDrag(scroll, static_cast<float>(moved->position.x),
                    box.getPosition().x, box.getSize().x, maxX(wrapped));
        }
    }
}
void ExpressionPanel::draw(sf::RenderWindow& window, const std::string& expression)
{
    const auto wrapped = lines(expression);
    const float availableWidth = box.getSize().x - 2.f * padding;
    const auto prepared = prepareExpressionLines(wrapped, availableWidth, font, 28);
    const float scrollY = maxY(prepared.size());
    float contentWidth = 0.f;
    for (const auto& line : prepared)
        contentWidth = std::max(contentWidth, line.width);
    const float scrollX = calculateMaxScrollX(contentWidth, availableWidth);
    setExpressionScroll(scroll, scroll.offsetX, scroll.offsetY, scrollX, scrollY);

    box.setOutlineColor(keyboardFocused ? sf::Color(90, 160, 255) : sf::Color(100, 100, 100));
    box.setOutlineThickness(keyboardFocused ? 3.f : 2.f);
    window.draw(box);
    sf::Text text(font, "", 28);
    text.setFillColor(sf::Color::White);
    const sf::View previousView = window.getView();
    window.setView(createExpressionView(window, {box.getPosition(), box.getSize()}));
    drawExpressionLines(window, text, prepared,
        box.getPosition().x + padding, box.getPosition().y + padding,
        lineHeight, box.getPosition().y + box.getSize().y, scroll);
    window.setView(previousView);

    drawVerticalScrollbar(window, box.getPosition().x + box.getSize().x + scrollbarPadding,
        box.getPosition().y, scrollbarWidth, box.getSize().y, scroll.offsetY, scrollY);
    drawHorizontalScrollbar(window, box.getPosition().x,
        box.getPosition().y + box.getSize().y + scrollbarPadding,
        box.getSize().x, horizontalHeight, scroll.offsetX, scrollX);
}


bool ExpressionPanel::contains(sf::Vector2f pos) const {
    const auto p = box.getPosition(), size = box.getSize();
    return pos.x >= p.x && pos.x <= p.x + size.x &&
           pos.y >= p.y && pos.y <= p.y + size.y;
}

// Pozycje znakow sa liczone w Unicode, nie w bajtach UTF-8.
std::size_t ExpressionPanel::cursorAt(sf::Vector2f pos, const std::string& expression) const
{
    const auto rows = visualLines(expression);
    const int index = std::clamp(static_cast<int>(std::floor(
        (pos.y - box.getPosition().y - padding + scroll.offsetY) / lineHeight)),
        0, static_cast<int>(rows.size()) - 1);
    return cursorAtX(rows[index], pos.x - box.getPosition().x - padding + scroll.offsetX);
}

void ExpressionPanel::drawEditor(sf::RenderWindow& window, const std::string& expression,
    std::size_t cursor, std::size_t selectionStart, std::size_t selectionEnd, bool active)
{
    if (!active) return;
    const auto rows = visualLines(expression);
    const std::size_t cursorRow = rowForCursor(rows, cursor);
    const sf::View previous = window.getView();
    window.setView(createExpressionView(window, {box.getPosition(), box.getSize()}));
    for (std::size_t index = 0; index < rows.size(); ++index)
    {
        const auto& row = rows[index];
        const float y = box.getPosition().y + padding + index * lineHeight - scroll.offsetY;
        const float x = box.getPosition().x + padding - scroll.offsetX;
        if (selectionStart < row.end && selectionEnd > row.start)
        {
            const float a = cursorX(row, std::max(selectionStart, row.start));
            const float b = cursorX(row, std::min(selectionEnd, row.end));
            sf::RectangleShape highlight({std::max(0.f, b-a), 32.f});
            highlight.setPosition({x+a, y});
            highlight.setFillColor(sf::Color(50, 110, 210, 130));
            window.draw(highlight);
        }
        if (index == cursorRow)
        {
            sf::RectangleShape caretShape({2.f, 30.f});
            caretShape.setPosition({x + cursorX(row, cursor), y});
            caretShape.setFillColor(sf::Color::White);
            window.draw(caretShape);
        }
    }
    window.setView(previous);
}
