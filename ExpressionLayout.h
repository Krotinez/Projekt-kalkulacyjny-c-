#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <SFML/Graphics.hpp>

bool isNumberCharacter(char c);

std::vector<std::string> splitExpression(
    const std::string& expression
);

bool isAttachedOperator(
    const std::string& element
);

bool canWrapBefore(
    const std::vector<std::string>& elements,
    std::size_t index
);

int getWrapPriority(
    const std::vector<std::string>& elements,
    std::size_t index
);

std::vector<int> findMatchingParentheses(
    const std::vector<std::string>& elements
);

std::vector<std::string> getParenthesesGroup(
    const std::vector<std::string>& elements,
    const std::vector<int>& matching,
    std::size_t openingIndex
);

float getTextWidth(
    const std::string& text,
    const sf::Font& font,
    unsigned int characterSize
);

std::size_t findWrapPosition(
    const std::vector<std::string>& elements,
    std::size_t start,
    float maxWidth,
    const sf::Font& font,
    unsigned int characterSize
);

std::vector<std::string> buildExpressionLines(
    const std::vector<std::string>& elements,
    float maxWidth,
    const sf::Font& font,
    unsigned int characterSize
);

std::vector<bool> findOverflowLines(
    const std::vector<std::string>& lines,
    float maxWidth,
    const sf::Font& font,
    unsigned int characterSize
);

struct ExpressionLine
{
    std::string text;
    float width = 0.f;
    bool overflow = false;
    float scrollOffset = 0.f;
};

std::vector<ExpressionLine> prepareExpressionLines(
    const std::vector<std::string>& lines,
    float maxWidth,
    const sf::Font& font,
    unsigned int characterSize
);
