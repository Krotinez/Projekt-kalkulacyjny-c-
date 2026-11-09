#include "ExpressionView.h"
#include "PowerVisual.h"
#include "FractionModel.h"
#include "FractionVisual.h"
#include "RootModel.h"
#include "RootVisual.h"
#include "LogModel.h"
#include "LogVisual.h"
#include <algorithm>

namespace {
char32_t visibleBracket(char32_t c) {
    switch(c) {
        case U'\uE130': return U')'; case U'\uE131': return U']';
        case U'\uE132': return U'}'; case U'\uE133': return U'|';
        case U'<': case U'>': return U'|'; default: return c;
    }
}
bool isGhost(char32_t c) { return c>=U'\uE130' && c<=U'\uE133'; }
}
namespace {
// Pierwszy etap rysowania: ulamki maja osobne obszary i pozioma kreske.
// Obsluguje rowniez ulamki zagniezdzone do dwoch poziomow.
struct Piece {
    float width = 0.f;
    float above = 0.f;
    float below = 0.f;
};
Piece measure(const sf::String& chars, std::size_t& at, const sf::Font& font,
              unsigned size, int depth, bool stopAtMarker) {
    Piece result;
    while (at < chars.getSize()) {
        const char32_t ch = chars[at];
        if (stopAtMarker && (ch == FractionModel::Separator || ch == FractionModel::End || ch == RootModel::Separator || ch == RootModel::End || ch == LogModel::Separator || ch == LogModel::End)) break;
        if (ch == LogModel::Begin) {
            ++at;
            const unsigned small = std::max(12u, size * 55 / 100);
            const auto base = measure(chars, at, font, small, depth, true);
            if (at < chars.getSize() && chars[at] == LogModel::Separator) ++at;
            const auto argument = measure(chars, at, font, size, depth, true);
            if (at < chars.getSize() && chars[at] == LogModel::End) ++at;
            const auto geometry = layoutLog(font, size, base.width, argument.width, argument.above + argument.below);
            result.width += geometry.width;
            result.above = std::max(result.above, geometry.height);
            result.below = std::max(result.below, size * .25f);
        } else if (ch == RootModel::Begin) {
            ++at;
            const unsigned small = std::max(12u, size * 55 / 100);
            const auto index = measure(chars, at, font, small, depth, true);
            if (at < chars.getSize() && chars[at] == RootModel::Separator) ++at;
            const auto radicand = measure(chars, at, font, size, depth, true);
            if (at < chars.getSize() && chars[at] == RootModel::End) ++at;
            const auto geometry = layoutRoot(radicand.width, size + radicand.above + radicand.below,
                                             index.width, small);
            result.width += geometry.width;
            result.above = std::max(result.above, geometry.height);
            result.below = std::max(result.below, size * .25f);
        } else if (ch == FractionModel::Begin && depth < 2) {
            ++at;
            const auto top = measure(chars, at, font, std::max(12u,size*2/3), depth+1, true);
            if (at < chars.getSize() && chars[at] == FractionModel::Separator) ++at;
            const auto bottom = measure(chars, at, font, std::max(12u,size*2/3), depth+1, true);
            if (at < chars.getSize() && chars[at] == FractionModel::End) ++at;
            const auto geom = layoutFraction(top.width, bottom.width, static_cast<float>(size), 6.f);
            result.width += geom.width;
            result.above = std::max(result.above, top.above + top.below + 3.f + size*0.35f);
            result.below = std::max(result.below, bottom.above + bottom.below + 3.f + size*0.35f);
        } else {
            result.width += font.getGlyph(ch==U'<' || ch==U'>' ? U'|' : ch, size, false).advance;
            result.above = std::max(result.above, size*0.8f);
            result.below = std::max(result.below, size*0.25f);
            ++at;
        }
    }
    return result;
}
void paint(sf::RenderWindow& window, const sf::String& chars, std::size_t& at,
           const sf::Font& font, unsigned size, int depth, float& x, float baseline,
           sf::Color color, bool stopAtMarker) {
    while (at < chars.getSize()) {
        const char32_t ch = chars[at];
        if (stopAtMarker && (ch == FractionModel::Separator || ch == FractionModel::End || ch == RootModel::Separator || ch == RootModel::End || ch == LogModel::Separator || ch == LogModel::End)) break;
        if (ch == LogModel::Begin) {
            const std::size_t baseStart = ++at;
            const unsigned small = std::max(12u, size * 55 / 100);
            auto probe = at;
            const auto base = measure(chars, probe, font, small, depth, true);
            const std::size_t separator = probe;
            if (probe < chars.getSize() && chars[probe] == LogModel::Separator) ++probe;
            const auto argument = measure(chars, probe, font, size, depth, true);
            const std::size_t finish = probe;
            const auto geometry = layoutLog(font, size, base.width, argument.width, argument.above + argument.below);
            const float top = baseline - size * .8f;
            drawLogLabel(window, font, size, {x, top}, geometry, color);
            float baseX = x + geometry.basePosition.x;
            at = baseStart;
            paint(window, chars, at, font, small, depth, baseX,
                  top + geometry.basePosition.y + small * .8f, color, true);
            float argumentX = x + geometry.argumentPosition.x;
            at = separator;
            if (at < chars.getSize() && chars[at] == LogModel::Separator) ++at;
            paint(window, chars, at, font, size, depth, argumentX,
                  baseline, color, true);
            at = finish;
            if (at < chars.getSize() && chars[at] == LogModel::End) ++at;
            x += geometry.width;
        } else if (ch == RootModel::Begin) {
            const std::size_t indexStart = ++at;
            const unsigned small = std::max(12u, size * 55 / 100);
            auto probe = at;
            const auto index = measure(chars, probe, font, small, depth, true);
            const std::size_t separator = probe;
            if (probe < chars.getSize() && chars[probe] == RootModel::Separator) ++probe;
            const auto radicand = measure(chars, probe, font, size, depth, true);
            const std::size_t finish = probe;
            const auto geometry = layoutRoot(radicand.width, size + radicand.above + radicand.below,
                                             index.width, small);
            const float top = baseline - size * .8f - geometry.radicandY;
            drawRootSymbol(window, {x, top}, geometry, color);
            float indexX = x + geometry.indexX;
            at = indexStart;
            paint(window, chars, at, font, small, depth, indexX,
                  top + geometry.indexY + small * .8f, color, true);
            float innerX = x + geometry.radicandX;
            at = separator;
            if (at < chars.getSize() && chars[at] == RootModel::Separator) ++at;
            paint(window, chars, at, font, size, depth, innerX,
                  top + geometry.radicandY + size * .8f, color, true);
            at = finish;
            if (at < chars.getSize() && chars[at] == RootModel::End) ++at;
            x += geometry.width;
        } else if (ch == FractionModel::Begin && depth < 2) {
            const std::size_t start = ++at;
            auto probe = at;
            const unsigned smaller = std::max(12u,size*2/3);
            const auto top = measure(chars, probe, font, smaller, depth+1, true);
            const std::size_t separator = probe;
            if (probe < chars.getSize() && chars[probe] == FractionModel::Separator) ++probe;
            const auto bottom = measure(chars, probe, font, smaller, depth+1, true);
            const std::size_t finish = probe;
            const auto geom = layoutFraction(top.width, bottom.width, static_cast<float>(size), 6.f);
            const float barY = baseline - size*0.2f;
            sf::RectangleShape bar({geom.width,2.f});
            bar.setFillColor(color); bar.setPosition({x,barY}); window.draw(bar);
            float topX = x + (geom.width-top.width)/2.f;
            at = start;
            paint(window, chars, at, font, smaller, depth+1, topX,
                  barY - 3.f - top.below, color, true);
            float bottomX = x + (geom.width-bottom.width)/2.f;
            at = separator;
            if (at < chars.getSize() && chars[at] == FractionModel::Separator) ++at;
            paint(window, chars, at, font, smaller, depth+1, bottomX,
                  barY + 3.f + bottom.above, color, true);
            at = finish;
            if (at < chars.getSize() && chars[at] == FractionModel::End) ++at;
            x += geom.width;
        } else {
            sf::Text glyph(font, sf::String(visibleBracket(ch)), size);
            glyph.setFillColor(isGhost(ch) ? sf::Color(145,145,145,190) : color);
            glyph.setPosition({x, baseline - size*0.8f});
            window.draw(glyph);
            x += font.getGlyph(ch==U'<' || ch==U'>' ? U'|' : ch,size,false).advance;
            ++at;
        }
    }
}
}


