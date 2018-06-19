#include "interpreter.h"

TerminalExpression::TerminalExpression(const std::string& reg_expr)
    : Expression (reg_expr, Expression::Type::TerminalExpression)
    , reg_exp_(reg_expr, std::regex::ECMAScript)
{
}

bool TerminalExpression::interpret(const std::string& input)
{
    return std::regex_match(input, reg_exp_);
}
