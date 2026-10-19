#include "ExpressionView.h"

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

