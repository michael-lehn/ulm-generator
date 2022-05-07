#include <cinttypes>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "ioview.hpp"
#include "udb_cpp.hpp"

using namespace finalcut;

IOView::IOView(finalcut::FWidget *parent)
  : FDialog{ parent }
  , input{ this }
{
    notify();
}

void
IOView::notify()
{
    static bool first = true;
    static std::size_t udb_out_len = 0;

    if (udb_eof) {
	inBuf.setText(udb_in + "[EOF]");
    } else {
	inBuf.setText(udb_in);
    }
    inBuf.redraw();

    if (first) {
	first = false;
    } else {
	if (udb_out.length() == udb_out_len) {
	    return;
	}
	assert(scrolltext.getRows());
	if (scrolltext.getRows()) {
	    scrolltext.deleteLine(scrolltext.getRows() - 1);
	}
    }

    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = udb_out.find("\n", prev)) != std::string::npos) {
	scrolltext.append(udb_out.substr(prev, pos - prev));
	prev = pos + 1;
    }

    udb_out = udb_out.substr(prev);
    std::string lastLine;
    if (udb_out.length()) {
	// scrolltext.append(udb_out + "_");
	for (pos = udb_out.length(); pos-- > 0;) {
	    auto state = std::mbstate_t();
	    auto substr = udb_out.substr(0, pos + 1);
	    auto csubstr = substr.c_str();
	    if (std::mbsrtowcs(nullptr, &csubstr, 0, &state) + 1 != 0) {
		lastLine = substr;
		break;
	    }
	}
    }
    scrolltext.append(lastLine + "_");
    udb_out_len = udb_out.length();
    scrolltext.redraw();
}

IOView::InputBuffer::InputBuffer(IOView *ioView)
  : FLineEdit{ ioView }
  , ioView{ ioView }
{
    udb_addCallback(this, &IOView::InputBuffer::notify);
}

IOView::InputBuffer::~InputBuffer()
{
    udb_delListener(this);
}

void
IOView::InputBuffer::onKeyPress(FKeyEvent *ev)
{
    if (isReadOnly())
	return;

    const auto key = ev->key();
    if (key == FKey::Erase || key == FKey::Backspace) {
	stepCursorBackward();
	deletesCharacter();
	ev->accept();
    } else if (key == FKey::Tab) {
	inputText("\\t");
	stepCursorForward(2);
	ev->accept();
    } else if (key == FKey::Ctrl_d) {
	inputText("^D");
	stepCursorForward(2);
	ev->accept();
    } else if (isEnterKey(key)) {
	udb_in += getText().toString();
	udb_out += getText()
		     .replace("\\n", "\n")
		     .replace("\\t", "\t")
		     .replace("\\\\", "\\")
		     .toString();
	udb_out += "\n";
	setText("");
	ev->accept();
    } else if (key == FKey::Back_tab || key == FKey::Up || key == FKey::Down) {
	ev->ignore();
    } else {
	FLineEdit::onKeyPress(ev);
    }
    if (isEnterKey(key) || key == FKey::Ctrl_d) {
	ioView->notify();
    }

    if (ev->isAccepted()) {
	redraw();
	forceTerminalUpdate();
    }
}

void
IOView::InputBuffer::notify()
{
}

void
IOView::initLayout()
{
    FDialog::setText("I/O");
    setMinimumSize(FSize{ 40, 15 });
    auto window_size = FSize{ 40, 15 };
    auto x = 1;
    auto y = int(getDesktopHeight() - 1 - window_size.getHeight());
    FDialog::setGeometry(FPoint{ x, y }, window_size);
    adjustSize();
    input.setFocus();
}

void
IOView::adjustSize()
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

    scrolltext.setGeometry(FPoint{ 1, 2 },
			   FSize{ getWidth(), getHeight() - 5 });

    inputLabel.setText("Keyboard:");
    inputLabel.setGeometry(FPoint{ 2, int(getHeight() - 4) }, FSize{ 12, 1 });
    input.setGeometry(FPoint{ 12, int(getHeight() - 4) },
		      FSize{ getWidth() - 12, 1 });
    input.setShadow(false);

    inBufLabel.setText("Buffer:");
    inBufLabel.setGeometry(FPoint{ 2, int(getHeight() - 2) },
			   FSize{ getWidth() - 2, 1 });
    inBuf.setGeometry(FPoint{ 12, int(getHeight() - 2) },
		      FSize{ getWidth() - 12, 1 });
    inBuf.setShadow(false);
    inBuf.setReadOnly(true);

    FDialog::initLayout();

    setBorder(false);
    setResizeable();
    scrolltext.ignorePadding();

    input.setFocus();
    inBuf.setFocusable(false);
    scrolltext.setFocusable(false);
}

void
IOView::setInputFocus()
{
    input.setFocus();
}

void
IOView::onAccel(finalcut::FAccelEvent *ev)
{
    close();
    ev->accept();
}
