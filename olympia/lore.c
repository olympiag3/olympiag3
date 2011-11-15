
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"z.h"
#include	"oly.h"


static void
lore_function(int who, char *func)
{
	int i;

	if (strcmp(func, "capturable_animals") == 0)
	{
		loop_item(i)
		{
			if (item_animal(i) && item_capturable(i))
				out(who, "    %s", box_name(i));
		}
		next_item;

		return;
	}

	if (strcmp(func, "animal_fighters") == 0)
	{
		loop_item(i)
		{
			if (item_animal(i) && is_fighter(i))
				out(who, "    %-20s %s", box_name(i),
						sout("(%d,%d,%d)",
							item_attack(i),
							item_defense(i),
							item_missile(i)));
		}
		next_item;

		return;
	}

	fprintf(stderr, "bad lore sheet function: %s\n", func);
	assert(FALSE);
}


static void
deliver_lore_sheet(int who, int num, int display_num)
{
	FILE *fp;
	char *fnam;
	char *line;
	char buf[20];
	char *q, *p, *t;
	char buf2[LEN];

#if 1
	out(who, "");
	match_lines(who, box_name(display_num));
#else
	if (kind(num) == T_skill && skill_school(num) != num)
		match_lines(who, sout("Skill:  %s", box_name(num)));
	else
		match_lines(who, sout("Lore for %s", box_name(num)));
#endif

	if (kind(num) == T_skill)
		fnam = sout("%s/lore/%d/%d", libdir, skill_school(num), num);
	else
		fnam = sout("%s/lore/etc/%d", libdir, num);

	fp = fopen(fnam, "r");

	if (fp == NULL)
	{
		out(who, "<lore sheet not available>");
		fprintf(stderr, "can't open %s: ", fnam);
		perror("");
		return;
	}

	strcpy(buf, box_code_less(display_num));

	while (line = getlin(fp))
	{
		if (*line == '#')
			continue;

		if (*line == '$')
		{
			lore_function(who, line+1);
			continue;
		}

		p = line;
		q = buf2;

		while (*p)
		{
			if (*p == '@')
			{
				for (t = buf; *t; t++)
					*q++ = *t;
				p++;
				while (*p == '@')
					p++;
			}
			else
				*q++ = *p++;
		}
		*q = '\0';

		out(who, "%s", buf2);
	}

	fclose(fp);
}


char *
np_req_s(int skill)
{
	int np;

	np = skill_np_req(skill);

	if (np < 1)
		return "";

	if (np == 1)
		return ", 1 NP req'd";

	return sout(", %d NP req'd", np);
}


static void
out_skill_line(int who, int sk)
{

	out(who, "%-*s  %-34s %s%s",
			CHAR_FIELD, box_code_less(sk),
			just_name(sk),
			weeks(learn_time(sk)),
			np_req_s(sk));
}


static void
deliver_skill_lore(int who, int sk, int show_research)
{
	struct entity_skill *p;
	int i;

	deliver_lore_sheet(who, sk, sk);

	p = rp_skill(sk);

	if (p && ilist_len(p->offered))
	{
		out(who, "");
		wout(who, "The following skills may be studied directly "
			"once %s is known:", just_name(sk));
		out(who, "");

		indent += 3;
		out(who, "%-*s  %-34s %13s",
			CHAR_FIELD, "num", "skill", "time to learn");
		out(who, "%-*s  %-34s %13s",
			CHAR_FIELD, "---", "-----", "-------------");

		for (i = 0; i < ilist_len(p->offered); i++)
			out_skill_line(who, p->offered[i]);

		indent -= 3;

		out(who, "");
		if (ilist_len(p->research))
		{
		    wout(who, "Further skills may be found through research.");
		}
	}
	else if (p && ilist_len(p->research))
	{
		out(who, "");
		wout(who, "Skills within this category may be found "
			"through research.");
	}

	if (show_research && p && ilist_len(p->research))
	{
		out(who, "");

		indent += 3;
		for (i = 0; i < ilist_len(p->research); i++)
			out_skill_line(who, p->research[i]);
		indent -= 3;
	}
}


