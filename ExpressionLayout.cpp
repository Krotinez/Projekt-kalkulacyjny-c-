#include "ExpressionLayout.h"
#include <stack>

bool isNumberCharacter(char c)
{
    return (c >= '0' && c <= '9') ||
           (c >= 'A' && c <= 'F') ||
           (c >= 'a' && c <= 'f') ||
           c == ',';
}

std::vector<std::string> splitExpression(const std::string& expression)
{
    std::vector<std::string> elements;
    std::string currentNumber;

    for (std::size_t i = 0; i < expression.size();)
    {
        unsigned char c = static_cast<unsigned char>(expression[i]);

        // Ustalamy długość znaku UTF-8
        std::size_t charLength = 1;

        if ((c & 0xE0) == 0xC0)
            charLength = 2;
        else if ((c & 0xF0) == 0xE0)
            charLength = 3;
        else if ((c & 0xF8) == 0xF0)
            charLength = 4;

        // Zabezpieczenie przed wyjściem poza tekst
        if (i + charLength > expression.size())
            charLength = 1;

        std::string znak = expression.substr(i, charLength);
        i += charLength;

        // Łączymy cyfry w jedną liczbę
        if (znak.size() == 1 && isNumberCharacter(znak[0]))
        {
            currentNumber += znak;
        }
        else
        {
            // Zapisujemy poprzednią liczbę
            if (!currentNumber.empty())
            {
                elements.push_back(currentNumber);
                currentNumber.clear();
            }

            // Operator lub nawias jako osobny element
            elements.push_back(znak);
        }
    }

    // Zapisujemy ostatnią liczbę
    if (!currentNumber.empty())
    {
        elements.push_back(currentNumber);
    }

    return elements;
}

bool isAttachedOperator(const std::string& element)
{
    return element == "%" ||
           element == "‰" ||
           element == "!" ||
           element == "^";
}

bool canWrapBefore(
    const std::vector<std::string>& elements,
    std::size_t index
)
{
    if (index == 0 || index >= elements.size())
        return false;

    // Nie przenosimy operatora, jeżeli nie ma po nim argumentu.
    if (index + 1 >= elements.size())
        return false;

    // Nie oddzielamy operatora od nawiasu otwierającego.
    if (elements[index] == "(" ||
        elements[index] == "[" ||
        elements[index] == "{" ||
        elements[index] == "<")
    {
        return false;
    }

    const std::string& previous = elements[index - 1];
    const std::string& current = elements[index];

    // Nie rozdzielamy nawiasu otwierającego od zawartości.
    if (previous == "(" || previous == "[" ||
    previous == "{" || previous == "<")
    return false;

    // Nie zaczynamy nowej linii nawiasem zamykającym.
    if (current == ")" || current == "]" ||
    current == "}" || current == ">")
    return false;

    // Operator musi pozostać z następnym elementem.
    if (previous == "+" || previous == "-" ||
        previous == "*" || previous == "/" ||
        previous == "×" || previous == "÷")
        return false;

        // Nie oddzielamy procentów, promili, silni i potęg.
        if (isAttachedOperator(current))
            return false;

        // Nie oddzielamy wykładnika od znaku potęgowania.
        if (previous == "^")
            return false;

    // Preferujemy zawijanie przed operatorami.
    if (current == "+" || current == "-" ||
        current == "*" || current == "/" ||
        current == "×" || current == "÷")
        return true;

    return false;
}

int getWrapPriority(
    const std::vector<std::string>& elements,
    std::size_t index
)
{
    // Najpierw sprawdzamy, czy podział jest dozwolony.
    if (!canWrapBefore(elements, index))
        return 0;

    const std::string& current = elements[index];

    // Najlepsze miejsca podziału.
    if (current == "+" || current == "-")
        return 2;

    // Miejsca podziału używane w drugiej kolejności.
    if (current == "*" || current == "/" ||
        current == "×" || current == "÷")
        return 1;

    return 0;
}

std::vector<int> findMatchingParentheses(
    const std::vector<std::string>& elements
)
{
    std::vector<int> matching(elements.size(), -1);
    std::stack<std::size_t> openParentheses;

    for (std::size_t i = 0; i < elements.size(); ++i)
    {
        const std::string& current = elements[i];

        // Nawiasy otwierające
        if (current == "(" || current == "[" || current == "{" || current == "<")
        {
            openParentheses.push(i);
        }

        // Nawiasy zamykające
        else if (current == ")" || current == "]" || current == "}" || current == ">")
        {
            if (!openParentheses.empty())
            {
                std::size_t opening = openParentheses.top();
                const std::string& previous = elements[opening];

                bool correctPair =
                    (previous == "(" && current == ")") ||
                    (previous == "[" && current == "]") ||
                    (previous == "{" && current == "}") ||
                    (previous == "<" && current == ">");

                if (correctPair)
                {
                    openParentheses.pop();

                    matching[opening] = static_cast<int>(i);
                    matching[i] = static_cast<int>(opening);
                }
            }
        }
    }

    return matching;
}

