#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include <ulm2/vmem.h>

void
printSeg(uint64_t addr)
{
    uint64_t segOffset, segSize;
    ulm_vmemGetSegment(addr, &segOffset, &segSize);

    if (segSize) {
	printf("addr 0x%016" PRIX64 " is in segment 0x%016" PRIX64
	       " - 0x%016" PRIX64 "\n",
	       addr, segOffset, segOffset + segSize - 1);
    } else {
	printf("addr 0x%016" PRIX64 " does not belong to any segment\n", addr);
    }
}

void
ulm_halt(uint64_t code)
{
    exit(code);
}

int
main()
{
    ulm_store8(1, 42);
    ulm_vmemSegmentPrint();

    ulm_store8(2049, 42);
    ulm_vmemSegmentPrint();

    ulm_store8(1025, 42);
    ulm_vmemSegmentPrint();

    ulm_store8(4099, 42);
    ulm_vmemSegmentPrint();

    ulm_store8(-1, 42);
    ulm_vmemSegmentPrint();

    printSeg(42);
    printSeg(4200);
    printSeg(42000);
}
