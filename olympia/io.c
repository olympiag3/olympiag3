
#include	<stdio.h>
#include	<string.h>
#include	<sys/types.h>
#include	<libc/sys/stat.h>
#include	<libc/dirent.h>
#include	<libc/unistd.h>
#include	"z.h"
#include	"oly.h"



/*
 *  io.c -- load and save the entity database
 */


/*
 *  linehash macro
 *
 *	data is stored in the form:
 *		xy data
 *	were 'xy' is a key followed by one character of whitespace
 *
 *	linehash macro returns 'xy' crunched into an int
 *
 *  Example:
 *
 *	s = "na Name field";
 *	c = linehash(s);
 *	assert(c == 'na');
 */

#define	linehash(t)	(strlen(t) < 2 ? 0 : ((t[0]) << 8 | (t[1])))
#define	t_string(t)	(strlen(t) >= 4 ? &t[3] : "")


/*
 *  advance lets us do a one-line lookahead in the scanning routines
 */

static char *line;

#define		advance		line = readlin()


static int ext_boxnum;


int seed_has_been_run = FALSE;


/*
 *  Routine to check if a structure is completely empty.
 *
 *  Since structures may have elements which are not saved by save_db(),
 *  this routine may return true when, in fact, no data from the structure
 *  will be saved.  However, the next turn run will clear this up, since
 *  the re-loaded empty structure will now pass zero_check.
 *
 *  Using zero_check is more reliable than element testing, since we might
 *  forget to add one to the check list.  Also, our concern is long-term
 *  buildup of unused empty structure, so keeping one around for a turn
 *  is not a big deal.
 */

static int
zero_check(void *ptr, unsigned size)
{
	char *p = ptr;
	int sum = 0;

	while (size-- > 0)
	{
		sum |= *p++;
	}

	return sum == 0;
}


/*
 *  Returns the entity name in parenthesis, if available, to make the
 *  data files easier to read.
 */

static char *
if_name(int num)		/* to make the data files easier to read */
{
	char *s;
	extern int pretty_data_files;

	if (!pretty_data_files)
		return "";

	if (!valid_box(num))
		return "";

	s = name(num);

	if (s && *s)
		return sout(" (%s)", s);

	return "";
}


static int
box_scan(char *t)
{
	int n;

	n = atoi(t);

	if (valid_box(n))
		return n;

	fprintf(stderr, "box_scan(%d): bad reference: %s\n", ext_boxnum, line);

	return 0;
}


static void
box_print(FILE *fp, char *header, int n)
{

	if (valid_box(n))
		fprintf(fp, "%s%d%s\n", header, n, if_name(n));
}



/*
 *  boxlist0_scan, boxlist0_print:
 *  same as boxlist_xxx, but allows zero
 */

static void
boxlist0_scan(char *s, int box_num, ilist *l)
{
	int n;

	while (1)
	{
		while (*s && iswhite(*s))
			s++;

		if (isdigit(*s))
		{
			n = atoi(s);

			if (n == 0 || valid_box(n))
				ilist_append(l, n);
			else
				fprintf(stderr, "boxlist_scan(%d): bad box "
					"reference: %d\n", box_num, n);

			while (*s && isdigit(*s))
				s++;
		}
		else if (*s == '\\')		/* continuation line follows */
			s = readlin_ew();
		else
			break;
	}
}


static void
boxlist0_print(FILE *fp, char *header, ilist l)
{
	int i;
	int count = 0;

	for (i = 0; i < ilist_len(l); i++)
		if (l[i] == 0 || valid_box(l[i]))
		{
			count++;
			if (count == 1)
				fputs(header, fp);

			if (count % 11 == 10)		/* continuation line */
				fputs("\\\n\t", fp);

			fprintf(fp, "%d ", l[i]);
		}

	if (count)
		fprintf(fp, "\n");
}



static void
boxlist_scan(char *s, int box_num, ilist *l)
{
	int n;

	while (1)
	{
		while (*s && iswhite(*s))
			s++;

		if (isdigit(*s))
		{
			n = atoi(s);

			if (valid_box(n))
				ilist_append(l, n);
			else
				fprintf(stderr, "boxlist_scan(%d): bad box "
					"reference: %d\n", box_num, n);

			while (*s && isdigit(*s))
				s++;
		}
		else if (*s == '\\')		/* continuation line follows */
			s = readlin_ew();
		else
			break;
	}
}


static void
boxlist_print(FILE *fp, char *header, ilist l)
{
	int i;
	int count = 0;

	for (i = 0; i < ilist_len(l); i++)
		if (valid_box(l[i]))
		{
			count++;
			if (count == 1)
				fputs(header, fp);

			if (count % 11 == 10)		/* continuation line */
				fputs("\\\n\t", fp);

			fprintf(fp, "%d ", l[i]);
		}

	if (count)
		fprintf(fp, "\n");
}


static void
admit_print_sup(FILE *fp, struct admit *p)
{
	int i;
	int count = 2;

	if (!valid_box(p->targ))
		return;

	if (p->sense == 0 && ilist_len(p->l) == 0)
		return;

	fprintf(fp, " am %d %d ", p->targ, p->sense);

	for (i = 0; i < ilist_len(p->l); i++)
		if (valid_box(p->l[i]))
		{
			count++;

			if (count % 11 == 10)		/* continuation line */
				fputs("\\\n\t", fp);

			fprintf(fp, "%d ", p->l[i]);
		}

	if (count)
		fprintf(fp, "\n");
}


static void
admit_print(FILE *fp, struct entity_player *p)
{
	int i;

	for (i = 0; i < ilist_len(p->admits); i++)
		admit_print_sup(fp, p->admits[i]);
}


static void
admit_scan(char *s, int box_num, struct entity_player *pp)
{
	int n;
	int count = 0;
	struct admit *p;

	p = my_malloc(sizeof(*p));

	while (1)
	{
		while (*s && iswhite(*s))
			s++;

		if (isdigit(*s))
		{
			n = atoi(s);

			switch (count++)
			{
			case 0:
				p->targ = n;
				break;

			case 1:
				p->sense = n;
				break;

			default:
				if (valid_box(n))
					ilist_append(&p->l, n);
				else
					fprintf(stderr, "admit_scan(%d): "
					"bad box reference: %d\n", box_num, n);
			}

			while (*s && isdigit(*s))
				s++;
		}
		else if (*s == '\\')		/* continuation line follows */
			s = readlin_ew();
		else
			break;
	}

	if (!valid_box(p->targ))
	{
		fprintf(stderr, "admit_scan(%d): bad targ %d\n",
				box_num, p->targ);
		my_free(p);
		return;
	}

	ilist_append((ilist *) &(pp->admits), (int) p);
}



static void
ilist_print(FILE *fp, char *header, ilist l)
{
	int i;

	if (ilist_len(l) > 0)
	{
		fputs(header, fp);

		for (i = 0; i < ilist_len(l); i++)
		{
			if (i % 11 == 10)		/* continuation line */
				fprintf(fp, "\\\n\t");

			fprintf(fp, "%d ", l[i]);
		}

		fprintf(fp, "\n");
	}
}


static void
ilist_scan(char *s, ilist *l)
{

	while (1)
	{
		while (*s && iswhite(*s))
			s++;

		if (isdigit(*s))
		{
			ilist_append(l, atoi(s));

			while (*s && isdigit(*s))
				s++;
		}
		else if (*s == '\\')		/* continuation line follows */
			s = readlin_ew();
		else
			break;
	}
}


static void
known_print(FILE *fp, char *header, sparse kn)
{
	int i;
	int count = 0;
	int first = TRUE;

	loop_known(kn, i)
	{
		if (!valid_box(i))
			continue;

		if (first)
		{
			fputs(header, fp);
			first = FALSE;
		}

		if (count++ % 11 == 10)
			fprintf(fp, "\\\n\t");

		fprintf(fp, "%d ", i);
	}
	next_known;

	if (!first)
		fprintf(fp, "\n");
}


static void
known_scan(char *s, sparse *kn, int box_num)
{
	int n;

	while (1)
	{
		while (*s && iswhite(*s))
			s++;

		if (isdigit(*s))
		{
			n = atoi(s);

			if (valid_box(n))
				set_bit(kn, n);
			else
				fprintf(stderr, "known_scan(%d): bad box "
					"reference: %d\n", box_num, n);

			while (*s && isdigit(*s))
				s++;
		}
		else if (*s == '\\')		/* continuation line follows */
			s = readlin_ew();
		else
			break;
	}
}


