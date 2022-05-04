#include <cinttypes>

#include "memview.hpp"
#include "udb_cpp.hpp"

using namespace finalcut;

MemView::MemView(FWidget *parent)
  : FDialog{ parent }
  , cell{ numCells }
{
    initScrollbar(vbar, Orientation::Vertical, this, &MemView::cb_vbarChange);
    init();
}

void
MemView::initSegInfo()
{
    FString val;

    ulm_vmemGetSegment(segOffset, &segOffset, &segSize);
    val.sprintf("Virtual Memory (0x%016" PRIX64 " : 0x%016" PRIX64 " : %" PRIu64
		"bytes)",
		segOffset, segOffset + segSize - 1, segSize);
    setText(val);

    vbar->setMinimum(0);
    vbar->setMaximum(segSize - numCells);
    vbar->setPageSize(segSize, numCells);
    vbar->setSteps(numCellCols);

    if (segOffset + segSize == 0) {
	vbar->setValue(segSize - numCells);
    } else {
	vbar->setValue(0);
    }
    vbar->calculateSliderValues();
    vbar->redraw();
}

void
MemView::init()
{
    FString val;

    setSize(FSize{ 90, 25 });
    initSegInfo();

    vbar->setGeometry(FPoint{ int(getWidth() - 3), 3 },
		      FSize{ 4, numCellRows + 2 });
    vbar->setFocusable(true);
    vbar->setFocus(true);
    vbar->show();

    for (std::size_t j = 0; j < numCellCols; ++j) {
	colLabel[j] = std::make_unique<FLabel>(this);
	colLabel[j]->setGeometry(FPoint{ int(j * 4 + 21), 1 }, FSize{ 3, 1 });
	val.sprintf("x%1zX", j);
	colLabel[j]->setText(val);
    }
    for (std::size_t i = 0; i < numCellRows; ++i) {
	rowLabel[i] = std::make_unique<FLabel>(this);
	rowLabel[i]->setGeometry(FPoint{ 1, int(i + 2) }, FSize{ 18, 1 });
    }
    for (std::size_t i = 0; i < numCellRows; ++i) {
	for (std::size_t j = 0; j < numCellCols; ++j) {
	    int id = int(i * numCellCols + j);
	    cell[id] = std::make_unique<FLineEdit>(this);
	    cell[id]->setGeometry(FPoint{ int(j * 4 + 20), int(i + 2) },
				  FSize{ 4, 1 });
	    cell[id]->setShadow(false);
	    cell[id]->setReadOnly();
	    cell[id]->setFocusable(false);
	}
    }
    segLabel.setGeometry(FPoint{ 1, numCellRows + 4 }, FSize{ 18, 1 });
    segLabel.setText("Address range:");
    segLabel.setAlignment(Align::Right);
    segChoice.setGeometry(FPoint{ 20, numCellRows + 3 },
			  FSize{ numCellCols * 4, 4 });
    segChoice.addCallback("clicked", this, &MemView::selectSegment);
    segChoice.addCallback("row-changed", this, &MemView::selectSegment);

    defaultForegroundColor = cell[0]->getForegroundColor();
    defaultBackgoundColor = cell[0]->getBackgroundColor();
    mapKeyFunctions();
    updateCell();
    updateAddrRange();
}

void
MemView::selectSegment()
{
    FListBoxItem item = segChoice.getItem(segChoice.currentItem());
    segOffset = item.getData<std::uint64_t>();
    initSegInfo();
    updateCell();
    redraw();
}

void
MemView::updateAddrRange()
{
    segChoice.clear();
    for (const SegmentNode *n = ulm_vmemGetSegmentList(); n; n = n->next) {
	FString val;
	val.sprintf("0x%016" PRIX64 " : 0x%016" PRIX64, n->offset,
		    n->offset + n->size - 1);
	FListBoxItem item{ val, n->offset };
	segChoice.insert(item);
    }
    redraw();
}

void
MemView::mapKeyFunctions()
{
    key_map[FKey::Up] = [this] { stepBackward(); };
    key_map[FKey::Down] = [this] { stepForward(); };
    key_map[FKey::Page_up] = [this] { pageBackward(); };
    key_map[FKey::Page_down] = [this] { pageForward(); };
}

