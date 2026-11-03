#include <SFML/Graphics.hpp>
#include "GUI/Button.h"

void mainMenu(sf::RenderWindow& window);

int main()
{
    sf::RenderWindow window(
        sf::VideoMode({1200, 800}),
        "Program Kalkulacyjny"
    );

    sf::Font font;

    if (!font.openFromFile("fonts/DejaVuSans.ttf"))
    {
        window.close();
    }

    Button startButton(
        font,
        "Start",
        sf::Vector2f(475.f, 250.f)
    );

    Button exitButton(
        font,
        "Wyjdz",
        sf::Vector2f(475.f, 350.f)
    );

    sf::Text title(font, "Program Kalkulacyjny", 40);

    sf::FloatRect titleBounds = title.getLocalBounds();

    title.setOrigin({
        titleBounds.position.x + titleBounds.size.x / 2.f,
        titleBounds.position.y + titleBounds.size.y / 2.f
    });

    title.setPosition({600.f, 120.f});

    int selectedButton = -1;

    while (window.isOpen())
    {
        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);

        sf::Vector2f mouseWorldPosition =
            window.mapPixelToCoords(mousePosition);

        startButton.update(mouseWorldPosition);
        exitButton.update(mouseWorldPosition);

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            startButton.handleEvent(*event);
            exitButton.handleEvent(*event);

            if (event->is<sf::Event::MouseButtonPressed>())
            {
                const auto* mouseEvent =
                    event->getIf<sf::Event::MouseButtonPressed>();

                if (mouseEvent->button == sf::Mouse::Button::Left)
                {
                    selectedButton = -1;

                    startButton.setSelected(false);
                    exitButton.setSelected(false);
                }
            }

            if (event->is<sf::Event::MouseMoved>())
            {
                selectedButton = -1;

                startButton.setSelected(false);
                exitButton.setSelected(false);
            }

            if (event->is<sf::Event::KeyPressed>())
            {
                const auto* keyEvent =
                    event->getIf<sf::Event::KeyPressed>();

                if (keyEvent->code == sf::Keyboard::Key::Up ||
                    keyEvent->code == sf::Keyboard::Key::Down)
                {
                    if (selectedButton == -1)
                    {
                        if (startButton.isHovered())
                        {
                            selectedButton = 0;
                        }
                        else if (exitButton.isHovered())
                        {
                            selectedButton = 1;
                        }
                        else
                        {
                            selectedButton = 0;
                        }
                    }
                    else if (selectedButton == 0)
                    {
                        selectedButton = 1;
                    }
                    else
                    {
                        selectedButton = 0;
                    }

                    startButton.setSelected(selectedButton == 0);
                    exitButton.setSelected(selectedButton == 1);
                }
            }

            if (event->is<sf::Event::KeyPressed>())
            {
                const auto* keyEvent =
                    event->getIf<sf::Event::KeyPressed>();

                if (keyEvent->code == sf::Keyboard::Key::Escape)
                {
                    window.close();
                }
            }
        }

        if (startButton.isActivated())
        {
            selectedButton = -1;
            startButton.setSelected(false);
            exitButton.setSelected(false);

            mainMenu(window);
        }

        if (exitButton.isActivated())
        {
            window.close();
        }

        window.clear();

        window.draw(title);

        startButton.draw(window);
        exitButton.draw(window);

        window.display();
    }

    return 0;
}

