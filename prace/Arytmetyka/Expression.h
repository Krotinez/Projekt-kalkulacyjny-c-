#pragma once

#include <string>

class Expression
{
private:
    std::string expression;

public:
    void add(const std::string& text);
    void removeLast();
    void clear();

    std::string get() const;
};
