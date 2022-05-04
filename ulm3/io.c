#include <stdio.h>

#include <ulm2/io.h>

//-- Implementation for "../ulm0/hdw_io.h" -------------------------------------

void
ulm_printChar(int c)
{
    putchar(c);
}

int
ulm_readChar()
{
    return getchar();
}
