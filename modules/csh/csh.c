#include "stdint.h"
#include "stdio.h"
#include "process.h"
#include "string.h"

int main(int argc, char* args[])
{
    char epath[512];
    char instr[512];

    cd("ibin");

    while(1) {
        getExecPath(epath);
        printf("$%s> ", epath);
        getln(instr);

        char* cmd = strtoknc(instr, " ");

        if(cmd != 0) {
            char* pargs[64];
            int n = 0;

            do {
                pargs[n] = strtoknc(0, " ");
            } while(pargs[n++] != 0);

            sexec(instr, pargs);

            printf("\n");
        }
    }

    return 0;
}
