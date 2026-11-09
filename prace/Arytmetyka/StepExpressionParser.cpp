#include "StepExpressionParser.h"
#include "ExpressionSyntax.h"
#include "NumericLiteral.h"
#include <cctype>
#include <stdexcept>

namespace StepExpressionParser {
void Parser::skip() { while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i; }

bool Parser::eat(const std::string& t) { skip(); if (s.compare(i, t.size(), t) == 0) { i += t.size(); return true; } return false; }

bool Parser::bracket() { skip(); return i < s.size() && ExpressionSyntax::isOpeningBracket(s[i]); }

Ptr Parser::atom() {
        skip();
        const auto function = ExpressionSyntax::consumeBinaryFunction(s, i);
        if (function != ExpressionSyntax::BinaryFunction::None) {
            auto first = sum();
            skip();
            if (!ExpressionSyntax::consumeArgumentSeparator(s, i))
                throw std::runtime_error(function == ExpressionSyntax::BinaryFunction::Logarithm
                    ? "Brak separatora argumentow logarytmu" : "Brak separatora argumentow pierwiastka");
            auto second = sum();
            if (!eat(")"))
                throw std::runtime_error(function == ExpressionSyntax::BinaryFunction::Logarithm
                    ? "Brak zamkniecia logarytmu" : "Brak zamkniecia pierwiastka");
            return std::make_unique<Node>(function == ExpressionSyntax::BinaryFunction::Logarithm ? "log" : "root",
                                          std::move(first), std::move(second));
        }
        if (bracket()) {
            const char c = s[i++], end = ExpressionSyntax::closingBracket(c); 
            auto n = sum();
            if (!eat(std::string(1,end))) throw std::runtime_error("Brak nawiasu zamykajacego");
            auto group = std::make_unique<Node>(c == '<' ? "abs" : "group",std::move(n));
            group->open = std::string(1,c);
            return group;
        }
        const auto start = i;
        NumericLiteral::parse(s, i, base);
        return std::make_unique<Node>(s.substr(start, i-start));
    }

Ptr Parser::postfix() {
        auto n = atom();
        while (true) {
            skip();
            switch (ExpressionSyntax::consumePostfix(s, i)) {
                case ExpressionSyntax::Postfix::Percent: n = std::make_unique<Node>("%", std::move(n)); break;
                case ExpressionSyntax::Postfix::Permille: n = std::make_unique<Node>("‰", std::move(n)); break;
                case ExpressionSyntax::Postfix::Factorial: n = std::make_unique<Node>("!", std::move(n)); break;
                case ExpressionSyntax::Postfix::None: return n;
            }
        }
        return n;
    }

Ptr Parser::power() {
        auto n = postfix();
        if (ExpressionSyntax::consumeBinaryOperator(s, i, ExpressionSyntax::BinaryOperator::Power)
            == ExpressionSyntax::BinaryOperator::None) return n;
        if (exponentDepth >= 2) throw std::runtime_error("Maksymalnie dwa poziomy potegowania");
        ++exponentDepth;
        auto exponent = unary();
        --exponentDepth;
        return std::make_unique<Node>("^",std::move(n),std::move(exponent));
    }

Ptr Parser::unary() {
        if (eat("+")) return std::make_unique<Node>("u+",unary());
        if (eat("-")) return std::make_unique<Node>("u-",unary());
        return power();
    }

Ptr Parser::product() {
        auto n=unary();
        while(true) {
            const auto op = ExpressionSyntax::consumeBinaryOperator(
                s, i, ExpressionSyntax::BinaryOperator::Multiply,
                ExpressionSyntax::BinaryOperator::Divide);
            if (op == ExpressionSyntax::BinaryOperator::Multiply)
                n = std::make_unique<Node>("×", std::move(n), unary());
            else if (op == ExpressionSyntax::BinaryOperator::Divide)
                n = std::make_unique<Node>("÷", std::move(n), unary());
            else {
                skip();
                const bool nextStructure = bracket() ||
                    s.compare(i, 5, "root(") == 0 || s.compare(i, 4, "log(") == 0;
                // A numeral immediately following a completed root/logarithm
                // means multiplication, just as a numeral before it does.
                // Do not reinterpret adjacent ordinary digits as multiplication.
                const bool nextNumber = i < s.size() &&
                    (std::isdigit(static_cast<unsigned char>(s[i])) ||
                     (base == 16 && std::isxdigit(static_cast<unsigned char>(s[i]))));
                const auto endsInStructure = [](const Node* node) {
                    while (node && node->op == "group") node = node->left.get();
                    return node && (node->op == "root" || node->op == "log");
                };
                // A product can also end with a graphical structure.
                const Node* tail = n.get();
                while (tail && tail->op == "×") tail = tail->right.get();
                if (nextStructure || (nextNumber && endsInStructure(tail)))
                    n = std::make_unique<Node>("×", std::move(n), unary());
                else break;
            }
        }
        return n;
    }

Ptr Parser::sum() {
        auto n=product();
        while(true) {
            const auto op = ExpressionSyntax::consumeBinaryOperator(
                s, i, ExpressionSyntax::BinaryOperator::Add,
                ExpressionSyntax::BinaryOperator::Subtract);
            if (op == ExpressionSyntax::BinaryOperator::Add)
                n = std::make_unique<Node>("+", std::move(n), product());
            else if (op == ExpressionSyntax::BinaryOperator::Subtract)
                n = std::make_unique<Node>("-", std::move(n), product());
            else break;
        }
        return n;
    }
} // namespace StepExpressionParser