void
MemView::updateCell()
{
    std::uint64_t addrOffset =
      segOffset + (vbar->getValue() / numCellCols * numCellCols);

    uint64_t sp = *ulm_regDevice(2);
    uint64_t sf = *ulm_regDevice(1);
    uint64_t ip = ulm_instrPtr;

    for (std::size_t i = 0; i < numCellRows; ++i) {
	FString val;
	val.sprintf("0x%016zX", addrOffset + i * numCellCols);
	rowLabel[i]->setText(val);
	rowLabel[i]->redraw();
	for (std::size_t j = 0; j < numCellCols; ++j) {
	    int id = int(i * numCellCols + j);
	    std::uint64_t addr = addrOffset + id;
	    val.sprintf("%02" PRIX8, ulm_vmemPeek(addr));
	    cell[id]->setText(val);
	    auto mark = udb_mem[udb_activeMsg].find(addr);
	    if (mark != udb_mem[udb_activeMsg].end()) {
		if (mark->second & UDB_READ) {
		    cell[id]->setForegroundColor(memRead.getForegroundColor());
		    cell[id]->setBackgroundColor(memRead.getBackgroundColor());
		} else if (mark->second & UDB_WRITE) {
		    cell[id]->setForegroundColor(memWrite.getForegroundColor());
		    cell[id]->setBackgroundColor(memWrite.getBackgroundColor());
		}
	    } else if (sp && addr >= sp && addr <= sf - 1) {
		cell[id]->setForegroundColor(inFrame.getForegroundColor());
		cell[id]->setBackgroundColor(inFrame.getBackgroundColor());
	    } else if (sp && addr >= sp) {
		cell[id]->setForegroundColor(onStack.getForegroundColor());
		cell[id]->setBackgroundColor(onStack.getBackgroundColor());
	    } else if (addr >= ip && addr <= ip + 3) {
		cell[id]->setForegroundColor(instr.getForegroundColor());
		cell[id]->setBackgroundColor(instr.getBackgroundColor());
	    } else {
		cell[id]->setForegroundColor(defaultForegroundColor);
		cell[id]->setBackgroundColor(defaultBackgoundColor);
	    }
	    cell[id]->redraw();
	}
    }
}

void
MemView::updateVbar()
{
    vbar->calculateSliderValues();
    vbar->redraw();
    updateCell();
}

void
MemView::notify()
{
    if (numPages != udb_numPages) {
	numPages = udb_numPages;
	updateAddrRange();
    }
    updateCell();
}

void
MemView::draw()
{
    FDialog::draw();
    vbar->calculateSliderValues();
    vbar->redraw();
}

void
MemView::stepBackward()
{
    vbar->setValue(vbar->getValue() - numCellCols);
    updateVbar();
}

void
MemView::stepForward()
{
    vbar->setValue(vbar->getValue() + numCellCols);
    updateVbar();
}

void
MemView::pageBackward()
{
    vbar->setValue(vbar->getValue() - numCells);
    updateVbar();
}

void
MemView::pageForward()
{
    vbar->setValue(vbar->getValue() + numCells);
    updateVbar();
}

void
MemView::cb_vbarChange(const FWidget *)
{
    auto scroll_type = vbar->getScrollType();
    switch (scroll_type) {
	case FScrollbar::ScrollType::WheelDown:
	case FScrollbar::ScrollType::PageForward:
	    pageForward();
	    break;
	case FScrollbar::ScrollType::WheelUp:
	case FScrollbar::ScrollType::PageBackward:
	    pageBackward();
	    break;
	case FScrollbar::ScrollType::StepForward:
	    stepForward();
	    break;
	case FScrollbar::ScrollType::StepBackward:
	    stepBackward();
	    break;
	default:;
	    updateVbar();
    }
}

void
MemView::onKeyPress(FKeyEvent *ev)
{
    const auto &entry = key_map[ev->key()];
    if (entry) {
	entry();
	ev->accept();
    }
}

void
MemView::onWheel(finalcut::FWheelEvent *ev)
{
    if (ev->getWheel() == MouseWheel::Up) {
	pageBackward();
    }
    if (ev->getWheel() == MouseWheel::Down) {
	pageForward();
    }
}
