#include <cinttypes>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "codeview.hpp"
#include "udb_cpp.hpp"

using namespace finalcut;

CodeView::CodeView(finalcut::FWidget *parent)
  : FDialog{ parent }
{
    setResizeable();
    scrolltext.ignorePadding();
    scrolltext.setFocus();

    FString instrStr;
    char asmStr[40];

    FTextView::FTextViewList::size_type line = 0;
    for (uint64_t addr = 0; addr < ulm_brk; addr += 4, ++line) {
	uint32_t instr;
	ulm_fetch32(addr, &instr);
	ulm_asm(instr, asmStr, sizeof(asmStr));

	instrStr.sprintf("    0x%016" PRIX64 ": %02X %02X %02X %02X  # %s\n",
			 addr, int(instr >> 24 & 0xFF), int(instr >> 16 & 0xFF),
			 int(instr >> 8 & 0xFF), int(instr >> 0 & 0xFF),
			 asmStr);
	scrolltext.append(instrStr);
	auto &highlight = scrolltext.getLine(line).highlight;
	highlight = { hideBP, normInstr };
	if (udb_breakpoint.find(addr) != udb_breakpoint.end()) {
	    highlight[0] = showBP;
	    scrolltext.getLine(line).text.wc_str()[1] = 'B';
	}
    }
    notify();
}

void
CodeView::notify()
{
    update(true);
}

void
CodeView::update(bool scrollToIP)
{
    static std::uint64_t prevInstrPtr, currInstrPtr;

    scrolltext.getLine(prevInstrPtr).highlight[1] = normInstr;
    scrolltext.getLine(currInstrPtr).highlight[1] = normInstr;
    prevInstrPtr = udb_getPrevIP() / 4;
    currInstrPtr = udb_getIP() / 4;

    const auto &l = scrolltext.getLines();
    if (currInstrPtr >= l.size()) {
	return;
    }

    scrolltext.getLine(prevInstrPtr).highlight[1] = prevInstr;
    scrolltext.getLine(currInstrPtr).highlight[1] = currInstr;

    int offsetIP = currInstrPtr - scrolltext.getScrollPos().getY();
    if (scrollToIP &&
	(offsetIP < 0 ||
	 offsetIP > int(scrolltext.getTextVisibleSize().getHeight())))
    {
	scrolltext.scrollBy(0, offsetIP);
    } else {
	scrolltext.redraw();
    }
}

void
CodeView::initLayout()
{
    FDialog::setText("Program (Stage D: %IP points to next instruction)");
    setMinimumSize(FSize{ 51, 6 });
    auto x = 1 + int((getDesktopWidth() - 72));
    auto y = 1;
    auto window_size = FSize{ 72, getDesktopHeight() - y -1 };
    FDialog::setGeometry(FPoint{ x, y }, window_size);
    scrolltext.setGeometry(FPoint{ 1, 2 },
			   FSize{ getWidth(), getHeight() - 1 });
    FDialog::initLayout();
}

void
CodeView::adjustSize()
{
    finalcut::FDialog::adjustSize();

    if (!isZoomed()) {
	auto desktop_size = FSize{ getDesktopWidth(), getDesktopHeight() };
	FRect screen(FPoint{ 1, 1 }, desktop_size);
	bool center = false;

	if (getWidth() > getDesktopWidth()) {
	    setWidth(getDesktopWidth() * 9 / 10);
	    center = true;
	}

	if (getHeight() > getDesktopHeight()) {
	    setHeight(getDesktopHeight() * 7 / 8);
	    center = true;
	}

	// Centering the window when it is no longer
	// in the visible area of the terminal
	if (!screen.contains(getPos()) || center) {
	    int x = 1 + int((getDesktopWidth() - getWidth()) / 2);
	    int y = 1 + int((getDesktopHeight() - getHeight()) / 2);
	    setPos(FPoint{ x, y });
	}
    }
    scrolltext.setGeometry(FPoint{ 1, 2 }, FSize(getWidth(), getHeight() - 1));
}

void
CodeView::onAccel(finalcut::FAccelEvent *ev)
{
    close();
    ev->accept();
}

CodeView::TextView::TextView(CodeView *codeView)
  : FTextView{ codeView }
  , codeView{ codeView }
{
}

void
CodeView::TextView::onMouseDown(FMouseEvent *ev)
{
    FTextView::onMouseDown(ev);
    if (ev->getButton() == MouseButton::Left && ev->getX() < int(getWidth())) {
	std::size_t line =
	  codeView->scrolltext.getScrollPos().getY() + ev->getY() - 2;
	std::uint64_t addr = line * 4;
	if (udb_breakpoint.find(addr) != udb_breakpoint.end()) {
	    udb_breakpoint.erase(addr);
	    codeView->scrolltext.addHighlight(line, codeView->hideBP);
	    codeView->scrolltext.getLine(line).text.wc_str()[1] = ' ';
	} else {
	    udb_breakpoint[addr] = addr;
	    codeView->scrolltext.addHighlight(line, codeView->showBP);
	    codeView->scrolltext.getLine(line).text.wc_str()[1] = 'B';
	}
	codeView->update(false);
    }
}
