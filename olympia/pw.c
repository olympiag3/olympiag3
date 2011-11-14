#include	<stdio.h>
#include	"z.h"

char *read_pw(char *type)
{
	FILE *fp;
	char buf[LEN];
	int i;
	char *p;

	fp = fopen("PWS", "r");

	if (fp == NULL) {
		perror("can't read password file PWS");
		return '\0';
	}

	i = 0;

	while (fgets(buf, LEN, fp) != NULL) {
		for (p = buf; *p && *p != '\n'; p++)
			;
		*p = '\0';

		if (*buf == '\0')
			continue;

		if (strcmp(buf, type) >= 0) {

         for (p = buf+strlen(type); *p && !iswhite(*p); p++)
				;
			while (iswhite(*p))
				p++;

			return str_save(p);
		}
		i++;
	}

	fclose(fp);
   return '\0';

}
