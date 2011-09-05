
#include	<stdio.h>
#include	"z.h"
#include	"oly.h"


/*
 *  G2 Entity coding system:
 *
 *  range	      extent	use
 *  1-999               999	items
 *  1000-8999	       8000	chars
 *  9000-9999          1000	skills
 *
 *  10,000-19,999    10,000	provinces		(CCNN: AA00-DV99)
 *  20,000-49,999    20,000	more provinces		(CCNN: DW00-ZZ99)
 *  50,000-56,759      6760	player entities		(CCN)
 *  56,760-58,759      2000	lucky locs		(CNN)
 *  58,760-58,999	240	regions			(NNNNN)
 *  59,000-78,999    20,000	sublocs, misc		(CNNN: A000-Z999)
 *  79,000-100,000   21,000     storms			(NNNNN)
 *
 *  Note: restricted alphabet, no vowels (except a) or l:
 *	"abcdfghjkmnpqrstvwxz"
 */


static char *letters = "abcdefghijklmnopqrstuvwxyz";
static char *letters2 = "abcdfghjkmnpqrstvwxz";


int
letter_val(char c, char *let)
{
	char *p;

	for (p = let; *p; p++)
		if (*p == c)
			return p-let;

	return 0;	/* error */
}


char *
int_to_code(int l)
{
	int n, a, b, c;

	if (l < 10000)
		return sout("%d", l);

	if (l < 50000)		/* CCNN */
	{
		l -= 10000;

		n = l % 100;
		l /= 100;

		a = l % 20;
		b = l / 20;

		return sout("%c%c%02d", letters2[b], letters2[a], n);
	}

	if (l < 56760)		/* CCN */
	{
		l -= 50000;

		n = l % 10;
		l /= 10;

		a = l % 26;
		b = l / 26;

		return sout("%c%c%d", letters[b], letters[a], n);
	}

	else if (l < 58760)		/* CNN */
	{
		l -= 56760;

		n = l % 10;
		l /= 10;

		a = l % 10;
		b = l / 10;

		return sout("%c%d%d", letters2[b], a, n);
	}

	if (l < 59000)
	{
		return sout("%d", l);
	}

	if (l < 79000)
	{
		l -= 59000;

		a = l / 1000;
		n = l % 1000;

		return sout("%c%03d", letters2[a], n);
	}

	return sout("%d", l);
}


int
code_to_int(char *s)
{
	char a, b, c, d;

	if (isdigit(*s))
		return atoi(s);

	if (!isalpha(*s))
		return 0;

	switch (strlen(s))
	{
	case 3:
		if (isalpha(*(s+1)) && isdigit(*(s+2)))		/* CCN */
		{
			a = tolower(*s) - 'a';
			b = tolower(*(s+1)) - 'a';
			c = *(s+2) - '0';

			return a * 260 + b * 10 + c + 50000;
		}

		if (isdigit(*(s+1)) && isdigit(*(s+2)))		/* CNN */
		{
			a = letter_val(tolower(*s), letters2);
			b = *(s+1) - '0';
			c = *(s+2) - '0';

			return a * 100 + b * 10 + c + 56760;
		}

		return 0;

	case 4:							/* CCNN */
		if (isalpha(*(s+1)) && isdigit(*(s+2)) && isdigit(*(s+3)))
		{
			a = letter_val(tolower(*s), letters2);
			b = letter_val(tolower(*(s+1)), letters2);
			c = *(s+2) - '0';
			d = *(s+3) - '0';


			return a * 2000 + b * 100 + c * 10 + d + 10000;
		}

		if (isdigit(*(s+1)) && isdigit(*(s+2)) && isdigit(*(s+3)))
		{
			a = letter_val(tolower(*s), letters2);
			b = *(s+1) - '0';
			c = *(s+2) - '0';
			d = *(s+3) - '0';


			return a * 1000 + b * 100 + c * 10 + d + 59000;
		}
		return 0;

	default:
		return 0;
	}
}


void
print_box_usage_sup(int low, int high, char *s)
{
	int i;
	int used = 0, unused = 0;

	for (i = low; i <= high; i++)
	{
		if (bx[i])
			used++;
		else
			unused++;
	}

	fprintf(stderr, "\t%5d - %5d  %4s  %d/%d used (%d%%)\n",
			low, high,
			s,
			used, (high-low), used * 100 / (high - low));
}

void
print_box_usage()
{

	fprintf(stderr, "entity space usage:\n");

	print_box_usage_sup(1000, 9999, "char");
	print_box_usage_sup(20000,59999, "locs");
 	print_box_usage_sup(50000,56759, "play");
	print_box_usage_sup(56760,58759, "lloc");
	print_box_usage_sup(59000,78999, "subl");
	print_box_usage_sup(79900,MAX_BOXES-1, "rest");
}


int
scode(char *s)
{
	int n;

	if (*s == '[' || *s == '(')
		s++;

	return code_to_int(s);
}


char *
name(int n)
{

	assert(valid_box(n));

	if (bx[n]->name)
		return bx[n]->name;

	return "";
}


