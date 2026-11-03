#include "ButtonNavigation.h"
#include <cmath>
#include <limits>

ButtonNavigation::ButtonNavigation(std::vector<Button>& buttons) : buttons(buttons) {}

void ButtonNavigation::clearFocus()
{
        if (focusedIndex)
        {
            buttons[*focusedIndex].setSelected(false);
            focusedIndex.reset();
        }
    }

void ButtonNavigation::setFocus(std::size_t newIndex)
{
        if (focusedIndex && *focusedIndex != newIndex)
        {
            buttons[*focusedIndex].setSelected(false);
        }

        if (newIndex >= buttons.size() || !buttons[newIndex].isEnabled()) return;
        focusedIndex = newIndex;
        buttons[newIndex].setSelected(true);
    }

sf::Vector2f ButtonNavigation::buttonCenter(std::size_t index) const
{
        const sf::Vector2f pos = buttons[index].getPosition();
        const sf::Vector2f size = buttons[index].getSize();
        return sf::Vector2f(pos.x + size.x / 2.f, pos.y + size.y / 2.f);
    }

std::optional<std::size_t> ButtonNavigation::nearestInDirection(std::size_t fromIndex, sf::Keyboard::Key direction) const
{
        const sf::Vector2f from = buttonCenter(fromIndex);
        std::optional<std::size_t> best;
        float bestDistance = std::numeric_limits<float>::max();

        for (std::size_t i = 0; i < buttons.size(); ++i)
        {
            if (i == fromIndex || !buttons[i].isEnabled())
                continue;

            const sf::Vector2f to = buttonCenter(i);
            const float dx = to.x - from.x;
            const float dy = to.y - from.y;

            bool valid = false;

            if (direction == sf::Keyboard::Key::Right) valid = dx > 0.f;
            if (direction == sf::Keyboard::Key::Left)  valid = dx < 0.f;
            if (direction == sf::Keyboard::Key::Down)  valid = dy > 0.f;
            if (direction == sf::Keyboard::Key::Up)    valid = dy < 0.f;

            if (!valid)
                continue;

            float primaryDistance = 0.f;
            float secondaryDistance = 0.f;

            if (direction == sf::Keyboard::Key::Right ||
                direction == sf::Keyboard::Key::Left)
            {
                primaryDistance = std::abs(dx);
                secondaryDistance = std::abs(dy);
            }
            else
            {
                primaryDistance = std::abs(dy);
                secondaryDistance = std::abs(dx);
            }

            const float distance = primaryDistance + secondaryDistance * 10.f;

            if (distance < bestDistance)
            {
                bestDistance = distance;
                best = i;
            }
        }

        return best;
    }

std::optional<std::size_t> ButtonNavigation::wrapInDirection(std::size_t fromIndex, sf::Keyboard::Key direction) const
{
        const sf::Vector2f from = buttonCenter(fromIndex);
        std::optional<std::size_t> best;
        float bestScore = std::numeric_limits<float>::max();

        for (std::size_t i = 0; i < buttons.size(); ++i)
        {
            if (i == fromIndex || !buttons[i].isEnabled())
                continue;

            const sf::Vector2f to = buttonCenter(i);
            const float perpendicular =
                (direction == sf::Keyboard::Key::Left ||
                 direction == sf::Keyboard::Key::Right)
                    ? std::abs(to.y - from.y)
                    : std::abs(to.x - from.x);

            float edgeDistance = 0.f;

            if (direction == sf::Keyboard::Key::Down)
                edgeDistance = to.y;
            else if (direction == sf::Keyboard::Key::Up)
                edgeDistance = -to.y;
            else if (direction == sf::Keyboard::Key::Right)
                edgeDistance = to.x;
            else if (direction == sf::Keyboard::Key::Left)
                edgeDistance = -to.x;

            const float score = perpendicular * 1000.f + edgeDistance;

            if (score < bestScore)
            {
                bestScore = score;
                best = i;
            }
        }

        return best;
    }

std::size_t ButtonNavigation::nearestToMouse(sf::Vector2f mousePos) const
{
        std::size_t best = 0;
        float bestDistance = std::numeric_limits<float>::max();

        for (std::size_t i = 0; i < buttons.size(); ++i)
        {
            if (!buttons[i].isEnabled()) continue;
            const sf::Vector2f center = buttonCenter(i);
            const float dx = center.x - mousePos.x;
            const float dy = center.y - mousePos.y;
            const float distance = dx * dx + dy * dy;

            if (distance < bestDistance)
            {
                bestDistance = distance;
                best = i;
            }
        }

        return best;
    }
