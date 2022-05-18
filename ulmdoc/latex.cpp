#include <sstream>

#include "latex.hpp"

namespace ulmdoc {

std::string
truncateInt(ExprWrapper val, std::size_t bits)
{
    auto intVal = makeInt(val.exprPtr);
    auto str = latex(intVal);
    if (intVal->isAdditiveBinaryExpr()) {
	str = "\\left(" + str + "\\right)";
    }

    std::ostringstream exp;
    exp << bits;

    return str + " \\bmod 2^{" + exp.str() + "}";
}

std::string
unsignedReg(ExprWrapper dest)
{
    return "u\\left(" + latex(ulm_regVal(dest)) + "\\right)";
}

std::string
signedReg(ExprWrapper dest)
{
    return "s\\left(" + latex(ulm_regVal(dest)) + "\\right)";
}

std::string
readFromUnsignedReg(ExprWrapper dest, std::size_t numBytes)
{
    return truncateInt(ulm_regVal(dest), 8 * numBytes) + " \\to ";
}

std::string
writeToUnsignedReg(ExprWrapper dest)
{
    return " \\to " + unsignedReg(dest);
}

std::string
writeToSignedReg(ExprWrapper dest)
{
    return " \\to " + unsignedReg(dest);
}

} // namespace ulmdoc
