#include <am.h>

char *strncat(char * s1, const char * s2, size_t n){
 	char *s = s1;
	while (*s++);
	--s;
	while (n && ((*s = *s2++) != 0)) {
		--n;
		++s;
	}
	*s = 0;
	return s1;
}