namespace {
void locateCaret(const sf::String& chars, std::size_t& at, const sf::Font& font,
                 unsigned size, int depth, float& x, float baseline,
                 bool stopAtMarker, std::vector<sf::Vector2f>& positions) {
    while (at < chars.getSize()) {
        const char32_t ch = chars[at];
        if (stopAtMarker && (ch == FractionModel::Separator || ch == FractionModel::End || ch == RootModel::Separator || ch == RootModel::End || ch == LogModel::Separator || ch == LogModel::End)) break;
        positions[at] = {x, baseline - size * 0.8f};
        if (ch == LogModel::Begin) {
            const std::size_t baseStart = ++at;
            const unsigned small = std::max(12u, size * 55 / 100);
            auto probe = at;
            const auto base = measure(chars, probe, font, small, depth, true);
            const std::size_t separator = probe;
            if (probe < chars.getSize() && chars[probe] == LogModel::Separator) ++probe;
            const auto argument = measure(chars, probe, font, size, depth, true);
            const std::size_t finish = probe;
            const auto geometry = layoutLog(font, size, base.width, argument.width, argument.above + argument.below);
            const float top = baseline - size * .8f;
            float baseX = x + geometry.basePosition.x;
            at = baseStart;
            positions[at] = {baseX, top + geometry.basePosition.y};
            locateCaret(chars, at, font, small, depth, baseX,
                        top + geometry.basePosition.y + small * .8f, true, positions);
            positions[separator] = {baseX, top + geometry.basePosition.y};
            float argumentX = x + geometry.argumentPosition.x;
            at = separator;
            if (at < chars.getSize() && chars[at] == LogModel::Separator) ++at;
            positions[at] = {argumentX, top};
            locateCaret(chars, at, font, size, depth, argumentX, baseline, true, positions);
            positions[finish] = {argumentX, top};
            at = finish;
            if (at < chars.getSize() && chars[at] == LogModel::End) ++at;
            x += geometry.width;
            positions[at] = {x, top};
        } else if (ch == RootModel::Begin) {
            const std::size_t indexStart = ++at;
            const unsigned small = std::max(12u, size * 55 / 100);
            auto probe = at;
            const auto index = measure(chars, probe, font, small, depth, true);
            const std::size_t separator = probe;
            if (probe < chars.getSize() && chars[probe] == RootModel::Separator) ++probe;
            const auto radicand = measure(chars, probe, font, size, depth, true);
            const std::size_t finish = probe;
            const auto geometry = layoutRoot(radicand.width, size + radicand.above + radicand.below,
                                             index.width, small);
            const float top = baseline - size * .8f - geometry.radicandY;
            float indexX = x + geometry.indexX;
            at = indexStart;
            positions[at] = {indexX, top + geometry.indexY};
            locateCaret(chars, at, font, small, depth, indexX,
                        top + geometry.indexY + small * .8f, true, positions);
            positions[separator] = {indexX, top + geometry.indexY};
            float innerX = x + geometry.radicandX;
            at = separator;
            if (at < chars.getSize() && chars[at] == RootModel::Separator) ++at;
            positions[at] = {innerX, top + geometry.radicandY};
            locateCaret(chars, at, font, size, depth, innerX,
                        top + geometry.radicandY + size * .8f, true, positions);
            positions[finish] = {innerX, top + geometry.radicandY};
            at = finish;
            if (at < chars.getSize() && chars[at] == RootModel::End) ++at;
            x += geometry.width;
            positions[at] = {x, baseline - size * .8f};
        } else if (ch == FractionModel::Begin && depth < 2) {
            const std::size_t begin = ++at;
            auto probe = at;
            const unsigned smaller = std::max(12u, size * 2 / 3);
            const auto top = measure(chars, probe, font, smaller, depth + 1, true);
            const std::size_t separator = probe;
            if (probe < chars.getSize() && chars[probe] == FractionModel::Separator) ++probe;
            const auto bottom = measure(chars, probe, font, smaller, depth + 1, true);
            const std::size_t finish = probe;
            const auto geom = layoutFraction(top.width, bottom.width, static_cast<float>(size), 6.f);
            const float barY = baseline - size * 0.2f;
            float topX = x + (geom.width - top.width) / 2.f;
            at = begin;
            locateCaret(chars, at, font, smaller, depth + 1, topX,
                        barY - 3.f - top.below, true, positions);
            // Pozycja przed separatorem jest koncem licznika.
            positions[separator] = {topX, barY - 3.f - top.below - smaller * 0.8f};
            float bottomX = x + (geom.width - bottom.width) / 2.f;
            at = separator;
            if (at < chars.getSize() && chars[at] == FractionModel::Separator) ++at;
            positions[at] = {bottomX, barY + 3.f + bottom.above - smaller * 0.8f};
            locateCaret(chars, at, font, smaller, depth + 1, bottomX,
                        barY + 3.f + bottom.above, true, positions);
            positions[finish] = {bottomX, barY + 3.f + bottom.above - smaller * 0.8f};
            at = finish;
            if (at < chars.getSize() && chars[at] == FractionModel::End) ++at;
            x += geom.width;
            positions[at] = {x, baseline - size * 0.8f};
        } else {
            x += font.getGlyph(ch==U'<' || ch==U'>' ? U'|' : ch, size, false).advance;
            ++at;
            positions[at] = {x, baseline - size * 0.8f};
        }
    }
}
}

