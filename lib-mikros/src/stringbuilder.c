#include "stringbuilder.h"
#include "stdlib.h"
#include "string.h"
#include "list.h"

struct stringbuilder* stringbuilder_new() {
	struct stringbuilder* strbuilder = malloc(sizeof(struct stringbuilder));

	strbuilder->strings = list_new();
	strbuilder->size = 0;

	return strbuilder;
}

struct stringbuilder* stringbuilder_append(struct stringbuilder* strbuilder,
		char* string) {
	if (!string)
		return 0;

	strbuilder->size += strlen(string);
	list_append(strbuilder->strings, strclone(string));

	return strbuilder;
}

char* stringbuilder_tostring(struct stringbuilder* strbuilder) {
	char* string = malloc(sizeof(char) * (strbuilder->size + 1));

	struct list* lst = strbuilder->strings;
	int pos = 0;

	while ((lst = lst->next)) {
		char* str = lst->data;
		int len = strlen(str);

		memcpy(string + pos, str, len * sizeof(char));

		free(str);

		pos += len;
	}
	string[pos] = '\0';

	free(strbuilder);

	return string;
}
