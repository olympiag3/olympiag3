
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"z.h"
#include	"oly.h"



static struct admit *
rp_admit(int pl, int targ)
{
	int i;
	struct entity_player *p;

	assert(kind(pl) == T_player);
	p = p_player(pl);

	for (i = 0; i < ilist_len(p->admits); i++)
		if (p->admits[i]->targ == targ)
			return p->admits[i];

	return NULL;
}


static struct admit *
p_admit(int pl, int targ)
{
	int i;
	struct entity_player *p;
	struct admit *new;

	assert(kind(pl) == T_player);
	p = p_player(pl);

	for (i = 0; i < ilist_len(p->admits); i++)
		if (p->admits[i]->targ == targ)
			return p->admits[i];

	new = my_malloc(sizeof(*new));
	new->targ = targ;

	ilist_append((ilist *) &p->admits, (int) new);

	return new;
}


/*
 *  Will pl admit who into targ?
 */

int
will_admit(int pl, int who, int targ)
{
	struct admit *p;
	int found;
	int found_pl;

	if (default_garrison(targ))
		return TRUE;

	pl = player(pl);

	if (player(who) == pl)
		return TRUE;

	p = rp_admit(pl, targ);

	if (p == NULL)
		return FALSE;

	found = ilist_lookup(p->l, who) >= 0;
	found_pl = ilist_lookup(p->l, player(who)) >= 0;

	if (p->sense)
	{
		if (found || found_pl)
			return FALSE;
		return TRUE;
	}
	else
	{
		if (found || found_pl)
			return TRUE;
		return FALSE;
	}
}


int
v_admit(struct command *c)
{
	int targ = c->a;
	int pl = player(c->who);
	struct admit *p;

	if (!valid_box(targ))
	{
		wout(c->who, "Must specify an entity for admit.");
		return FALSE;
	}

	cmd_shift(c);

	p = p_admit(pl, targ);

	if (!p->flag)
	{
		p->sense = FALSE;
		ilist_clear(&p->l);
		p->flag = TRUE;
	}

	while (numargs(c) > 0)
	{
		if (i_strcmp(c->parse[1], "all") == 0)
		{
			p->sense = TRUE;
		}
		else if (kind(c->a) == T_char ||
			 kind(c->a) == T_player ||
			 kind(c->a) == T_unform)
		{
			ilist_append(&p->l, c->a);
		}
		else
		{
			wout(c->who, "%s isn't a valid entity to admit.",
						c->parse[1]);
		}

		cmd_shift(c);
	}

	return TRUE;
}


static int
admit_comp(a, b)
struct admit **a;
struct admit **b;
{

	return (*a)->targ - (*b)->targ;
}


static void
print_admit_sup(int pl, struct admit *p)
{
	char buf[LEN];
	int i;
	int count = 0;

	sprintf(buf, "admit %4s", box_code_less(p->targ));

	if (p->sense)
	{
		strcat(buf, "  all");
		count++;
	}

	for (i = 0; i < ilist_len(p->l); i++)
	{
		if (!valid_box(p->l[i]))
			continue;

		if (++count >= 12)
		{
			out(pl, "%s", buf);
#if 0
			sprintf(buf, "admit %4s", p->targ);
#else
			strcpy(buf, "          ");
#endif
			count = 1;
		}

		strcat(buf, sout(" %4s", box_code_less(p->l[i])));
	}

	if (count)
		out(pl, "%s", buf);
}


void
print_admit(int pl)
{
	struct entity_player *p;
	int i;
	int first = TRUE;

	assert(kind(pl) == T_player);

	p = p_player(pl);

	if (ilist_len(p->admits) > 0)
	    qsort(p->admits, ilist_len(p->admits), sizeof(int), admit_comp);

	for (i = 0; i < ilist_len(p->admits); i++)
	{
		if (valid_box(p->admits[i]->targ))
		{
			if (first)
			{
				out(pl, "");
				out(pl, "Admit permissions:");
				out(pl, "");
				indent += 3;
				first = FALSE;
			}

			print_admit_sup(pl, p->admits[i]);
		}
	}

	if (!first)
		indent -= 3;
}


void
clear_all_att(int who)
{
	struct att_ent *p;

	p = rp_disp(who);
	if (p == NULL)
		return;

	ilist_clear(&p->neutral);
	ilist_clear(&p->hostile);
	ilist_clear(&p->defend);
}


