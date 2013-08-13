
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#ifdef _WIN32
#include	<libc/unistd.h>
#else
#include	<unistd.h>
#endif
#include	"z.h"




int malloc_size = 0;
int realloc_size = 0;


/*
 *  malloc safety checks:
 *
 *      Space for three extra ints is is allocated beyond what the client
 *      asks for.  The size of the malloc'd region is stored at the
 *      beginning, followed by a magic number (0xDEADBEEF), then a magic
 *	number is placed at the end of the region (0xBABEFACE).  realloc's
 *      and free's check the integrity of these markers.  This protects
 *      against overruns, makes sure that non-malloc'd memory isn't freed,
 *      and that memory isn't freed twice.
 *
 *	-------------------------------------
 *	0-3	malloced size + 2*sizeof(int)
 *	4-7	0xDEADBEEF
 *	...	client memory
 *	n	0xBABEFACE
 *	-------------------------------------
 *
 *  Any assertion failures in this file indicate that the caller
 *  to the respective function has done something bad, either by
 *  overrunning a region returned from malloc/realloc, or by calling
 *  free with a pointer that wasn't obtained from malloc or free.
 */

void *my_malloc(unsigned size)
{
	char *p;

	size += sizeof(int)*2;
	p = malloc(size + sizeof(int));

	if (p == NULL)
	{
		fprintf(stderr, "my_malloc: out of memory (can't malloc "
				"%d bytes)\n", size);
		assert(0);
		exit(1);
	}

	memset(p, '\0', size);

	*((int *) p) = size;
	*((int *) (p+sizeof(int))) = 0xDEADBEEF;
	*((int *) (p + size)) = 0xBABEFACE;

	return p + sizeof(int)*2;
}


void *my_realloc(void *ptr, unsigned size)
{
	char *p = ptr;
  
	if (p == NULL)
		return my_malloc(size);

	p -= sizeof(int)*2;

	assert(*((int *) (p+sizeof(int))) == 0xDEADBEEF);
	assert(*((int *) (p + *(int *) p)) == 0xBABEFACE);

	size += sizeof(int)-1;		/* integer alignment */
	size -= size % sizeof(int);

	size += sizeof(int)*2;

	p = realloc(p, size + sizeof(int));

	*((int *)p) = size;
	*((int *) (p+sizeof(int))) = 0xDEADBEEF;
	*((int *) (p + size)) = 0xBABEFACE;

	if (p == NULL)
	{
		fprintf(stderr, "my_realloc: out of memory (can't realloc "
				"%d bytes)\n", size);
		assert(0);
		exit(1);
	}

	return p + sizeof(int)*2;
}


void my_free(void *ptr)
{
	char *p = ptr;
  
	p -= sizeof(int)*2;

	assert(*((int *) (p+sizeof(int))) == 0xDEADBEEF);
	assert(*((int *) (p + *(int *) p)) == 0xBABEFACE);
	*((int *) (p + *(int *) p)) = 0;
	*((int *) p) = 0;

	free(p);
}


char *
str_save(char *s)
{
	char *p;

	p = my_malloc(strlen(s) + 1);
	strcpy(p, s);

	return p;
}


void
asfail(char *file, int line, char *cond)
{
	fprintf(stderr, "assertion failure: %s (%d): %s\n",
						file, line, cond);
	abort();
	exit(1);
}


void
lcase(s)
char *s;
{

	while (*s)
	{
		*s = tolower(*s);
		s++;
	}
}


/*
 *  Line reader with no size limits
 *  strips newline off end of line
 */

#define	GETLIN_ALLOC	4096

char *
getlin(FILE *fp)
{
	static char *buf = NULL;
	static unsigned int size = 0;
	unsigned int len;
	int c;

	len = 0;

	while ((c = fgetc(fp)) != EOF)
	{
		if (len >= size)
		{
			size += GETLIN_ALLOC;
			buf = my_realloc(buf, size + 1);
		}

		if (c == '\n')
		{
			buf[len] = '\0';
			return buf;
		}

		buf[len++] = (char) c;
	}

	if (len == 0)
		return NULL;

	buf[len] = '\0';
#if 1
	if (len >= 1023)
		buf[1023] = '\0';
#endif

	return buf;
}


