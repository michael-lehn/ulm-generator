#ifndef ULMDOC0_HDW_HPP
#define ULMDOC0_HDW_HPP

#include "expr.hpp"
#include "exprwrapper.hpp"

namespace ulmdoc {

enum
{
    ULM_ZF = 0,
    ULM_CF,
    ULM_SF,
    ULM_OF,
    ULM_NUM_STATUS_FLAGS,
};

extern ExprWrapper ulm_statusReg[ULM_NUM_STATUS_FLAGS];

Expr::ExprPtr ulm_readChar();

void ulm_printChar(ExprWrapper val);

void ulm_trap(ExprWrapper reg0, ExprWrapper reg1, ExprWrapper reg2);

void ulm_setReg(ExprWrapper val, ExprWrapper reg);

void ulm_ipSet(ExprWrapper val);

void ulm_add64(ExprWrapper a, ExprWrapper b, ExprWrapper dest);

void ulm_sub64(ExprWrapper a, ExprWrapper b, ExprWrapper dest);

void ulm_mul64(ExprWrapper a, ExprWrapper b, ExprWrapper dest);

void ulm_mul128(ExprWrapper a, ExprWrapper b, ExprWrapper dest0,
		ExprWrapper dest1);

void ulm_div128(ExprWrapper a, ExprWrapper b0, ExprWrapper b1,
		ExprWrapper dest0, ExprWrapper dest1, ExprWrapper dest2);

void ulm_idiv64(ExprWrapper a, ExprWrapper b, ExprWrapper dest);

void ulm_shiftLeft64(ExprWrapper a, ExprWrapper b, ExprWrapper dest);

void ulm_shiftRightSigned64(ExprWrapper a, ExprWrapper b, ExprWrapper dest);

void ulm_shiftRightUnsigned64(ExprWrapper a, ExprWrapper b, ExprWrapper dest);

void ulm_and64(ExprWrapper a, ExprWrapper b, ExprWrapper dest);

void ulm_or64(ExprWrapper a, ExprWrapper b, ExprWrapper dest);

void ulm_not64(ExprWrapper a, ExprWrapper dest);

void ulm_absJump(ExprWrapper addr, ExprWrapper retReg);

void ulm_unconditionalRelJump(ExprWrapper offset);

void ulm_conditionalRelJump(ExprWrapper condition, ExprWrapper offset);

void ulm_halt(ExprWrapper code);

extern ExprWrapper ULM_ZERO_EXT;
extern ExprWrapper ULM_SIGN_EXT;

void ulm_fetch64(ExprWrapper disp, ExprWrapper base, ExprWrapper index,
		 ExprWrapper scale, ExprWrapper ext, ExprWrapper numBytes,
		 ExprWrapper dest);

void ulm_store64(ExprWrapper disp, ExprWrapper base, ExprWrapper index,
		 ExprWrapper scale, ExprWrapper numBytes, ExprWrapper src);

} // namespace ulmdoc

#endif // ULMDOC0_HDW_HPP
