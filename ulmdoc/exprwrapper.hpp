#ifndef ULMDOC0_EXPRWRAPPER_HPP
#define ULMDOC0_EXPRWRAPPER_HPP

#include "expr.hpp"

namespace ulmdoc {

class ExprWrapper
{
  public:
    ExprWrapper(const Expr::ExprPtr exprPtr);
    ExprWrapper(std::int64_t literal);

    bool isLiteral() const;
    bool isLiteralZero() const;
    bool isLiteralOne() const;
    bool isZeroReg() const;
    bool isIntEnum(std::string sym) const;

    std::int64_t getLiteral() const;

    Expr::ExprPtr exprPtr;
};

std::string latex(ExprWrapper expr);
Expr::ExprPtr ulm_regVal(ExprWrapper reg);
Expr::ExprPtr ulm_ipVal();

ExprWrapper declareBitField(std::string sym,
			    Expr::BitPatternType bitPatternType,
			    std::size_t numBits);

std::ostream &operator<<(std::ostream &out, const ExprWrapper expr);

ExprWrapper operator+(ExprWrapper left, ExprWrapper right);
ExprWrapper operator-(ExprWrapper left, ExprWrapper right);
ExprWrapper operator*(ExprWrapper left, ExprWrapper right);
ExprWrapper operator/(ExprWrapper left, ExprWrapper right);
ExprWrapper operator%(ExprWrapper left, ExprWrapper right);
ExprWrapper operator&(ExprWrapper left, ExprWrapper right);
ExprWrapper operator|(ExprWrapper left, ExprWrapper right);
ExprWrapper operator&&(ExprWrapper left, ExprWrapper right);
ExprWrapper operator||(ExprWrapper left, ExprWrapper right);
ExprWrapper operator==(ExprWrapper left, ExprWrapper right);
ExprWrapper operator!=(ExprWrapper left, ExprWrapper right);
ExprWrapper operator<<(ExprWrapper left, ExprWrapper right);

} // namespace ulmdoc

#endif // ULMDOC0_EXPRWRAPPER_HPP