std::vector<sf::Vector2f> fractionCaretPositions(
    const std::string& line, const sf::Font& font, unsigned size) {
    const auto chars = sf::String::fromUtf8(line.begin(), line.end());
    std::vector<sf::Vector2f> positions(chars.getSize() + 1, {0.f, 26.f - size * 0.8f});
    std::size_t at = 0;
    float x = 0.f;
    locateCaret(chars, at, font, size, 0, x, 26.f, false, positions);
    return positions;
}

void drawExpressionLine(
    sf::RenderWindow& window,
    sf::Text& expressionText,
    const std::string& line,
    float x,
    float y
)
{
    std::string displayLine = line;

    // Zamiana wewnętrznych symboli wartości bezwzględnej.
    for (char& c : displayLine)
    {
        if (c == '<' || c == '>')
            c = '|';
    }

    // Przygotowanie tekstu do wyświetlenia.
    expressionText.setString(
        sf::String::fromUtf8(
            displayLine.begin(),
            displayLine.end()
        )
    );

    // Ustawienie pozycji tekstu.
    expressionText.setPosition({x, y});

    // Narysowanie tekstu.
    const auto fractionChars = sf::String::fromUtf8(line.begin(), line.end());
    bool containsFraction = false;
    for (std::size_t i=0; i<fractionChars.getSize(); ++i)
        if (isGhost(fractionChars[i]) || fractionChars[i] == FractionModel::Begin || fractionChars[i] == RootModel::Begin || fractionChars[i] == LogModel::Begin) { containsFraction = true; break; }
    if (containsFraction && line.find('^') == std::string::npos) {
        std::size_t at=0;
        float drawX=x;
        paint(window,fractionChars,at,expressionText.getFont(),
              expressionText.getCharacterSize(),0,drawX,y+26.f,
              expressionText.getFillColor(),false);
    }
    else if (line.find('^') != std::string::npos)
        drawPowerVisual(window, displayLine, expressionText.getFont(), expressionText.getCharacterSize(), {x,y}, expressionText.getFillColor());
    else
        window.draw(expressionText);
}