void
deliver_lore(int who, int num)
{

	switch (kind(num))
	{
	case T_skill:
		deliver_skill_lore(who, num, FALSE);
		break;

	case T_item:
		deliver_lore_sheet(who, item_lore(num), num);
		break;

	default:
		deliver_lore_sheet(who, num, num);
	}

	out(who, "");
}


/*
 *  Show a player a lore sheet
 *  Set anyway to show them the lore sheet even if they've
 *  seen it before.
 */

void
queue_lore(int who, int num, int anyway)
{
	int pl = player(who);
	struct entity_player *p;

	assert(kind(pl) == T_player);

	if (!anyway && test_known(pl, num))
		return;

	p = p_player(pl);

	ilist_append(&p->deliver_lore, num);
	set_known(pl, num);
}


static int
lore_comp(a, b)
int *a;
int *b;
{

	return *a - *b;
}


void
show_lore_sheets()
{
	int pl;
	struct entity_player *p;
	int i;

	stage("show_lore_sheets()");

	out_path = MASTER;
	out_alt_who = OUT_LORE;

	loop_player(pl)
	{
		p = rp_player(pl);
		if (p == NULL || ilist_len(p->deliver_lore) <= 0)
			continue;

		qsort(p->deliver_lore, (unsigned) ilist_len(p->deliver_lore),
				sizeof(int), lore_comp);

		for (i = 0; i < ilist_len(p->deliver_lore); i++)
		{
/*
 *  Weed out duplicates in p->deliver_lore
 */
		    if (i > 0 && p->deliver_lore[i] == p->deliver_lore[i-1])
			continue;

		    deliver_lore(pl, p->deliver_lore[i]);
		}
	}
	next_player;

	out_path = 0;
	out_alt_who = 0;
}


void
gm_show_all_skills(int pl)
{
	int sk;
	int i;

	out_path = MASTER;
	out_alt_who = OUT_LORE;

	out(pl, "");
	out(pl, "Skill listing:");
	out(pl, "--------------");

	out(pl, "");
	out(pl, "Skill schools:");
	out(pl, "");

	loop_skill(sk)
	{
		if (skill_school(sk) == sk)
			out_skill_line(pl, sk);
	}
	next_skill;

	out(pl, "");

	loop_skill(sk)
	{
		if (skill_school(sk) == sk)
		{
			out(pl, "%s", box_name(sk));
			indent += 3;

			loop_skill(i)
			{
				if (skill_school(i) == sk && i != sk)
					out_skill_line(pl, i);
			}
			next_skill;

			indent -= 3;
			out(pl, "");
		}
	}
	next_skill;

/*
 *  Output lore sheets for all skills
 */

	loop_skill(sk)
	{
		if (skill_school(sk) == sk)
		{
			deliver_skill_lore(pl, sk, TRUE);
			out(pl, "");
		}
	}
	next_skill;

	loop_skill(sk)
	{
		if (skill_school(sk) != sk)
		{
			deliver_skill_lore(pl, sk, TRUE);
			out(pl, "");
		}
	}
	next_skill;

	out_path = 0;
	out_alt_who = 0;
}


void
scan_char_skill_lore()
{
	int who;
	struct skill_ent *e;

	loop_char(who)
	{
		loop_char_skill(who, e)
		{
			queue_lore(who, e->skill, FALSE);
		}
		next_char_skill;
	}
	next_char;
}


void
scan_char_item_lore()
{
	int who;
	struct item_ent *e;
	int lore;

	loop_char(who)
	{
		loop_inv(who, e)
		{
			lore = item_lore(e->item);

			if (lore && !test_known(who, e->item))
			{
				queue_lore(who, e->item, FALSE);
			}
		}
		next_inv;
	}
	next_char;
}

