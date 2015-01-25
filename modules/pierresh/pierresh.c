#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "process.h"
#include "string.h"
#include "list.h"

static struct list* gc_list = 0;

static void* gc(void* p) {
	if (!gc_list)
		gc_list = list_new();

	struct list* first = list_first(gc_list);
	if (first) {
		free(first->data);
		list_free(first);
	}

	list_append(gc_list, p);

	return p;
}

static char* format(char* str) {
	return replaceAll(gc(replaceAll(str, "\"", "", "\\")), "\\\"", "\"", "");
}

int main(int argc, char* args[]) {
	char epath[512];
	char instr[512];

	cd("ibin");

	while (1) {
		getExecPath(epath);
		printf("#%s> ", epath);
		getln(instr);

		char* split = split(instr, " ", "\"", "\\");

		if (split[0] != 0) {
			int type = 0;

			char* arg;
			char* pargs[64];
			int n = 0;
			int i = 0;

			char* outstream = 0;

			do {
				arg = gc(format(split[i]));

				if (!strcmp(arg, ">")) {
					type = 1;
				} else if (type == 1) {
					outstream = arg;
					type = 0;
				} else {
					pargs[n++] = arg;
				}
				i++;
			} while (arg != 0);

			fsexec(gc(format(split[0])), pargs, 0, outstream, 0);

			printf("\n");
		}
	}

	return 0;
}
