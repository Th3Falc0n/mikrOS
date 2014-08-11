#include "stdint.h"
#include "stdio.h"
#include "process.h"
 
int main(int argc, char* args[])
{
    if(argc < 2) {
        printf("%s: usage: %s [-m] FILENAME\n\n-m: Limit output to 1KB\n\n", args[0], args[0]);
        exit(1);
    }

    char* filePath = 0;

    //Following is general parser code.

    uint32_t* maxSize = 0;
    uint32_t ms = 1024;

    for(int i = 1; i < argc; i++) {
        if(args[i][0] == '-') {
            switch(args[i][1]) {
            case 'm':
                maxSize = &ms;
                //TODO implement int parse
                break;
            }
        }
        else
        {
            if(filePath == 0) {
                filePath = args[i];
            }
        }
    }

    if(filePath == 0) {
        printf("%s: No file specified\n", args[0]);
        printf("%s: usage: %s [-m] FILENAME\n\n-m: Limit output to 1KB\n\n", args[0], args[0]);
        exit(1);
    }

    HANDLE hdl = fopen(filePath, FM_READ);

    if(!hdl) {
        printf("%s: ", args[0]);
        printFilesystemError(filePath, getLastVFSErr());
    }

    char buffer[1024];
    for(uint32_t n = 0; maxSize == 0 || n < *maxSize; n++) {
        uint32_t length = 1024;
        uint32_t avail = favailable(hdl);

        if(avail == 0) exit(0);

        if(avail < length) length = avail;


        uint32_t res = fread(hdl, buffer, length);
                       fwrite(PMID_STDOUT, buffer, length);
    }

    return 0;
}
