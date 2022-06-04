#ifndef ULM2_IO_H
#define ULM2_IO_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
 * Implements the IO specified here:
 **/
#include "../ulm0/hdw_io.h"

void ulm_setInputCallback(int(*callBack)());
void ulm_setOutputCallback(void(*callBack)(int));

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // ULM2_IO_H
