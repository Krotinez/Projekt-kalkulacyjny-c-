#include "ExpressionCalculator.h"
#include "ArithmeticOperations.h"
#include "NumberFormatting.h"
#include "NumericLiteral.h"
#include "ExpressionSyntax.h"
#include <cctype>
#include <stdexcept>

namespace {
struct Parser {
    const std::string& s;
    std::size_t p = 0;
    int base;
    int exponentDepth = 0;
    void spaces() { while (p < s.size() && std::isspace(static_cast<unsigned char>(s[p]))) ++p; }
    bool eat(char c) { spaces(); if (p < s.size() && s[p] == c) { ++p; return true; } return false; }
    double number() {
        spaces();
        return NumericLiteral::parse(s, p, base);
    }
    bool implicitStart() {
        spaces();
        if (p >= s.size()) return false;
        if (ExpressionSyntax::isOpeningBracket(s[p])) return true;
        // A function is an atomic factor: 2root(...), root(...)3, 2log(...)5.
        if (s.compare(p, 5, "root(") == 0 || s.compare(p, 4, "log(") == 0) return true;
        // Adjacent number following a completed structure also multiplies.
        return std::isdigit(static_cast<unsigned char>(s[p]));
    }
    double primary() {
        spaces();
        const auto function = ExpressionSyntax::consumeBinaryFunction(s, p);
        if (function != ExpressionSyntax::BinaryFunction::None) {
            const double first = sum();
            spaces();
            if (!ExpressionSyntax::consumeArgumentSeparator(s, p))
                throw std::runtime_error(function == ExpressionSyntax::BinaryFunction::Logarithm
                    ? "Brak separatora argumentow logarytmu" : "Brak separatora argumentow pierwiastka");
            const double second = sum();
            if (!eat(')'))
                throw std::runtime_error(function == ExpressionSyntax::BinaryFunction::Logarithm
                    ? "Brak zamkniecia logarytmu" : "Brak zamkniecia pierwiastka");
            return function == ExpressionSyntax::BinaryFunction::Logarithm
                ? ArithmeticOperations::logarithm(first, second)
                : ArithmeticOperations::root(first, second);
        }
        if (eat('(') || eat('[') || eat('{') || eat('<')) {
            const char open = s[p - 1];
            const char close = ExpressionSyntax::closingBracket(open); 
            const double v = sum();
            if (!eat(close)) throw std::runtime_error("Brak nawiasu zamykajacego");
            return open == '<' ? ArithmeticOperations::absolute(v) : v;
        }
        return number();
    }
    double postfix() {
        double v = primary();
        while (true) {
            spaces();
            switch (ExpressionSyntax::consumePostfix(s, p)) {
                case ExpressionSyntax::Postfix::Percent: v = ArithmeticOperations::percent(v); break;
                case ExpressionSyntax::Postfix::Permille: v = ArithmeticOperations::permille(v); break;
                case ExpressionSyntax::Postfix::Factorial: v = ArithmeticOperations::factorial(v); break;
                case ExpressionSyntax::Postfix::None: return v;
            }
        }
        return v;
    }
    double power() {
        const double a = postfix();
        if (ExpressionSyntax::consumeBinaryOperator(s, p, ExpressionSyntax::BinaryOperator::Power)
            == ExpressionSyntax::BinaryOperator::None) return a;
        if (exponentDepth >= 2) throw std::runtime_error("Maksymalnie dwa poziomy potegowania");
        ++exponentDepth;
        const double b = unary();
        --exponentDepth;
        return ArithmeticOperations::power(a, b);
    }
    double unary() {
        if (eat('+')) return unary();
        if (eat('-')) return ArithmeticOperations::unaryMinus(unary());
        return power();
    }
    double product() {
        double v = unary();
        while (true) {
            const auto op = ExpressionSyntax::consumeBinaryOperator(
                s, p, ExpressionSyntax::BinaryOperator::Multiply,
                ExpressionSyntax::BinaryOperator::Divide);
            if (op == ExpressionSyntax::BinaryOperator::Multiply) v *= unary();
            else if (op == ExpressionSyntax::BinaryOperator::Divide)
                v = ArithmeticOperations::divide(v, unary());
            else if (implicitStart()) v *= unary();
            else break;
        }
        return v;
    }
    double sum() {
        double v = product();
        while (true) {
            const auto op = ExpressionSyntax::consumeBinaryOperator(
                s, p, ExpressionSyntax::BinaryOperator::Add,
                ExpressionSyntax::BinaryOperator::Subtract);
            if (op == ExpressionSyntax::BinaryOperator::Add) v += product();
            else if (op == ExpressionSyntax::BinaryOperator::Subtract) v -= product();
            else break;
        }
        return v;
    }
};

}
CalculationResult calculateExpression(const std::string& expression, int base) {
    try {
        if (base != 2 && base != 8 && base != 10 && base != 16)
            throw std::runtime_error("Nieznany system liczbowy");
        Parser parser{expression, 0, base};
        parser.spaces();
        if (parser.p == expression.size()) throw std::runtime_error("Puste wyrazenie");
        const double value = parser.sum();
        parser.spaces();
        if (parser.p != expression.size()) throw std::runtime_error("Nieobslugiwany znak lub operator");
        return {true, NumberFormatting::formatBase(value, base), ""};
    } catch (const std::exception& e) {
        return {false, "", e.what()};
    }
}
