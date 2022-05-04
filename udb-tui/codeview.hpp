#ifndef UDBTUI_CODEVIEW_HPP
#define UDBTUI_CODEVIEW_HPP

#include <final/final.h>

class CodeView final : public finalcut::FDialog
{
  public:
    explicit CodeView(finalcut::FWidget * = nullptr);
    void notify();
    void update(bool scrollToIP);

  private:
    using MatchList = std::vector<std::size_t>;
    using StringPos = std::wstring::size_type;

    void initLayout() override;
    void adjustSize() override;

    void onAccel(finalcut::FAccelEvent *) override;

    using FTextView = finalcut::FTextView;
    using FTextHighlight = FTextView::FTextHighlight;
    using FColor = finalcut::FColor;
    using FColorPair = finalcut::FColorPair;

    struct TextView : public FTextView
    {
	TextView(CodeView *);
	void onMouseDown(finalcut::FMouseEvent *) override;
	CodeView *codeView;
    };

    TextView scrolltext{ this };

    FColor fgColor{ getForegroundColor() };
    FColor bgColor{ getBackgroundColor() };
    FTextHighlight normInstr{ 4, FColorPair{ fgColor, bgColor } };
    FTextHighlight currInstr{ 4, FColorPair{ FColor::White, FColor::Blue } };
    FTextHighlight prevInstr{ 4, FColorPair{ FColor::Black, FColor::Grey89 } };
    FTextHighlight hideBP{ 0, 3, FColorPair{ bgColor, bgColor } };
    FTextHighlight showBP{ 0, 3, FColorPair{ FColor::White, FColor::Red } };
};

#endif // UDBTUI_CODEVIEW_HPP

