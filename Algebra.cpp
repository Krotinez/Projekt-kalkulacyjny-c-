#include <SFML/Graphics.hpp>

void algebra(sf::RenderWindow& window)
{
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            if (event->is<sf::Event::KeyPressed>())
            {
                auto keyEvent = event->getIf<sf::Event::KeyPressed>();

                if (keyEvent->code == sf::Keyboard::Key::Escape)
                {
                    return;
                }
            }
        }

        window.clear(sf::Color(30, 30, 30));

        window.display();
    }
}