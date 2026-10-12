#include "Expression.h"

void Expression::add(const std::string& text)
{
    expression += text;
}

void Expression::removeLast()
{
    if (!expression.empty())
    {
        expression.pop_back();
    }
}

void Expression::clear()
{
    expression.clear();
}

std::string Expression::get() const
{
    return expression;
}

