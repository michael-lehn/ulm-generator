#include <cinttypes>

#include "../ulm2/ulm.h"

#include "udb_cpp.hpp"
#include "ulmapp.hpp"

using namespace finalcut;

UlmApp::UlmApp(const int &argc, char *argv[])
  : FApplication{ argc, argv }
  , menuBar{ this }
  , statusbar{ &menuBar }

  , ulm{ "&ULM Debugger", &menuBar }
  , ulmAbout{ "About ULM Debuger", &ulm }
  , ulmLine1{ &ulm }
  , ulmPref{ "Preferences", &ulm }
  , ulmLine2{ &ulm }
  , ulmQuit{ "&Quit", &ulm }

  , file{ "&File", &menuBar }
  , filePrg{ "&Load Program", &file }

  , view{ "&View", &menuBar }
  , viewCpu{ "&CPU", &view }
  , viewPrg{ "&Program", &view }
  , viewMem{ "&Virtual Memory", &view }
  , viewIO{ "&IO", &view }

  , debug{ "&Debug", &menuBar }
  , debugRun{ "&Run", &debug }
  , debugStep{ "&Step", &debug }
  , debugTerminate{ "&Terminate", &debug }
{
    ulmQuit.addAccelerator(FKey::Ctrl_q);
    filePrg.addAccelerator(FKey::Ctrl_l);
    viewCpu.addAccelerator(FKey::Ctrl_c);
    viewPrg.addAccelerator(FKey::Ctrl_p);
    viewMem.addAccelerator(FKey::Ctrl_v);
    viewIO.addAccelerator(FKey::Ctrl_i);
    debugStep.addAccelerator(FKey::Ctrl_s);
    debugRun.addAccelerator(FKey::Ctrl_r);
    ulmLine1.setSeparator();
    ulmLine2.setSeparator();

    ulmQuit.addCallback("clicked", this, &UlmApp::cb_exitApp, this);

    viewCpu.addCallback("clicked", this, &UlmApp::openCpuView, this);
    viewPrg.addCallback("clicked", this, &UlmApp::openCodeView, this);
    viewMem.addCallback("clicked", this, &UlmApp::openMemView, this);
    viewIO.addCallback("clicked", this, &UlmApp::openIOView, this);

    debugStep.addCallback("clicked", this, &UlmApp::step);
    debugRun.addCallback("clicked", this, &UlmApp::run);

    udb_addCallback(this, &UlmApp::notify);
}

void
UlmApp::openCpuView(FWidget *)
{
    if (!cpuView) {
	cpuView = std::make_unique<CpuView>(&menuBar);
	cpuView->addCallback(
	  "destroy",
	  [](std::unique_ptr<CpuView> &cpuView) mutable
	  {
	      udb_delListener(cpuView.get());
	      cpuView.release();
	  },
	  std::ref(cpuView));
	cpuView->show();
    } else {
	cpuView->setFocus();
    }
    udb_addCallback(cpuView.get(), &CpuView::notify);
}

void
UlmApp::openCodeView(FWidget *)
{
    if (!codeView) {
	codeView = std::make_unique<CodeView>(&menuBar);
	codeView->addCallback(
	  "destroy",
	  [](std::unique_ptr<CodeView> &codeView) mutable
	  {
	      udb_delListener(codeView.get());
	      codeView.release();
	  },
	  std::ref(codeView));
	codeView->show();
    } else {
	codeView->setFocus();
    }
    udb_addCallback(codeView.get(), &CodeView::notify);
}

void
UlmApp::openMemView(FWidget *)
{
    if (!memView) {
	memView = std::make_unique<MemView>(&menuBar);
	memView->addCallback(
	  "destroy",
	  [](std::unique_ptr<MemView> &memView) mutable
	  {
	      udb_delListener(memView.get());
	      memView.release();
	  },
	  std::ref(memView));
	memView->show();
    } else {
	memView->setFocus();
    }
    udb_addCallback(memView.get(), &MemView::notify);
}

void
UlmApp::openIOView(FWidget *)
{
    if (!ioView) {
	ioView = std::make_unique<IOView>(&menuBar);
	ioView->addCallback(
	  "destroy",
	  [](std::unique_ptr<IOView> &ioView) mutable
	  {
	      udb_delListener(ioView.get());
	      ioView.release();
	  },
	  std::ref(ioView));
	ioView->show();
    } else {
	ioView->setFocus();
	ioView->setInputFocus();
    }
    udb_addCallback(ioView.get(), &IOView::notify);
}

void
UlmApp::step()
{
    udb_step();
    if (udb_illegalInstruction) {
	notify();
	return;
    }
    udb_notify();
}

void
UlmApp::run()
{
    udb_run();
    if (udb_illegalInstruction) {
	notify();
	return;
    }
    udb_notify();
}

void
UlmApp::notify()
{
    if (ulm_halted) {
	FString msg;
	msg.sprintf("Program halted with exit code 0x%" PRIu64, ulm_exitCode);
	FMessageBox::info(&menuBar, "Program halted", msg);
    } else if (udb_illegalInstruction) {
	FString msg;
	msg.sprintf("Illegal instruction 0x%" PRIu32, ulm_instrReg);
	FMessageBox::info(&menuBar, "Program crashed", msg);
    }
}