void
set_name(int n, char *s)
{

	assert(valid_box(n));

	if (bx[n]->name)
		my_free(bx[n]->name);

	bx[n]->name = str_save(s);

	{
		char *t;

		for (t = bx[n]->name; *t; t++)
		{
			switch (*t)
			{
			case '[':
				*t = '{';
				break;

			case ']':
				*t = '}';
				break;
			}
		}
	}
}


void
set_banner(int n, char *s)
{
	struct entity_misc *p;

	p = p_misc(n);

	if (p->display)
	{
		my_free(p->display);
		p->display = NULL;
	}

	if (s && *s)
	{
		if (strlen(s) > 50)
			s[50] = '\0';
		p->display = str_save(s);
	}
}


char *
display_name(int n)
{
	char *s;
	int i;

	if (!valid_box(n))
		return "";

	s = name(n);

	if (s && *s)
		return s;

	switch (kind(n))
	{
	case T_player:		return "Player";
	case T_gate:		return "Gate";
	case T_post:		return "Sign";
	}

	if (i = noble_item(n))
		return cap(plural_item_name(i, 1));

	return cap(subkind_s[subkind(n)]);
}


char *
display_kind(int n)
{
	char *s;

	switch (subkind(n))
	{
	case sub_city:
		if (is_port_city(n))
			s = "port city";
		else
			s = "city";
		break;

	case sub_fog:
	case sub_rain:
	case sub_wind:
		s = "storm";
		break;

	default:
		s = subkind_s[subkind(n)];
	}

	return s;
}


/*
 *  Same as box code, less the brackets
 */

char *
box_code_less(int n)
{

	return int_to_code(n);
}


char *
box_code(int n)
{
	char *s;

	if (n == garrison_magic)
		return "Garrison";

	return sout("[%s]", int_to_code(n));
}


char *
box_name(int n)
{
	char *s;

	if (n == garrison_magic)
		return "Garrison";

	if (valid_box(n))
	{
		s = display_name(n);
		if (s && *s)
		{
			return sout("%s~%s", s, box_code(n));
		}
	}

	return box_code(n);
}


char *
just_name(int n)
{
	char *s;

	if (n == garrison_magic)
		return "Garrison";

	if (valid_box(n))
	{
		s = display_name(n);
		if (s && *s)
			return s;
	}

	return box_code(n);
}


char *
plural_item_name(int item, int qty)
{
	char *s;

	if (qty == 1)
		return display_name(item);

	s = rp_item(item) ? rp_item(item)->plural_name : "";

	if (s == NULL || *s == '\0')
	{
		fprintf(stderr, "warning: plural name not set for "
				"item %s\n", box_code(item));
		s = display_name(item);
	}

	return s;
}


char *
plural_item_box(int item, int qty)
{
	char *s;

	if (qty == 1)
		return box_name(item);

	s = plural_item_name(item, qty);

	return sout("%s~%s", s, box_code(item));
}


char *
just_name_qty(int item, int qty)
{

	return sout("%s~%s", nice_num(qty), plural_item_name(item, qty));
}


char *
box_name_qty(int item, int qty)
{

	return sout("%s~%s", nice_num(qty), plural_item_box(item, qty));
}


char *
box_name_kind(int n)
{

	return sout("%s, %s", box_name(n), display_kind(n));
}



/*
 *  Routines for allocating entities and threading like entities
 *  together (kind_first, kind_next)
 */

static void
add_next_chain(int n)
{
	int kind;
	int i;
	static int cache_last = 0;
	static int cache_kind = 0;

	assert(bx[n] != NULL);
	kind = bx[n]->kind;

	if (kind == 0)
		return;

/*  optim! */

	if (cache_kind == kind &&
	    n > cache_last &&
	    bx[cache_last]->x_next_kind == 0)
	{
		bx[cache_last]->x_next_kind = n;
		bx[n]->x_next_kind = 0;
		cache_last = n;
		return;
	}

	cache_last = n;
	cache_kind = kind;

	if (box_head[kind] == 0)
	{
		box_head[kind] = n;
		bx[n]->x_next_kind = 0;
		return;
	}

	if (n < box_head[kind])
	{
		bx[n]->x_next_kind = box_head[kind];
		box_head[kind] = n;
		return;
	}

	i = box_head[kind];
	while (bx[i]->x_next_kind > 0 && bx[i]->x_next_kind < n)
		i = bx[i]->x_next_kind;

	bx[n]->x_next_kind = bx[i]->x_next_kind;
	bx[i]->x_next_kind = n;
}


static void
remove_next_chain(int n)
{
	int i;

	assert(bx[n] != NULL);

	i = box_head[bx[n]->kind];

	if (i == n)
	{
		box_head[bx[n]->kind] = bx[n]->x_next_kind;
	}
	else
	{
		while (i > 0 && bx[i]->x_next_kind != n)
			i = bx[i]->x_next_kind;

		assert(i > 0);

		bx[i]->x_next_kind = bx[n]->x_next_kind;
	}

	bx[n]->x_next_kind = 0;
}


