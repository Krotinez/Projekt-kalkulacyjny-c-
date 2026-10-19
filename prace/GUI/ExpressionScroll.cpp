#include "ExpressionScroll.h"

#include <algorithm>

void setExpressionScroll(
    ExpressionScroll& scroll,
    float x,
    float y,
    float maxX,
    float maxY
)
{
    scroll.offsetX = std::clamp(
        x,
        0.f,
        std::max(0.f, maxX)
    );

    scroll.offsetY = std::clamp(
        y,
        0.f,
        std::max(0.f, maxY)
    );
}

float calculateMaxScrollY(
    std::size_t lineCount,
    float lineHeight,
    float visibleHeight
)
{
    float contentHeight =
        static_cast<float>(lineCount) * lineHeight;

    return std::max(
        0.f,
        contentHeight - visibleHeight
    );
}

void drawVerticalScrollbar(
    sf::RenderWindow& window,
    float x,
    float y,
    float width,
    float height,
    float offsetY,
    float maxScrollY
)
{
    // Suwak nie jest potrzebny, jeśli wszystko się mieści.
    if (maxScrollY <= 0.f)
        return;

    // Tło suwaka.
    sf::RectangleShape track({width, height});
    track.setPosition({x, y});
    track.setFillColor(sf::Color(65, 65, 65));

    // Długość uchwytu zależy od ilości zawartości.
    float thumbHeight =
        height * height / (height + maxScrollY);

    thumbHeight = std::clamp(
        thumbHeight,
        20.f,
        height
    );

    // Pozycja uchwytu zależy od przewinięcia.
    float thumbY = y +
        (offsetY / maxScrollY) * (height - thumbHeight);

    sf::RectangleShape thumb({width, thumbHeight});
    thumb.setPosition({x, thumbY});
    thumb.setFillColor(sf::Color(160, 160, 160));

    window.draw(track);
    window.draw(thumb);
}

bool isVerticalScrollbarClicked(
    sf::Vector2f mousePosition,
    float x,
    float y,
    float width,
    float height,
    float offsetY,
    float maxScrollY
)
{
    if (maxScrollY <= 0.f)
        return false;

    // Obliczamy wysokość uchwytu.
    float thumbHeight =
        height * height / (height + maxScrollY);

    thumbHeight = std::clamp(
        thumbHeight,
        20.f,
        height
    );

    // Obliczamy pozycję uchwytu.
    float thumbY = y +
        (offsetY / maxScrollY) * (height - thumbHeight);

    // Sprawdzamy, czy mysz znajduje się na uchwycie.
    return mousePosition.x >= x &&
           mousePosition.x <= x + width &&
           mousePosition.y >= thumbY &&
           mousePosition.y <= thumbY + thumbHeight;
}

void beginVerticalScrollbarDrag(
    ExpressionScroll& scroll,
    sf::Vector2f mousePosition,
    float x,
    float y,
    float width,
    float height,
    float maxScrollY
)
{
    // Sprawdzamy, czy kliknięto uchwyt suwaka.
    if (!isVerticalScrollbarClicked(
        mousePosition,
        x,
        y,
        width,
        height,
        scroll.offsetY,
        maxScrollY
    ))
    {
        return;
    }

    // Obliczamy wysokość uchwytu.
    float thumbHeight =
        height * height / (height + maxScrollY);

    thumbHeight = std::clamp(
        thumbHeight,
        20.f,
        height
    );

    // Obliczamy aktualną pozycję uchwytu.
    float thumbY = y +
        (scroll.offsetY / maxScrollY) *
        (height - thumbHeight);

    // Rozpoczynamy przeciąganie.
    scroll.draggingY = true;

    // Zapamiętujemy miejsce kliknięcia na uchwycie.
    scroll.dragOffsetY = mousePosition.y - thumbY;
}

