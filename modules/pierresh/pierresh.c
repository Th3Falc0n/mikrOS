#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "process.h"
#include "string.h"
#include "list.h"

static char* format(char* str) {
	char* tmp = replaceAll(str, "\"", "", "\\");
	char* ret = replaceAll(tmp, "\\\"", "\"", "");
	free(tmp);
	return ret;
}

static void interpret(struct list* args) {
	struct list* i = list_first(args);

	if (i) {
		int type = 0;
		struct list* newArgs = list_new();
		char* outstream = 0;

		while ((i = i->next)) {
			char* str = i->data;
			if (!strcmp(str, ">")) {
				type = 0;
			} else if (type == 1) {
				outstream = str;
				type = 0;
			} else {
				list_append(newArgs, str);
			}
		}

		fsexec(list_first(args)->data, (char**) list_toarray(newArgs, -1), 0,
				outstream, 0);

		printf("\n");
	}
}

static void func_reformat(struct list* entry) {
	char* oldStr = (char*) entry->data;
	entry->data = format(oldStr);
	free(oldStr);
}

int main(int argc, char* args[]) {
	char epath[512];
	char instr[512];

	cd("ibin");

	while (1) {
		getExecPath(epath);
		printf("#%s> ", epath);
		getln(instr);

		char** array = split(instr, " ", "\"", "\\");

		struct list* newArgs = list_fromarray((void**) array, -1);
		free(array);

		list_foreach(newArgs, func_reformat);

		interpret(newArgs);
	}

	return 0;
}