static void
skill_list_print(FILE *fp, char *header, struct skill_ent **l)
{
	int i;
	int count = 0;

	for (i = 0; i < ilist_len(l); i++)
		if (valid_box(l[i]->skill))
		{
			count++;
			if (count == 1)
				fputs(header, fp);

			if (count > 1)
				fputs(" \\\n\t", fp);

			fprintf(fp, "%d %d %d %d 0",
					l[i]->skill,
					l[i]->know,
					l[i]->days_studied,
					l[i]->experience);
		}

	if (count)
		fputs("\n", fp);
}


static void
skill_list_scan(char *s, struct skill_ent ***l, int box_num)
{
	struct skill_ent *new;
	int dummy;
	int know, experience;

	while (1)
	{
		new = my_malloc(sizeof(*new));
		sscanf(s, "%d %d %d %d %d",
					&new->skill,
					&know,
					&new->days_studied,
					&experience,
					&dummy);

		new->know = know;
		new->experience = experience;

		if (valid_box(new->skill))
			ilist_append((ilist *) l, (int) new);
		else
		{
			fprintf(stderr, "skill_list_scan(%d): bad skill %d\n",
					box_num, new->skill);

			my_free(new);
		}

		if (s[strlen(s)-1] == '\\')	/* another entry follows */
			s = readlin_ew();
		else
			break;
	}
}


static void
item_list_print(FILE *fp, char *header, struct item_ent **l)
{
	int i;
	int count = 0;

	for (i = 0; i < ilist_len(l); i++)
		if (valid_box(l[i]->item) && l[i]->qty > 0)
		{
			count++;
			if (count == 1)
				fputs(header, fp);

			if (count > 1)
				fputs(" \\\n\t", fp);

			fprintf(fp, "%d %d",
					l[i]->item,
					l[i]->qty);
		}

	if (count)
		fputs("\n", fp);
}


static void
item_list_scan(char *s, struct item_ent ***l, int box_num)
{
	struct item_ent *new;

	while (1)
	{
		new = my_malloc(sizeof(*new));
		sscanf(s, "%d %d", &new->item, &new->qty);

		if (valid_box(new->item))
			ilist_append((ilist *) l, (int) new);
		else
		{
			fprintf(stderr, "item_list_scan(%d): bad item %d\n",
					box_num, new->item);

			my_free(new);
		}

		if (s[strlen(s)-1] == '\\')	/* another entry follows */
			s = readlin_ew();
		else
			break;
	}
}


static void
trade_list_print(FILE *fp, char *header, struct trade **l)
{
	int i;
	int count = 0;

	for (i = 0; i < ilist_len(l); i++)
		if (valid_box(l[i]->item))
		{
/*
 *  Weed out completed or cleared BUY and SELL trades, but don't
 *  touch PRODUCE or CONSUME zero-qty trades.
 */

			if ((l[i]->kind == BUY || l[i]->kind == SELL) &&
			    l[i]->qty <= 0)
				continue;

			count++;
			if (count == 1)
				fputs(header, fp);

			if (count > 1)
				fputs(" \\\n\t", fp);

			fprintf(fp, "%d %d %d %d %d %d %d %d",
					l[i]->kind,
					l[i]->item,
					l[i]->qty,
					l[i]->cost,
					l[i]->cloak,
					l[i]->have_left,
					l[i]->month_prod,
					l[i]->expire);
		}

	if (count)
		fputs("\n", fp);
}


static void
trade_list_scan(char *s, struct trade ***l, int box_num)
{
	struct trade *new;

	while (1)
	{
		new = my_malloc(sizeof(*new));
		sscanf(s, "%d %d %d %d %d %d %d %d",
				&new->kind,
				&new->item,
				&new->qty,
				&new->cost,
				&new->cloak,
				&new->have_left,
				&new->month_prod,
				&new->expire);

		new->who = box_num;

		if (valid_box(new->item))
			ilist_append((ilist *) l, (int) new);
		else
		{
			fprintf(stderr, "trade_list_scan(%d): bad item %d\n",
					box_num, new->item);

			my_free(new);
		}

		if (s[strlen(s)-1] == '\\')	/* another entry follows */
			s = readlin_ew();
		else
			break;
	}
}


static void
req_list_print(FILE *fp, char *header, struct req_ent **l)
{
	int i;
	int count = 0;

	for (i = 0; i < ilist_len(l); i++)
		if (valid_box(l[i]->item))
		{
			count++;
			if (count == 1)
				fputs(header, fp);

			if (count > 1)
				fputs(" \\\n\t", fp);

			fprintf(fp, "%d %d %d",
					l[i]->item,
					l[i]->qty,
					l[i]->consume);
		}

	if (count)
		fputs("\n", fp);
}


static void
req_list_scan(char *s, struct req_ent ***l, int box_num)
{
	struct req_ent *new;
	int consume;

	while (1)
	{
		new = my_malloc(sizeof(*new));
		sscanf(s, "%d %d %d", &new->item, &new->qty, &consume);
		new->consume = consume;

		if (valid_box(new->item))
			ilist_append((ilist *) l, (int) new);
		else
		{
			fprintf(stderr, "req_list_scan(%d): bad item %d\n",
					box_num, new->item);

			my_free(new);
		}

		if (s[strlen(s)-1] == '\\')	/* another entry follows */
			s = readlin_ew();
		else
			break;
	}
}


void
olytime_scan(char *s, olytime *p)
{

	sscanf(s, "%hd %hd %d", &p->turn, &p->day, &p->days_since_epoch);
}


void
olytime_print(FILE *fp, char *header, olytime *p)
{

	if (p->turn || p->day || p->days_since_epoch)
		fprintf(fp, "%s%d %d %d\n", header,
				p->turn, p->day, p->days_since_epoch);
}


static void
print_loc_info(FILE *fp, struct loc_info *p)
{

	if (zero_check(p, sizeof(*p)))
		return;

	fprintf(fp, "LI\n");
	box_print(fp, " wh ", p->where);
	boxlist_print(fp, " hl ", p->here_list);
}


static void
scan_loc_info(struct loc_info *p, int box_num)
{
	char *t;
	int c;

	advance;
	while (line && iswhite(*line))
	{
		if (*line == '#')
			continue;

		line++;
		c = linehash(line);
		t = t_string(line);

		switch (c)
		{
		case 'wh':	p->where = box_scan(t);		break;

		case 'hl':
			boxlist_scan(t, box_num, (ilist *) &(p->here_list));
			break;

		case 0:
		default:
			fprintf(stderr, "scan_loc_info(%d):  bad line: %s\n",
						box_num, line);
		}
		advance;
	}
}


static void
print_magic(FILE *fp, struct char_magic *p)
{

	if (zero_check(p, sizeof(*p)))
		return;

	fprintf(fp, "CM\n");

	if (p->magician)
		fprintf(fp, " im %d\n", p->magician);

	if (p->max_aura)
		fprintf(fp, " ma %d\n", p->max_aura);

	if (p->cur_aura)
		fprintf(fp, " ca %d\n", p->cur_aura);

	if (p->ability_shroud)
		fprintf(fp, " as %d\n", p->ability_shroud);

	if (p->hinder_meditation)
		fprintf(fp, " hm %d\n", p->hinder_meditation);

	if (p->quick_cast)
		fprintf(fp, " qc %d\n", p->quick_cast);

	if (p->aura_reflect)
		fprintf(fp, " rb %d\n", p->aura_reflect);

	if (p->hide_self)
		fprintf(fp, " hs %d\n", p->hide_self);

	if (p->hide_mage)
		fprintf(fp, " cm %d\n", p->hide_mage);

	if (p->pray)
		fprintf(fp, " pr %d\n", p->pray);

	if (p->knows_weather)
		fprintf(fp, " kw %d\n", p->knows_weather);

	if (p->default_garr)
		fprintf(fp, " dg %d\n", p->default_garr);

	if (p->swear_on_release)
		fprintf(fp, " sr %d\n", p->swear_on_release);

	if (p->fee)
		fprintf(fp, " bf %d\n", p->fee);

	if (p->vis_protect)
		fprintf(fp, " vp %d\n", p->vis_protect);

	box_print(fp, " pl ", p->pledge);
	box_print(fp, " pc ", p->project_cast);
	box_print(fp, " ar ", p->auraculum);
	box_print(fp, " ot ", p->token);	/* our token artifact */
	known_print(fp, " vi ", p->visions);
}


