#include <final/final.h>
#include <iostream>

#include "../ulm2/ulm.h"
#include "ulmapp.hpp"

int
main(int argc, char *argv[])
{
    UlmApp ulmApp{ argc, argv };

    if (argc != 2) {
	std::cerr << "usage: " << argv[0] << " file" << std::endl;
	return 1;
    }
    if (!ulm_load(argv[1])) {
	std::cerr << "can not load program " << argv[1] << std::endl;
    }
    udb_on = true;

    finalcut::FWidget::setMainWidget(&ulmApp);
    ulmApp.show();
    ulmApp.openCpuView(&ulmApp);
    ulmApp.openMemView(&ulmApp);
    ulmApp.openIOView(&ulmApp);
    ulmApp.openCodeView(&ulmApp);

    return ulmApp.exec();
}
