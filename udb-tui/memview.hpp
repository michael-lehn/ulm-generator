#ifndef UDBTUI_MEMVIEW_HPP
#define UDBTUI_MEMVIEW_HPP

#include <final/final.h>

class MemView final : public finalcut::FDialog
{
  public:
    MemView(finalcut::FWidget *parent);
    void notify();

    void draw() override;

  private:
    void initSegInfo();
    void init();
    void updateAddrRange();
    void mapKeyFunctions();
    void updateCell();
    void updateVbar();
    void stepBackward();
    void stepForward();
    void pageBackward();
    void pageForward();
    void cb_vbarChange(const FWidget *);
    void onKeyPress(finalcut::FKeyEvent *ev) override;
    void onWheel(finalcut::FWheelEvent *ev) override;
    void selectSegment();

    std::uint64_t segOffset{ 0 }; // address of top left mem cell
    std::uint64_t segSize{ 0 };	  // address of top left mem cell
    std::uint64_t numPages{ 0 };
    static constexpr std::size_t numCellCols{ 16 };
    static constexpr std::size_t numCellRows{ 16 };
    static constexpr std::size_t numCells{ numCellRows * numCellCols };
    finalcut::FScrollbarPtr vbar{ nullptr };
    std::vector<std::unique_ptr<finalcut::FLabel>> colLabel{ numCellCols };
    std::vector<std::unique_ptr<finalcut::FLabel>> rowLabel{ numCellRows };
    std::vector<std::unique_ptr<finalcut::FLineEdit>> cell;

    using FColor = finalcut::FColor;
    using FColorPair = finalcut::FColorPair;

    FColor defaultForegroundColor, defaultBackgoundColor;
    FColorPair memRead{ FColor::White, FColor::Green };
    FColorPair memWrite{ FColor::White, FColor::Red };
    FColorPair onStack{ FColor::White, FColor::LightCyan };
    FColorPair inFrame{ FColor::White, FColor::LightMagenta };
    FColorPair instr{ FColor::Black, FColor::Yellow };

    finalcut::FLabel segLabel{ this };
    finalcut::FListBox segChoice{ this };

    using KeyMap = std::unordered_map<finalcut::FKey, std::function<void()>,
				      finalcut::EnumHash<finalcut::FKey>>;
    KeyMap key_map{};
};

#endif // UDBTUI_MEMVIEW_HPP