static void
scan_magic(struct char_magic *p, int box_num)
{
	char *t;
	int c;

	advance;
	while (line && iswhite(*line))
	{
		if (*line == '#')
			continue;

		line++;
		c = linehash(line);
		t = t_string(line);

		switch (c)
		{
		case 'im':	p->magician = atoi(t);		break;
		case 'ma':	p->max_aura = atoi(t);		break;
		case 'ca':	p->cur_aura = atoi(t);		break;
		case 'as':	p->ability_shroud = atoi(t);	break;
		case 'hm':	p->hinder_meditation = atoi(t);	break;
		case 'pc':	p->project_cast = box_scan(t);	break;
		case 'qc':	p->quick_cast = atoi(t);	break;
		case 'ot':	p->token = box_scan(t);		break;
		case 'pl':	p->pledge = box_scan(t);	break;
		case 'ar':	p->auraculum = box_scan(t);	break;
		case 'rb':	p->aura_reflect = atoi(t);	break;
		case 'hs':	p->hide_self = atoi(t);		break;
		case 'cm':	p->hide_mage = atoi(t);		break;
		case 'pr':	p->pray = atoi(t);		break;
		case 'sr':	p->swear_on_release = atoi(t);	break;
		case 'kw':	p->knows_weather = atoi(t);	break;
		case 'vp':	p->vis_protect = atoi(t);	break;
		case 'dg':	p->default_garr = atoi(t);	break;
		case 'bf':	p->fee = atoi(t);		break;

		case 'vi':
			known_scan(t, &p->visions, box_num);
			break;

		case 0:
		default:
			fprintf(stderr, "scan_magic(%d):  bad line: %s\n",
						box_num, line);
		}
		advance;
	}
}


static void
print_char(FILE *fp, struct entity_char *p)
{

	fprintf(fp, "CH\n");

	box_print(fp, " ni ", p->unit_item);
	box_print(fp, " lo ", p->unit_lord);
	box_print(fp, " pl ", p->prev_lord);

	if (p->health)
		fprintf(fp, " he %d\n", p->health);

	if (p->sick)
		fprintf(fp, " si %d\n", p->sick);

	if (p->loy_kind)
		fprintf(fp, " lk %d\n", p->loy_kind);
	if (p->loy_rate)
		fprintf(fp, " lr %d\n", p->loy_rate);

	skill_list_print(fp, " sl\t", p->skills);

	if (p->prisoner)
		fprintf(fp, " pr %d\n", p->prisoner);

	if (p->moving)
		fprintf(fp, " mo %d\n", p->moving);

	if (p->behind)
		fprintf(fp, " bh %d\n", p->behind);

	if (p->guard)
		fprintf(fp, " gu %d\n", p->guard);

	if (p->time_flying)
		fprintf(fp, " tf %d\n", p->time_flying);

	if (p->break_point)
		fprintf(fp, " bp %d\n", p->break_point);

	if (p->rank)
		fprintf(fp, " ra %d\n", p->rank);

	if (p->attack)
		fprintf(fp, " at %d\n", p->attack);

	if (p->defense)
		fprintf(fp, " df %d\n", p->defense);

	if (p->missile)
		fprintf(fp, " mi %d\n", p->missile);

	if (p->npc_prog)
		fprintf(fp, " po %d\n", p->npc_prog);

	boxlist_print(fp, " ct ", p->contact);

	olytime_print(fp, " dt ", &p->death_time);
}


static void
scan_char(struct entity_char *p, int box_num)
{
	char *t;
	int c;

	advance;
	while (line && iswhite(*line))
	{
		if (*line == '#')
			continue;

		line++;
		c = linehash(line);
		t = t_string(line);

		switch (c)
		{
		case 'ni':	p->unit_item = box_scan(t);	break;
		case 'lo':	p->unit_lord = box_scan(t);	break;
		case 'pl':	p->prev_lord = box_scan(t);	break;
		case 'he':	p->health = atoi(t);		break;
		case 'si':	p->sick = atoi(t);		break;
		case 'pr':	p->prisoner = atoi(t);		break;
		case 'mo':	p->moving = atoi(t);		break;
		case 'bh':	p->behind = atoi(t);		break;
		case 'lk':	p->loy_kind = atoi(t);		break;
		case 'lr':	p->loy_rate = atoi(t);		break;
		case 'gu':	p->guard = atoi(t);		break;
		case 'tf':	p->time_flying = atoi(t);	break;
		case 'bp':	p->break_point = atoi(t);	break;
		case 'ra':	p->rank = atoi(t);		break;
		case 'at':	p->attack = atoi(t);		break;
		case 'df':	p->defense = atoi(t);		break;
		case 'mi':	p->missile = atoi(t);		break;
		case 'po':	p->npc_prog = atoi(t);		break;

		case 'ct':
			boxlist_scan(t, box_num, &(p->contact));
			break;

		case 'sl':
			skill_list_scan(t, &p->skills, box_num);
			break;

		case 'dt':
			olytime_scan(t, &p->death_time);
			break;

		case 0:
		default:
			fprintf(stderr, "scan_char(%d):  bad line: %s\n",
						box_num, line);
		}
		advance;
	}
}


static void
print_loc(FILE *fp, struct entity_loc *p)
{

	if (zero_check(p, sizeof(*p)))
		return;

	fprintf(fp, "LO\n");

	boxlist0_print(fp, " pd ", p->prov_dest);

	if (p->hidden)
		fprintf(fp, " hi %d\n", p->hidden);

	if (p->shroud)
		fprintf(fp, " sh %d\n", p->shroud);

	if (p->barrier)
		fprintf(fp, " ba %d\n", p->barrier);

	if (p->dist_from_gate)
		fprintf(fp, " dg %d\n", p->dist_from_gate);

	if (p->sea_lane)
		fprintf(fp, " sl %d\n", p->sea_lane);

	if (p->civ)
		fprintf(fp, " lc %d\n", p->civ);

#if 0
	box_print(fp, " ng ", p->near_grave);
#endif
}


static void
scan_loc(struct entity_loc *p, int box_num)
{
	char *t;
	int c;

	advance;
	while (line && iswhite(*line))
	{
		if (*line == '#')
			continue;

		line++;
		c = linehash(line);
		t = t_string(line);

		switch (c)
		{
		case 'hi':	p->hidden = atoi(t);		break;
		case 'sh':	p->shroud = atoi(t);		break;
		case 'ba':	p->barrier = atoi(t);		break;
		case 'dg':	p->dist_from_gate = atoi(t);	break;
		case 'lc':	p->civ = atoi(t);		break;
		case 'sl':	p->sea_lane = atoi(t);		break;

		case 'pd':
			boxlist0_scan(t, box_num, &(p->prov_dest));
			break;

		case 0:
		default:
			fprintf(stderr, "scan_loc(%d):  bad line: %s\n",
						box_num, line);
		}
		advance;
	}
}


static void
print_subloc(FILE *fp, struct entity_subloc *p)
{

	fprintf(fp, "SL\n");

	boxlist_print(fp, " te ", p->teaches);

	if (p->damage)
		fprintf(fp, " da %d\n", p->damage);

	if (p->defense)
		fprintf(fp, " de %d\n", p->defense);

	if (p->capacity)
		fprintf(fp, " ca %d\n", p->capacity);

	if (p->build_materials)
		fprintf(fp, " bm %d\n", p->build_materials);

	if (p->effort_required)
		fprintf(fp, " er %d\n", p->effort_required);

	if (p->effort_given)
		fprintf(fp, " eg %d\n", p->effort_given);

	if (p->moving)
		fprintf(fp, " mo %d\n", p->moving);

	if (p->galley_ram)
		fprintf(fp, " gr %d\n", p->galley_ram);

	if (p->shaft_depth)
		fprintf(fp, " sd %d\n", p->shaft_depth);

	if (p->castle_lev)
		fprintf(fp, " cl %d\n", p->castle_lev);

	if (p->safe)
		fprintf(fp, " sh %d\n", p->safe);

	if (p->major)
		fprintf(fp, " mc %d\n", p->major);

	if (p->opium_econ)
		fprintf(fp, " op %d\n", p->opium_econ);

	if (p->loot)
		fprintf(fp, " lo %d\n", p->loot);

	if (p->prominence)
		fprintf(fp, " cp %d\n", p->prominence);

	if (p->uldim_flag)
		fprintf(fp, " uf %d\n", p->uldim_flag);

	if (p->summer_flag)
		fprintf(fp, " sf %d\n", p->summer_flag);

	if (p->quest_late)
		fprintf(fp, " ql %d\n", p->quest_late);

	if (p->tunnel_level)
		fprintf(fp, " td %d\n", p->tunnel_level);

	boxlist_print(fp, " nc ", p->near_cities);

	boxlist_print(fp, " lt ", p->link_to);
	boxlist_print(fp, " lf ", p->link_from);
	boxlist_print(fp, " bs ", p->bound_storms);

	if (p->link_when)
		fprintf(fp, " lw %d\n", p->link_when);

	if (p->link_open)
		fprintf(fp, " lp %d\n", p->link_open);
}


