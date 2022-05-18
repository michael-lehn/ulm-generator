#include "expr.hpp"

#include <cassert>
#include <iomanip>
#include <sstream>

namespace ulmdoc {

bool
Expr::isBinaryExpr() const
{
    return std::holds_alternative<IntBinaryOp>(repr);
}

bool
Expr::isAdditiveBinaryExpr() const
{
    if (isBinaryExpr()) {
	switch (std::get<Expr::IntBinaryOp>(repr).op) {
	    case ADD:
	    case SUB:
		return true;
	    default:
		return false;
	}
    }
    return false;
}

bool
Expr::isLiteral() const
{
    return std::holds_alternative<Expr::IntLiteral>(repr);
}

bool
Expr::isLiteralZero() const
{
    return isLiteral() && std::get<Expr::IntLiteral>(repr).val == 0;
}

bool
Expr::isLiteralOne() const
{
    return isLiteral() && std::get<Expr::IntLiteral>(repr).val == 1;
}

bool
Expr::isZeroReg() const
{
    if (!std::holds_alternative<Expr::RegBitPattern>(repr)) {
	return false;
    }
    const auto &t = std::get<Expr::RegBitPattern>(repr);
    return t.exprPtr->isLiteralZero();
}

bool
Expr::isIntEnum(std::string sym) const
{
    if (!std::holds_alternative<Expr::IntEnum>(repr)) {
	return false;
    }
    const auto &t = std::get<Expr::IntEnum>(repr);
    return t.sym == sym;
}

std::string
Expr::binaryIntOp(enum BinaryIntOp op)
{
    switch (op) {
	case LOGICAL_EQUAL:
	    return "==";
	case LOGICAL_NOT_EQUAL:
	    return "!=";
	case LOGICAL_AND:
	    return "&&";
	case LOGICAL_OR:
	    return "||";
	case BITWISE_LEFT_SHIFT:
	    return "<<";
	default:
	    return std::string{ 1, char(op) };
    }
}

//------------------------------------------------------------------------------

Expr::ExprPtr
makeTypedBitPattern(std::string sym, Expr::BitPatternType bitPatternType,
		    std::size_t numBits)
{
    return std::make_shared<Expr>(
      Expr::TypedBitPattern{ sym, bitPatternType, numBits });
}

Expr::ExprPtr
makeRegBitPattern(const Expr::ExprPtr exprPtr)
{
    return std::make_shared<Expr>(Expr::RegBitPattern{ exprPtr });
}

Expr::ExprPtr
makeIntLiteral(std::int64_t val)
{
    return std::make_shared<Expr>(Expr::IntLiteral{ val });
}

Expr::ExprPtr
makeIntEnum(std::string sym)
{
    return std::make_shared<Expr>(Expr::IntEnum{ sym });
}

Expr::ExprPtr
makeIntBitPattern(const Expr::ExprPtr exprPtr)
{
    enum Expr::IntType type = Expr::U_INT;
    std::size_t numBits = 0;

    const auto &repr = exprPtr->repr;

    if (std::holds_alternative<Expr::TypedBitPattern>(repr)) {
	const auto &t = std::get<Expr::TypedBitPattern>(repr);
	assert(t.bitPatternType != Expr::JUMP_OFFSET);
	type = t.bitPatternType == Expr::UNSIGNED ? Expr::U_INT : Expr::S_INT;

	numBits = t.numBits;
    } else if (std::holds_alternative<Expr::RegBitPattern>(repr)) {
	const auto &t = std::get<Expr::RegBitPattern>(repr);

	numBits = t.numBits;
    } else {
	assert(0);
    }

    return std::make_shared<Expr>(
      Expr::IntBitPattern{ exprPtr, type, numBits });
}

Expr::ExprPtr
makeInt(const Expr::ExprPtr exprPtr)
{
    const auto &repr = exprPtr->repr;

    bool isBitPattern = std::holds_alternative<Expr::RegBitPattern>(repr) ||
			std::holds_alternative<Expr::TypedBitPattern>(repr);

    return isBitPattern ? makeIntBitPattern(exprPtr) : exprPtr;
}

Expr::ExprPtr
makeIntBinaryOp(enum Expr::BinaryIntOp op, const Expr::ExprPtr left,
		const Expr::ExprPtr right)
{
    auto x = makeInt(left);
    auto y = makeInt(right);

    return std::make_shared<Expr>(Expr::IntBinaryOp{ op, x, y });
}

//------------------------------------------------------------------------------

std::string
latex(const Expr::ExprPtr exprPtr)
{
    const auto &repr = exprPtr->repr;

    if (std::holds_alternative<Expr::TypedBitPattern>(repr)) {
	const auto &t = std::get<Expr::TypedBitPattern>(repr);

	return "\\mathit{" + t.sym + "}";
    } else if (std::holds_alternative<Expr::IntLiteral>(repr)) {
	const auto &t = std::get<Expr::IntLiteral>(repr);

	std::ostringstream ss;
	ss << t.val;

	return ss.str();
    } else if (std::holds_alternative<Expr::IntEnum>(repr)) {
	const auto &t = std::get<Expr::IntEnum>(repr);

	return t.sym;
    } else if (std::holds_alternative<Expr::RegBitPattern>(repr)) {
	const auto &t = std::get<Expr::RegBitPattern>(repr);

	if (t.exprPtr->isBinaryExpr()) {
	    return "\\%\\{" + latex(t.exprPtr) + "\\}";
	}
	return "\\%" + latex(t.exprPtr);
    } else if (std::holds_alternative<Expr::IntBitPattern>(repr)) {
	const auto &t = std::get<Expr::IntBitPattern>(repr);

	return (t.type == Expr::U_INT ? "u(" : "s(") + latex(t.exprPtr) + ")";
    } else if (std::holds_alternative<Expr::IntBinaryOp>(repr)) {
	const auto &t = std::get<Expr::IntBinaryOp>(repr);

	switch (t.op) {
	    case Expr::ADD:
		return latex(t.left) + " + " + latex(t.right);
	    case Expr::SUB:
		return latex(t.left) + " - " + latex(t.right);
	    case Expr::MUL:
		return latex(t.left) + " \\cdot " + latex(t.right);
	    case Expr::DIV:
		return "\\left\\lfloor\\frac{" + latex(t.left) + "}{" +
		       latex(t.right) + "}\\right\\rfloor_0";
	    case Expr::MOD:
		return latex(t.left) + "\\bmod " + latex(t.right);
	    case Expr::BITWISE_AND:
		return latex(t.left) + " \\land_b " + latex(t.right);
	    case Expr::BITWISE_OR:
		return latex(t.left) + " | " + latex(t.right);
	    case Expr::BITWISE_LEFT_SHIFT:
		return latex(t.left) + " << " + latex(t.right);
	    case Expr::LOGICAL_EQUAL:
		return latex(t.left) + " = " + latex(t.right);
	    case Expr::LOGICAL_NOT_EQUAL:
		return latex(t.left) + " \\neq " + latex(t.right);
	    case Expr::LOGICAL_AND:
		return latex(t.left) + " \\land " + latex(t.right);
	    case Expr::LOGICAL_OR:
		return latex(t.left) + " \\lor " + latex(t.right);
	}
    }
    return "?";
}

void
printTree(std::ostream &out, std::size_t level, const Expr::ExprPtr exprPtr)
{
    out << std::setw(level * 4) << ' ';

    const auto &repr = exprPtr->repr;

    if (std::holds_alternative<Expr::TypedBitPattern>(repr)) {
	const auto &t = std::get<Expr::TypedBitPattern>(repr);

	out << t.sym;
	out << "[" << t.numBits << char(t.bitPatternType) << "]";
    } else if (std::holds_alternative<Expr::IntLiteral>(repr)) {
	const auto &t = std::get<Expr::IntLiteral>(repr);

	out << "Literal";
	out << "[" << t.val << "]";

    } else if (std::holds_alternative<Expr::IntEnum>(repr)) {
	const auto &t = std::get<Expr::IntEnum>(repr);

	out << "Enum";
	out << "[" << t.sym << "]";

    } else if (std::holds_alternative<Expr::RegBitPattern>(repr)) {
	const auto &t = std::get<Expr::RegBitPattern>(repr);

	out << "Register";
	out << "[" << t.numBits << "]";
	out << std::endl;
	printTree(out, level + 1, t.exprPtr);
    } else if (std::holds_alternative<Expr::IntBitPattern>(repr)) {
	const auto &t = std::get<Expr::IntBitPattern>(repr);

	out << (t.type == Expr::U_INT ? "unsigned" : "signed");
	out << "[" << t.numBits << "]";
	out << std::endl;

	printTree(out, level + 1, t.exprPtr);
    } else if (std::holds_alternative<Expr::IntBinaryOp>(repr)) {
	const auto &t = std::get<Expr::IntBinaryOp>(repr);

	out << "binary operation: " << Expr::binaryIntOp(t.op);
	out << std::endl;
	printTree(out, level + 1, t.left);
	out << std::endl;
	printTree(out, level + 1, t.right);
    }
}

} // namespace ulmdoc
