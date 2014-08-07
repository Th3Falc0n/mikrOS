#ifndef PROCESS_H
#define PROCESS_H

#include "syscall.h"
#include "stdint.h"

void _start();
void exit(int returncode);
void yield();
void getExecPath(char* dest);
int  changeExecPath(char* path);
void cd(char* path);
int  exec(char* path, char** args);
int  texec(char* path, char** args);
void printexecerror(char* path, uint32_t code);

#endif
