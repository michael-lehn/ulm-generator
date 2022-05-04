#ifndef ULM_VMEM_H
#define ULM_VMEM_H

#include <stddef.h>
#include <stdint.h>

#include "alu.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
 * Implements the VMEM part specified here:
 **/
#include "../ulm0/hdw_vmem.h"

// allow the debugger to look at single bytes
uint8_t ulm_vmemPeek(uint64_t addr);

struct SegmentNode
{
    uint64_t offset, size;
    struct SegmentNode *next;
};

const struct SegmentNode *ulm_vmemGetSegmentList();
void ulm_vmemGetSegment(uint64_t addr, uint64_t *segOffset, uint64_t *segSize);
void ulm_vmemSegmentPrint(void);

// sweep clean: release all allocated memory
void ulm_vmemClear();

// for fetching an instruction
void ulm_fetch32(uint64_t addr, uint32_t *val);

uint8_t ulm_fetch8(uint64_t addr);
void ulm_store8(uint64_t addr, uint8_t val);

void ulm_printVMemMapped();
void ulm_printVMem(uint64_t begin, uint64_t end, size_t chunkSize);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // ULM_VMEM_H
