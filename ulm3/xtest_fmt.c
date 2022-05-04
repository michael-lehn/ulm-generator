#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <ulm2/ulm.h>

int
main()
{
    uint32_t instrReg = 0x30FF0203;

    char str[40];
    ulm_asm(instrReg, str, sizeof(str));
    printf("  %s\n", str);
}
