#include "string.h"
#include "stdlib.h"
#include "list.h"

void* memset(void* buf, int c, size_t n) {
    unsigned char* p = buf;

    while (n--) {
        *p++ = c;
    }

    return buf;
}

void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;

    if(n == 0) return 0;

    while (n--) {
        *(d++) = *(s++);
    }

    return dest;
}

int memcmp(const void* ptr1, const void* ptr2, size_t num) {
    if (ptr1 == 0)
        return -1;
    if (ptr2 == 0)
        return 1;
    if (num == 0)
        return 0;

    const uint8_t* pa = ptr1;
    const uint8_t* pb = ptr2;

    for (uint32_t i = 0; i < num; i++) {
        if (pa[i] != pb[i]) {
            return pa[i] - pb[i];
        }
    }
    return 0;
}

int strcmp(const char* str1, const char* str2) {
    int l1 = strlen(str1);
    int l2 = strlen(str2);
    l1 = l1 < l2 ? l1 : l2;

    return memcmp(str1, str2, (l1 + 1) * sizeof(char));
}

unsigned int strlen(const char* str) {
    if (str == 0)
        return 0;
    int i;
    for (i = 0; str[i] != '\0'; i++)
        ;
    return i;
}

char* strcpy(char* dest, const char* src) {
    memcpy(dest, src, (strlen(src) + 1) * sizeof(char));
    return dest;
}

char* strclone(char* str) {
    char* ret = malloc(sizeof(char) * (strlen(str) + 1));
    strcpy(ret, str);

    return ret;
}

static void* fr = NULL;

char* strtok(char* str, const char* delimiters) {
    if(str != 0) { //TODO better but if you call strtok and after that strtoknc will still be a memory leak. -> Don't use strtoknc
        if(fr != 0) {
            free(fr);
        }
        fr = strclone(str);
        return strtoknc(fr, delimiters);
    }
    return strtoknc(0, delimiters);
}

static char* sp = NULL; /* the start position of the string */

char* strtoknc(char* str, const char* delimiters) {
    int i = 0;
    int len = strlen(delimiters);

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
        for (i = 0; i < len; i++) {
            if (*sp == delimiters[i]) {
                *sp = '\0';
                break;
            }
        }

        sp++;
        if (i < len)
            break;
    }

    return p_start;
}

//

char* strtokncesc(char* str, const char* delimiters,
		const char* escChars, const char* escEscChars) {
	int i = 0;
	int len = strlen(delimiters);
	int lenEsc = strlen(escChars);
	int lenEscEsc = strlen(escEscChars);

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
		sp = 0;
		return sp;
	}

	int esc = 0;
	int escEsc;

	/* find the end of the substring, and replace the delimiter with null*/
	while (*sp != '\0') {
		escEsc = 0;
		for (int i2 = 0; i2 < lenEscEsc; i2++) {
			if (*sp == escEscChars[i2])
				escEsc = 1;
		}

		if (!escEsc) {
			for (int i2 = 0; i2 < lenEsc; i2++) {
				if (*sp == escChars[i2])
					esc = !esc;
			}
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

char** split(char* str, const char* delimiters, const char* escChars,
		const char* escEscChars) {
	struct list* lst = list_new();
	list_append(lst, strtokncesc(str, delimiters, escChars, escEscChars));

	char* part;
	while ((part = strtokncesc(0, delimiters, escChars, escEscChars))) {
		list_append(lst, part);
	}

	char** ret = (char**)list_toarray(lst, -1);

	list_free(lst);

	return ret;
}

char* replaceAll(const char* str, const char* pattern, const char* repl, const char* escChars) {
	if (!str)
		return 0;

	int lenPat = strlen(pattern);
	int lenRepl = strlen(repl);
	int lenEsc = strlen(escChars);
	int lenStr = 0;
	int lenRet = 0;

	int pat;
	int esc = 0;

	// Get the length of str and calculate the length of the new string
	while (str[lenStr] != '\0') {
		pat = 1;

		if (!esc) {
			for (int i = 0; i < lenPat; i++) {
				if (pattern[i] != str[lenStr + i]) {
					pat = 0;
					break;
				}
			}
		}

		esc = 0;
		for (int i = 0; i < lenEsc; i++) {
			if (escChars[i] == str[lenStr]) {
				esc = 1;
				break;
			}
		}

		if (pat) {
			lenRet += lenRepl;
			lenStr += lenPat;
		} else {
			lenRet++;
			lenStr++;
		}
	}

	// Fill the new string
	char* ret = malloc(sizeof(char) * (lenRet + 1));
	int pos = 0;

	esc = 0;

	int i = 0;
	while (i < lenStr) {
		pat = 1;

		if (!esc) {
			for (int i2 = 0; i2 < lenPat; i2++) {
				if (pattern[i2] != str[i + i2]) {
					pat = 0;
					break;
				}
			}
		} else {
			pat = 0;
		}

		esc = 0;
		for (int i2 = 0; i2 < lenEsc; i2++) {
			if (escChars[i2] == str[i]) {
				esc = 1;
				break;
			}
		}

		if (pat) {
			for (int i2 = 0; i2 < lenRepl; i2++) {
				ret[pos + i2] = repl[i2];
			}
			pos += lenRepl;
			i += lenPat;
		} else {
			ret[pos] = str[i];
			pos++;
			i++;
		}
	}
	ret[pos] = '\0';

	return ret;
}
