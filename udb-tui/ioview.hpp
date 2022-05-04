#ifndef UDBTUI_IOVIEW_HPP
#define UDBTUI_IOVIEW_HPP

#include <final/final.h>

class IOView final : public finalcut::FDialog
{
  public:
    explicit IOView(finalcut::FWidget * = nullptr);
    void notify();

    class InputBuffer final : public finalcut::FLineEdit
    {
      public:
	explicit InputBuffer(IOView *);
	~InputBuffer();
	void onKeyPress(finalcut::FKeyEvent *) override;
	void notify();

      private:
	IOView *ioView;
    };

  private:
    using MatchList = std::vector<std::size_t>;
    using StringPos = std::wstring::size_type;

    void initLayout() override;
    void adjustSize() override;

    void onAccel(finalcut::FAccelEvent *) override;

    finalcut::FTextView scrolltext{ this };
    finalcut::FLabel inBufLabel{ this };
    finalcut::FLabel inputLabel{ this };
    InputBuffer input{ this };
    finalcut::FLineEdit inBuf{ this };
};

#endif // UDBTUI_IOVIEW_HPP

