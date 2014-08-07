#include "stdint.h"
#include "stdio.h"
#include "process.h"
#include "string.h"
 
char currentPath[512] = "/ibin/";

int main(int argc, char* args[])
{
    char instr[512];
    char execstr[512];

    while(1) {
        printf("$%s> ", currentPath);
        getln(instr);

        if(instr[0] == '/') { //absolute path
            strcpy(execstr, instr);
        }
        else //relative path (TODO .. and .)
        {
            strcpy(execstr, currentPath); //TODO for every search path, not only current path
            strcpy((void*)((uint32_t)execstr + strlen(currentPath)), instr);
        }

        texec(execstr, 0);
    }

    return 0;
}
