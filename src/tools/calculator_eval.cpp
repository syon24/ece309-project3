#include "tools/calculator.h"

#include <stack>

using Kind = CalcToken::Kind;

static int precedence(Kind op) {
    if (op == Kind::Times || op == Kind::Divide) return 2;
    if (op == Kind::Plus || op == Kind::Minus) return 1;
    return 0;
}

static bool apply(std::stack<double>& values, std::stack<Kind>& ops,
                  std::string* error) {
    if (ops.empty() || values.size() < 2) {
        *error = "missing operand or operator";
        return false;
    }
    Kind op = ops.top();
    ops.pop();
    // The right operand is on top because it was read last.
    double right = values.top();
    values.pop();
    double left = values.top();
    values.pop();

    switch (op) {
        case Kind::Plus:  values.push(left + right); break;
        case Kind::Minus: values.push(left - right); break;
        case Kind::Times: values.push(left * right); break;
        case Kind::Divide:
            if (right == 0.0) {
                *error = "division by zero";
                return false;
            }
            values.push(left / right);
            break;
        default:
            *error = "unexpected operator";
            return false;
    }
    return true;
}

bool calc_evaluate(const std::vector<CalcToken>& tokens, double* out,
                   std::string* error) {
    std::stack<double> values;
    std::stack<Kind> ops;

    for (const CalcToken& token : tokens) {
        if (token.kind == Kind::Number) {
            values.push(token.value);
        } else if (token.kind == Kind::LParen) {
            ops.push(token.kind);
        } else if (token.kind == Kind::RParen) {
            while (!ops.empty() && ops.top() != Kind::LParen) {
                if (!apply(values, ops, error)) return false;
            }
            if (ops.empty()) {
                *error = "unmatched closing parenthesis";
                return false;
            }
            ops.pop();
        } else {
            if (precedence(token.kind) == 0) {
                *error = "unexpected operator";
                return false;
            }
            while (!ops.empty() && ops.top() != Kind::LParen &&
                   precedence(ops.top()) >= precedence(token.kind)) {
                if (!apply(values, ops, error)) return false;
            }
            ops.push(token.kind);
        }
    }

    while (!ops.empty()) {
        if (ops.top() == Kind::LParen) {
            *error = "unmatched opening parenthesis";
            return false;
        }
        if (!apply(values, ops, error)) return false;
    }
    if (values.size() != 1) {
        *error = "expected one result";
        return false;
    }
    *out = values.top();
    return true;
}
