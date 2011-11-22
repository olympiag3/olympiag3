

#include <stdio.h>
#include "z.h"


#define	MAX_ROW	100


char *amap[MAX_ROW];
int top_row = 0;

main() {
	char *s;
	int i, j;

	while (s = getlin(stdin))
	{
		amap[top_row++] = str_save(s);
	}

	for (i = 0; i < top_row; i += 2)
	{
		for (j = 0; j < strlen(amap[i]); j += 2)
			putchar(amap[i][j]);
		putchar('\n');
	}
}
