#include <cinttypes>

#include "udb_cpp.hpp"

#include "cpuview.hpp"

using namespace finalcut;

//-- CpuView -----------------------------------------------------------------

CpuView::CpuView(FWidget *parent)
  : FDialog{ parent }
  , cuView{ this }
  , aluView{ this }
  , statusView{ this }
{
    auto fgColor = getForegroundColor();
    auto bgColor = getBackgroundColor();

    setBorder(false);
    setSize(FSize{ 50, 23 });

    cuView.setGeometry(FPoint{ 0, 1 }, FSize{ getWidth(), 5 });
    cuView.setFocusable();
    cuView.setForegroundColor(fgColor);
    cuView.setBackgroundColor(bgColor);

    aluView.setGeometry(FPoint{ 0, 6 }, FSize{ getWidth(), 12 });
    aluView.setFocusable();
    statusView.setGeometry(FPoint{ 0, 19 }, FSize{ getWidth(), 1 });
    notify();
}

void
CpuView::notify()
{
    FString val;
    val.sprintf("CPU (After cycle #%" PRIu64 ")", udb_getNumCycles());
    setText(val);
    redraw();

    aluView.notify();
    cuView.notify();
    statusView.notify();
}

//-- CpuView::CuView ---------------------------------------------------------

CpuView::CuView::CuView(CpuView *cpu)
  : FScrollView{ cpu }
  , ip{ this }
  , ir{ this }
  , asmCode{ this }
  , stage{ this }
{
    setText("Control Unit");
    ip.setGeometry(FPoint{ 6, 1 }, FSize{ 18, 1 });
    ip.setLabelText("%IP");
    ip.setText("0000000000000000");
    ip.setShadow(false);

    ir.setGeometry(FPoint{ 30, 1 }, FSize{ 10, 1 });
    ir.setLabelText("%IR");
    ir.setText("00000000");
    ir.setShadow(false);

    asmCode.setGeometry(FPoint{ 6, 3 }, FSize{ 42, 1 });
    asmCode.setLabelText("ASM");
    asmCode.setShadow(false);
    asmCode.setReadOnly();
    asmCode.setDisable();

    stage.setText("Stage D");
    stage.setGeometry(FPoint{ 41, 1 }, FSize{ 7, 1 });
}

void
CpuView::CuView::setGeometry(const FPoint &p, const FSize &s, bool adj)
{
    FScrollView::setGeometry(p, s, adj);
    setScrollWidth(46);
}

void
CpuView::CuView::notify()
{
    FString val;

    val.sprintf("%016" PRIX64, ulm_instrPtr);
    ip.setText(val);
    ip.redraw();

    val.sprintf("%08" PRIX32, ulm_instrReg);
    ir.setText(val);
    ir.redraw();

    char asmStr[40];
    ulm_asm(ulm_instrReg, asmStr, sizeof(asmStr));
    asmCode.setText(asmStr);
    asmCode.redraw();
}

//-- CpuView::AluView ----------------------------------------------------------

CpuView::AluView::AluView(CpuView *cpu)
  : FScrollView{ cpu }
{
    setText("ALU Registers");

    for (int col = 0; col < regCols; ++col) {
	for (int row = 0; row < regRows; ++row) {
	    int i = row + col * regRows;
	    FString label, val;
	    label.sprintf("%%%02X", i);

	    uint64_t regVal = i == 0 ? 0 : *ulm_regDevice(i);
	    val.sprintf("%016" PRIX64, regVal);

	    reg[i] = std::make_unique<FLineEdit>(val, this);
	    reg[i]->setGeometry(
	      FPoint{ col * (regWidth + regSep) + regSep, row + 2 },
	      FSize{ (std::size_t)regWidth, 1 });
	    reg[i]->setLabelText(label);
	    reg[i]->setShadow(false);
	}
    }
    defaultForegroundColor = reg[0]->getForegroundColor();
    defaultBackgoundColor = reg[0]->getBackgroundColor();
    reg[0]->setReadOnly();
    reg[0]->setDisable();
}

void
CpuView::AluView::setGeometry(const FPoint &p, const FSize &s, bool adj)
{
    FScrollView::setGeometry(p, s, adj);
    setScrollWidth((regWidth + regSep) * regCols);
}

void
CpuView::AluView::notify()
{
    for (auto r : udb_reg[udb_lastMsg]) {
	int id = int(r.first);
	reg[id]->setForegroundColor(defaultForegroundColor);
	reg[id]->setBackgroundColor(defaultBackgoundColor);
	reg[id]->redraw();
    }

    for (auto r : udb_reg[udb_activeMsg]) {
	int id = int(r.first);

	FString val;
	val.sprintf("%016" PRIX64, ulm_regVal(id));
	reg[id]->setText(val);

	if (r.second & UDB_WRITE) {
	    reg[id]->setForegroundColor(FColor::White);
	    reg[id]->setBackgroundColor(FColor::Red);
	} else if (r.second & UDB_READ) {
	    reg[id]->setForegroundColor(FColor::White);
	    reg[id]->setBackgroundColor(FColor::Green);
	} else {
	    // TODO: write to log that we have an unhandled case in AluView
	}
	reg[id]->redraw();
    }
}

//-- CpuView::Status View ------------------------------------------------------

CpuView::StatusView::StatusView(CpuView *cpu)
  : FScrollView{ cpu }
  , zf{ this }
  , cf{ this }
  , of{ this }
  , sf{ this }
{
    setText("Status Registers");
    zf.setLabelText(" Zero Flag");
    cf.setLabelText("Carry Flag");
    of.setLabelText("Overflow Flag");
    sf.setLabelText("    Sign Flag");
    zf.setText("0");
    cf.setText("0");
    of.setText("0");
    sf.setText("0");
    zf.setShadow(false);
    cf.setShadow(false);
    of.setShadow(false);
    sf.setShadow(false);
    zf.setDisable();
    cf.setDisable();
    of.setDisable();
    sf.setDisable();
    zf.setGeometry(FPoint{ 13, 1 }, FSize{ 3, 1 });
    cf.setGeometry(FPoint{ 13, 2 }, FSize{ 3, 1 });
    of.setGeometry(FPoint{ 40, 1 }, FSize{ 3, 1 });
    sf.setGeometry(FPoint{ 40, 2 }, FSize{ 3, 1 });
}

void
CpuView::StatusView::setGeometry(const FPoint &p, const FSize &s, bool adj)
{
    FScrollView::setGeometry(p, s, adj);
    setScrollWidth(46);
}

void
CpuView::StatusView::notify()
{
    if (ulm_statusReg[ULM_ZF]) {
	zf.setText("1");
    } else {
	zf.setText("0");
    }
    if (ulm_statusReg[ULM_CF]) {
	cf.setText("1");
    } else {
	cf.setText("0");
    }
    if (ulm_statusReg[ULM_SF]) {
	sf.setText("1");
    } else {
	sf.setText("0");
    }
    if (ulm_statusReg[ULM_OF]) {
	of.setText("1");
    } else {
	of.setText("0");
    }
    zf.redraw();
    cf.redraw();
    sf.redraw();
    of.redraw();
}
