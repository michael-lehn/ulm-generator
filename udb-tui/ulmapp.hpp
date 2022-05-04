#ifndef UDB_TUI_ULMAPP_HPP
#define UDB_TUI_ULMAPP_HPP

#include <cstdint>
#include <final/final.h>

#include "codeview.hpp"
#include "cpuview.hpp"
#include "memview.hpp"
#include "ioview.hpp"

class UlmApp final : public finalcut::FApplication
{
  public:
    UlmApp(const int &, char *[]);

    void openCpuView(finalcut::FWidget *);
    void openCodeView(finalcut::FWidget *);
    void openMemView(finalcut::FWidget *);
    void openIOView(finalcut::FWidget *);
    void step();
    void run();
    void notify();

  private:
    finalcut::FMenuBar menuBar;
    finalcut::FStatusBar statusbar;

    // menu group 'ULM Debug'
    finalcut::FMenu ulm;
    finalcut::FMenuItem ulmAbout;
    finalcut::FMenuItem ulmLine1;
    finalcut::FMenuItem ulmPref;
    finalcut::FMenuItem ulmLine2;
    finalcut::FMenuItem ulmQuit;

    // menu group 'File'
    finalcut::FMenu file;
    finalcut::FMenuItem filePrg;

    // menu group 'View'
    finalcut::FMenu view;
    finalcut::FMenuItem viewCpu;
    finalcut::FMenuItem viewPrg;
    finalcut::FMenuItem viewMem;
    finalcut::FMenuItem viewIO;

    // menu group 'Debug'
    finalcut::FMenu debug;
    finalcut::FMenuItem debugRun;
    finalcut::FMenuItem debugStep;
    finalcut::FMenuItem debugTerminate;

    std::unique_ptr<CpuView> cpuView{ nullptr };
    std::unique_ptr<CodeView> codeView{ nullptr };
    std::unique_ptr<MemView> memView{ nullptr };
    std::unique_ptr<IOView> ioView{ nullptr };
};

#endif // UDB_TUI_ULMAPP_HPP
