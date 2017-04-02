#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define BUFSIZE 1024
void malloc();

int main ()
{
    int32_t cnt;
    uint8_t buf[BUFSIZE];

    ece391_fdputs (1, (uint8_t*)"Hi, what's your name? ");
    if (-1 == (cnt = ece391_read (0, buf, BUFSIZE-1))) {
        ece391_fdputs (1, (uint8_t*)"Can't read name from keyboard.\n");
        return 3;
    }
    buf[cnt] = '\0';
    ece391_fdputs (1, (uint8_t*)"Hello, ");
    ece391_fdputs (1, buf);

    malloc();

    return 0;
}

void malloc(){
    char * r1 = (char *)ece391_kmalloc(8); // 8 bytes
    ece391_fdputs (1, (uint8_t*)"1st malloc");
    r1[0] = '1';
    r1[1] = '2';
    r1[2] = '3';
    r1[3] = '4';
    r1[4] = '5';
    r1[5] = '6';
    r1[6] = '7';
    r1[7] = '\0';
    char * r2 = (char *)ece391_kmalloc(5);
    r2[0] = 'a';
    r2[1] = 'b';
    r2[2] = 'c';
    r2[3] = 'd';
    r2[4] = '\0';
    char * r3 = (char *)ece391_kmalloc(9);
    r3[0] = 'p';
    r3[1] = 'o';
    r3[2] = 'i';
    r3[3] = 'u';
    r3[4] = 'y';
    r3[5] = 't';
    r3[6] = 'r';
    r3[7] = 'e';
    r3[8] = '\0';

    ece391_fdputs (1, (uint8_t*)"kmalloc: r1: ");
    ece391_fdputs (1, (uint8_t*)r1);
    ece391_fdputs (1, (uint8_t*)"  r2: ");
    ece391_fdputs (1, (uint8_t*)r2);
    ece391_fdputs (1, (uint8_t*)"  r3: ");
    ece391_fdputs (1, (uint8_t*)r3);
    ece391_fdputs (1, (uint8_t*)"\n");

    ece391_kfree(r1);
    ece391_kfree(r2);

    ece391_fdputs (1, (uint8_t*)"free: r1: ");
    ece391_fdputs (1, (uint8_t*)r1);
    ece391_fdputs (1, (uint8_t*)"  r2: ");
    ece391_fdputs (1, (uint8_t*)r2);
    ece391_fdputs (1, (uint8_t*)"  r3: ");
    ece391_fdputs (1, (uint8_t*)r3);
    ece391_fdputs (1, (uint8_t*)"\n");

    char * r4 = (char *)ece391_kmalloc(13);
    r4[0] = '1';
    r4[1] = '2';
    r4[2] = '3';
    r4[3] = '4';
    r4[4] = '5';
    r4[5] = '6';
    r4[6] = '7';
    r4[7] = '8';
    r4[8] = '9';
    r4[9] = '1';
    r4[10] = '2';
    r4[11] = '3';
    r4[12] = '\0';

    ece391_fdputs (1, (uint8_t*)"after merge: r1: ");
    ece391_fdputs (1, (uint8_t*)r1);
    ece391_fdputs (1, (uint8_t*)"  r2: ");
    ece391_fdputs (1, (uint8_t*)r2);
    ece391_fdputs (1, (uint8_t*)"  r3: ");
    ece391_fdputs (1, (uint8_t*)r3);
    ece391_fdputs (1, (uint8_t*)"  r4: ");
    ece391_fdputs (1, (uint8_t*)r4);
    ece391_fdputs (1, (uint8_t*)"\n");

    ece391_kfree(r1);
    ece391_kfree(r2);
    ece391_kfree(r3);
    ece391_kfree(r4);

    ece391_fdputs (1, (uint8_t*)"after free: r1: ");
    ece391_fdputs (1, (uint8_t*)r1);
    ece391_fdputs (1, (uint8_t*)"  r2: ");
    ece391_fdputs (1, (uint8_t*)r2);
    ece391_fdputs (1, (uint8_t*)"  r3: ");
    ece391_fdputs (1, (uint8_t*)r3);
    ece391_fdputs (1, (uint8_t*)"  r4: ");
    ece391_fdputs (1, (uint8_t*)r4);
    ece391_fdputs (1, (uint8_t*)"\n");
    return 0;


}