void
set_att(int who, int targ, int disp)
{
	struct att_ent *p;
	extern int int_comp();

	p = p_disp(who);

	ilist_rem_value(&p->neutral, targ);
	ilist_rem_value(&p->hostile, targ);
	ilist_rem_value(&p->defend, targ);

	switch (disp)
	{
	case NEUTRAL:
	    ilist_append(&p->neutral, targ);
	    qsort(p->neutral, ilist_len(p->neutral), sizeof(int), int_comp);
	    break;

	case HOSTILE:
	    ilist_append(&p->hostile, targ);
	    qsort(p->hostile, ilist_len(p->hostile), sizeof(int), int_comp);
	    break;

	case DEFEND:
	    ilist_append(&p->defend, targ);
	    qsort(p->defend, ilist_len(p->defend), sizeof(int), int_comp);
	    break;

	case ATT_NONE:
		break;

	default:
	    assert(FALSE);
	}
}


int
is_hostile(who, targ)
int who;
int targ;
{
	struct att_ent *p;

	if (player(who) == player(targ))
		return FALSE;

	if (npc_program(who) == PROG_subloc_monster &&
	    !is_npc(targ) &&
	    only_defeatable(who) == 0)
	{
		return TRUE;
	}

	if (subkind(who) == sub_garrison)
	{
		struct entity_misc *p;

		p = rp_misc(who);
		if (p && ilist_lookup(p->garr_host, targ) >= 0)
			return TRUE;
			
	}

	if (p = rp_disp(who))
	{
		if (ilist_lookup(p->hostile, targ) >= 0)
			return TRUE;
	}

	if (p = rp_disp(player(who)))
	{
		if (ilist_lookup(p->hostile, targ) >= 0)
			return TRUE;
	}

	return FALSE;
}


int
is_defend(who, targ)
int who;
int targ;
{
	struct att_ent *p;
	int pl;

	if (is_hostile(who, targ))
		return FALSE;

	if (default_garrison(who))
		return TRUE;

	if (p = rp_disp(who))
	{
		if (ilist_lookup(p->defend, targ) >= 0)
			return TRUE;
		if (ilist_lookup(p->neutral, targ) >= 0)
			return FALSE;

		if (ilist_lookup(p->defend, player(targ)) >= 0)
			return TRUE;
		if (ilist_lookup(p->neutral, player(targ)) >= 0)
			return FALSE;
	}

	pl = player(who);

	if (p = rp_disp(pl))
	{
		if (ilist_lookup(p->defend, targ) >= 0)
			return TRUE;
		if (ilist_lookup(p->neutral, targ) >= 0)
			return FALSE;

		if (ilist_lookup(p->defend, player(targ)) >= 0)
			return TRUE;
		if (ilist_lookup(p->neutral, player(targ)) >= 0)
			return FALSE;
	}

	if (pl == player(targ) && pl != indep_player)
	{
		if (cloak_lord(who))
			return FALSE;
		return TRUE;
	}

	return FALSE;
}


static int
v_set_att(struct command *c, int k)
{

	while (numargs(c) > 0)
	{
		if (!valid_box(c->a))
		{
		    wout(c->who, "%s is not a valid entity.", c->parse[1]);
		}
		else if (k == HOSTILE && player(c->who) == player(c->a) &&
			player(c->who) != indep_player)
		{
		    wout(c->who, "Can't be hostile to a unit in the "
						"same faction.");
		}
		else
		{
		    set_att(c->who, c->a, k);
		}

		cmd_shift(c);
	}

	return TRUE;
}


int
v_hostile(struct command *c)
{
	return v_set_att(c, HOSTILE);
}


int
v_defend(struct command *c)
{
	return v_set_att(c, DEFEND);
}


int
v_neutral(struct command *c)
{
	return v_set_att(c, NEUTRAL);
}


int
v_att_clear(struct command *c)
{
	return v_set_att(c, ATT_NONE);
}


static void
print_att_sup(int who, ilist l, char *header, int *first)
{
	int i;
	int count = 0;
	char buf[LEN];
	extern int int_comp();

	if (ilist_len(l) == 0)
		return;

	strcpy(buf, header);

	qsort(l, ilist_len(l), sizeof(int), int_comp);

	for (i = 0; i < ilist_len(l); i++)
	{
		if (!valid_box(l[i]))
			continue;

		if (*first)
		{
			out(who, "");
			out(who, "Declared attitudes:");
			indent += 3;
			*first = FALSE;
		}

		if (++count >= 12)
		{
			out(who, "%s", buf);
			sprintf(buf, "%s",
				&spaces[spaces_len - strlen(header)]);
			count = 1;
		}

		strcat(buf, sout(" %4s", box_code_less(l[i])));
	}

	if (count)
		out(who, "%s", buf);
}


void
print_att(int who, int n)
{
	int first = TRUE;
	struct att_ent *p;

	p = rp_disp(n);

	if (p == NULL)
		return;

	print_att_sup(who, p->hostile, "hostile", &first);
	print_att_sup(who, p->neutral, "neutral", &first);
	print_att_sup(who, p->defend,  "defend ", &first);

	if (!first)
	{
		indent -= 3;
	}
}

