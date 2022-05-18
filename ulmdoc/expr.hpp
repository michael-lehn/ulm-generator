#ifndef ULMDOC0_EXPR_HPP
#define ULMDOC0_EXPR_HPP

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>

namespace ulmdoc {

static constexpr std::size_t ulm_numRegsExp = 8;
static constexpr std::size_t ulm_numRegs = 1 << ulm_numRegsExp;

class ExprWrapper;

class Expr
{
  public:
    enum IntType
    {
	U_INT,
	S_INT,
    };

    enum BitPatternType
    {
	UNSIGNED = 'u',
	SIGNED = 's',
	JUMP_OFFSET = 'j',
    };

    enum UnaryOp
    {
	REGISTER,
	INT,
    };

    enum BinaryIntOp
    {
	ADD = '+',
	SUB = '-',
	MUL = '*',
	DIV = '/',
	MOD = '%',
	BITWISE_AND = '&',
	BITWISE_OR = '|',
	BITWISE_LEFT_SHIFT,
	LOGICAL_EQUAL,
	LOGICAL_NOT_EQUAL,
	LOGICAL_AND,
	LOGICAL_OR,
    };

    bool isBinaryExpr() const;
    bool isAdditiveBinaryExpr() const;
    bool isLiteral() const;
    bool isLiteralZero() const;
    bool isLiteralOne() const;
    bool isZeroReg() const;
    bool isIntEnum(std::string sym) const;

    using ExprPtr = std::shared_ptr<Expr>;

    friend class ExprWrapper;
    friend class InstrFmt;
    friend ExprPtr makeTypedBitPattern(std::string sym,
				       BitPatternType bitPatternType,
				       std::size_t numBits);
    friend ExprPtr makeRegBitPattern(const ExprPtr);
    friend ExprPtr makeIntLiteral(std::int64_t);
    friend ExprPtr makeIntEnum(std::string);
    friend ExprPtr makeInt(const ExprPtr);
    friend ExprPtr makeIntBitPattern(const ExprPtr);
    friend ExprPtr makeIntBinaryOp(enum BinaryIntOp, const ExprPtr,
				   const ExprPtr);
    friend std::string latex(const ExprPtr);
    friend void printTree(std::ostream &, std::size_t, const ExprPtr);

    template<typename T>
    Expr(const T &repr)
      : repr{ repr }
    {
    }

  private:
    std::string static binaryIntOp(enum BinaryIntOp op);

    struct TypedBitPattern
    {
	std::string sym;
	BitPatternType bitPatternType;
	std::size_t numBits;
    };


    struct IntLiteral
    {
	std::int64_t val;
    };

    struct IntEnum
    {
	std::string sym;
    };

    struct RegBitPattern
    {
	/*
	 * bit pattern with at most ulm_numRegsExp bits, or
	 * signed/unsigned integer in corresponding range
	 */
	const ExprPtr exprPtr;
	std::size_t numBits{ 64 };
    };

    struct IntBitPattern
    {
	/*
	 * TypedBitPattern or RegBitPattern
	 */
	const ExprPtr exprPtr;
	enum IntType type;
	std::size_t numBits;
    };

    struct IntBinaryOp
    {
	enum BinaryIntOp op;
	const ExprPtr left, right;
    };

    std::variant<TypedBitPattern, IntLiteral, IntEnum, RegBitPattern,
		 IntBitPattern, IntBinaryOp>
      repr;
};

//------------------------------------------------------------------------------

Expr::ExprPtr makeTypedBitPattern(std::string sym,
				  Expr::BitPatternType bitPatternType,
				  std::size_t numBits);
Expr::ExprPtr makeRegBitPattern(const Expr::ExprPtr exprPtr);
Expr::ExprPtr makeIntLiteral(std::int64_t val);
Expr::ExprPtr makeIntEnum(std::string sym);
Expr::ExprPtr makeInt(const Expr::ExprPtr exprPtr);
Expr::ExprPtr makeIntBitPattern(const Expr::ExprPtr exprPtr);
Expr::ExprPtr makeIntBinaryOp(enum Expr::BinaryIntOp op,
			      const Expr::ExprPtr left,
			      const Expr::ExprPtr right);

//------------------------------------------------------------------------------

std::string latex(const Expr::ExprPtr);

void printTree(std::ostream &out, std::size_t level,
	       const Expr::ExprPtr exprPtr);

} // namespace ulmdoc

#endif // ULMDOC0_EXPR_HPP