void updateVerticalScrollbarDrag(
    ExpressionScroll& scroll,
    float mouseY,
    float trackY,
    float trackHeight,
    float maxScrollY
)
{
    // Aktualizujemy pozycję tylko podczas przeciągania.
    if (!scroll.draggingY)
        return;

    if (maxScrollY <= 0.f || trackHeight <= 0.f)
    {
        scroll.offsetY = 0.f;
        return;
    }

    // Wysokość uchwytu suwaka.
    float thumbHeight =
        trackHeight * trackHeight /
        (trackHeight + maxScrollY);

    thumbHeight = std::clamp(
        thumbHeight,
        20.f,
        trackHeight
    );

    // Maksymalna odległość ruchu uchwytu.
    float maxThumbMovement = trackHeight - thumbHeight;

    if (maxThumbMovement <= 0.f)
        return;

    // Pozycja uchwytu względem początku prowadnicy.
    float thumbPosition =
        mouseY - trackY - scroll.dragOffsetY;

    thumbPosition = std::clamp(
        thumbPosition,
        0.f,
        maxThumbMovement
    );

    // Zamiana pozycji uchwytu na przewinięcie zawartości.
    float newOffsetY =
        (thumbPosition / maxThumbMovement) * maxScrollY;

    setExpressionScroll(
        scroll,
        scroll.offsetX,
        newOffsetY,
        scroll.offsetX,
        maxScrollY
    );
}

void endVerticalScrollbarDrag(
    ExpressionScroll& scroll
)
{
    scroll.draggingY = false;
    scroll.dragOffsetY = 0.f;
}

float calculateMaxScrollX(
    float contentWidth,
    float visibleWidth
)
{
    return std::max(
        0.f,
        contentWidth - visibleWidth
    );
}

void drawHorizontalScrollbar(
    sf::RenderWindow& window,
    float x,
    float y,
    float width,
    float height,
    float offsetX,
    float maxScrollX
)
{
    // Jeśli cała zawartość się mieści, suwak jest zbędny.
    if (maxScrollX <= 0.f)
        return;

    // Tło poziomego suwaka.
    sf::RectangleShape track({width, height});
    track.setPosition({x, y});
    track.setFillColor(sf::Color(65, 65, 65));

    // Szerokość uchwytu.
    float thumbWidth =
        width * width / (width + maxScrollX);

    thumbWidth = std::clamp(
        thumbWidth,
        20.f,
        width
    );

    // Pozycja uchwytu.
    float thumbX = x +
        (offsetX / maxScrollX) * (width - thumbWidth);

    // Uchwyt poziomego suwaka.
    sf::RectangleShape thumb({thumbWidth, height});
    thumb.setPosition({thumbX, y});
    thumb.setFillColor(sf::Color(160, 160, 160));

    window.draw(track);
    window.draw(thumb);
}

bool isHorizontalScrollbarClicked(
    sf::Vector2f mousePosition,
    float x,
    float y,
    float width,
    float height,
    float offsetX,
    float maxScrollX
)
{
    if (maxScrollX <= 0.f || width <= 0.f)
        return false;

    // Obliczamy szerokość uchwytu.
    float thumbWidth =
        width * width / (width + maxScrollX);

    thumbWidth = std::clamp(
        thumbWidth,
        20.f,
        width
    );

    // Obliczamy położenie uchwytu.
    float thumbX = x +
        (offsetX / maxScrollX) * (width - thumbWidth);

    // Sprawdzamy, czy kliknięto uchwyt.
    return mousePosition.x >= thumbX &&
           mousePosition.x <= thumbX + thumbWidth &&
           mousePosition.y >= y &&
           mousePosition.y <= y + height;
}

void beginHorizontalScrollbarDrag(
    ExpressionScroll& scroll,
    sf::Vector2f mousePosition,
    float x,
    float y,
    float width,
    float height,
    float maxScrollX
)
{
    // Sprawdzamy, czy kliknięto uchwyt.
    if (!isHorizontalScrollbarClicked(
        mousePosition,
        x,
        y,
        width,
        height,
        scroll.offsetX,
        maxScrollX
    ))
    {
        return;
    }

    // Obliczamy szerokość uchwytu.
    float thumbWidth =
        width * width / (width + maxScrollX);

    thumbWidth = std::clamp(
        thumbWidth,
        20.f,
        width
    );

    // Obliczamy aktualną pozycję uchwytu.
    float thumbX = x +
        (scroll.offsetX / maxScrollX) *
        (width - thumbWidth);

    // Rozpoczynamy przeciąganie.
    scroll.draggingX = true;

    // Zapamiętujemy miejsce kliknięcia.
    scroll.dragOffsetX = mousePosition.x - thumbX;
}

