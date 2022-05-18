#ifndef ULMDOC0_LATEX_HPP
#define ULMDOC0_LATEX_HPP

#include <cstddef>
#include "exprwrapper.hpp"

namespace ulmdoc {

std::string truncateInt(ExprWrapper val, std::size_t bits);
std::string unsignedReg(ExprWrapper dest);
std::string signedReg(ExprWrapper dest);
std::string readFromUnsignedReg(ExprWrapper dest, std::size_t numBytes);
std::string writeToUnsignedReg(ExprWrapper dest);
std::string writeToSignedReg(ExprWrapper dest);

} // namespace ulmdoc

#endif // ULMDOC0_LATEX_HPP

