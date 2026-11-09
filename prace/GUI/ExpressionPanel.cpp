#include "ExpressionPanel.h"
#include "ExpressionView.h"
#include "PowerVisual.h"
#include "FractionModel.h"
#include "RootModel.h"
#include "LogModel.h"
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
float ExpressionPanel::rowHeight(const VisualLine& row) const
{
    const auto points = fractionCaretPositions(row.text, font, 28);
    float low = 0.f, high = 30.f;
    for (const auto& p : points) {
        low = std::min(low, p.y);
        high = std::max(high, p.y + 30.f);
    }
    return std::max(lineHeight, high - low + 8.f);
}
float ExpressionPanel::rowTop(const VisualLine& row) const
{
    const auto points = fractionCaretPositions(row.text, font, 28);
    float low = 0.f;
    for (const auto& p : points) low = std::min(low, p.y);
    return std::max(0.f, -low + 4.f);
}
float ExpressionPanel::rowOffset(const std::vector<VisualLine>& rows, std::size_t index) const
{
    float y = 0.f;
    for (std::size_t i = 0; i < index && i < rows.size(); ++i)
        y += rowHeight(rows[i]);
    return y;
}
float ExpressionPanel::contentHeight(const std::vector<VisualLine>& rows) const
{
    return rowOffset(rows, rows.size());
}
std::size_t ExpressionPanel::rowAtY(const std::vector<VisualLine>& rows, float y) const
{
    if (rows.empty()) return 0;
    float top = 0.f;
    for (std::size_t i = 0; i + 1 < rows.size(); ++i) {
        top += rowHeight(rows[i]);
        if (y < top) return i;
    }
    return rows.size() - 1;
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
    const std::size_t index = rowAtY(rows,
        pos.y - box.getPosition().y - padding + scroll.offsetY);
    endOfPreviousLine = index + 1 < static_cast<int>(rows.size()) &&
        rows[index].end == cursor && rows[index+1].start == cursor;
}
float ExpressionPanel::cursorX(const VisualLine& row, std::size_t cursor) const
{
    std::string display = row.text;
    for (char& c : display) if (c == '<' || c == '>') c = '|';
    sf::Text probe(font, sf::String::fromUtf8(display.begin(), display.end()), 28);
    const auto len = row.end - row.start;
    if (row.text.find('^') != std::string::npos) {
        const auto visual=makePowerVisual(row.text,font,28);
        return visual.x[std::min(len,cursor>row.start?cursor-row.start:std::size_t{0})];
    }
    return probe.findCharacterPos(std::min(len, cursor > row.start ? cursor - row.start : std::size_t{0})).x;
}
std::size_t ExpressionPanel::cursorAtX(const VisualLine& row, float x) const
{
    std::string display = row.text;
    for (char& c : display) if (c == '<' || c == '>') c = '|';
    sf::Text probe(font, sf::String::fromUtf8(display.begin(), display.end()), 28);
    float best = 1e30f;
    std::size_t nearest = 0;
    for (std::size_t i = 0; i <= row.end - row.start; ++i)
    {
        const float px = row.text.find('^') != std::string::npos ? makePowerVisual(row.text,font,28).x[i] : probe.findCharacterPos(i).x;
        const float d = std::abs(px - x);
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

int ExpressionPanel::powerEndDepth(const std::string& expression, std::size_t cursor) const
{
    int count = 0;
    const auto rows = visualLines(expression);
    for (const auto& row : rows) {
        if (cursor < row.start || cursor > row.end) continue;
        const auto visual = makePowerVisual(row.text, font, 28);
        for (const auto& exponent : visual.exponents)
            if (row.start + exponent.end == cursor && exponent.end > exponent.start) ++count;
    }
    return count;
}

bool ExpressionPanel::isPowerEnd(const std::string& expression, std::size_t cursor) const
{
    const auto rows = visualLines(expression);
    for (const auto& row : rows) {
        if (cursor < row.start || cursor > row.end) continue;
        const auto visual = makePowerVisual(row.text, font, 28);
        for (const auto& exponent : visual.exponents)
            if (row.start + exponent.end == cursor && exponent.end > exponent.start)
                return true;
    }
    return false;
}

void ExpressionPanel::ensureCursorVisible(const std::string& expression, std::size_t cursor)
{
    const auto rows = visualLines(expression);
    const auto index = rowForCursor(rows, cursor);
    const float availableHeight = box.getSize().y - 2.f * padding;
    const float availableWidth = box.getSize().x - 2.f * padding;
    const float y = rowOffset(rows, index);
    const float x = cursorX(rows[index], cursor);
    if (y < scroll.offsetY) scroll.offsetY = y;
    if (y + rowHeight(rows[index]) > scroll.offsetY + availableHeight)
        scroll.offsetY = y + rowHeight(rows[index]) - availableHeight;
    if (x < scroll.offsetX) scroll.offsetX = x;
    if (x + 5.f > scroll.offsetX + availableWidth)
        scroll.offsetX = x + 5.f - availableWidth;
    float widest = 0.f;
    for (const auto& row : rows) widest = std::max(widest, getTextWidth(row.text, font, 28));
    setExpressionScroll(scroll, scroll.offsetX, scroll.offsetY,
        calculateMaxScrollX(widest, availableWidth), std::max(0.f, contentHeight(rows) - availableHeight));
}
float ExpressionPanel::maxY(std::size_t count) const
{
    return calculateMaxScrollY(count, lineHeight, box.getSize().y - 2.f * padding);
}
float ExpressionPanel::maxX(const std::vector<std::string>& wrapped) const
{
    return calculateMaxScrollX(calculateContentWidth(wrapped, font), box.getSize().x - 2.f * padding);
}
void ExpressionPanel::scrollToBottom(const std::string& expression)
{
    const auto wrapped = lines(expression);
    scroll.offsetY = std::max(0.f, contentHeight(visualLines(expression)) - (box.getSize().y - 2.f * padding));
    scroll.offsetX = 0.f;
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
            scrollExpressionWithWheel(scroll, wheel->delta, wheelSpeed, std::max(0.f, contentHeight(visualLines(expression)) - (box.getSize().y - 2.f * padding)));
    }
    if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (pressed->button == sf::Mouse::Button::Left)
        {
            const auto wrapped = lines(expression);
            beginExpressionScrollbarDrag(scroll, mousePos, {box.getPosition(), box.getSize()},
                scrollbarPadding, scrollbarWidth, horizontalHeight, std::max(0.f, contentHeight(visualLines(expression)) - (box.getSize().y - 2.f * padding)), maxX(wrapped));
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
                    {box.getPosition(), box.getSize()}, std::max(0.f, contentHeight(visualLines(expression)) - (box.getSize().y - 2.f * padding)));
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
    const auto rows = visualLines(expression);
    const float scrollY = std::max(0.f, contentHeight(rows) - (box.getSize().y - 2.f * padding));
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
    for (std::size_t i = 0; i < prepared.size() && i < rows.size(); ++i) {
        const float y = box.getPosition().y + padding +
            rowOffset(rows, i) + rowTop(rows[i]) - scroll.offsetY;
        if (y < box.getPosition().y + box.getSize().y &&
            y + rowHeight(rows[i]) > box.getPosition().y)
            drawExpressionLine(window, text, prepared[i].text,
                box.getPosition().x + padding - scroll.offsetX, y);
    }
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
    const std::size_t index = rowAtY(rows,
        pos.y - box.getPosition().y - padding + scroll.offsetY);
    const auto& row = rows[index];
    const float localX = pos.x - box.getPosition().x - padding + scroll.offsetX;
    const float localY = pos.y - box.getPosition().y - padding + scroll.offsetY -
        rowOffset(rows, index) - rowTop(row);
    // Graphical structures have their own two-dimensional caret positions.
    // The same geometry is already used to draw the editing caret.
    const auto structured = fractionCaretPositions(row.text, font, 28);
    if (!FractionModel::spans(row.text).empty() ||
        row.text.find(RootModel::emptyRoot().substr(0,3)) != std::string::npos ||
        row.text.find(LogModel::emptyLog().substr(0,3)) != std::string::npos)
    {
        const auto count = std::min(row.end - row.start + 1, structured.size());
        float best = 1e30f;
        std::size_t nearest = 0;
        for (std::size_t i = 0; i < count; ++i) {
            const float dx = structured[i].x - localX;
            const float dy = (structured[i].y + 12.f) - localY;
            const float distance = dx * dx + dy * dy;
            if (distance < best) { best = distance; nearest = i; }
        }
        return row.start + nearest;
    }
    if (row.text.find('^') == std::string::npos)
        return cursorAtX(row, localX);

    // Indeksy gorne sa na innej wysokosci niz podstawa. Wybieramy
    // najblizsza dwuwymiarowo pozycje edycji, a nie tylko najblizsze X.
    const auto visual = makePowerVisual(row.text, font, 28);
    const auto count = row.end - row.start;
    std::size_t nearest = 0;
    float best = 1e30f;
    for (std::size_t i = 0; i <= count; ++i) {
        const float dx = visual.x[i] - localX;
        const float dy = (14.f - visual.caretRise[i]) - localY;
        const float distance = dx * dx + dy * dy;
        if (distance < best) { best = distance; nearest = i; }
    }
    return row.start + nearest;
}

void ExpressionPanel::drawEditor(sf::RenderWindow& window, const std::string& expression,
    std::size_t cursor, std::size_t selectionStart, std::size_t selectionEnd, bool active, bool drawCaret)
{
    if (!active) return;
    const auto rows = visualLines(expression);
    const std::size_t cursorRow = rowForCursor(rows, cursor);
    const sf::View previous = window.getView();
    window.setView(createExpressionView(window, {box.getPosition(), box.getSize()}));
    for (std::size_t index = 0; index < rows.size(); ++index)
    {
        const auto& row = rows[index];
        const float y = box.getPosition().y + padding + rowOffset(rows, index) + rowTop(row) - scroll.offsetY;
        const float x = box.getPosition().x + padding - scroll.offsetX;
        if (selectionStart < row.end && selectionEnd > row.start)
        {
            const auto start = std::max(selectionStart, row.start) - row.start;
            const auto end = std::min(selectionEnd, row.end) - row.start;
            const bool structured = !FractionModel::spans(row.text).empty() ||
                row.text.find(RootModel::emptyRoot().substr(0,3)) != std::string::npos ||
                row.text.find(LogModel::emptyLog().substr(0,3)) != std::string::npos;
            if (structured) {
                const auto positions = fractionCaretPositions(row.text, font, 28);
                // Draw the selection per visible segment, rather than as one
                // horizontal rectangle across different fraction/root levels.
                // The visible "log" label has no ordinary text characters:
                // it is rendered from the LogModel::Begin marker. Highlight it
                // when the complete logarithm is selected, including its end.
                const sf::String chars = sf::String::fromUtf8(row.text.begin(), row.text.end());
                std::vector<std::size_t> logStack;
                for (std::size_t j = 0; j < chars.getSize(); ++j) {
                    if (chars[j] == LogModel::Begin) {
                        logStack.push_back(j);
                    } else if (chars[j] == LogModel::End && !logStack.empty()) {
                        const std::size_t begin = logStack.back();
                        logStack.pop_back();
                        if (begin < start || j >= end || begin + 1 >= positions.size())
                            continue;
                        const float width = positions[begin + 1].x - positions[begin].x;
                        if (width <= 0.f) continue;
                        sf::RectangleShape labelHighlight({width, 30.f});
                        labelHighlight.setPosition({x + positions[begin].x,
                            y + positions[begin].y});
                        labelHighlight.setFillColor(sf::Color(50, 110, 210, 130));
                        window.draw(labelHighlight);
                    }
                }
                for (std::size_t j = start; j < end && j + 1 < positions.size(); ++j) {
                    const auto a = positions[j];
                    const auto b = positions[j + 1];
                    const float dx = b.x - a.x;
                    const float dy = b.y - a.y;
                    if (dx <= 0.f || std::abs(dy) > 9.f) continue;
                    sf::RectangleShape highlight({dx, 25.f});
                    highlight.setPosition({x + a.x, y + a.y});
                    highlight.setFillColor(sf::Color(50, 110, 210, 130));
                    window.draw(highlight);
                }
            } else {
                const float a = cursorX(row, std::max(selectionStart, row.start));
                const float b = cursorX(row, std::min(selectionEnd, row.end));
                sf::RectangleShape highlight({std::max(0.f, b-a), 32.f});
                highlight.setPosition({x+a, y});
                highlight.setFillColor(sf::Color(50, 110, 210, 130));
                window.draw(highlight);
            }
        }
        // Edytowalne pole wykladnika: aktywne jest niebieskie,
        // a puste pozostaje widoczne jako miejsce do wpisywania.
        PowerVisual power = makePowerVisual(row.text, font, 28);
        for (const auto& exponent : power.exponents)
        {
            const bool empty = exponent.start == exponent.end;
            const std::size_t begin = row.start + exponent.start;
            const std::size_t end = row.start + exponent.end;
            const bool focused = index == cursorRow && cursor >= begin && cursor <= end && !(cursor == end && powerExitLevel > 0);
            // Puste pole pozostaje widoczne; wypelnione tylko podczas edycji.
            const float left = x + power.x[exponent.start] - 3.f;
            const float width = empty ? 18.f : std::max(18.f, power.x[exponent.end] - power.x[exponent.start] + 6.f);
            const float rise = exponent.level == 2 ? 22.f : 12.f;
            const float height = exponent.level == 2 ? 23.f : 30.f;
            if (!empty && !(focused && drawCaret)) continue;
            sf::RectangleShape frame({width, height});
            frame.setPosition({left, y - rise - 2.f});
            frame.setFillColor(sf::Color::Transparent);
            frame.setOutlineThickness(1.f);
            frame.setOutlineColor(sf::Color(185, 185, 185));
            window.draw(frame);
        }
        if (drawCaret && index == cursorRow)
        {
            const std::size_t local = std::min(cursor > row.start ? cursor-row.start : std::size_t{0}, power.caretRise.size()-1);
            sf::RectangleShape caretShape({2.f, power.caretRise[local] > 0.f && !(powerExitLevel > 0 && isPowerEnd(expression, cursor)) ? 22.f : 30.f});
            caretShape.setPosition({x + cursorX(row, cursor), y - ((powerExitLevel > 0 && isPowerEnd(expression, cursor)) ?
                (powerExitLevel >= powerEndDepth(expression, cursor) ? 0.f : 12.f) : power.caretRise[local])});
            if (!FractionModel::spans(row.text).empty() || row.text.find(RootModel::emptyRoot().substr(0,3)) != std::string::npos || row.text.find(LogModel::emptyLog().substr(0,3)) != std::string::npos) {
                const auto positions = fractionCaretPositions(row.text, font, 28);
                if (local < positions.size()) {
                    caretShape.setPosition({x + positions[local].x, y + positions[local].y});
                    caretShape.setSize({2.f, (RootModel::inDegree(row.text, local) || LogModel::inBase(row.text, local)) ? 15.f : 24.f});
                }
            }
            caretShape.setFillColor(sf::Color::White);
            window.draw(caretShape);
        }
    }
    window.setView(previous);
}

void ExpressionPanel::drawHighlights(sf::RenderWindow& window, const std::string& displayed,
    const std::vector<ExpressionHighlight>& highlights)
{
    if (highlights.empty()) return;
    const auto rows=visualLines(displayed);
    const sf::View previous=window.getView();
    window.setView(createExpressionView(window,{box.getPosition(),box.getSize()}));
    // Draw the blue underlay first, then the opaque green/red overlays.
    for(int layer=0;layer<2;++layer) for(const auto& h:highlights) {
        if(static_cast<int>(h.result || h.error)!=layer) continue;
        for(std::size_t i=0;i<rows.size();++i) {
            const auto& row=rows[i];
            if(h.start>=row.end || h.end<=row.start) continue;
            const auto a=std::max(row.start,h.start), b=std::min(row.end,h.end);
            const float x=box.getPosition().x+padding-scroll.offsetX;
            const float y=box.getPosition().y+padding+rowOffset(rows,i)+rowTop(row)-scroll.offsetY;
            const sf::Color color = h.error ? sf::Color(210,55,55,230) :
                h.result ? sf::Color(165,220,40,190) : sf::Color(45,110,220,115);
            const float inset = h.result ? 4.f : 0.f;
            // Superscripts are drawn above the baseline and at a smaller size.
            // Split the highlighted range at each change of visual level so
            // that a selection follows the glyphs instead of covering one
            // full-height rectangle across the entire expression.
            const bool hasStructures = row.text.find("\xEE\x84") != std::string::npos;
            // Structured layout owns horizontal positions when both graphical
            // structures and superscripts occur in the same expression.
            // A plain PowerVisual has no knowledge of fraction/root/log markers.
            if (!hasStructures && row.text.find('^') != std::string::npos) {
                const PowerVisual power = makePowerVisual(row.text,font,28);
                const std::size_t first = a - row.start;
                const std::size_t last = b - row.start;
                std::size_t begin = first;
                while (begin < last) {
                    const float rise = power.rise[begin];
                    const unsigned size = power.size[begin];
                    std::size_t end = begin + 1;
                    while (end < last && power.rise[end] == rise && power.size[end] == size)
                        ++end;
                    // '^' itself has zero width in PowerVisual, so ranges
                    // containing it cannot introduce an extra visible bar.
                    const float width = std::max(0.f,power.x[end]-power.x[begin]);
                    if (width > 0.f) {
                        const float height = rise == 0.f ? 32.f :
                            (rise >= 22.f ? 19.f : 24.f);
                        const float localInset = std::min(inset,height/4.f);
                        sf::RectangleShape mark({width,height-2.f*localInset});
                        mark.setPosition({x+power.x[begin],y-rise+localInset});
                        mark.setFillColor(color);
                        window.draw(mark);
                    }
                    begin = end;
                }
            } else if (hasStructures) {
                // Structured fractions, roots and logarithms have glyphs at
                // different heights. Use the same layout coordinates as the
                // text caret instead of a single full-height rectangle.
                const sf::String chars = sf::String::fromUtf8(row.text.begin(), row.text.end());
                const auto positions = fractionCaretPositions(row.text, font, 28);
                const auto fractions = FractionModel::spans(row.text);
                const bool hasPowers = row.text.find('^') != std::string::npos;
                const PowerVisual power = hasPowers ? makePowerVisual(row.text, font, 28) : PowerVisual{};
                const std::size_t first = a - row.start;
                const std::size_t last = std::min(b - row.start, chars.getSize());
                // In K mode a fully selected logarithm is one rectangular
                // highlight, including its label, argument and lowered base.
                // Partially selected logarithms keep the regular glyph marks.
                struct LogSelection { std::size_t begin, end; };
                std::vector<LogSelection> wholeLogs;
                std::vector<std::size_t> logBegins;
                for (std::size_t j = 0; j < chars.getSize(); ++j) {
                    if (chars[j] == LogModel::Begin) logBegins.push_back(j);
                    else if (chars[j] == LogModel::End && !logBegins.empty()) {
                        const std::size_t begin = logBegins.back();
                        logBegins.pop_back();
                        if (begin < first || j >= last || j + 1 >= positions.size()) continue;
                        float minX = positions[begin].x, maxX = positions[j+1].x;
                        float minY = 0.f, maxY = 32.f;
                        for (std::size_t k = begin; k <= j + 1; ++k) {
                            minX = std::min(minX, positions[k].x);
                            maxX = std::max(maxX, positions[k].x);
                            minY = std::min(minY, positions[k].y);
                            maxY = std::max(maxY, positions[k].y + 26.f);
                        }
                        if (maxX > minX) {
                            sf::RectangleShape whole({maxX-minX+3.f, maxY-minY+3.f});
                            whole.setPosition({x+minX-1.f,y+minY-1.f});
                            whole.setFillColor(color);
                            window.draw(whole);
                            wholeLogs.push_back({begin,j});
                        }
                    }
                }
                const auto coveredByWholeLog = [&](std::size_t pos) {
                    for (const auto& log : wholeLogs)
                        if (pos >= log.begin && pos <= log.end) return true;
                    return false;
                };
                // Include the non-text parts of a fully selected structure.
                // The editor stores these as invisible Unicode markers; their
                // visible symbols need their own highlight geometry.
                struct Structure { char32_t kind; std::size_t begin; std::size_t separator; };
                std::vector<Structure> openStructures;
                for (std::size_t j = 0; j < chars.getSize(); ++j) {
                    const char32_t ch = chars[j];
                    if (ch == FractionModel::Begin || ch == RootModel::Begin || ch == LogModel::Begin) {
                        openStructures.push_back({ch, j, chars.getSize()});
                        continue;
                    }
                    if (openStructures.empty()) continue;
                    auto& current = openStructures.back();
                    const char32_t separator = current.kind == FractionModel::Begin ? FractionModel::Separator :
                        current.kind == RootModel::Begin ? RootModel::Separator : LogModel::Separator;
                    const char32_t ending = current.kind == FractionModel::Begin ? FractionModel::End :
                        current.kind == RootModel::Begin ? RootModel::End : LogModel::End;
                    // A nested structure owns its own separator. Never let a
                    // separator or closing marker from another level change
                    // the boundaries of the current (innermost) structure.
                    if (ch == separator && current.separator == chars.getSize())
                        current.separator = j;
                    if (ch != ending) continue;
                    const auto structure = current;
                    openStructures.pop_back();
                    if (structure.kind == LogModel::Begin && coveredByWholeLog(structure.begin)) continue;
                    if (structure.begin < first || j >= last ||
                        structure.separator >= chars.getSize() ||
                        structure.separator <= structure.begin ||
                        structure.separator >= j ||
                        structure.begin + 1 >= positions.size() ||
                        structure.separator + 1 >= positions.size() ||
                        j + 1 >= positions.size()) continue;
                    const float left = positions[structure.begin].x;
                    const float right = positions[j+1].x;
                    float markLeft = left, markTop = 0.f, markWidth = 0.f, markHeight = 0.f;
                    if (structure.kind == FractionModel::Begin) {
                        // Fraction bar is drawn at baseline - size * 0.2.
                        markTop = 26.f - 28.f * 0.2f;
                        markWidth = right - left;
                        markHeight = 3.f;
                    } else if (structure.kind == LogModel::Begin) {
                        // The label 'log' precedes the small base field.
                        markWidth = positions[structure.begin+1].x - left;
                        markTop = positions[structure.begin].y;
                        markHeight = 30.f;
                    } else {
                        // Radical hook and overbar occupy the region before
                        // the radicand; highlight the symbol, not empty space.
                        markWidth = positions[structure.separator+1].x - left;
                        markTop = positions[structure.separator+1].y - 8.f;
                        markHeight = 34.f;
                    }
                    if (markWidth <= 0.f) continue;
                    const float localInset = std::min(inset, markHeight / 4.f);
                    sf::RectangleShape symbolMark({markWidth, markHeight - 2.f * localInset});
                    symbolMark.setPosition({x + markLeft, y + markTop + localInset});
                    symbolMark.setFillColor(color);
                    window.draw(symbolMark);
                }
                for (std::size_t j = first; j < last && j + 1 < positions.size(); ++j) {
                    if (coveredByWholeLog(j)) continue;
                    const char32_t ch = chars[j];
                    const bool structural = ch == FractionModel::Begin || ch == FractionModel::Separator ||
                        ch == FractionModel::End || ch == RootModel::Begin ||
                        ch == RootModel::Separator || ch == RootModel::End ||
                        ch == LogModel::Begin || ch == LogModel::Separator || ch == LogModel::End;
                    if (structural) continue;
                    unsigned glyphSize = 28;
                    for (const auto& span : fractions)
                        if ((j > span.begin && j < span.separator) ||
                            (j > span.separator && j < span.end))
                            glyphSize = std::max(12u, 28u * 3 / 4);
                    if (RootModel::inDegree(row.text, j) || LogModel::inBase(row.text, j))
                        glyphSize = std::max(12u, 28u * 55 / 100);
                    // In mixed expressions, preserve the structured caret's X/Y
                    // while following the superscript's smaller glyph height.
                    // '^' has no visible glyph and must never receive a mark.
                    if (hasPowers) {
                        if (ch == U'^') continue;
                        glyphSize = std::min(glyphSize, power.size[j]);
                    }
                    const float advance = font.getGlyph(ch == U'<' || ch == U'>' ? U'|' : ch,
                        glyphSize, false).advance;
                    const float width = std::min(advance,
                        std::max(0.f, positions[j+1].x - positions[j].x));
                    if (width <= 0.f) continue;
                    const float height = static_cast<float>(glyphSize) + 4.f;
                    const float localInset = std::min(inset, height / 4.f);
                    sf::RectangleShape mark({width, height - 2.f * localInset});
                    const float exponentRise = hasPowers ? power.rise[j] : 0.f;
                    mark.setPosition({x + positions[j].x,
                        y + positions[j].y - exponentRise + localInset});
                    mark.setFillColor(color);
                    window.draw(mark);
                }
            } else {
                const float x1=cursorX(row,a),x2=cursorX(row,b);
                sf::RectangleShape mark({std::max(0.f,x2-x1),32.f-2.f*inset});
                mark.setPosition({x+x1,y+inset});
                mark.setFillColor(color);
                window.draw(mark);
            }
        }
    }
    window.setView(previous);
}