char *
eat_leading_trailing_whitespace(char *s)
{
	char *t;

	while (*s && iswhite(*s))
		s++;			/* eat leading whitespace */

	for (t = s; *t; t++)
		;

	t--;
	while (t >= s && iswhite(*t))
	{				/* eat trailing whitespace */
		*t = '\0';
		t--;
	}

	return s;
}


/*
 *  Get line, remove leading and trailing whitespace
 */

char *
getlin_ew(FILE *fp)
{
	char *line;
	char *p;

	line = getlin(fp);

	if (line)
	{
		while (*line && iswhite(*line))
			line++;			/* eat leading whitespace */

		for (p = line; *p; p++)
			if (*p < 32 || *p == '\t')	/* remove ctrl chars */
				*p = ' ';
		p--;
		while (p >= line && iswhite(*p))
		{				/* eat trailing whitespace */
			*p = '\0';
			p--;
		}
	}

	return line;
}

#define MAX_BUF         8192

static char linebuf[MAX_BUF];
static int nread;
static int line_fd = -1;
static char *point;


void
closefile(char *path) {
	if (line_fd >= 0)
		close(line_fd);
}

int
readfile(char *path)
{


	line_fd = open(path, 0);

	if (line_fd < 0)
	{
		fprintf(stderr, "can't open %s: ", path);
		perror("");
		return FALSE;
	}

	nread = read(line_fd, linebuf, MAX_BUF);
	point = linebuf;

	return TRUE;
}


char *
readlin()
{
	static char *buf = NULL;
	static unsigned int size = 0;
	unsigned int len;
	int c;

	len = 0;

	while (1)
	{
		if (point >= &linebuf[nread])
		{
			if (nread > 0)
				nread = read(line_fd, linebuf, MAX_BUF);

			if (nread < 1)
				break;

			point = linebuf;
		}

		c = *point++;

		if (len >= size)
		{
			size += GETLIN_ALLOC;
			buf = my_realloc(buf, size + 1);
		}

		if (c == '\n')
		{
			buf[len] = '\0';
			if (len >= LEN) {
				buf[LEN-1] = '\0';
			}
			return buf;
		}

		buf[len++] = (char) c;
	}

	if (len == 0)
		return NULL;

	buf[len] = '\0';
	if (len >= LEN) {
		buf[LEN-1] = '\0';
	}

	return buf;
}


char *
readlin_ew()
{
	char *line;
	char *p;

	line = readlin();

	if (line)
	{
		while (*line && iswhite(*line))
			line++;			/* eat leading whitespace */

		for (p = line; *p; p++)
			if (*p < 32 || *p == '\t')	/* remove ctrl chars */
				*p = ' ';
		p--;
		while (p >= line && iswhite(*p))
		{				/* eat trailing whitespace */
			*p = '\0';
			p--;
		}
	}

	return line;
}



#define	COPY_LEN	1024

void
copy_fp(a, b)
FILE *a;
FILE *b;
{
	char buf[COPY_LEN];

	while (fgets(buf, COPY_LEN, a) != NULL)
		fputs(buf, b);
}


char lower_array[256];


void
init_lower()
{
	int i;

	for (i = 0; i < 256; i++)
		lower_array[i] = i;

	for (i = 'A'; i <= 'Z'; i++)
		lower_array[i] = i - 'A' + 'a';
}


int
i_strcmp(char *s, char *t)
{
	char a, b;

	do {
		a = tolower(*s);
		b = tolower(*t);
		s++;
		t++;
		if (a != b)
			return a - b;
	} while (a);

	return 0;
}


int
i_strncmp(char *s, char *t, int n)
{
	char a, b;

	do {
		a = tolower(*s);
		b = tolower(*t);
		if (a != b)
			return a - b;
		s++;
		t++;
		n--;
	} while (a && n > 0);

	return 0;
}


static int
fuzzy_transpose(char *one, char *two, int l1, int l2)
{
	int i;
	char buf[LEN];
	char tmp;

	if (l1 != l2)
		return FALSE;

	strcpy(buf, two);

	for (i = 0; i < l2 - 1; i++)
	{
		tmp = buf[i];
		buf[i] = buf[i+1];
		buf[i+1] = tmp;

		if (i_strcmp(one, buf) == 0)
			return TRUE;

		tmp = buf[i];
		buf[i] = buf[i+1];
		buf[i+1] = tmp;
	}

	return FALSE;
}


