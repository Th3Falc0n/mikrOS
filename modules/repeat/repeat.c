#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "process.h"
#include "string.h"
#include "list.h"

int main(int argc, char* args[]) {
	int num = args[1][0];
	for (int i = 0; i < num; i++)
		fsexec(args[2], args + 3, 0, 0, 0);

	return 0;
}