static void
scan_subloc(struct entity_subloc *p, int box_num)
{
	char *t;
	int c;

	advance;
	while (line && iswhite(*line))
	{
		if (*line == '#')
			continue;

		line++;
		c = linehash(line);
		t = t_string(line);

		switch (c)
		{
		case 'da':	p->damage = atoi(t);			break;
		case 'de':	p->defense = atoi(t);			break;
		case 'ca':	p->capacity = atoi(t);			break;
		case 'er':	p->effort_required = atoi(t);		break;
		case 'eg':	p->effort_given = atoi(t);		break;
		case 'bm':	p->build_materials = atoi(t);		break;
		case 'mo':	p->moving = atoi(t);			break;
		case 'gr':	p->galley_ram = atoi(t);		break;
		case 'sd':	p->shaft_depth = atoi(t);		break;
		case 'sh':	p->safe = atoi(t);			break;
		case 'mc':	p->major = atoi(t);			break;
		case 'op':	p->opium_econ = atoi(t);		break;
		case 'lo':	p->loot = atoi(t);			break;
		case 'cp':	p->prominence = atoi(t);		break;
		case 'lw':	p->link_when = atoi(t);			break;
		case 'lp':	p->link_open = atoi(t);			break;
		case 'uf':	p->uldim_flag = atoi(t);		break;
		case 'sf':	p->summer_flag = atoi(t);		break;
		case 'ql':	p->quest_late = atoi(t);		break;
		case 'td':	p->tunnel_level = atoi(t);		break;
		case 'cl':	p->castle_lev = atoi(t);		break;

		case 'lt':
			boxlist_scan(t, box_num, &(p->link_to));
			break;

		case 'lf':
			boxlist_scan(t, box_num, &(p->link_from));
			break;

		case 'te':
			boxlist_scan(t, box_num, &(p->teaches));
			break;

		case 'nc':
			boxlist_scan(t, box_num, (ilist *) &(p->near_cities));
			break;

		case 'bs':
			boxlist_scan(t, box_num, (ilist *) &(p->bound_storms));
			break;

		case 0:
		default:
			fprintf(stderr, "scan_subloc(%d):  bad line: %s\n",
						box_num, line);
		}
		advance;
	}
}


static void
print_item(FILE *fp, struct entity_item *p)
{

	if (zero_check(p, sizeof(*p)))
		return;

	fprintf(fp, "IT\n");

	if (p->plural_name && *p->plural_name)
		fprintf(fp, " pl %s\n", p->plural_name);

	if (p->weight)
		fprintf(fp, " wt %d\n", p->weight);

	if (p->land_cap)
		fprintf(fp, " lc %d\n", p->land_cap);

	if (p->ride_cap)
		fprintf(fp, " rc %d\n", p->ride_cap);

	if (p->ride_cap)
		fprintf(fp, " fc %d\n", p->fly_cap);

	if (p->is_man_item)
		fprintf(fp, " mu %d\n", p->is_man_item);

	if (p->prominent)
		fprintf(fp, " pr %d\n", p->prominent);

	if (p->animal)
		fprintf(fp, " an %d\n", p->animal);

	if (p->attack)
		fprintf(fp, " at %d\n", p->attack);

	if (p->defense)
		fprintf(fp, " df %d\n", p->defense);

	if (p->missile)
		fprintf(fp, " mi %d\n", p->missile);

	if (p->base_price)
		fprintf(fp, " bp %d\n", p->base_price);

	if (p->capturable)
		fprintf(fp, " ca %d\n", p->capturable);

	box_print(fp, " un ", p->who_has);
}


static void
scan_item(struct entity_item *p, int box_num)
{
	char *t;
	int c;

	advance;
	while (line && iswhite(*line))
	{
		if (*line == '#')
			continue;

		line++;
		c = linehash(line);
		t = t_string(line);

		switch (c)
		{
		case 'pl':	p->plural_name = str_save(t);	break;
		case 'wt':	p->weight = atoi(t);		break;
		case 'lc':	p->land_cap = atoi(t);		break;
		case 'rc':	p->ride_cap = atoi(t);		break;
		case 'fc':	p->fly_cap = atoi(t);		break;
		case 'mu':	p->is_man_item = atoi(t);	break;
		case 'pr':	p->prominent = atoi(t);		break;
		case 'an':	p->animal = atoi(t);		break;
		case 'un':	p->who_has = box_scan(t);	break;
		case 'at':	p->attack = atoi(t);		break;
		case 'df':	p->defense = atoi(t);		break;
		case 'mi':	p->missile = atoi(t);		break;
		case 'bp':	p->base_price = atoi(t);	break;
		case 'ca':	p->capturable = atoi(t);	break;

		case 0:
		default:
			fprintf(stderr, "scan_item(%d):  bad line: %s\n",
						box_num, line);
		}
		advance;
	}
}


static void
print_item_magic(FILE *fp, struct item_magic *p)
{

	if (zero_check(p, sizeof(*p)))
		return;

	fprintf(fp, "IM\n");

	if (p->aura)
		fprintf(fp, " au %d\n", p->aura);

	if (p->curse_loyalty)
		fprintf(fp, " cl %d\n", p->curse_loyalty);

	if (p->cloak_region)
		fprintf(fp, " cr %d\n", p->cloak_region);

	if (p->cloak_creator)
		fprintf(fp, " cc %d\n", p->cloak_creator);

	if (p->use_key)
		fprintf(fp, " uk %d\n", p->use_key);

	if (p->quick_cast)
		fprintf(fp, " qc %d\n", p->quick_cast);

	if (p->attack_bonus)
		fprintf(fp, " ab %d\n", p->attack_bonus);

	if (p->defense_bonus)
		fprintf(fp, " db %d\n", p->defense_bonus);

	if (p->missile_bonus)
		fprintf(fp, " mb %d\n", p->missile_bonus);

	if (p->aura_bonus)
		fprintf(fp, " ba %d\n", p->aura_bonus);

	if (p->relic_decay)
		fprintf(fp, " rd %d\n", p->relic_decay);

	if (p->token_num)
		fprintf(fp, " tn %d\n", p->token_num);

	if (p->orb_use_count)
		fprintf(fp, " oc %d\n", p->orb_use_count);

	box_print(fp, " ti ", p->token_ni);

	box_print(fp, " rc ", p->region_created);
	box_print(fp, " pc ", p->project_cast);
	box_print(fp, " ct ", p->creator);
	box_print(fp, " lo ", p->lore);

	boxlist_print(fp, " mu ", p->may_use);
	boxlist_print(fp, " ms ", p->may_study);
}


static void
scan_item_magic(struct item_magic *p, int box_num)
{
	char *t;
	int c;

	advance;
	while (line && iswhite(*line))
	{
		if (*line == '#')
			continue;

		line++;
		c = linehash(line);
		t = t_string(line);

		switch (c)
		{
		case 'au':	p->aura = atoi(t);			break;
		case 'cl':	p->curse_loyalty = atoi(t);		break;
		case 'cr':	p->cloak_region = atoi(t);		break;
		case 'cc':	p->cloak_creator = atoi(t);		break;
		case 'uk':	p->use_key = atoi(t);			break;
		case 'rc':	p->region_created = box_scan(t);	break;
		case 'pc':	p->project_cast = box_scan(t);		break;
		case 'ct':	p->creator = box_scan(t);		break;
		case 'lo':	p->lore = box_scan(t);			break;
		case 'qc':	p->quick_cast = atoi(t);		break;
		case 'ab':	p->attack_bonus = atoi(t);		break;
		case 'db':	p->defense_bonus = atoi(t);		break;
		case 'mb':	p->missile_bonus = atoi(t);		break;
		case 'ba':	p->aura_bonus = atoi(t);		break;
		case 'rd':	p->relic_decay = atoi(t);		break;
		case 'tn':	p->token_num = atoi(t);			break;
		case 'ti':	p->token_ni = atoi(t);			break;
		case 'oc':	p->orb_use_count = atoi(t);		break;

		case 'mu':
			boxlist_scan(t, box_num, &(p->may_use));
			break;

		case 'ms':
			boxlist_scan(t, box_num, &(p->may_study));
			break;

		case 0:
		default:
			fprintf(stderr, "scan_item_magic(%d):  bad line: %s\n",
						box_num, line);
		}
		advance;
	}
}


