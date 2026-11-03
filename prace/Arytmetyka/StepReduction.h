#pragma once
#include "StepExpressionParser.h"
#include <cstddef>
#include <string>
#include <utility>

namespace StepReduction {
using StepExpressionParser::Node;
using StepExpressionParser::Ptr;
std::string render(const Node& n, int parent=0, bool right=false, const std::string& parentOp="", const Node* marked=nullptr);
void simplify(Ptr& n);
bool reduce(Ptr& n, int base, const Node*& oldTarget, const Node*& newTarget, std::string& replacementLiteral);
std::pair<std::string,std::pair<std::size_t,std::size_t>> markedRender(const Node& root,const Node* target);
const Node* nextCandidate(const Node* root);
}
