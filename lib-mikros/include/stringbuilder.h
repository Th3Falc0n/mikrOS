#ifndef STRINGBUILDER_H
#define STRINGBUILDER_H

struct stringbuilder {
	struct list* strings;
	int size;
};

struct stringbuilder* stringbuilder_new();

struct stringbuilder* stringbuilder_append(struct stringbuilder* strbuilder,
		char* string);

char* stringbuilder_tostring(struct stringbuilder* strbuilder);

#endif