static void
print_player(FILE *fp, struct entity_player *p)
{

	fprintf(fp, "PL\n");

	if (p->full_name && *(p->full_name))
		fprintf(fp, " fn %s\n", p->full_name);

	if (p->email && *(p->email))
		fprintf(fp, " em %s\n", p->email);

	if (p->vis_email && *(p->vis_email))
		fprintf(fp, " ve %s\n", p->vis_email);

	if (p->password && *(p->password))
		fprintf(fp, " pw %s\n", p->password);

	if (p->noble_points)
		fprintf(fp, " np %d\n", p->noble_points);

	if (p->fast_study)
		fprintf(fp, " fs %d\n", p->fast_study);

	if (p->first_turn)
		fprintf(fp, " ft %d\n", p->first_turn);

	if (p->format)
		fprintf(fp, " fo %d\n", p->format);

	if (p->notab)
		fprintf(fp, " nt %d\n", p->notab);

	if (p->first_tower)
		fprintf(fp, " tf %d\n", p->first_tower);

	if (p->split_lines)
		fprintf(fp, " sl %d\n", p->split_lines);

	if (p->split_bytes)
		fprintf(fp, " sb %d\n", p->split_bytes);

	if (p->sent_orders)
		fprintf(fp, " so %d\n", p->sent_orders);

	if (p->dont_remind)
		fprintf(fp, " dr %d\n", p->dont_remind);

	if (p->compuserve)
		fprintf(fp, " ci %d\n", p->compuserve);

	if (p->broken_mailer)
		fprintf(fp, " bm %d\n", p->broken_mailer);

	if (p->last_order_turn)
		fprintf(fp, " lt %d\n", p->last_order_turn);

	known_print(fp, " kn ", p->known);
	boxlist_print(fp, " un ", p->units);
	boxlist_print(fp, " uf ", p->unformed);
	admit_print(fp, p);
}


static void
scan_player(struct entity_player *p, int box_num)
{
	char *t;
	int c;

	advance;
	while (line && iswhite(*line))
	{
		if (*line == '#')
			continue;

		line++;
		c = linehash(line);
		t = t_string(line);

		switch (c)
		{
		case 'fn':	p->full_name = str_save(t);	break;
		case 'em':	p->email = str_save(t);		break;
		case 've':	p->vis_email = str_save(t);	break;
		case 'pw':	p->password = str_save(t);	break;
		case 'np':	p->noble_points = atoi(t);	break;
		case 'fs':	p->fast_study = atoi(t);	break;
		case 'ft':	p->first_turn = atoi(t);	break;
		case 'fo':	p->format = atoi(t);		break;
		case 'nt':	p->notab = atoi(t);		break;
		case 'tf':	p->first_tower = atoi(t);	break;
		case 'so':	p->sent_orders = atoi(t);	break;
		case 'lt':	p->last_order_turn = atoi(t);	break;
		case 'sl':	p->split_lines = atoi(t);	break;
		case 'sb':	p->split_bytes = atoi(t);	break;
		case 'ci':	p->compuserve = atoi(t);	break;
		case 'bm':	p->broken_mailer = atoi(t);	break;
		case 'dr':	p->dont_remind = atoi(t);	break;

		case 'kn':
			known_scan(t, &p->known, box_num);
			break;

		case 'un':
			boxlist_scan(t, box_num, &(p->units));
			break;

		case 'uf':
			boxlist_scan(t, box_num, &(p->unformed));
			break;

		case 'am':
			admit_scan(t, box_num, p);
			break;

		case 0:
		default:
			fprintf(stderr, "scan_player(%d):  bad line: %s\n",
						box_num, line);
		}
		advance;
	}
}


static void
print_skill(FILE *fp, struct entity_skill *p)
{

	fprintf(fp, "SK\n");
	fprintf(fp, " tl %d\n", p->time_to_learn);
	box_print(fp, " rs ", p->required_skill);

	boxlist_print(fp, " of ", p->offered);
	boxlist_print(fp, " re ", p->research);
	req_list_print(fp, " rq\t", p->req);
	box_print(fp, " pr ", p->produced);

	if (p->np_req)
		fprintf(fp, " np %d\n", p->np_req);
	if (p->no_exp)
		fprintf(fp, " ne %d\n", p->no_exp);
}


static void
scan_skill(struct entity_skill *p, int box_num)
{
	char *t;
	int c;

	advance;
	while (line && iswhite(*line))
	{
		if (*line == '#')
			continue;

		line++;
		c = linehash(line);
		t = t_string(line);

		switch (c)
		{
		case 'tl':	p->time_to_learn = atoi(t);	 break;
		case 'ne':	p->no_exp = atoi(t);		 break;
		case 'np':	p->np_req = atoi(t);		 break;
		case 'rs':	p->required_skill = box_scan(t); break;
		case 'pr':	p->produced = box_scan(t);	 break;

		case 'of':
			boxlist_scan(t, box_num, &(p->offered));
			break;

		case 're':
			boxlist_scan(t, box_num, &(p->research));
			break;

		case 'rq':
			req_list_scan(t, &p->req, box_num);
			break;

		case 0:
		default:
			fprintf(stderr, "scan_skill(%d):  bad line: %s\n",
						box_num, line);
		}
		advance;
	}
}


static void
print_command(FILE *fp, struct command *p)
{

	if (p->cmd == 0)
		return;

	fprintf(fp, "CO\n");
	fprintf(fp, " li %s\n", p->line);

	fprintf(fp, " ar %d %d %d %d %d %d %d %d\n",
			p->a, p->b, p->c, p->d, p->e, p->f, p->g, p->h);

	if (p->state)
		fprintf(fp, " cs %d\n", p->state);

	if (p->wait)
		fprintf(fp, " wa %d\n", p->wait);

	if (p->status)
		fprintf(fp, " st %d\n", p->status);

	if (p->use_skill)
		box_print(fp, " us ", p->use_skill);

	if (p->use_exp)
		fprintf(fp, " ue %d\n", p->use_exp);

	if (p->days_executing)
		fprintf(fp, " de %d\n", p->days_executing);

	if (p->poll)
		fprintf(fp, " po %d\n", p->poll);

	if (p->pri)
		fprintf(fp, " pr %d\n", p->pri);

	if (p->inhibit_finish)
		fprintf(fp, " if %d\n", p->inhibit_finish);
}


static void
scan_command(struct command *p, int box_num)
{
	char *t;
	int c;

	p->who = box_num;

	advance;
	while (line && iswhite(*line))
	{
		if (*line == '#')
			continue;

		line++;
		c = linehash(line);
		t = t_string(line);

		switch (c)
		{
		case 'li':
			if (!oly_parse_cmd(p, t))
			{
				fprintf(stderr, "scan_command(%d): "
					"bad cmd %s\n", box_num, t);
			}
			break;

		case 'cs':	p->state = atoi(t);		break;
		case 'wa':	p->wait = atoi(t);		break;
		case 'st':	p->status = atoi(t);		break;
		case 'de':	p->days_executing = atoi(t);	break;
		case 'po':	p->poll = atoi(t);		break;
		case 'pr':	p->pri = atoi(t);		break;
		case 'if':	p->inhibit_finish = atoi(t);	break;
		case 'us':	p->use_skill = box_scan(t);	break;
		case 'ue':	p->use_exp = atoi(t);		break;

		case 'ar':
			sscanf(t, "%d %d %d %d %d %d %d %d",
				&p->a, &p->b, &p->c, &p->d,
				&p->e, &p->f, &p->g, &p->h);
			break;

		case 0:
		default:
			fprintf(stderr, "scan_command(%d):  bad line: %s\n",
						box_num, line);
		}
		advance;
	}
}


