#include <final/final.h>

using namespace finalcut;

class LineView : public FScrollView
{
  public:
    LineView(FWidget *parent)
      : FScrollView{ parent }
    {
    }

    void
    draw() override
    {
        clearArea();
        for (auto y{ 1 }; y <= int(numLines); ++y) {
            setPrintPos(FPoint{ 1, y });
            print() << "Line = " << y;
        }
        FScrollView::draw();
    }

    void
    setGeometry(const FPoint &pos, const FSize &size,
                bool adjust = true) override
    {
        FScrollView::setGeometry(pos, size, adjust);
        auto w = std::max(numCols, getWidth());
        auto h = std::max(numLines, getHeight());

        setScrollSize(FSize{ w, h });
    }

  private:
    std::size_t numCols{ 10 };
    std::size_t numLines{ 42 };
};

class PrgView final : public FDialog
{
  public:
    PrgView(FWidget *parent)
      : FDialog{ parent }
      , lineView{ this } // show 42 lines
    {
    }

    void
    initLayout() override
    {
        FDialog::setGeometry(FPoint{ 16, 3 }, FSize{ 50, 19 });

        setText("Program");
        setBorder(false);
        setResizeable();
        lineView.setGeometry(FPoint{ 1, 1 },
                             FSize{ getWidth() - 1, getHeight() - 1 });
        FDialog::initLayout();
    }

    void
    setGeometry(const FPoint &p, const FSize &s, bool adj = true) override
    {
        FDialog::setGeometry(p, s, adj);
    }

    void
    adjustSize() override
    {
        FDialog::adjustSize();
        lineView.setGeometry(FPoint{ 1, 1 },
                             FSize{ getWidth() - 1, getHeight() - 1 });
    }

    LineView lineView;
};

int
main(int argc, char *argv[])
{
    FApplication app{ argc, argv };

    PrgView prgView{ &app };

    FWidget::setMainWidget(&prgView);
    app.show();
    return app.exec();
}
