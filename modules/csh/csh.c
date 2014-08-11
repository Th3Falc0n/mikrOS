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
                n++;
            } while(pargs[n-1] != 0);

            if(!strcmp(cmd, "cd")) {
                if(pargs[0] != 0) {
                    cd((char*)pargs[0]);
                }
                else
                {
                    printf("cd: usage: \"cd [PATH)\"\n");
                }

                continue;
            }

            int PID = texec(instr, pargs);

            while(pexists(PID)) {
                yield();
            }

            printf("\n");
        }
    }

    return 0;
}
