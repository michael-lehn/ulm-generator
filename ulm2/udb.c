#include "udb.h"

bool udb_on;
bool udb_inStep = 0;
int udb_activeMsg;
int udb_lastMsg = 1;
uint64_t udb_numPages;
extern bool udb_eof;
bool udb_illegalInstruction = false;
bool udb_badAlignment = false;
bool udb_useStderr = true;
