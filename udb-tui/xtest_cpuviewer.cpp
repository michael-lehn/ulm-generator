#include <final/final.h>

#include "cpuview.hpp"
#include "udb_cpp.hpp"

int
main(int argc, char *argv[])
{
    finalcut::FApplication app{ argc, argv };

    CpuView cpuView{ &app };
    finalcut::FWidget::setMainWidget(&cpuView);
    app.show();

    udb_addCallback(&cpuView, &CpuView::notify);

    udb_addRegMsg(3, UDB_WRITE);
    udb_notify();
    udb_addRegMsg(6, UDB_READ);
    udb_notify();

    return app.exec();
}