static void
add_sub_chain(int n)
{
	int kind;
	int i;
	static int cache_last = 0;
	static int cache_kind = -1;

	assert(bx[n] != NULL);
	kind = bx[n]->skind;

/*  optim! */

	if (cache_kind == kind &&
	    n > cache_last &&
	    bx[cache_last]->x_next_sub == 0)
	{
		bx[cache_last]->x_next_sub = n;
		bx[n]->x_next_sub = 0;
		cache_last = n;
		return;
	}

	cache_last = n;
	cache_kind = kind;

	if (sub_head[kind] == 0)
	{
		sub_head[kind] = n;
		bx[n]->x_next_sub = 0;
		return;
	}

	if (n < sub_head[kind])
	{
		bx[n]->x_next_sub = sub_head[kind];
		sub_head[kind] = n;
		return;
	}

	i = sub_head[kind];
	while (bx[i]->x_next_sub > 0 && bx[i]->x_next_sub < n)
		i = bx[i]->x_next_sub;

	bx[n]->x_next_sub = bx[i]->x_next_sub;
	bx[i]->x_next_sub = n;
}


static void
remove_sub_chain(int n)
{
	int i;

	assert(bx[n] != NULL);

	i = sub_head[bx[n]->skind];

	if (i == n)
	{
		sub_head[bx[n]->skind] = bx[n]->x_next_sub;
	}
	else
	{
		while (i > 0 && bx[i]->x_next_sub != n)
			i = bx[i]->x_next_sub;

		assert(i > 0);

		bx[i]->x_next_sub = bx[n]->x_next_sub;
	}

	bx[n]->x_next_sub = 0;
}


void
delete_box(int n)
{

	remove_next_chain(n);
	remove_sub_chain(n);
	bx[n]->kind = T_deleted;
}


void
change_box_kind(int n, int kind)
{

	remove_next_chain(n);
	bx[n]->kind = kind;
	add_next_chain(n);
}


void
change_box_subkind(int n, int sk)
{

	if (subkind(n) == sk)
		return;

	remove_sub_chain(n);
	bx[n]->skind = sk;
	add_sub_chain(n);
}


void
alloc_box(int n, int kind, int sk)
{

	assert(n > 0 && n < MAX_BOXES);

	if (bx[n] != NULL)
	{
		fprintf(stderr, "alloc_box: DUP box %d\n", n);
		assert(FALSE);
	}

	bx[n] = (struct box *) my_malloc(sizeof(struct box));
	bx[n]->kind = kind;
	bx[n]->skind = sk;
	add_next_chain(n);
	add_sub_chain(n);
}
 

#if 0
static int
okay_entity_code(int n)
{
	char *s, *p;

	s = box_code_less(n);

	for (p = s; *p; p++)
		if (*p == 'o' || *p == '0' ||
		    *p == 'l' || *p == '1' || *p == 'i')
		return FALSE;

	return TRUE;
}
#endif
 

static int
rnd_alloc_num(int low, int high)
{
	int n;
	int i;

	n = rnd(low, high);

#if 0
	for (i = n; i <= high; i++)
		if (bx[i] == NULL && okay_entity_code(i))
			return i;

	for (i = low; i < n; i++)
		if (bx[i] == NULL && okay_entity_code(i))
			return i;
#endif

	for (i = n; i <= high; i++)
		if (bx[i] == NULL)
			return i;

	for (i = low; i < n; i++)
		if (bx[i] == NULL)
			return i;

#if 0
	fprintf(stderr, "rnd_alloc_num(%d,%d) failed\n", low, high);
#endif

	return -1;
}



int
new_ent(int kind, int sk)
{
	int n = -1;

	switch (kind)
	{
	case T_player:
		n = rnd_alloc_num(50000, 56759);
		break;

	case T_char:
	case T_unform:
		if (sk == sub_ni)
			n = rnd_alloc_num(79000, MAX_BOXES);
		else
			n = rnd_alloc_num(1000,9999);

		if (n < 0)
			n = rnd_alloc_num(59000, 78999);

		if (n < 0)
			n = rnd_alloc_num(79000, MAX_BOXES);
		break;

	case T_skill:
		assert(FALSE);
		break;

	case T_loc:
		if (sk == sub_city)
		{
			n = rnd_alloc_num(56760, 58759);
		}
		else if (sk == sub_region)
		{
			n = rnd_alloc_num(58760, 58999);
		}
		else if (sk == sub_under || sk == sub_forest ||
			 sk == sub_ocean || sk == sub_cloud ||
			 sk == sub_tunnel)
		{
			n = rnd_alloc_num(20000, 49999);
		}
		else
		{
			n = rnd_alloc_num(59000, 78999);
		}
		break;

	case T_storm:
		n = rnd_alloc_num(79000, MAX_BOXES);
		break;

	default:
		n = rnd_alloc_num(59000, 78999);
		break;
	}

	if (n < 0)
		n = rnd_alloc_num(79000, MAX_BOXES);

	alloc_box(n, kind, sk);

	return n;
}

