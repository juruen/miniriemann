#include <expression.h>
#include <util.h>
#include <glog/logging.h>

inline std::string QueryNode::indent(unsigned int d) {
  return std::string(d * 2, ' ');
}

QueryNode::~QueryNode() {};

QueryTrue::QueryTrue() : QueryNode()
{
}

void QueryTrue::print(std::ostream &os, unsigned int depth) const {
  os << indent(depth) << "true" << std::endl;
}

query_f_t QueryTrue::evaluate() const {
  return [=](const Event& e) {
    UNUSED_VAR(e);
    VLOG(3) << "QueryTrue()::evaluate()";
    return true;
  };
}

QueryTagged::QueryTagged(std::string* string) : QueryNode(), string(string)
{
}

void QueryTagged::print(std::ostream &os, unsigned int depth) const {
  os << indent(depth) << "tagged = " << *string << std::endl;
}

query_f_t QueryTagged::evaluate() const {
  std::string tag(string->substr(1, string->size() - 2));
  return [=](const Event& e) {
    VLOG(3) << "QueryTagged()::evaluate() tag: " << tag;
    return tag_exists(e, tag);
  };
}

QueryAnd::QueryAnd(QueryNode* left, QueryNode* right) :
  QueryNode(),
  left(left),
  right(right)
{
}

void QueryAnd::print(std::ostream &os, unsigned int depth) const {
  os << indent(depth) << " and" << std::endl;
  left->print(os, depth + 1);
  right->print(os, depth + 1);
}

query_f_t QueryAnd::evaluate() const {
  query_f_t left_f = left->evaluate();
  query_f_t right_f = right->evaluate();
  return [=](const Event& e) {
    VLOG(3) << "QueryAnd()::evaluate()";
    return (left_f(e) && right_f(e));
  };
}

QueryOr::QueryOr(QueryNode* left, QueryNode* right) :
  QueryNode(),
  left(left),
  right(right)
{
}

void QueryOr::print(std::ostream &os, unsigned int depth) const {
  os << indent(depth) << " or" << std::endl;
  left->print(os, depth + 1);
  right->print(os, depth + 1);
}

query_f_t QueryOr::evaluate() const {
  query_f_t left_f = left->evaluate();
  query_f_t right_f = right->evaluate();
  return [=](const Event& e) {
    VLOG(3) << "QueryOr()::evaluate()";
    return (left_f(e) || right_f(e));
  };
}

QueryNot::QueryNot(QueryNode* right) :
  QueryNode(),
  right(right)
{
}

void QueryNot::print(std::ostream &os, unsigned int depth) const {
  os << indent(depth) << " not" << std::endl;
  right->print(os, depth + 1);
}

query_f_t QueryNot::evaluate() const {
  query_f_t right_f = right->evaluate();
  return [=](const Event& e) {
    VLOG(3) << "QueryNot()::evaluate()";
    return (!right_f(e));
  };
}


QueryContext::QueryContext() : expression(0) {};

QueryContext::~QueryContext()
{
  clearExpressions();
}

void	QueryContext::clearExpressions()
{
  if (!expression) {
    delete expression;
    expression = 0;
  }
}