void drawExpressionLines(
    sf::RenderWindow& window,
    sf::Text& expressionText,
    const std::vector<ExpressionLine>& preparedLines,
    float startX,
    float startY,
    float lineHeight,
    float maxY,
    const ExpressionScroll& scroll
)
{
    for (std::size_t i = 0; i < preparedLines.size(); ++i)
    {
        float y = startY
                + static_cast<float>(i) * lineHeight
                - scroll.offsetY;
      
        if (y + lineHeight <= startY - 20.f)
        {
            continue;
        }

        if (y >= maxY)
        {
            continue;
        }

        drawExpressionLine(
            window,
            expressionText,
            preparedLines[i].text,
            startX - scroll.offsetX,
            y
        );
    }
}

sf::View createExpressionView(
    const sf::RenderWindow& window,
    const sf::FloatRect& area
)
{
    sf::View view = window.getDefaultView();

    const sf::Vector2u windowSize = window.getSize();

    // Przeliczenie położenia pola na współrzędne
    // względne okna (od 0 do 1).
    sf::FloatRect viewport(
        {
            area.position.x / windowSize.x,
            area.position.y / windowSize.y
        },
        {
            area.size.x / windowSize.x,
            area.size.y / windowSize.y
        }
    );

    view.setViewport(viewport);

    // Ustawienie widoku na obszar pola.
    view.setSize(area.size);
    view.setCenter({
        area.position.x + area.size.x / 2.f,
        area.position.y + area.size.y / 2.f
    });

    return view;
}

