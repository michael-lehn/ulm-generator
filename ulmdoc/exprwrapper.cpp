#include "exprwrapper.hpp"

namespace ulmdoc {

ExprWrapper::ExprWrapper(const Expr::ExprPtr exprPtr)
  : exprPtr{ exprPtr }
{
}

ExprWrapper::ExprWrapper(std::int64_t literal)
  : exprPtr{ makeIntLiteral(literal) }
{
}

bool
ExprWrapper::isLiteral() const
{
    return exprPtr->isLiteral();
}

bool
ExprWrapper::isLiteralZero() const
{
    return exprPtr->isLiteralZero();
}

bool
ExprWrapper::isLiteralOne() const
{
    return exprPtr->isLiteralOne();
}

bool
ExprWrapper::isZeroReg() const
{
    return exprPtr->isZeroReg();
}

bool
ExprWrapper::isIntEnum(std::string sym) const
{
    return exprPtr->isIntEnum(sym);
}

std::int64_t
ExprWrapper::getLiteral() const
{
    if (!isLiteral()) {
	throw std::logic_error("ExprWrapper does not hold a int literal");
    }
    return std::get<Expr::IntLiteral>(exprPtr->repr).val;
}

std::string
latex(ExprWrapper expr)
{
    return latex(expr.exprPtr);
}

Expr::ExprPtr
ulm_regVal(ExprWrapper reg)
{
    return makeRegBitPattern(reg.exprPtr);
}

ExprWrapper
declareBitField(std::string sym, Expr::BitPatternType bitPatternType,
		std::size_t numBits)
{
    return makeTypedBitPattern(sym, bitPatternType, numBits);
}

//------------------------------------------------------------------------------

std::ostream &
operator<<(std::ostream &out, const ExprWrapper expr)
{
    out << std::endl;
    printTree(out, 1, expr.exprPtr);
    return out;
}

//------------------------------------------------------------------------------

ExprWrapper
operator+(ExprWrapper left, ExprWrapper right)
{
    return makeIntBinaryOp(Expr::ADD, left.exprPtr, right.exprPtr);
}

ExprWrapper
operator-(ExprWrapper left, ExprWrapper right)
{
    return makeIntBinaryOp(Expr::SUB, left.exprPtr, right.exprPtr);
}

ExprWrapper
operator*(ExprWrapper left, ExprWrapper right)
{
    return makeIntBinaryOp(Expr::MUL, left.exprPtr, right.exprPtr);
}

ExprWrapper
operator/(ExprWrapper left, ExprWrapper right)
{
    return makeIntBinaryOp(Expr::DIV, left.exprPtr, right.exprPtr);
}

ExprWrapper
operator%(ExprWrapper left, ExprWrapper right)
{
    return makeIntBinaryOp(Expr::MOD, left.exprPtr, right.exprPtr);
}

ExprWrapper
operator&(ExprWrapper left, ExprWrapper right)
{
    return makeIntBinaryOp(Expr::BITWISE_AND, left.exprPtr, right.exprPtr);
}

ExprWrapper
operator|(ExprWrapper left, ExprWrapper right)
{
    return makeIntBinaryOp(Expr::BITWISE_OR, left.exprPtr, right.exprPtr);
}

ExprWrapper
operator&&(ExprWrapper left, ExprWrapper right)
{
    return makeIntBinaryOp(Expr::LOGICAL_AND, left.exprPtr, right.exprPtr);
}

ExprWrapper
operator||(ExprWrapper left, ExprWrapper right)
{
    return makeIntBinaryOp(Expr::LOGICAL_OR, left.exprPtr, right.exprPtr);
}

ExprWrapper
operator==(ExprWrapper left, ExprWrapper right)
{
    return makeIntBinaryOp(Expr::LOGICAL_EQUAL, left.exprPtr, right.exprPtr);
}

ExprWrapper
operator!=(ExprWrapper left, ExprWrapper right)
{
    return makeIntBinaryOp(Expr::LOGICAL_NOT_EQUAL, left.exprPtr,
			   right.exprPtr);
}

ExprWrapper
operator<<(ExprWrapper left, ExprWrapper right)
{
    return makeIntBinaryOp(Expr::BITWISE_LEFT_SHIFT, left.exprPtr,
			   right.exprPtr);
}

} // namespace ulmdoc