static int
fuzzy_one_less(char *one, char *two, int l1, int l2)
{
	int count = 0;
	int i, j;

	if (l1 != l2 + 1)
		return FALSE;

	for (j = 0, i = 0; j < l2; i++, j++)
	{
		if (tolower(one[i]) != tolower(two[j]))
		{
			if (count++)
				return FALSE;
			j--;
		}
	}

	return TRUE;
}


static int
fuzzy_one_extra(char *one, char *two, int l1, int l2)
{
	int count = 0;
	int i, j;

	if (l1 != l2 - 1)
		return FALSE;

	for (j = 0, i = 0; i < l1; i++, j++)
	{
		if (tolower(one[i]) != tolower(two[j]))
		{
			if (count++)
				return FALSE;
			i--;
		}
	}

	return TRUE;
}


static int
fuzzy_one_bad(char *one, char *two, int l1, int l2)
{
	int count = 0;
	int i;

	if (l1 != l2)
		return FALSE;

	for (i = 0; i < l2; i++)
		if (tolower(one[i]) != tolower(two[i]) && count++)
			return FALSE;

	return TRUE;
}


int
fuzzy_strcmp(char *one, char *two)
{
	int l1 = strlen(one);
	int l2 = strlen(two);

	if (l2 >= 4 && fuzzy_transpose(one, two, l1, l2))
		return TRUE;

	if (l2 >= 5 && fuzzy_one_less(one, two, l1, l2))
		return TRUE;

	if (l2 >= 5 && fuzzy_one_extra(one, two, l1, l2))
		return TRUE;

	if (l2 >= 5 && fuzzy_one_bad(one, two, l1, l2))
		return TRUE;

	return FALSE;
}


void
test_random()
{
	int i;

	if (isatty(1))
	    for (i = 0; i < 10; i++)
		printf("%3d  %3d  %3d  %3d  %3d  %3d  %3d  %3d  %3d  %3d\n",
			rnd(1, 10), rnd(1, 10), rnd(1, 10), rnd(1, 10),
			rnd(1, 10), rnd(1, 10), rnd(1, 10), rnd(1, 10),
			rnd(1, 10), rnd(1, 10));
	else
	    for (i = 0; i < 100; i++)
		printf("%d\n", rnd(1, 10));

	for (i = -10; i >= -16; i--)
		printf("rnd(%d, %d) == %d\n", i, -3, rnd(i, -3));

	for (i = 0; i < 100; i++)
		printf("%d\n", rnd(1000,9999));

	{
		ilist l = NULL;
		int i;

		for (i = 1; i <= 10; i++)
			ilist_append(&l, i);

		ilist_scramble(l);

		printf("Scramble:\n");

		for (i = 0; i < ilist_len(l); i++)
			printf("%d\n", l[i]);
	}
}



#define		ILIST_ALLOC	4


/*
 *  Reallocing array handler
 *
 *  Length is stored in ilist[0], maximum in ilist[1].
 *  The user-visible ilist is shifted to &ilist[2], so
 *  that iterations can proceed from index 0.
 */

void
ilist_append(ilist *l, int n)
{
	int *base;

	if (*l == NULL)
	{
		base = my_malloc(sizeof(**l) * ILIST_ALLOC);
		base[1] = ILIST_ALLOC;

		*l = &base[2];
	}
	else
	{
		base = (*l)-2;
		assert(&base[2] == *l);

		if (base[0] + 2 >= base[1])
		{
			base[1] += ILIST_ALLOC;
			base = my_realloc(base, base[1] * sizeof(*base));
			*l = &base[2];
		}
	}

	base[ base[0] + 2] = n;
	base[0]++;
}


void
ilist_prepend(ilist *l, int n)
{
	int *base;
	int i;

	if (*l == NULL)
	{
		base = my_malloc(sizeof(**l) * ILIST_ALLOC);
		base[1] = ILIST_ALLOC;

		*l = &base[2];
	}
	else
	{
		base = (*l)-2;
		assert(&base[2] == *l);

		if (base[0] + 2 >= base[1])
		{
			base[1] += ILIST_ALLOC;
			base = my_realloc(base, base[1] * sizeof(*base));
			*l = &base[2];
		}
	}

	base[0]++;
	for (i = base[0]+1; i > 2; i--)
		base[i] = base[i-1];
	base[2] = n;
}


#if 0

