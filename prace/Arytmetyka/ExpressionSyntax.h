#pragma once

#include <cstddef>
#include <string>

// Shared lexical rules for the regular calculator and step-by-step parser.
namespace ExpressionSyntax {
inline bool isOpeningBracket(char c) {
    return c == '(' || c == '[' || c == '{' || c == '<';
}
inline char closingBracket(char opening) {
    switch (opening) {
        case '(': return ')';
        case '[': return ']';
        case '{': return '}';
        case '<': return '>';
        default: return '\0';
    }
}
// Shared recognition of two-argument mathematical functions.
enum class BinaryFunction { None, Logarithm, Root };
inline BinaryFunction consumeBinaryFunction(const std::string& expression, std::size_t& position) {
    if (expression.compare(position, 4, "log(") == 0) {
        position += 4;
        return BinaryFunction::Logarithm;
    }
    if (expression.compare(position, 5, "root(") == 0) {
        position += 5;
        return BinaryFunction::Root;
    }
    return BinaryFunction::None;
}
inline bool consumeArgumentSeparator(const std::string& expression, std::size_t& position) {
    if (position < expression.size() && (expression[position] == ';' || expression[position] == ',')) {
        ++position;
        return true;
    }
    return false;
}

// The two parsers share token recognition and operator precedence, but keep
// their different outputs (a number versus a step-by-step syntax tree).
enum class BinaryOperator { None, Add, Subtract, Multiply, Divide, Power };
inline BinaryOperator consumeBinaryOperator(const std::string& expression,
                                            std::size_t& position,
                                            BinaryOperator first,
                                            BinaryOperator second = BinaryOperator::None) {
    while (position < expression.size() &&
           (expression[position] == ' ' || expression[position] == '\t' ||
            expression[position] == '\n' || expression[position] == '\r')) ++position;
    const auto matches = [&](BinaryOperator op) -> std::size_t {
        switch (op) {
            case BinaryOperator::Add: return expression.compare(position, 1, "+") == 0 ? 1 : 0;
            case BinaryOperator::Subtract: return expression.compare(position, 1, "-") == 0 ? 1 : 0;
            case BinaryOperator::Multiply:
                if (expression.compare(position, 1, "*") == 0) return 1;
                return expression.compare(position, 2, "×") == 0 ? 2 : 0;
            case BinaryOperator::Divide:
                if (expression.compare(position, 1, "/") == 0) return 1;
                return expression.compare(position, 2, "÷") == 0 ? 2 : 0;
            case BinaryOperator::Power: return expression.compare(position, 1, "^") == 0 ? 1 : 0;
            case BinaryOperator::None: return 0;
        }
        return 0;
    };
    if (const auto size = matches(first)) { position += size; return first; }
    if (const auto size = matches(second)) { position += size; return second; }
    return BinaryOperator::None;
}
inline int precedence(BinaryOperator op) {
    switch (op) {
        case BinaryOperator::Add:
        case BinaryOperator::Subtract: return 1;
        case BinaryOperator::Multiply:
        case BinaryOperator::Divide: return 2;
        case BinaryOperator::Power: return 4;
        case BinaryOperator::None: return 0;
    }
    return 0;
}
inline BinaryOperator binaryOperator(const std::string& op) {
    if (op == "+") return BinaryOperator::Add;
    if (op == "-") return BinaryOperator::Subtract;
    if (op == "×" || op == "*") return BinaryOperator::Multiply;
    if (op == "÷" || op == "/") return BinaryOperator::Divide;
    if (op == "^") return BinaryOperator::Power;
    return BinaryOperator::None;
}

enum class Postfix { None, Percent, Permille, Factorial };
inline Postfix consumePostfix(const std::string& expression, std::size_t& position) {
    if (position < expression.size() && expression[position] == '%') {
        ++position;
        return Postfix::Percent;
    }
    if (expression.compare(position, 3, "‰") == 0) {
        position += 3;
        return Postfix::Permille;
    }
    if (position < expression.size() && expression[position] == '!') {
        ++position;
        return Postfix::Factorial;
    }
    return Postfix::None;
}
}
