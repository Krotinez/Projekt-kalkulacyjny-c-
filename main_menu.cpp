#include <SFML/Graphics.hpp>
#include "GUI/Button.h"

void runArytmetyka(sf::RenderWindow& window, sf::Font& font, sf::Font& mathFont);
void algebra(sf::RenderWindow& window);
void geometria(sf::RenderWindow& window);

void mainMenu(sf::RenderWindow& window)
{
    sf::Font font;
    if (!font.openFromFile("fonts/DejaVuSans.ttf"))
    {
        window.close();
    }

    sf::Font mathFont;
    if (!mathFont.openFromFile("fonts/DejaVuSans.ttf")) // LUB podaj ścieżkę do czcionki matematycznej, np. "fonts/arial.ttf"
    {
        window.close();
    }

    std::string titleText = "Wybierz dzia\xC5\x82";
    sf::Text title(
        font,
        sf::String::fromUtf8(titleText.begin(), titleText.end()),
        40
    );

    sf::FloatRect titleBounds = title.getLocalBounds();

    title.setOrigin({
        titleBounds.position.x + titleBounds.size.x / 2.f,
        titleBounds.position.y + titleBounds.size.y / 2.f
    });

    title.setPosition({600.f, 120.f});

    Button arithmeticButton(
        font,
        "Arytmetyka",
        sf::Vector2f(475.f, 200.f)
    );

    Button algebraButton(
        font,
        "Algebra",
        sf::Vector2f(475.f, 300.f)
    );

    Button geometryButton(
        font,
        "Geometria",
        sf::Vector2f(475.f, 400.f)
    );

    Button backButton(
        font,
        "Cofnij",
        sf::Vector2f(475.f, 500.f)
    );

    int selectedButton = -1;

    while (window.isOpen())
    {
        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);

        sf::Vector2f mouseWorldPosition =
            window.mapPixelToCoords(mousePosition);

        arithmeticButton.update(mouseWorldPosition);
        algebraButton.update(mouseWorldPosition);
        geometryButton.update(mouseWorldPosition);
        backButton.update(mouseWorldPosition);

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            arithmeticButton.handleEvent(*event);
            algebraButton.handleEvent(*event);
            geometryButton.handleEvent(*event);
            backButton.handleEvent(*event);

            if (event->is<sf::Event::MouseMoved>())
            {
                selectedButton = -1;

                arithmeticButton.setSelected(false);
                algebraButton.setSelected(false);
                geometryButton.setSelected(false);
                backButton.setSelected(false);
            }

            if (event->is<sf::Event::MouseButtonPressed>())
            {
                const auto* mouseEvent =
                    event->getIf<sf::Event::MouseButtonPressed>();

                if (mouseEvent->button == sf::Mouse::Button::Left)
                {
                    selectedButton = -1;

                    arithmeticButton.setSelected(false);
                    algebraButton.setSelected(false);
                    geometryButton.setSelected(false);
                    backButton.setSelected(false);
                }
            }

            if (event->is<sf::Event::KeyPressed>())
            {
                const auto* keyEvent =
                    event->getIf<sf::Event::KeyPressed>();

                if (keyEvent->code == sf::Keyboard::Key::Up)
                {
                    if (selectedButton == -1)
                    {
                        if (arithmeticButton.isHovered())
                        {
                            selectedButton = 0;
                        }
                        else if (algebraButton.isHovered())
                        {
                            selectedButton = 1;
                        }
                        else if (geometryButton.isHovered())
                        {
                            selectedButton = 2;
                        }
                        else if (backButton.isHovered())
                        {
                            selectedButton = 3;
                        }
                        else
                        {
                            selectedButton = 0;
                        }
                    }
                    else
                    {
                        selectedButton--;

                        if (selectedButton < 0)
                        {
                            selectedButton = 3;
                        }
                    }

                    arithmeticButton.setSelected(selectedButton == 0);
                    algebraButton.setSelected(selectedButton == 1);
                    geometryButton.setSelected(selectedButton == 2);
                    backButton.setSelected(selectedButton == 3);
                }

                if (keyEvent->code == sf::Keyboard::Key::Down)
                {
                    if (selectedButton == -1)
                    {
                        if (arithmeticButton.isHovered())
                        {
                            selectedButton = 0;
                        }
                        else if (algebraButton.isHovered())
                        {
                            selectedButton = 1;
                        }
                        else if (geometryButton.isHovered())
                        {
                            selectedButton = 2;
                        }
                        else if (backButton.isHovered())
                        {
                            selectedButton = 3;
                        }
                        else
                        {
                            selectedButton = 0;
                        }
                    }
                    else
                    {
                        selectedButton++;

                        if (selectedButton > 3)
                        {
                            selectedButton = 0;
                        }
                    }

                    arithmeticButton.setSelected(selectedButton == 0);
                    algebraButton.setSelected(selectedButton == 1);
                    geometryButton.setSelected(selectedButton == 2);
                    backButton.setSelected(selectedButton == 3);
                }

                if (keyEvent->code == sf::Keyboard::Key::Escape)
                {
                    return;
                }
            }
        }

        if (arithmeticButton.isActivated())
        {
            selectedButton = -1;

            arithmeticButton.setSelected(false);
            algebraButton.setSelected(false);
            geometryButton.setSelected(false);
            backButton.setSelected(false);

            runArytmetyka(window, font, mathFont);
        }

        if (algebraButton.isActivated())
        {
            selectedButton = -1;

            arithmeticButton.setSelected(false);
            algebraButton.setSelected(false);
            geometryButton.setSelected(false);
            backButton.setSelected(false);

            algebra(window);
        }

        if (geometryButton.isActivated())
        {
            selectedButton = -1;

            arithmeticButton.setSelected(false);
            algebraButton.setSelected(false);
            geometryButton.setSelected(false);
            backButton.setSelected(false);

            geometria(window);
        }

        if (backButton.isActivated())
        {
            return;
        }

        window.clear(sf::Color(30, 30, 30));

        window.draw(title);

        arithmeticButton.draw(window);
        algebraButton.draw(window);
        geometryButton.draw(window);
        backButton.draw(window);

        window.display();
    }
}