std::vector<std::string> getParenthesesGroup(
    const std::vector<std::string>& elements,
    const std::vector<int>& matching,
    std::size_t openingIndex
)
{
    std::vector<std::string> group;

    if (openingIndex >= elements.size() ||
        openingIndex >= matching.size())
        return group;

    const std::string& opening = elements[openingIndex];

    // Sprawdzamy, czy wskazany element otwiera grupę.
    if (opening != "(" && opening != "[" &&
        opening != "{" && opening != "<")
        return group;

    int closingIndex = matching[openingIndex];

    // Brak pasującego nawiasu zamykającego.
    if (closingIndex < 0)
        return group;

    // Pobieramy całą grupę wraz z nawiasami.
    for (std::size_t i = openingIndex;
         i <= static_cast<std::size_t>(closingIndex);
         ++i)
    {
        group.push_back(elements[i]);
    }

    return group;
}

float getTextWidth(
    const std::string& text,
    const sf::Font& font,
    unsigned int characterSize
)
{
    sf::Text measuredText(font, "", characterSize);

    std::string displayText = text;

    // Wewnętrzne znaczniki wartości bezwzględnej
    for (char& c : displayText)
    {
        if (c == '<' || c == '>')
            c = '|';
    }

    measuredText.setString(
        sf::String::fromUtf8(
            displayText.begin(),
            displayText.end()
        )
    );

    return measuredText.getLocalBounds().size.x;
}

std::size_t findWrapPosition(
    const std::vector<std::string>& elements,
    std::size_t start,
    float maxWidth,
    const sf::Font& font,
    unsigned int characterSize
)
{
    std::string line;

    std::size_t bestPosition = elements.size();
    int bestPriority = 0;

    for (std::size_t i = start; i < elements.size(); ++i)
    {
        std::string candidate = line + elements[i];

        if (getTextWidth(candidate, font, characterSize) > maxWidth)
        {
            // Jeżeli znaleźliśmy bezpieczne miejsce podziału,
            // wykorzystujemy je.
            if (bestPosition != elements.size())
                return bestPosition;

            // Jeśli bieżąca linia jest już za szeroka,
            // ale nie można jej jeszcze podzielić,
            // szukamy następnego bezpiecznego miejsca.
            for (std::size_t j = i + 1; j < elements.size(); ++j)
            {
                if (canWrapBefore(elements, j))
                    return j;
            }

            // Nie ma żadnego miejsca podziału.
            return elements.size();
        }

        line = candidate;

        if (i + 1 < elements.size())
        {
            int priority = getWrapPriority(elements, i + 1);

            if (priority == 0)
                continue;

            // Pierwsze dozwolone miejsce podziału.
            if (bestPriority == 0)
            {
                bestPriority = priority;
                bestPosition = i + 1;
            }
            // Preferujemy wyższy priorytet.
            else if (priority > bestPriority)
            {
                bestPriority = priority;
                bestPosition = i + 1;
            }
            // Przy tym samym priorytecie wybieramy późniejszy podział.
            else if (priority == bestPriority)
            {
                bestPosition = i + 1;
            }
        }
    }

    // Całe pozostałe wyrażenie mieści się w jednej linii.
    return elements.size();
}

std::vector<std::string> buildExpressionLines(
    const std::vector<std::string>& elements,
    float maxWidth,
    const sf::Font& font,
    unsigned int characterSize
)
{
    std::vector<std::string> lines;
    std::size_t start = 0;

    while (start < elements.size())
    {
        std::size_t end = findWrapPosition(
            elements,
            start,
            maxWidth,
            font,
            characterSize
        );

        // Nie kończymy podziału samotnym operatorem.
        if (end < elements.size() &&
            end > start &&
            (elements[end - 1] == "+" ||
            elements[end - 1] == "-" ||
            elements[end - 1] == "*" ||
            elements[end - 1] == "/"))
        {
            --end;
        }

        // Brak bezpiecznego miejsca podziału.
        // Zachowujemy pozostały fragment w całości.
        if (end <= start)
        {
            // Szukamy pierwszego dozwolonego miejsca podziału.
            std::size_t next = start + 1;

            while (next < elements.size() &&
                !canWrapBefore(elements, next))
            {
                ++next;
            }

            // Jeśli nie ma miejsca podziału,
            // pozostawiamy fragment w całości.
            if (next == elements.size())
            {
                std::string line;

                for (std::size_t i = start; i < elements.size(); ++i)
                    line += elements[i];

                lines.push_back(line);
                break;
            }

            end = next;
        }
        else if (end == elements.size())
        {
            std::string line;

            for (std::size_t i = start; i < elements.size(); ++i)
                line += elements[i];

            lines.push_back(line);
            break;
        }

        // Budujemy tekst bieżącej linii.
        std::string line;

        for (std::size_t i = start; i < end; ++i)
            line += elements[i];

        lines.push_back(line);

        // Następna linia zaczyna się od znalezionego miejsca.
        start = end;
    }

    return lines;
}

std::vector<bool> findOverflowLines(
    const std::vector<std::string>& lines,
    float maxWidth,
    const sf::Font& font,
    unsigned int characterSize
)
{
    std::vector<bool> overflow;

    for (const std::string& line : lines)
    {
        float width = getTextWidth(
            line,
            font,
            characterSize
        );

        overflow.push_back(width > maxWidth);
    }

    return overflow;
}

std::vector<ExpressionLine> prepareExpressionLines(
    const std::vector<std::string>& lines,
    float maxWidth,
    const sf::Font& font,
    unsigned int characterSize
)
{
    std::vector<ExpressionLine> result;

    for (const std::string& line : lines)
    {
        ExpressionLine info;

        info.text = line;

        info.width = getTextWidth(
            line,
            font,
            characterSize
        );

        info.overflow = info.width > maxWidth;

        info.scrollOffset = 0.f;

        result.push_back(info);
    }

    return result;
}