/*  not tested  */

void
ilist_insert(ilist *l, int pos, int n)
{
	int *base;
	int i;

	if (*l == NULL)
	{
		base = my_malloc(sizeof(**l) * ILIST_ALLOC);
		base[1] = ILIST_ALLOC;

		*l = &base[2];
	}
	else
	{
		base = (*l)-2;
		assert(&base[2] == *l);

		if (base[0] + 2 >= base[1])
		{
			base[1] += ILIST_ALLOC;
			base = my_realloc(base, base[1] * sizeof(*base));
			*l = &base[2];
		}
	}

	base[0]++;
	pos += 2;

	for (i = base[0]+1; i > pos; i--)
		base[i] = base[i-1];

	base[pos] = n;
}

#endif


void
ilist_delete(ilist *l, int i)
{
	int *base;
	int j;

	assert(i >= 0 && i < ilist_len(*l));		/* bounds check */
	base = (*l)-2;

	for (j = i+2; j <= base[0]; j++)
		base[j] = base[j+1];

	base[0]--;
}


void
ilist_clear(ilist *l)
{
	int *base;

	if (*l != NULL)
	{
		base = (*l)-2;
		base[0] = 0;
	}
}


void
ilist_reclaim(ilist *l)
{
	int *base;

	if (*l != NULL)
	{
		base = (*l)-2;
		my_free(base);
	}
	*l = NULL;
}


int
ilist_lookup(ilist l, int n)
{
	int i;

	if (l == NULL)
		return -1;

	for (i = 0; i < ilist_len(l); i++)
		if (l[i] == n)
			return i;

	return -1;
}


void
ilist_rem_value(ilist *l, int n)
{
	int i;

	for (i = ilist_len(*l) - 1; i >= 0; i--)
		if ((*l)[i] == n)
			ilist_delete(l, i);
}


void
ilist_rem_value_uniq(ilist *l, int n)
{
	int i;

	for (i = ilist_len(*l) - 1; i >= 0; i--)
		if ((*l)[i] == n)
		{
			ilist_delete(l, i);
			break;
		}
}


#if 1

ilist
ilist_copy(ilist l)
{
	int *base;
	int *copy_base;

	if (l == NULL)
		return NULL;

	base = l-2;
	assert(&base[2] == l);

	copy_base = my_malloc(base[1] * sizeof(*base));
	memcpy(copy_base, base, (base[0] + 2) * sizeof(*base));

	return &copy_base[2];
}

#else

ilist
ilist_copy(ilist l)
{
	ilist new = NULL;
	int i;

	for (i = 0; i < ilist_len(l); i++)
		ilist_append(&new, l[i]);

	return new;
}

#endif


/* 
 *  Knuth, The Art of Computer Programming, Vol. 2 (Addison Wesley).
 *  Essentially, to shuffle A[1]...A[N]:
 *  1) put I = 1;
 *  2) generate a random number R in the range I..N;
 *  3) if R is not I, swap A[R] and A[I];
 *  4) I <- I+1;
 *  5) if I is less than N go to step 2
 */

void
ilist_scramble(ilist l)
{
	int i;
	int tmp;
	int r;
	int len = ilist_len(l) - 1;

	for (i = 0; i < len; i++)
	{
		r = rnd(i, len);
		if (r != i)
		{
			tmp = l[i];
			l[i] = l[r];
			l[r] = tmp;
		}
	}
}

/*
 *	Same as the above, but to store pointers instead of ints
 */

void
plist_append(plist *l, void *n)
{
	int *base;

	if (*l == NULL)
	{
		base = my_malloc(sizeof(**l) * ILIST_ALLOC);
		base[1] = ILIST_ALLOC;

		*l = &base[2];
	}
	else
	{
		base = *l;
		base -= 2;
		assert(&base[2] == *l);

		if (base[0] + 2 >= base[1])
		{
			base[1] += ILIST_ALLOC;
			base = my_realloc(base, base[1] * sizeof(**l));
			*l = &base[2];
		}
	}

	(*l)[base[0]] = n;
	base[0]++;
}


