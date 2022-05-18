#ifndef ULMDOC0_INSTRFMT_HPP
#define ULMDOC0_INSTRFMT_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "expr.hpp"

namespace ulmdoc {

class InstrFmt
{
  public:
    InstrFmt(std::vector<ExprWrapper> fmtField);

    std::string tikz(std::size_t opCode);

  private:
    std::string tikzHeader() const;
    std::string tikzFooter() const;
    std::string tikzCell(std::size_t numBits, std::size_t offset,
			 std::string label, bool used = true) const;
    std::string tikzCellOffset(std::size_t offset) const;

    std::vector<std::string> sym;
    std::vector<std::size_t> size, offset;
    std::vector<Expr::BitPatternType> type;

    double scale = 0.4;
};

} // namespace ulmdoc

#endif // ULMDOC0_INSTRFMT_HPP
