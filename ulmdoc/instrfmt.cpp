#include <iostream>
#include <sstream>
#include <stdexcept>

#include "expr.hpp"
#include "exprwrapper.hpp"
#include "instrfmt.hpp"

namespace ulmdoc {

InstrFmt::InstrFmt(std::vector<ExprWrapper> fmtField)
  : sym(fmtField.size())
  , size(fmtField.size())
  , offset(fmtField.size())
  , type(fmtField.size())
{
    for (std::size_t i = 0; i < fmtField.size(); ++i) {
	auto &repr = fmtField[i].exprPtr->repr;
	if (!std::holds_alternative<Expr::TypedBitPattern>(repr)) {
	    throw std::logic_error("field of an instruction format has to be "
				   "of type Expr::TypedBitPattern");
	}
	auto &t = std::get<Expr::TypedBitPattern>(repr);

	sym[i] = t.sym;
	size[i] = t.numBits;
	offset[i] = (i == 0 ? 32 : offset[i - 1]) - size[i];
	type[i] = t.bitPatternType;
    }
}

std::string
InstrFmt::tikz(std::size_t opCode)
{
    std::string str = tikzHeader();
    for (std::size_t i = 0; i < sym.size(); ++i) {
	std::ostringstream label;
	if (i == 0) {
	    if (size[i] % 4 == 0) {
		label << "0x";
		label.width(size[i]/4);
		label.fill('0');
		label << std::hex << std::uppercase << opCode;
	    } else {
		for (std::size_t j = size[i]; j-- > 0;) {
		    label << ((opCode & (1 << j)) >> j);
		}
	    }
	} else {
	    label << sym[i];
	    if (type[i] == Expr::JUMP_OFFSET) {
		label << "\\gg 2";
	    }
	}
	str += tikzCell(size[i], offset[i], label.str());
    }
    if (offset.back() != 0) {
	str += tikzCell(offset.back(), 0, "", false);
    }
    return str + tikzFooter();
}

std::string
InstrFmt::tikzHeader() const
{
    std::ostringstream ss;
    ss << "\\[\\begin{tikzpicture}[scale=0.6]" << std::endl;
    return ss.str();
}

std::string
InstrFmt::tikzFooter() const
{
    std::ostringstream ss;
    ss << "\\end{tikzpicture}\\]" << std::endl;
    return ss.str();
}

std::string
InstrFmt::tikzCell(std::size_t numBits, std::size_t offset,
		   std::string label, bool used) const
{
    std::ostringstream ss;
    ss << "\\filldraw[fill=" << (used ? "white" : "gray!30") << "]" << std::endl
       << " ({ " << scale * (32 - offset - numBits) << "}, {1}) --" << std::endl
       << " ({ " << scale * (32 - offset - numBits) << "}, {0}) --" << std::endl
       << " ({ " << scale * (32 - offset) << "}, {0}) --" << std::endl
       << " ({ " << scale * (32 - offset) << "}, {1}) --" << std::endl
       << " cycle;" << std::endl;
    ss << "\\draw ({" << scale * (32 - offset - numBits / 2.)
       << "}, {0.5}) node {" << label << "};" << std::endl;
    ss << tikzCellOffset(offset);
    if (offset + numBits == 32) {
	ss << tikzCellOffset(32);
    }
    return ss.str();
}

std::string
InstrFmt::tikzCellOffset(std::size_t offset) const
{
    std::ostringstream ss;
    ss << "\\draw ({" << scale * (32 - offset) << "}, {1}) --" << std::endl
       << " ({" << scale * (32 - offset) << "}, {1.2}) node[above] {" << offset
       << "};" << std::endl;
    return ss.str();
}

} // namespace ulmdoc