void
plist_prepend(plist *l, void *n)
{
	int *base;
	int i;

	if (*l == NULL)
	{
		base = my_malloc(sizeof(**l) * ILIST_ALLOC);
		base[1] = ILIST_ALLOC;

		*l = &base[2];
	}
	else
	{
		base = *l;
		base -= 2;
		assert(&base[2] == *l);

		if (base[0] + 2 >= base[1])
		{
			base[1] += ILIST_ALLOC;
			base = my_realloc(base, base[1] * sizeof(**l));
			*l = &base[2];
		}
	}

	base[0]++;
	for (i = base[0]-1; i > 0; i--)
		(*l)[i] = (*l)[i-1];
	(*l)[0] = n;
}


void
plist_delete(plist *l, int i)
{
	int *base;
	int j;

	assert(i >= 0 && i < plist_len(*l));		/* bounds check */
	base = *l;
	base -= 2;

	for (j = i; j < base[0] - 1; j++)
		(*l)[j] = (*l)[j+1];

	base[0]--;
}


void
plist_clear(plist *l)
{
	int *base;

	if (*l != NULL)
	{
		base = *l;
		base -= 2;
		base[0] = 0;
	}
}


void
plist_reclaim(plist *l)
{
	int *base;

	if (*l != NULL)
	{
		base = *l;
		base -= 2;
		my_free(base);
	}
	*l = NULL;
}


int
plist_lookup(plist l, void *n)
{
	int i;

	if (l == NULL)
		return -1;

	for (i = 0; i < plist_len(l); i++)
		if (l[i] == n)
			return i;

	return -1;
}


void
plist_rem_value(plist *l, void *n)
{
	int i;

	for (i = plist_len(*l) - 1; i >= 0; i--)
		if ((*l)[i] == n)
			plist_delete(l, i);
}


void
plist_rem_value_uniq(plist *l, void *n)
{
	int i;

	for (i = plist_len(*l) - 1; i >= 0; i--)
		if ((*l)[i] == n)
		{
			plist_delete(l, i);
			break;
		}
}


plist
plist_copy(plist l)
{
	int *base;
	int *copy_base;

	if (l == NULL)
		return NULL;

	base = l;
	base -= 2;
	assert(&base[2] == l);

	copy_base = my_malloc(base[1] * sizeof(*l));
	memcpy(copy_base, base, base[0] * sizeof(*l) + 2 * sizeof(int));

	return &copy_base[2];
}

void
plist_scramble(plist l)
{
	int i;
	void *tmp;
	int r;
	int len = plist_len(l) - 1;

	for (i = 0; i < len; i++)
	{
		r = rnd(i, len);
		if (r != i)
		{
			tmp = l[i];
			l[i] = l[r];
			l[r] = tmp;
		}
	}
}


void
ilist_test()
{
	int i;
	ilist x;
	ilist y;

	setbuf(stdout, NULL);
	memset(&x, '\0', sizeof (x));

	printf("len = %d\n", ilist_len(x));

	for (i = 0; i < 100; i++)
		ilist_append(&x, i);

	assert(x[ilist_len(x)-1] == 99);

	printf("len = %d\n", ilist_len(x));
	for (i = 0; i < ilist_len(x); i++)
		printf("%d ", x[i]);
	printf("\n");

	for (i = 900; i < 1000; i++)
	{
		ilist_prepend(&x, i);
		if (x[ilist_len(x)-1] != 99)
			fprintf(stderr, "fail: i = %d\n", i);
	}

	printf("len = %d\n", ilist_len(x));
	for (i = 0; i < ilist_len(x); i++)
		printf("%d ", x[i]);
	printf("\n");

	ilist_delete(&x, 100);

	printf("len = %d\n", ilist_len(x));
	for (i = 0; i < ilist_len(x); i++)
		printf("%d ", x[i]);
	printf("\n");

	printf("len before = %d\n", ilist_len(x));
	ilist_append(&x, 15);
	printf("len after = %d\n", ilist_len(x));
	printf("x[0] = %d\n", x[0]);

	printf("ilist_lookup(998) == %d\n", ilist_lookup(x, 998));

	y = ilist_copy(x);
	assert(ilist_len(x) == ilist_len(y));
	for (i = 0; i < ilist_len(x); i++)
	{
		assert(&x[i] != &y[i]);
		if (x[i] != y[i])
		{
			fprintf(stderr, "[%d] different\n", i);
			assert(FALSE);
		}
	}

	printf("ilist_lookup(998) == %d\n", ilist_lookup(x, 998));

	ilist_clear(&x);
	assert(ilist_len(x) == 0);
}

