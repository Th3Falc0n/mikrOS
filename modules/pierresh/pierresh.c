#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "process.h"
#include "string.h"

static char* sp = NULL; /* the start position of the string */

static char* strtokncesc(char* str, const char* delimiters,
		const char* escChars) {
	int i = 0;
	int len = strlen(delimiters);

	int escI = 0;
	int escLen = strlen(escChars);
	int esc = 0;

	/* check in the delimiters */
	if (len == 0)
		return 0;

	/* if the original string has nothing left */
	if (!str && !sp)
		return 0;

	/* initialize the sp during the first call */
	if (str) {
		sp = str;
	}

	/* find the start of the substring, skip delimiters */
	char* p_start = sp;

	while (1) {
		for (i = 0; i < len; i++) {
			if (*p_start == delimiters[i]) {
				p_start++;
				break;
			}
		}

		if (i == len) {
			sp = p_start;
			break;
		}
	}

	/* return NULL if nothing left */
	if (*sp == '\0') {
		sp = NULL;
		return sp;
	}

	/* find the end of the substring, and replace the delimiter with null*/
	while (*sp != '\0') {
		for (escI = 0; escI < escLen; escI++) {
			if (*sp == escChars[escI])
				esc = !esc;
		}
		if (!esc) {
			for (i = 0; i < len; i++) {
				if (*sp == delimiters[i]) {
					*sp = '\0';
					break;
				}
			}
		}

		sp++;
		if (i < len)
			break;
	}

	return p_start;
}

static char* strreplall(char* str, const char* patterns) {
	if (!str)
		return NULL ;

	int len = strlen(str);
	int i;
	int pos = 0;

	int patLen = strlen(patterns);
	int patI;
	int pat;

	char* ret = malloc(sizeof(char) * len + 1);

	for (i = 0; i < len; i++) {
		pat = 0;
		for (patI = 0; patI < patLen; patI++) {
			if (str[i] == patterns[patI]) {
				pat = 1;
				break;
			}
		}
		if (!pat) {
			ret[pos] = str[i];
			pos++;
		}
	}
	ret[pos] = '\0';

	return ret;
}

int main(int argc, char* args[]) {
	char epath[512];
	char instr[512];

	cd("ibin");

	while (1) {
		getExecPath(epath);
		printf("#%s> ", epath);
		getln(instr);

		char* cmd = strreplall(strtokncesc(instr, " ", "\""), "\"");

		if (cmd != 0) {
			int type = 0;

			char* arg;
			char* pargs[64];
			int n = 0;

			char* outstream = NULL;

			do {
				arg = strreplall(strtokncesc(0, " ", "\""), "\"");

				if (strcmp(arg, ">\0")) {
					type = 1;
				} else if (type == 1) {
					outstream = arg;
					type = 0;
				} else {
					pargs[n++] = arg;
				}
			} while (arg != 0);

			int pid = dexec(cmd, pargs);
			if (outstream) setpstdout(outstream, pid);
			while(pexists(pid)) yield();

			printf("\n");
		}
	}

	return 0;
}
