#include <am.h>
char *strstr(const char *s1, const char *s2){
	const char *s = s1;
	const char *p = s2;
	do {
		if (!*p) {
			return (char *) s1;;
		}
		if (*p == *s) {
			++p;
			++s;
		} else {
			p = s2;
			if (!*s) {
				return NULL;
			}
			s = ++s1;
		}
	} while (1);
}