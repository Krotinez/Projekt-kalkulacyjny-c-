#include "StepReduction.h"
#include "ArithmeticOperations.h"
#include "NumericLiteral.h"
#include "NumberFormatting.h"
#include "ExpressionSyntax.h"
#include "../GUI/RootModel.h"
#include "../GUI/LogModel.h"
#include <memory>
#include <stdexcept>
#include <utility>
#include <SFML/System/String.hpp>

namespace StepReduction {
int prec(const Node& n) {
    const int binary = ExpressionSyntax::precedence(ExpressionSyntax::binaryOperator(n.op));
    if (binary != 0) return binary;
    if(n.op=="u+"||n.op=="u-") return 3;
    if(n.op=="%"||n.op=="‰"||n.op=="!") return 5;
    return 5;
}
std::string render(const Node& n, int parent, bool right, const std::string& parentOp, const Node* marked) {
    if (&n == marked) {
        // Markers are removed after rendering; they do not count as characters.
        return "\x01" + render(n,parent,right,parentOp,nullptr) + "\x02";
    }
    if(n.op.empty()) return n.literal;
    if(n.op=="group" || n.op=="abs") {
        // A radical/logarithm is already a visually delimited atomic
        // expression. Its surrounding plain parentheses are redundant.
        // Preserve absolute-value delimiters and all other groups.
        if (n.op=="group" && n.left &&
            (n.left->op=="root" || n.left->op=="log" ||
             parentOp=="root-argument" || parentOp=="log-argument"))
            return render(*n.left,0,false,"",marked);
        const char end = ExpressionSyntax::closingBracket(n.open[0]);
        return n.open+render(*n.left,0,false,"",marked)+end;
    }
    if(n.op=="u+"||n.op=="u-") {
        const std::string sign = n.op=="u+" ? "+" : "-";
        const std::string body = render(*n.left,3,false,"",marked);
        const std::string unary = sign + body;
        // A negative base of a power needs parentheses. Without them,
        // (-1)^2 would be rendered as -1^2, which means -(1^2).
        return parentOp == "^" && !right ? "(" + unary + ")" : unary;
    }
    if(n.op=="log") {
        const auto base = render(*n.left,0,false,"",marked);
        const auto argument = render(*n.right,0,false,"log-argument",marked);
        sf::String visual;
        visual += LogModel::Begin;
        visual += sf::String::fromUtf8(base.begin(),base.end());
        visual += LogModel::Separator;
        visual += sf::String::fromUtf8(argument.begin(),argument.end());
        visual += LogModel::End;
        const auto bytes = visual.toUtf8();
        return std::string(bytes.begin(),bytes.end());
    }
    if(n.op=="root") {
        // Render K-mode roots using the same graphical markers as the editor.
        // The arithmetic tree remains unchanged.
        const auto degree = render(*n.left,0,false,"",marked);
        const auto radicand = render(*n.right,0,false,"root-argument",marked);
        sf::String visual;
        visual += RootModel::Begin;
        visual += sf::String::fromUtf8(degree.begin(),degree.end());
        visual += RootModel::Separator;
        visual += sf::String::fromUtf8(radicand.begin(),radicand.end());
        visual += RootModel::End;
        const auto bytes = visual.toUtf8();
        return std::string(bytes.begin(),bytes.end());
    }
    if(n.op=="%"||n.op=="‰"||n.op=="!") return render(*n.left,5,false,"",marked)+n.op;
    const int p=prec(n);
    std::string lhs=render(*n.left,p,false,n.op,marked), rhs=render(*n.right,p,true,n.op,marked);
    // Parentheses are semantic, not cosmetic, when a negative literal
    // becomes the base of an exponent after reducing a grouped expression.
    if(n.op=="^" && n.left && n.left->op.empty() &&
       !n.left->literal.empty() && n.left->literal.front()=='-')
        lhs="("+lhs+")";
    if((n.op=="×"||n.op=="÷") && !rhs.empty() && rhs[0]=='-') rhs="("+rhs+")";
    std::string s=lhs+n.op+rhs;
    if(p<parent || (right && p==parent && (parentOp=="-"||parentOp=="÷"))) s="("+s+")";
    return s;
}
// Compute the actual value without relying on the shortened displayed text.
double value(const Node& n, int base) {
    if(n.op.empty()) {
        if(n.computed) return n.value;
        std::size_t position = 0;
        const double result = NumericLiteral::parse(n.literal, position, base);
        if (position != n.literal.size())
            throw std::runtime_error("Nieobslugiwany znak lub operator");
        return result;
    }
    if(n.op=="group"||n.op=="u+") return value(*n.left,base);
    if(n.op=="abs") return ArithmeticOperations::absolute(value(*n.left,base));
    if(n.op=="u-") return ArithmeticOperations::unaryMinus(value(*n.left,base));
    if(n.op=="%") return ArithmeticOperations::percent(value(*n.left,base));
    if(n.op=="‰") return ArithmeticOperations::permille(value(*n.left,base));
    if(n.op=="!") return ArithmeticOperations::factorial(value(*n.left,base));
    const double a=value(*n.left,base),b=value(*n.right,base);
    if(n.op=="root") return ArithmeticOperations::root(a,b);
    if(n.op=="log") return ArithmeticOperations::logarithm(a,b);
    if(n.op=="+") return a+b;
    if(n.op=="-") return a-b;
    if(n.op=="×") return a*b;
    if(n.op=="^") return ArithmeticOperations::power(a,b);
    return ArithmeticOperations::divide(a,b);
}
// Both result modes use the same base formatter. The numeric value is
// retained in Node::value, so a shortened display is never parsed again.
// Collapse single-number brackets silently; don't emit cosmetic steps.
void simplify(Ptr& n, int base) {
    if(!n || n->op.empty()) return;
    simplify(n->left, base); simplify(n->right, base);
    if(n->op=="group" && n->left && n->left->op.empty())
        n=std::move(n->left);
    // A sign applied to an already numeric literal is not a calculation
    // step. Fold it before choosing the next highlighted operation.
    // In particular, (-3)^2 must highlight the power immediately, while
    // -3^2 must still mean -(3^2).
    if(n && (n->op=="u-" || n->op=="u+") &&
       n->left && n->left->op.empty()) {
        const double result=value(*n, base);
        auto literal=std::make_unique<Node>(NumberFormatting::formatStep(result,base));
        literal->computed=true;
        literal->value=result;
        n=std::move(literal);
    }
}
// A single arithmetic reduction, deepest-leftmost first.
bool reduce(Ptr& n,int base,const Node*& oldTarget,const Node*& newTarget,std::string& replacementLiteral) {
    if(!n||n->op.empty()) return false;
    if(n->left && reduce(n->left,base,oldTarget,newTarget,replacementLiteral)) return true;
    if(n->right && reduce(n->right,base,oldTarget,newTarget,replacementLiteral)) return true;
    oldTarget=n.get();
    const double result=value(*n,base);
    auto replacement=std::make_unique<Node>(NumberFormatting::formatStep(result,base));
    replacement->value=result;
    replacement->computed=true;
    n=std::move(replacement);
    newTarget=n.get();
    replacementLiteral=n->literal;
    return true;
}
// Locate the exact arithmetic subtree in the rendered expression. Offsets are
// Unicode character indices, matching SFML's caret and highlight positions.
std::pair<std::string,std::pair<std::size_t,std::size_t>> markedRender(const Node& root,const Node* target) {
    std::string raw=render(root,0,false,"",target);
    std::string clean;
    std::size_t begin=0,end=0;
    for(std::size_t i=0;i<raw.size();++i) {
        if(raw[i]=='\x01') {begin=sf::String::fromUtf8(clean.begin(),clean.end()).getSize();continue;}
        if(raw[i]=='\x02') {end=sf::String::fromUtf8(clean.begin(),clean.end()).getSize();continue;}
        clean.push_back(raw[i]);
    }
    return {clean,{begin,end}};
}
// Keep a separate, precise numeric tree: displayed digits with ... are not operands.

const Node* nextCandidate(const Node* node) {
    if (!node || node->op.empty()) return nullptr;
    if (const Node* left = nextCandidate(node->left.get())) return left;
    if (const Node* right = nextCandidate(node->right.get())) return right;
    return node;
}

} // namespace StepReduction
