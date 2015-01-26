extern "C" {
#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "process.h"
#include "string.h"
#include "list.h"
#include "stringbuilder.h"
}

int main(int argc, char* args[]) {
	struct stringbuilder* strbuilder = stringbuilder_new();

	stringbuilder_append(strbuilder, (char*) "Hello ");
	stringbuilder_append(strbuilder, (char*) "World!");

	printf("%s", stringbuilder_tostring(strbuilder));
	return 0;
}
