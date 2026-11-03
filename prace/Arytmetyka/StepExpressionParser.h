#pragma once
#include <cstddef>
#include <memory>
#include <string>
#include <utility>

namespace StepExpressionParser {
struct Node {
    std::string op, literal, open;
    double value = 0;
    bool computed = false;
    std::unique_ptr<Node> left, right;
    explicit Node(std::string s) : literal(std::move(s)) {}
    Node(std::string s, std::unique_ptr<Node> a, std::unique_ptr<Node> b = nullptr)
        : op(std::move(s)), left(std::move(a)), right(std::move(b)) {}
};
using Ptr = std::unique_ptr<Node>;
struct Parser {
    const std::string& s;
    std::size_t i = 0;
    int base;
    int exponentDepth = 0;
    void skip();
    bool eat(const std::string& t);
    bool bracket();
    Ptr atom();
    Ptr postfix();
    Ptr power();
    Ptr unary();
    Ptr product();
    Ptr sum();
};
} // namespace StepExpressionParser
