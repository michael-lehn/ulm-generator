#include <iostream>
#include <sstream>

#include "expr.hpp"
#include "hdw.hpp"
#include "latex.hpp"
#include "ulmdoc.hpp"

namespace ulmdoc {

ExprWrapper ulm_statusReg[ULM_NUM_STATUS_FLAGS] = {
    makeIntEnum("ZF"),
    makeIntEnum("CF"),
    makeIntEnum("SF"),
    makeIntEnum("OF"),
};

Expr::ExprPtr
ulm_readChar()
{
    return makeTypedBitPattern("\\text{\\textit{ulm\\_readChar()}}",
			       Expr::SIGNED, 16);
}

void
ulm_printChar(ExprWrapper val)
{
    std::ostringstream ss;
    ss << "\\[" << std::endl;
    ss << "\\text{\\textit{ulm\\_printChar}($" << latex(val) << "$)}"
       << std::endl;
    ss << "\\]" << std::endl;
    ulmDoc.addDescription(ss.str());
}

void
ulm_trap(ExprWrapper reg0, ExprWrapper reg1, ExprWrapper reg2)
{
    std::ostringstream ss;
    ss << "TODO: ulm\\_trap" << std::endl;
    ulmDoc.addDescription(ss.str());
}

void
ulm_setReg(ExprWrapper val, ExprWrapper reg)
{
    std::ostringstream ss;
    ss << "\\[" << std::endl;
    ss << truncateInt(val, 64) << writeToUnsignedReg(reg);
    ss << "\\]" << std::endl;

    ulmDoc.addDescription(ss.str());
}

void
ulm_ipSet(ExprWrapper val)
{
    std::ostringstream ss;
    ss << "TODO: ulm\\_ipSet" << std::endl;
    ulmDoc.addDescription(ss.str());
}

void
ulm_add64(ExprWrapper a, ExprWrapper b, ExprWrapper dest)
{
    auto str = truncateInt(b + a, 64) + writeToUnsignedReg(dest);
    std::ostringstream ss;
    ss << "\\[" << str << "\\]" << std::endl;
    ss << "Updates the status flags:\\\\[0cm]" << std::endl;
    ss << "\\begin{tabular}{ll}" << std::endl;
    ss << "Flag & Condition \\\\[0cm]" << std::endl;
    ss << "ZF & $" << latex(b + a == 0) << "$\\\\[0cm]" << std::endl;
    ss << "CF & $" << latex(b + a) << "\\geq 2^{64}$\\\\[0cm]" << std::endl;
    ss << "OF & $ s(" << latex(b) << ") + s(" << latex(a) << ") \\notin "
       << "\\{ -2^{63}, \\dots, 2^{63}\\}$ \\\\[0cm]" << std::endl;
    ss << "SF & $ s(" << latex(b) << ") + s(" << latex(a) << ") < 0$\\\\[0cm]"
       << std::endl;
    ss << "\\end{tabular}" << std::endl;
    ss << "\\vspace{-0.5cm}" << std::endl;
    ulmDoc.addDescription(ss.str());
}

void
ulm_sub64(ExprWrapper a, ExprWrapper b, ExprWrapper dest)
{
    auto str = truncateInt(b - a, 64) + writeToUnsignedReg(dest);
    std::ostringstream ss;
    ss << "\\[" << str << "\\]" << std::endl;
    ss << "Updates the status flags:\\\\[0.1cm]" << std::endl;
    ss << "\\begin{tabular}{ll}" << std::endl;
    ss << "Flag & Condition \\\\[0.1cm]" << std::endl;
    ss << "ZF & $" << latex(b - a == 0) << "$\\\\[0.1cm]" << std::endl;
    ss << "CF & $" << latex(b - a) << "< 0$\\\\[0.1cm]" << std::endl;
    ss << "OF & $ s(" << latex(b) << ") - s(" << latex(a) << ") \\notin "
       << "\\{ -2^{63}, \\dots, 2^{63}\\}$ \\\\[0.1cm]" << std::endl;
    ss << "SF & $ s(" << latex(b) << ") - s(" << latex(a) << ") < 0$\\\\[0.1cm]"
       << std::endl;
    ss << "\\end{tabular}" << std::endl;
    ulmDoc.addDescription(ss.str());
}

void
ulm_mul64(ExprWrapper a, ExprWrapper b, ExprWrapper dest)
{
    auto str = truncateInt(a * b, 64) + writeToUnsignedReg(dest);
    std::ostringstream ss;
    ss << "\\[" << str << "\\]" << std::endl;
    ulmDoc.addDescription(ss.str());
}

void
ulm_mul128(ExprWrapper a, ExprWrapper b, ExprWrapper dest0, ExprWrapper dest1)
{
    std::ostringstream ss;
    ss << "TODO: ulm\\_mul128" << std::endl;
    ulmDoc.addDescription(ss.str());
}

void
ulm_div128(ExprWrapper a, ExprWrapper b0, ExprWrapper b1, ExprWrapper dest0,
	   ExprWrapper dest1, ExprWrapper dest2)
{
    std::ostringstream ss;
    ss << "For the unsigned 128-bit numerator" << std::endl;
    ss << "\\[" << std::endl;
    ss << "b := u\\left(" << latex(b1) << latex(b0) << "\\right)";
    ss << "\\]" << std::endl;

    ss << "and unsigned 64-bit denominator" << std::endl;
    ss << "\\[" << std::endl;
    ss << "a := " << latex(makeInt(a.exprPtr));
    ss << "\\]" << std::endl;

    ss << "computes the divisor" << std::endl;
    ss << "\\[" << std::endl;
    ss << "\\left\\lfloor\\frac{b}{a}\\right\\rfloor \\bmod 2^{128}";
    ss << "\\to u\\left(" << latex(ulm_regVal(dest1))
       << latex(ulm_regVal(dest0)) << "\\right)" << std::endl;
    ss << "\\]" << std::endl;

    ss << "and the remainder" << std::endl;
    ss << "\\[" << std::endl;
    ss << "b \\bmod a \\to u\\left(" << latex(ulm_regVal(dest2)) << "\\right)"
       << std::endl;
    ss << "\\]" << std::endl;

    ulmDoc.addDescription(ss.str());
}

void
ulm_idiv64(ExprWrapper a, ExprWrapper b, ExprWrapper dest)
{
    std::ostringstream ss;
    ss << "Computes the divisor" << std::endl;
    ss << "\\[" << std::endl;
    ss << latex(b / a) + writeToUnsignedReg(dest);
    ss << "\\]" << std::endl;
    ss << "and the remainder" << std::endl;
    ss << "\\[" << std::endl;
    ss << latex(b % a) + writeToUnsignedReg(dest);
    ss << "\\]" << std::endl;
    ulmDoc.addDescription(ss.str());
}

void
ulm_shiftLeft64(ExprWrapper a, ExprWrapper b, ExprWrapper dest)
{
    std::ostringstream ss;
    ss << "TODO: ulm\\_shiftLeft64" << std::endl;
    ulmDoc.addDescription(ss.str());
}

void
ulm_shiftRightSigned64(ExprWrapper a, ExprWrapper b, ExprWrapper dest)
{
    std::ostringstream ss;
    ss << "TODO: ulm\\_shiftRightSigned64" << std::endl;
    ulmDoc.addDescription(ss.str());
}

void
ulm_shiftRightUnsigned64(ExprWrapper a, ExprWrapper b, ExprWrapper dest)
{
    std::ostringstream ss;
    ss << "TODO: ulm\\_shiftRightUnsigned64" << std::endl;
    ulmDoc.addDescription(ss.str());
}

void
ulm_and64(ExprWrapper a, ExprWrapper b, ExprWrapper dest)
{
    auto str = latex(a & b) + writeToUnsignedReg(dest);
    std::ostringstream ss;
    ss << "\\[" << str << "\\]" << std::endl;
    ulmDoc.addDescription(ss.str());
}

void
ulm_or64(ExprWrapper a, ExprWrapper b, ExprWrapper dest)
{
    std::ostringstream ss;
    ss << "TODO: ulm\\_or64" << std::endl;
    ulmDoc.addDescription(ss.str());
}

void
ulm_not64(ExprWrapper a, ExprWrapper dest)
{
    std::ostringstream ss;
    ss << "TODO: ulm\\_not64" << std::endl;
    ulmDoc.addDescription(ss.str());
}

void
ulm_absJump(ExprWrapper addr, ExprWrapper retReg)
{
    std::ostringstream ss;
    ss << "\\[\\begin{array}{lcl}" << std::endl;
    ss << "\\left( u(\\%\\mathit{IP}) + 4 \\right) \\bmod 2^{64} & \\to & u(\\%"
       << latex(retReg) << ")\\\\[0.2cm]" << std::endl;
    ss << "u(" << latex(addr) << ") & \\to & u(\\%\\mathit{IP}) \\\\"
       << std::endl;
    ss << "\\end{array}\\]" << std::endl;
    ulmDoc.addDescription(ss.str());
}

void
ulm_unconditionalRelJump(ExprWrapper offset)
{
    std::ostringstream ss;
    ss << "\\[" << std::endl;
    ss << "\\left(u(\\%\\mathit{IP}) + s(" << latex(offset)
       << ")\\right) \\bmod 2^{64} \\to u(\\%\\mathit{IP})" << std::endl;
    ss << "\\]" << std::endl;
    ulmDoc.addDescription(ss.str());
}

void
ulm_conditionalRelJump(ExprWrapper condition, ExprWrapper offset)
{
    std::ostringstream ss;
    ss << "If the condition" << std::endl;
    ss << "\\[" << std::endl;
    ss << latex(condition) << std::endl;
    ss << "\\]" << std::endl;
    ss << "evaluates to true then" << std::endl;
    ss << "\\[" << std::endl;
    ss << "\\left(u(\\%\\mathit{IP}) + s(" << latex(offset)
       << ")\\right) \\bmod 2^{64} \\to u(\\%\\mathit{IP})" << std::endl;
    ss << "\\]" << std::endl;
    ulmDoc.addDescription(ss.str());
}

void
ulm_halt(ExprWrapper code)
{
    std::ostringstream ss;
    ss << "\\[" << std::endl;
    ss << "\\text{halt program execution with exit code $"
       << truncateInt(code, 8) << "$}" << std::endl;
    ss << "\\]" << std::endl;

    ulmDoc.addDescription(ss.str());
}

ExprWrapper ULM_ZERO_EXT = makeIntEnum("ULM_ZERO_EXT");
ExprWrapper ULM_SIGN_EXT = makeIntEnum("ULM_SIGN_EXT");

void
ulm_fetch64(ExprWrapper disp, ExprWrapper base, ExprWrapper index,
	    ExprWrapper scale, ExprWrapper ext, ExprWrapper numBytes,
	    ExprWrapper dest)
{
    ExprWrapper addr = ulm_regVal(base);
    if (!index.isLiteralZero()) {
	ExprWrapper indexReg = ulm_regVal(index);
	if (!scale.isLiteralOne()) {
	    indexReg = indexReg * scale;
	}
	addr = addr + indexReg;
    }
    if (!disp.isLiteralZero()) {
	addr = disp + addr;
    }

    std::ostringstream ss;
    ss << "\\[" << std::endl;
    if (ext.isIntEnum("ULM_ZERO_EXT")) {
	ss << "u\\left(\\text{M}_{" << latex(numBytes) << "}"
	   << "\\left(\\mathit{addr}\\right)\\right)"
	   << writeToUnsignedReg(dest) << std::endl;
    } else {
	ss << "s\\left(\\text{M}_{" << latex(numBytes) << "}"
	   << "\\left(\\mathit{addr}\\right)\\right)" << writeToSignedReg(dest)
	   << std::endl;
    }

    ss << "\\;\\text{with}\\;\\mathit{addr} = " << truncateInt(addr, 64)
       << std::endl;
    ss << "\\]" << std::endl;

    ulmDoc.addDescription(ss.str());
}

void
ulm_store64(ExprWrapper disp, ExprWrapper base, ExprWrapper index,
	    ExprWrapper scale, ExprWrapper numBytes, ExprWrapper src)
{
    ExprWrapper addr = ulm_regVal(base);
    if (!index.isLiteralZero()) {
	ExprWrapper indexReg = ulm_regVal(index);
	if (!scale.isLiteralOne()) {
	    indexReg = indexReg * scale;
	}
	addr = addr + indexReg;
    }
    if (!disp.isLiteralZero()) {
	addr = disp + addr;
    }

    std::ostringstream ss;
    ss << "\\[" << std::endl;

    if (numBytes.isLiteral()) {
	ss << readFromUnsignedReg(src, numBytes.getLiteral());
    } else {
	ss << "\\text{TODO: case where <numBytes> is an expression\\\\}";
    }

    ss << "u\\left(\\text{M}_{" << latex(numBytes) << "}"
       << "\\left(\\mathit{addr}\\right)\\right)" << std::endl;
    ss << "\\;\\text{with}\\;\\mathit{addr} = " << truncateInt(addr, 64)
       << std::endl;
    ss << "\\]" << std::endl;

    ulmDoc.addDescription(ss.str());
}

} // namespace ulmdoc

