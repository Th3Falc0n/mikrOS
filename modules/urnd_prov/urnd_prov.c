#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "vmmcall.h"
#include "process.h"
 
uint32_t m_z = 1, m_w = 2;

static uint32_t get_next()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;
}

int main(int argc, char* args[])
{
    HANDLE out = fmkfifo("/dev/urandom");

    HANDLE initCtrl = fopen("/var/cntrl/init", FM_WRITE);
    fwrite(initCtrl, &(char){'K'}, sizeof(char));
    fclose(initCtrl);

    uint32_t* rnd = malloc(128 * sizeof(uint32_t));

    while(1) {
        for(int i = 0; i < 128; i++) {
            rnd[i] = get_next();
        }

        fwrite(out, rnd, 128 * sizeof(uint32_t));
    }

    return 0;
}
