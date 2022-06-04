#ifndef ULM2_LOADER_H
#define ULM2_LOADER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern uint64_t ulm_brk;

void ulm_loadRestart();
bool ulm_load(const char *prog);
bool ulm_load_fstream(FILE *in);
bool ulm_load_str(const char *line);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // ULM2_LOADER_H
