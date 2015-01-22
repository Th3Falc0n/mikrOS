#include "process.h"
#include "stdio.h"
#include "process_rpc.c"

extern int main(int argc, char* args[]);

static uint32_t getargs() {
    struct regstate state = {
      .eax = 4,
      .ebx = 0,
      .ecx = 0,
      .edx = 0,
      .esi = 0,
      .edi = 0
    };

    syscall(&state);

    return state.eax;
}

uint32_t pexists(int pid) {
    struct regstate state = {
      .eax = 2,
      .ebx = (uint32_t)pid,
      .ecx = 0,
      .edx = 0,
      .esi = 0,
      .edi = 0
    };

    syscall(&state);

    return state.eax;
}

void _start() {
  char** args = (char**) getargs();

  int argc = 0;

  if(args != 0) {
      while(args[argc] != 0) {
          argc++;
      }
  }

  set_rpc_handler();

  int result = main(argc, args);

  exit(result);
}

void yield() {
    struct regstate state = {
      .eax = 5,
      .ebx = 0,
      .ecx = 0,
      .edx = 0,
      .esi = 0,
      .edi = 0
    };

    syscall(&state);

    return;
}

void exit(int returncode) {
    struct regstate state = {
        .eax = 1,
        .ebx = returncode,
        .ecx = 0,
        .edx = 0,
        .esi = 0,
        .edi = 0
    };

    syscall(&state);

    while(1) {
      printf("error\n");
    }
}

void getExecPath(char* dest) {
    struct regstate state = {
        .eax = 6,
        .ebx = (uint32_t)dest,
        .ecx = 0,
        .edx = 0,
        .esi = 0,
        .edi = 0
    };

    syscall(&state);
}

int changeExecPath(char* path) {
    struct regstate state = {
        .eax = 7,
        .ebx = (uint32_t)path,
        .ecx = 0,
        .edx = 0,
        .esi = 0,
        .edi = 0
    };

    syscall(&state);

    return state.eax;
}

char* getPathChild(uint32_t index, char* strbuf, char* path) {
    struct regstate state = {
        .eax = 17,
        .ebx = index,
        .ecx = (uint32_t)strbuf,
        .edx = (uint32_t)path,
        .esi = 0,
        .edi = 0
    };

    syscall(&state);

    return (char*)state.eax;
}

void cd(char* path) {
    if(!changeExecPath(path)) printFilesystemError(path, getLastVFSErr());
}

int exec(char* path, char** args, int asSubtask) {
    struct regstate state = {
        .eax = 3,
        .ebx = (uint32_t) path,
        .ecx = (uint32_t) args,
        .edx = asSubtask,
        .esi = 0,
        .edi = 0
    };

    syscall(&state);

    return state.eax;
}

int dexec(char* path, char** args) {
    uint32_t res = exec(path, args, 0);

    if(!res) {
        printFilesystemError(path, getLastVFSErr());
    }

    return res;
}

int sexec(char* path, char** args) {
    uint32_t res = exec(path, args, 1);

    if(!res) {
        printFilesystemError(path, getLastVFSErr());
    }

    return res;
}

void printFilesystemError(char* path, uint32_t code) {
    switch(code) {
    case PE_NO_ERROR:
        printf("%s: Unknown error", path);
        break;
    case PE_INVALID:
        printf("%s: Specified path was invalid", path);
        break;
    case PE_CORRUPT_FILE:
        printf("%s: File is not a valid executable binary", path);
        break;
    case PE_FILESYSTEM:
        printf("%s: Filesystem error", path);
        break;
    case PE_PERM_DENIED:
        printf("%s: Permission to execute path denied (maybe a directory?)", path);
        break;
    case PE_FILE_NOT_FOUND:
        printf("%s: File not found", path);
        break;
    default:
        printf("%s: WTF", path);
        break;

    }
    printf("\n");
}