static void
print_gate(FILE *fp, struct entity_gate *p)
{

	fprintf(fp, "GA\n");

	box_print(fp, " tl ", p->to_loc);

	if (p->notify_jumps)
		box_print(fp, " nj ", p->notify_jumps);

	if (p->notify_unseal)
		box_print(fp, " nu ", p->notify_unseal);

	if (p->seal_key)
		fprintf(fp, " sk %d\n", p->seal_key);

	if (p->road_hidden)
		fprintf(fp, " rh %d\n", p->road_hidden);
}


static void
scan_gate(struct entity_gate *p, int box_num)
{
	char *t;
	int c;

	advance;
	while (line && iswhite(*line))
	{
		if (*line == '#')
			continue;

		line++;
		c = linehash(line);
		t = t_string(line);

		switch (c)
		{
		case 'tl':	p->to_loc = box_scan(t);	break;
		case 'nj':	p->notify_jumps = box_scan(t);	break;
		case 'nu':	p->notify_unseal = box_scan(t);	break;
		case 'sk':	p->seal_key = atoi(t);		break;
		case 'rh':	p->road_hidden = atoi(t);	break;

		case 0:
		default:
			fprintf(stderr, "scan_gate(%d):  bad line: %s\n",
						box_num, line);
		}
		advance;
	}
}


static void
print_misc(FILE *fp, struct entity_misc *p)
{

	if (zero_check(p, sizeof(*p)))
		return;

	fprintf(fp, "MI\n");

	box_print(fp, " sb ", p->summoned_by);

	if (p->npc_dir)
		fprintf(fp, " di %d\n", p->npc_dir);

	if (p->npc_created)
		fprintf(fp, " mc %d\n", p->npc_created);

	if (p->mine_delay)
		fprintf(fp, " md %d\n", p->mine_delay);

	if (p->storm_str)
		fprintf(fp, " ss %d\n", p->storm_str);

	if (p->cmd_allow)
		fprintf(fp, " ca %c\n", p->cmd_allow);

	box_print(fp, " gc ", p->garr_castle);
	box_print(fp, " mh ", p->npc_home);
	box_print(fp, " co ", p->npc_cookie);
	box_print(fp, " ov ", p->only_vulnerable);
	box_print(fp, " ol ", p->old_lord);
	box_print(fp, " bs ", p->bind_storm);

	if (p->save_name && *p->save_name)
		fprintf(fp, " sn %s\n", p->save_name);

	if (p->display && *p->display)
		fprintf(fp, " ds %s\n", p->display);

	known_print(fp, " nm ", p->npc_memory);
}


static void
scan_misc(struct entity_misc *p, int box_num)
{
	char *t;
	int c;

	advance;
	while (line && iswhite(*line))
	{
		if (*line == '#')
			continue;

		line++;
		c = linehash(line);
		t = t_string(line);

		switch (c)
		{
		case 'di':	p->npc_dir = atoi(t);			break;
		case 'mc':	p->npc_created = atoi(t);		break;
		case 'md':	p->mine_delay = atoi(t);		break;
		case 'ss':	p->storm_str = atoi(t);			break;
		case 'mh':	p->npc_home = box_scan(t);		break;
		case 'gc':	p->garr_castle = box_scan(t);		break;
		case 'sb':	p->summoned_by = box_scan(t);		break;
		case 'co':	p->npc_cookie = box_scan(t);		break;
		case 'ov':	p->only_vulnerable = box_scan(t);	break;
		case 'bs':	p->bind_storm = box_scan(t);		break;
		case 'ol':	p->old_lord = box_scan(t);		break;
		case 'sn':	p->save_name = str_save(t);		break;
		case 'ds':	p->display = str_save(t);		break;
		case 'ca':	p->cmd_allow = *t;			break;

		case 'nm':
			known_scan(t, &p->npc_memory, box_num);
			break;

		case 0:
		default:
			fprintf(stderr, "scan_misc(%d):  bad line: %s\n",
						box_num, line);
		}
		advance;
	}
}


static void
load_box(int n)
{
	int c;
	char *t;
	struct box *p;

/*
 *  The fast scan of libdir/master should have allocated all of the boxes
 *  If one was later added manually, it won't have been pre-allocated by
 *  the master fast scan.  Remove master file and let io.c do the slow
 *  scan to allocate all the boxes.  Save the database to recreate master.
 */

	if (!valid_box(n))
	{
		fprintf(stderr, "Unforseen box %d found in load phase.\n", n);
		fprintf(stderr, "Remove %s/master and retry.\n", libdir);
		assert(FALSE);
	}

	p = bx[n];

	ext_boxnum = n;

	advance;
	while (line && *line)
	{
		if (*line == '#')
		{
			advance;
			continue;
		}

		c = linehash(line);
		t = t_string(line);

		switch (c)
		{
		case 'na':
			set_name(n, t);
			advance;
			break;

		case 'il':
			item_list_scan(t, &bx[n]->items, n);
			advance;
			break;

		case 'tl':
			trade_list_scan(t, &bx[n]->trades, n);
			advance;
			break;

		case 'an':
			boxlist_scan(t, n, &(p_disp(n)->neutral));
			advance;
			break;

		case 'ad':
			boxlist_scan(t, n, &(p_disp(n)->defend));
			advance;
			break;

		case 'ah':
			boxlist_scan(t, n, &(p_disp(n)->hostile));
			advance;
			break;

		case 'CH':	scan_char(p_char(n), n);		break;
		case 'CM':	scan_magic(p_magic(n), n);		break;
		case 'LI':	scan_loc_info(p_loc_info(n), n);	break;
		case 'LO':	scan_loc(p_loc(n), n);			break;
		case 'SL':	scan_subloc(p_subloc(n), n);		break;
		case 'IT':	scan_item(p_item(n), n);		break;
		case 'PL':	scan_player(p_player(n), n);		break;
		case 'SK':	scan_skill(p_skill(n), n);		break;
		case 'GA':	scan_gate(p_gate(n), n);		break;
		case 'MI':	scan_misc(p_misc(n), n);		break;
		case 'IM':	scan_item_magic(p_item_magic(n), n);	break;
		case 'CO':	scan_command(p_command(n), n);		break;

		case 0:
		default:
			fprintf(stderr, "load_box(%d):  bad line: %s\n",
						n, line);

			advance;
			while (line && iswhite(*line))
				advance;
			continue;
		}
	}

	if (line)
	{
		assert(*line == '\0');
		advance;	/* advance over blank separating line */
	}
}


void
save_box(FILE *fp, int n)
{
	struct box *p;
	struct att_ent *pd;
	void *vp;

	if (kind(n) == T_deleted)
		return;

	assert(valid_box(n));

	p = bx[n];

	if (bx[n]->skind)
		fprintf(fp, "%d %s %s\n",
			n, kind_s[bx[n]->kind], subkind_s[bx[n]->skind]);
	else
		fprintf(fp, "%d %s 0\n",
			n, kind_s[bx[n]->kind]);

	if (p->name && *(p->name))
		fprintf(fp, "na %s\n", p->name);

	item_list_print(fp, "il\t", p->items);
	trade_list_print(fp, "tl\t", p->trades);

	if (pd = rp_disp(n))
	{
		boxlist_print(fp, "an ", pd->neutral);
		boxlist_print(fp, "ad ", pd->defend);
		boxlist_print(fp, "ah ", pd->hostile);
	}

	if (vp = rp_loc_info(n))	print_loc_info(fp, vp);
	if (vp = rp_char(n))		print_char(fp, vp);
	if (vp = rp_magic(n))		print_magic(fp, vp);
	if (vp = rp_loc(n))		print_loc(fp, vp);
	if (vp = rp_subloc(n))		print_subloc(fp, vp);
	if (vp = rp_item(n))		print_item(fp, vp);
	if (vp = rp_item_magic(n))	print_item_magic(fp, vp);
	if (vp = rp_player(n))		print_player(fp, vp);
	if (vp = rp_skill(n))		print_skill(fp, vp);
	if (vp = rp_gate(n))		print_gate(fp, vp);
	if (vp = rp_misc(n))		print_misc(fp, vp);
	if (vp = rp_command(n))		print_command(fp, vp);

	fprintf(fp, "\n");

	bx[n]->temp = 1;		/* mark for write_leftovers() */
}


static FILE *
open_write_fp(char *fnam)
{
	char *path;
	FILE *fp;

	path = sout("%s/%s", libdir, fnam);

	fp = fopen(path, "w");

	if (fp == NULL)
	{
		fprintf(stderr, "open_write_fp: can't open %s: ", path);
		perror("");
	}

	return fp;
}


