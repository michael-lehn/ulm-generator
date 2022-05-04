#ifndef UDBTUI_CPUVIEW_HPP
#define UDBTUI_CPUVIEW_HPP

#include <final/final.h>

class CpuView final : public finalcut::FDialog
{
  public:
    CpuView(finalcut::FWidget *parent);
    void notify();

  private:
    class CuView : public finalcut::FScrollView
    {
      public:
	CuView(CpuView *cpu);
	void setGeometry(const finalcut::FPoint &p, const finalcut::FSize &s,
			 bool adj = true) override;
	void notify();

      private:
	finalcut::FLineEdit ip, ir, asmCode;
	finalcut::FLabel stage;
    };

    class AluView : public finalcut::FScrollView
    {
      public:
	AluView(CpuView *cpu);
	void setGeometry(const finalcut::FPoint &p, const finalcut::FSize &s,
			 bool adj = true) override;
	void notify();

      private:
	constexpr static int numRegs{ 256 };
	constexpr static int regWidth{ 2 + 16 }; // num hexdigits + 2
	constexpr static int regSep{ 3 + 3 };	 // width of label + 3
	constexpr static int regRows{ 8 };
	constexpr static int regCols{ numRegs / regRows };
	std::unique_ptr<finalcut::FLineEdit> reg[numRegs];
	finalcut::FColor defaultForegroundColor, defaultBackgoundColor;

    };

    class StatusView : public finalcut::FScrollView
    {
      public:
	StatusView(CpuView *cpu);
	void setGeometry(const finalcut::FPoint &p, const finalcut::FSize &s,
			 bool adj = true) override;
	void notify();

      private:
	finalcut::FLineEdit zf, cf, of, sf;
    };

    CuView cuView;
    AluView aluView;
    StatusView statusView;
};

#endif // UDBTUI_CPUVIEW_HPP
