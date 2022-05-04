#include <final/final.h>
#include <fstream>

using namespace finalcut;

/*
inline bool FTextView::isPrintable (wchar_t ch) const
{
  // Check for printable characters

  const bool utf8 = (FVTerm::getFOutput()->getEncoding() == Encoding::UTF8);
  return ( (utf8 && std::iswprint(std::wint_t(ch)))
        || (! utf8 && std::isprint(char(ch))) );
}

class TextView final : public FTextView
{
  public:
    TextView(FWidget *parent)
      : FTextView{ parent }
    {
    }

    void
    drawText()
    {
	if (data.empty() || getHeight() <= 2 || getWidth() <= 2)
	    return;

	auto num = getTextHeight();

	if (num > getRows())
	    num = getRows();

	setColor();

	if (FVTerm::getFOutput()->isMonochron())
	    setReverse(true);

	for (std::size_t y{ 0 }; y < num; y++) // Line loop
	{
	    const std::size_t n = std::size_t(yoffset) + y;
	    const std::size_t pos = std::size_t(xoffset) + 1;
	    const auto text_width = getTextWidth();
	    FString line = getColumnSubString(data[n], pos, text_width);
	    FVTermBuffer line_buffer{};

	    std::size_t trailing_whitespace{ 0 };
	    const auto column_width = getColumnWidth(line);
	    if (column_width <= text_width)
		trailing_whitespace = text_width - column_width;

	    for (auto &&fchar : line_buffer) // Column loop
		if (!isPrintable(fchar.ch[0]))
		    fchar.ch[0] = L'.';

	    print() << FPoint{ 2, 2 - nf_offset + int(y) };
	    if (n == select) {
		setForegroundColor(FColor::White);
		setBackgroundColor(FColor::Black);
	    } else {
		setForegroundColor(FColor::Black);
		setBackgroundColor(FColor::White);
	    }
	    setColor();
	    line_buffer.print(line);

	    print(line_buffer);
	    print() << FString{ trailing_whitespace, L' ' };

	    if (n == breakPoint) {
		print() << FPoint{ 2, 2 - nf_offset + int(y) };
		setForegroundColor(FColor::White);
		setBackgroundColor(FColor::Red);
		setColor();
		print() << " B ";
	    }

	    setForegroundColor(FColor::Black);
	    setBackgroundColor(FColor::White);
	    setColor();
	}

	if (FVTerm::getFOutput()->isMonochron())
	    setReverse(false);
    }

    void
    onMouseDown(FMouseEvent *ev)
    {
	if (ev->getButton() == (MouseButton::Shift | MouseButton::Left)) {
	    setSelect(std::size_t(yoffset) + ev->getY() - 2 + nf_offset);
	}
    }

    void
    onKeyPress(FKeyEvent* ev) override
    {
    }

    void
    onKeyDown(FKeyEvent *ev) override
    {
	if (ev->key() == FKey::Up) {
	    setSelect(select - 1);
	} else if (ev->key() == FKey::Down) {
	    setSelect(select + 1);
	}
    }

    void
    setSelect(size_t select)
    {
	this->select = select + 1 > select ? select : 0;
	draw();
	processChanged();
    }

    std::size_t select{ 8 };
    std::size_t breakPoint{ 10 };
};

class PrgView final : public FDialog
{

  public:
    PrgView(FWidget *parent, const char *file)
      : FDialog{ parent }
      , lineView{ this }
    {
	std::ifstream fs;
	fs.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	std::string line;
	try {
	    for (fs.open(file); !fs.eof();) {
		std::getline(fs, line);
		line = "    " + line;
		lineView.append(line.c_str());
	    }
	} catch (std::ifstream::failure &e) {
	    std::cerr << "Exception opening/reading/closing file\n";
	}
    }

    void
    initLayout() override
    {
	FDialog::setGeometry(FPoint{ 16, 2 }, FSize{ 50, 17 });

	setText("Program");
	setBorder(false);
	setResizeable();
	lineView.setGeometry(FPoint{ 1, 1 },
			     FSize{ getWidth() - 1, getHeight() - 1 });
	FDialog::initLayout();
    }

    void
    adjustSize() override
    {
	FDialog::adjustSize();
	lineView.setGeometry(FPoint{ 1, 1 },
			     FSize{ getWidth() - 1, getHeight() - 1 });
    }

    TextView lineView;
};
*/
int
main(int argc, char *argv[])
{
    /*
    FApplication app{ argc, argv };

    PrgView prgView{ &app, "ulm_progs/hello" };
    FWidget::setMainWidget(&prgView);
    app.show();
    return app.exec();
    */
}