static void
write_kind(int box_kind, char *fnam)
{
	FILE *fp;
	int i;

	fp = open_write_fp(fnam);

	if (fp != NULL)
	{
		loop_kind(box_kind, i)
		{
			save_box(fp, i);
		}
		next_kind;

		fclose(fp);
	}
}


void
write_player(int pl)
{
	int who;
	FILE *fp;

	fp = open_write_fp(sout("fact/%d", pl));

	if (fp == NULL)
		return;

	save_box(fp, pl);

	loop_units(pl, who)
	{
		assert(kind(who) == T_char || kind(who) == T_deleted);

		save_box(fp, who);
	}
	next_unit;

	fclose(fp);
}


static void
write_chars()
{
	int i;

	loop_player(i)
	{
		write_player(i);
	}
	next_player;
}


static void
write_leftovers()
{
	FILE *fp;
	int i;

	fp = open_write_fp("misc");

	if (fp != NULL)
	{
		for (i = 0; i < MAX_BOXES; i++)
			if (bx[i] != NULL && bx[i]->temp == 0)
			{
				if (kind(i) != T_deleted)
					save_box(fp, i);
			}

		fclose(fp);
	}
}

static void close_box(char *fnam) {
	char *path;
	path = sout("%s/%s", libdir, fnam);
	closefile(path);
}

static void
read_boxes(char *fnam)
{
	int box_num;
	char *path;

	path = sout("%s/%s", libdir, fnam);
	if (!readfile(path))
		return;

	advance;

	while (line)
	{
		if (*line == '#')
			continue;		/* skip comment lines */

		box_num = atoi(line);
		if (box_num > 0)
			load_box(box_num);
		else
			fprintf(stderr, "read_boxes: unexpected line %s\n",
								line);
	}

	close_box(fnam);

}


static void
read_chars()
{
	DIR *d;
	struct dirent *e;
	char *fnam;
	char *last_e;

	fnam = sout("%s/fact", libdir);
	d = opendir(fnam);

	if (d == NULL)
	{
		fprintf(stderr, "read_chars: can't open %s: ", fnam);
		perror("");
		return;
	}

	while ((e = readdir(d)) != NULL)
	{
		if (*(e->d_name) >= '0' && *(e->d_name) <= '9')
		{
			last_e = sout("fact/%s", e->d_name);
			read_boxes(last_e);
		}
	}

	closedir(d);
}


static int
fast_scan()
{
	char *path;
	char *s, *p, *q;
	int num;
	int kind, sk;

	path = sout("%s/master", libdir);
	if (!readfile(path))
		return FALSE;

	while (s = readlin())
	{
		num = atoi(s);

		for (p = s; *p && isdigit(*p); p++)
			;
		while (*p && iswhite(*p))
			p++;




		for (q = p; *q && *q != '.'; q++)
			;
		if (*q == '.')
			*q++ = '\0';

		kind = atoi(p);
		sk = atoi(q);

		alloc_box(num, kind, sk);
	}

	return TRUE;
}


static void
scan_boxes(char *fnam)
{
	int box_num;
	char *path;
	char *s, *t;
	int kind;
	int sk;

	path = sout("%s/%s", libdir, fnam);
	if (!readfile(path))
		return;

	while (s = readlin())
	{
		if (*s == '#')
			continue;		/* skip comment lines */

/*
 *  Parse something of the form: box-number kind subkind
 *  example:  10 item artifact
 */

		box_num = atoi(s);

		while (*s && *s != ' ')		/* skip over space to kind */
			s++;
		if (*s == ' ')
			s++;

		for (t = s; *t && *t != ' '; t++)	/* to subkind */
			;
		if (*t == ' ')
			*t++ = '\0';

		kind = lookup(kind_s, s);

		if (kind < 0)
		{
			fprintf(stderr, "read_boxes(%d): bad kind: %s\n",
							box_num, s);
			kind = 0;
		}

		if (*t == '0')
			sk = 0;
		else
			sk = lookup(subkind_s, t);

		if (sk < 0)
		{
			fprintf(stderr, "read_boxes(%d): bad subkind: %s\n",
							box_num, t);
			sk = 0;
		}

		alloc_box(box_num, kind, sk);

		while (s = readlin())
		{
			if (*s == '#')		/* skip comments */
				continue;

			while (*s && iswhite(*s))
				s++;

			if (*s == '\0')		/* blank line, end of entry */
				break;
		}
	}
}


static void
scan_chars()
{
	DIR *d;
	struct dirent *e;
	char *fnam;

	fnam = sout("%s/fact", libdir);
	d = opendir(fnam);

	if (d == NULL)
	{
		fprintf(stderr, "scan_chars: can't open %s: ", fnam);
		perror("");
		return;
	}

	while ((e = readdir(d)) != NULL)
	{
		if (*(e->d_name) >= '0' && *(e->d_name) <= '9')
		{
			scan_boxes(sout("fact/%s", e->d_name));
			closefile(sout("fact/%s", e->d_name));
		}
	}

	closedir(d);
}


/*
 *  Scan through all of the entity data files, calling alloc_box
 *  for each entity once its number and kind are known.
 *
 *  We do this so it is possible to perform type and sanity checking when
 *  the contents of the boxes are read in the second pass (read_boxes).
 */

static void
scan_all_boxes()
{

	stage("fast_scan failed, scan_all_boxes()");

	scan_boxes("loc");
	scan_boxes("item");
	scan_boxes("skill");
	scan_boxes("gate");
	scan_boxes("road");
	scan_boxes("ship");
	scan_boxes("unform");
	scan_boxes("misc");

	scan_chars();
}


static void
read_all_boxes()
{

	read_boxes("loc");
	read_boxes("item");
	read_boxes("skill");
	read_boxes("gate");
	read_boxes("road");
	read_boxes("ship");
	read_boxes("unform");
	read_boxes("misc");

	read_chars();
}


static void
write_all_boxes()
{
	int i;

	for (i = 0; i < MAX_BOXES; i++)
		if (bx[i] != NULL)
			bx[i]->temp = 0;

	if (win_flag) {
		system(sout("del /s /Q /F %s\\fact\\*", libdir));
	} else {
		system(sout("rm -rf %s/fact", libdir));
		mkdir(sout("%s/fact", libdir), 0755);
	}

	write_kind(T_loc, "loc");
	write_kind(T_item, "item");
	write_kind(T_skill, "skill");
	write_kind(T_gate, "gate");
	write_kind(T_road, "road");
	write_kind(T_ship, "ship");
	write_kind(T_unform, "unform");

	write_chars();
	write_leftovers();
}


static void
write_master()
{
	FILE *fp;
	char *fnam;
	int i;
	char *s;

	fnam = sout("%s/master", libdir);
	fp = fopen(fnam, "w");

	if (fp == NULL)
	{
		fprintf(stderr, "can't write %s: ", fnam);
		perror("");
		return;
	}

	for (i = 0; i < MAX_BOXES; i++)
		if (bx[i] != NULL)
			bx[i]->temp = 0;

	for (i = 0; i < MAX_BOXES; i++)
	    if (kind(i) != T_deleted)
	    {
		s = name(i);

		switch (kind(i))
		{
		case 0:
			break;

		case T_loc:
		case T_item:
		case T_skill:
		case T_gate:
		case T_ship:
		case T_unform:
			fprintf(fp, "%d\t%d.%d\t%s\t\t%s\n",
					i,
					bx[i]->kind, bx[i]->skind,
					kind_s[bx[i]->kind], s);
			bx[i]->temp = 1;
			break;

		case T_player:
			fprintf(fp, "%d\t%d.%d\tfact/%d\t%s\n",
					i, bx[i]->kind, bx[i]->skind, i, s);
			bx[i]->temp = 1;
			break;

		case T_char:
			fprintf(fp, "%d\t%d.%d\tfact/%d\t%s\n",
					i,
					bx[i]->kind, bx[i]->skind,
					player(i), s);
			bx[i]->temp = 1;
			break;
		}
	    }

	for (i = 0; i < MAX_BOXES; i++)
		if (kind(i) != T_deleted && bx[i]->temp == 0)
		{
			s = name(i);
			fprintf(fp, "%d\t%d.%d\tmisc\t%s\n",
					i, bx[i]->kind, bx[i]->skind, s);
		}

	fclose(fp);
}