void updateHorizontalScrollbarDrag(
    ExpressionScroll& scroll,
    float mouseX,
    float trackX,
    float trackWidth,
    float maxScrollX
)
{
    // Aktualizujemy pozycję tylko podczas przeciągania.
    if (!scroll.draggingX)
        return;

    if (maxScrollX <= 0.f || trackWidth <= 0.f)
    {
        scroll.offsetX = 0.f;
        return;
    }

    // Szerokość uchwytu.
    float thumbWidth =
        trackWidth * trackWidth /
        (trackWidth + maxScrollX);

    thumbWidth = std::clamp(
        thumbWidth,
        20.f,
        trackWidth
    );

    // Maksymalna odległość ruchu uchwytu.
    float maxThumbMovement = trackWidth - thumbWidth;

    if (maxThumbMovement <= 0.f)
        return;

    // Pozycja uchwytu względem początku prowadnicy.
    float thumbPosition =
        mouseX - trackX - scroll.dragOffsetX;

    thumbPosition = std::clamp(
        thumbPosition,
        0.f,
        maxThumbMovement
    );

    // Zamieniamy pozycję uchwytu na przewinięcie zawartości.
    float newOffsetX =
        (thumbPosition / maxThumbMovement) * maxScrollX;

    setExpressionScroll(
        scroll,
        newOffsetX,
        scroll.offsetY,
        maxScrollX,
        scroll.offsetY
    );
}

void endHorizontalScrollbarDrag(
    ExpressionScroll& scroll
)
{
    scroll.draggingX = false;
    scroll.dragOffsetX = 0.f;
}

void scrollExpressionWithWheel(
    ExpressionScroll& scroll,
    float wheelDelta,
    float scrollSpeed,
    float maxScrollY
)
{
    setExpressionScroll(
        scroll,
        scroll.offsetX,
        scroll.offsetY - wheelDelta * scrollSpeed,
        scroll.offsetX,
        maxScrollY
    );
}

float calculateContentWidth(
    const std::vector<std::string>& lines,
    const sf::Font& font
)
{
    float contentWidth = 0.f;

    sf::Text measuredText(font);
    measuredText.setCharacterSize(28);

    for (const auto& line : lines)
    {
        measuredText.setString(line);

        contentWidth = std::max(
            contentWidth,
            measuredText.getLocalBounds().size.x
        );
    }

    return contentWidth;
}

void stopExpressionScrollbarDrag(ExpressionScroll& scroll)
{
    endVerticalScrollbarDrag(scroll);
    endHorizontalScrollbarDrag(scroll);
}

void beginExpressionScrollbarDrag(
    ExpressionScroll& scroll,
    sf::Vector2f mousePos,
    sf::FloatRect expressionArea,
    float scrollbarPadding,
    float scrollbarWidth,
    float horizontalScrollbarHeight,
    float maxScrollY,
    float maxScrollX
)
{
    beginVerticalScrollbarDrag(
        scroll,
        mousePos,
        expressionArea.position.x + expressionArea.size.x + scrollbarPadding,
        expressionArea.position.y,
        scrollbarWidth,
        expressionArea.size.y,
        maxScrollY
    );

    beginHorizontalScrollbarDrag(
        scroll,
        mousePos,
        expressionArea.position.x,
        expressionArea.position.y + expressionArea.size.y + scrollbarPadding,
        expressionArea.size.x,
        horizontalScrollbarHeight,
        maxScrollX
    );
}

void updateExpressionVerticalDrag(
    ExpressionScroll& scroll,
    float mouseY,
    sf::FloatRect expressionArea,
    float maxScrollY
)
{
    if (!scroll.draggingY)
        return;

    updateVerticalScrollbarDrag(
        scroll,
        mouseY,
        expressionArea.position.y,
        expressionArea.size.y,
        maxScrollY
    );
}