static void
load_system()
{
	FILE *fp;
	char *s;
	char *fname;

	fname = sout("%s/system", libdir);

	fp = fopen(fname, "r");

	if (fp == NULL)
	{
		fprintf(stderr, "load_system: can't read %s: ", fname);
		perror("");
		return;
	}

	while (s = getlin_ew(fp))
	{
		if (*s == '\0' || *s == '#')	/* comments and blank lines */
			continue;

		if (strncmp(s, "sysclock:", 9) == 0)
		{
			olytime_scan(&s[9], &sysclock);
		}
		else if (strncmp(s, "from_host=", 10) == 0)
		{
			from_host = str_save(&s[10]);
		}
		else if (strncmp(s, "reply_host=", 11) == 0)
		{
			reply_host = str_save(&s[11]);
		}
		else if (strncmp(s, "indep_player=", 13) == 0)
		{
			indep_player = atoi(&s[13]);
		}
		else if (strncmp(s, "gm_player=", 10) == 0)
		{
			gm_player = atoi(&s[10]);
		}
		else if (strncmp(s, "skill_player=", 13) == 0)
		{
			skill_player = atoi(&s[13]);
		}
		else if (strncmp(s, "post=", 5) == 0)
		{
			post_has_been_run = atoi(&s[5]);
		}
		else if (strncmp(s, "init=", 5) == 0)
		{
			seed_has_been_run = atoi(&s[5]);
		}
		else if (strncmp(s, "fr=", 3) == 0)
		{
			faery_region = atoi(&s[3]);
		}
		else if (strncmp(s, "tr=", 3) == 0)
		{
			tunnel_region = atoi(&s[3]);
		}
		else if (strncmp(s, "ur=", 3) == 0)
		{
			under_region = atoi(&s[3]);
		}
		else if (strncmp(s, "fp=", 3) == 0)
		{
			faery_player = atoi(&s[3]);
		}
		else if (strncmp(s, "hr=", 3) == 0)
		{
			hades_region = atoi(&s[3]);
		}
		else if (strncmp(s, "hp=", 3) == 0)
		{
			hades_pit = atoi(&s[3]);
		}
		else if (strncmp(s, "hl=", 3) == 0)
		{
			hades_player = atoi(&s[3]);
		}
		else if (strncmp(s, "nr=", 3) == 0)
		{
			nowhere_region = atoi(&s[3]);
		}
		else if (strncmp(s, "nl=", 3) == 0)
		{
			nowhere_loc = atoi(&s[3]);
		}
		else if (strncmp(s, "np=", 3) == 0)
		{
			npc_pl = atoi(&s[3]);
		}
		else if (strncmp(s, "cr=", 3) == 0)
		{
			cloud_region = atoi(&s[3]);
		}
		else if (strncmp(s, "cp=", 3) == 0)
		{
			combat_pl = atoi(&s[3]);
		}
		else
			fprintf(stderr, "load_system: unrecognized "
							"line: %s\n", s);
	}

	fclose(fp);
}


static void
save_system()
{
	FILE *fp;
	char *fname;

	fname = sout("%s/system", libdir);

	fp = fopen(fname, "w");

	if (fp == NULL)
	{
		fprintf(stderr, "load_system: can't write %s: ", fname);
		perror("");
		return;
	}

	olytime_print(fp, "sysclock: ", &sysclock);
	fprintf(fp, "indep_player=%d\n", indep_player);
	fprintf(fp, "gm_player=%d\n", gm_player);
	fprintf(fp, "skill_player=%d\n", skill_player);
	fprintf(fp, "from_host=%s\n", from_host);
	fprintf(fp, "reply_host=%s\n", reply_host);
	fprintf(fp, "post=%d\n", post_has_been_run);
	fprintf(fp, "init=%d\n", seed_has_been_run);
	fprintf(fp, "fr=%d\n", faery_region);
	fprintf(fp, "tr=%d\n", tunnel_region);
	fprintf(fp, "ur=%d\n", under_region);
	fprintf(fp, "fp=%d\n", faery_player);
	fprintf(fp, "hr=%d\n", hades_region);
	fprintf(fp, "hp=%d\n", hades_pit);
	fprintf(fp, "hl=%d\n", hades_player);
	fprintf(fp, "nr=%d\n", nowhere_region);
	fprintf(fp, "nl=%d\n", nowhere_loc);
	fprintf(fp, "np=%d\n", npc_pl);
	fprintf(fp, "cr=%d\n", cloud_region);
	fprintf(fp, "cp=%d\n", combat_pl);

	fclose(fp);
}


static void
delete_deadchars()
{
	int i;

	loop_kind(T_deadchar, i)
	{
/*
 *  Loc should have been zeroed already in kill_char
 */

		if (loc(i))
			set_where(i, 0);
		delete_box(i);
	}
	next_kind;
}


void
load_db() 
{
	char *pw;

	stage("load_db()");

/*
 *  Assertions to verify the sanity of the linehash macro
 *  Switch the byte ordering if this fails
 */

	assert(linehash("ab f") == 'ab');
	assert(linehash("") == 0);
	assert(linehash("na") == 'na');
	assert(linehash("ab ") == 'ab');

	load_seed(sout("%s/randseed", libdir));
	load_system();

	if (!fast_scan())	/* pass 1: call alloc_box for each entity */
		scan_all_boxes();

	read_all_boxes();	/* pass 2: read the entity attributes */
	load_orders();

	check_db();			/* check database integrity */
	determine_map_edges();		/* initialization for map routines */

	if (!post_has_been_run)
	{
		stage("INIT: post_production()");
		post_production();
	}

	if (!seed_has_been_run)
	{
		stage("INIT: seed_initial_locations()");
		seed_initial_locations();
	}

	if (faery_region == 0)
	{
		create_faery();
	}

	if (hades_region == 0)
	{
		create_hades();
	}

	if (nowhere_region == 0)
	{
		create_nowhere();
	}

	if (cloud_region == 0)
	{
		create_cloudlands();
	}

	if (tunnel_region == 0)
	{
		create_tunnels();
	}

	if (!seed_has_been_run)
	{
		seed_phase_two();
		seed_has_been_run = TRUE;
	}

	if (combat_pl == 0)
	{
		combat_pl = 210;

		alloc_box(combat_pl, T_player, sub_pl_npc);
		set_name(combat_pl, "Combat log");

		/* To override the default password below, create/edit the file "PWD" which contains:

fairy fairypassword
combat combatpassword

	   The string up to the first whitespace contains the keyword used to look up the password below
	   The string after the whitespace contains the password to use instead of the default one
		 */

		pw = read_pw("combat");
		if (pw == NULL)
			pw = "noyoudont";


		p_player(combat_pl)->password = pw;
		fprintf(stderr, "\tcreated combat player %d\n", combat_pl);
	}

	create_relics();
	init_ocean_chars();
	delete_deadchars();

	print_box_usage();
}


void
cleanup_posts()
{
	int i;

	loop_post(i)
	{
		set_where(i, 0);
		delete_box(i);
	}
	next_post;
}

void make_dir(char *dir) {
	if (win_flag) {
		mkdir(sout(dir));
	}
}

void
save_logdir()
{
	int ret;
	char *s, *t;

	if (win_flag) {
		printf("Deleting any existing save dirs (this might fail):\n");
		system(sout("rd /s /q %s\\save\\%d", libdir, sysclock.turn));
		mkdir(sout("%s\\save", libdir), 0755);
	} else {
		system(sout("rm -rf %s/save/%d", libdir, sysclock.turn));
		mkdir(sout("%s/save", libdir), 0755);
	}

	s = sout("%s/log", libdir);
	t = sout("%s/save/%d", libdir, sysclock.turn);

	ret = rename(s, t);

	if (ret < 0)
	{
		fprintf(stderr, "couldn't rename %s to %s:", s, t);
		perror("");
	}

	s = sout("%s/players", libdir);
	t = sout("%s/save/%d/players", libdir, sysclock.turn);

	ret = rename(s, t);

	if (ret < 0)
	{
		fprintf(stderr, "couldn't rename %s to %s:", s, t);
		perror("");
	}

	mkdir(sout("%s/log", libdir), 0755);
}


void
save_db()
{

	stage("save_db()");
	cleanup_posts();
	save_seed(sout("%s/randseed", libdir));
	save_system();
	write_all_boxes();
	write_master();
	save_orders();
	rename_act_join_files();
}

