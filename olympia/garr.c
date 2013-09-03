
#include	<stdio.h>
#include	<string.h>
#include	"z.h"
#include	"oly.h"


/*
 *	garrison -> castle -> owner [ -> char ]*
 *			     "admin"          "top ruler"
 */

/*

1-	touch loc on pledge of terroritory
2-	initial touch loc for both castle and ruler
3-	hook pledge into glob.c
4-	determine everyone's status at turn end
5-	display status in display.c
6.	who may get/give to a garrison
7-	maintenance of men in garrison
8-	forwarding of gold to castle
9-	circular pledge detector
10-	replace loc_owner with building_owner, province_ruler
11.	credit for castles as well as garrisoned provinces
12-	"garrison" keyword that matches the garrison in the current loc
13-	take -- leave behind 10
14-	there can't be an ni - 0, can there?  what if attacked?
15-	status = min(own status, pledge lord's status - 1)
16-	allow owner or admin to name province, sublocs
17-	garrison log for output

*/


/*
 *  Garrison should always be first; we should just have to look
 *  at the first character
 */

int
garrison_here(int where)
{
	int n;

	n = first_character(where);

	if (n && subkind(n) == sub_garrison)
		return n;

	return 0;
}


int
province_admin(int n)
{
	int garr;
	int castle;

	if (kind(n) == T_loc)
	{
		assert(loc_depth(n) == LOC_province);
		garr = garrison_here(n);
		if (garr == 0)
			return 0;
	}
	else
	{
		assert(subkind(n) == sub_garrison);
		garr = n;
	}

	castle = garrison_castle(garr);

	if (!valid_box(castle))
		return 0;

	return building_owner(castle);
}


int
top_ruler(int n)
{
	int i;
	int ret = 0;

	loop_loc_owner(n, i)
	{
		ret = i;
	}
	next_loc_owner;
	
	return ret;
}


/*
 *  is b pledged somewhere beneath a?
 */

static int
pledged_beneath(int a, int b)
{

	assert(kind(a) == T_char);
	assert(kind(b) == T_char);

	if (a == b)
		return FALSE;

	while (b > 0)
	{
		b = char_pledge(b);
		if (a == b)
			return TRUE;
	}

	return FALSE;
}


int
may_rule_here(int who, int where)
{
	int pl = player(who);
	int i;
	int ret = FALSE;

	if (is_loc_or_ship(where))
		where = province(where);
	else
		assert(subkind(where) == sub_garrison);

	loop_loc_owner(where, i)
	{
		if (player(i) == pl)
		{
			ret = TRUE;
			break;
		}
	}
	next_loc_owner;

	return ret;
}


ilist
players_who_rule_here(int where)
{
	static ilist l = NULL;
	int i;
	int pl;
#if 1
	int loop_check = 5000;
#endif

	ilist_clear(&l);

	loop_loc_owner(where, i)
	{
		pl = player(i);

		if (pl && ilist_lookup(l, pl) < 0)
			ilist_append(&l, pl);

#if 1
		if (loop_check <= 0)
		{
			int j;
			for (j = 0; j < ilist_len(l); j++)
				fprintf(stderr, "l[%d] = %d\n", j, l[j]);
			fprintf(stderr, "where = %d, i = %d\n", where, i);
		}
		assert(loop_check-- > 0);
#endif
	}
	next_loc_owner;

	return l;
}


#if 0
void
touch_garrison_locs()
{
	int garr;
	int where;
	int owner;

	loop_garrison(garr)
	{
		where = subloc(garr);

		loop_loc_owner(garr, owner)
		{
			touch_loc_pl(player(owner), where);
		}
		next_loc_owner;
	}
	next_garrison;
}
#endif


int
new_province_garrison(int where, int castle, int item, int qty)
{
	int new;

	new = new_char(sub_garrison, 0, where, -1, garr_pl, LOY_npc, 0, NULL);

	if (new < 0)
		return -1;

	gen_item(new, item, qty);
	p_misc(new)->cmd_allow = 'g';
	p_misc(new)->garr_castle = castle;
	p_char(new)->guard = TRUE;
	p_char(new)->break_point = 0;

	promote(new, 0);

	out(where, "%s now guards %s.", liner_desc(new), box_name(where));

	return new;
}


static int
garrison_allowed_here(int who, int where, int castle)
{
	struct exit_view **l;
	int i;
	int here;
	int garr;

	here = province_subloc(where, sub_castle);
	if (here > 0 && here != castle)
	{
		wout(who, "A garrison in this province must be bound to %s.",
				box_name(here));
		return FALSE;
	}

	if (here)
		return TRUE;

	l = exits_from_loc_nsew(who, where);

	for (i = 0; i < ilist_len(l); i++)
	{
		garr = garrison_here(l[i]->destination);

		if (garr && garrison_castle(garr) == castle)
			return TRUE;
	}

	wout(who, "A garrison may only be installed in a province adjoining "
		"an existing garrison (bound to the same castle), or in the "
		"province with the castle itself.");
	return FALSE;
}


int
v_garrison(struct command *c)
{
	int castle = c->a;
	int where = subloc(c->who);
	int new;
	int level;

	if (loc_depth(where) != LOC_province)
	{
		out(c->who, "Garrisons may only be installed at "
					"province level.");
		return FALSE;
	}

	if (garrison_here(where))
	{
		out(c->who, "There is already a garrison here.");
		return FALSE;
	}

#if 0
	if (first_character(where) != c->who)
	{
		out(c->who, "Must be the first unit in the location to "
				"install a garrison.");
		return FALSE;
	}
#endif

	if (numargs(c) < 1)
	{
		out(c->who, "Must specify a castle to claim the province "
						"in the name of.");
		return FALSE;
	}

	if (subkind(castle) == sub_castle_notdone)
	{
		out(c->who, "%s is not finished.  Garrisons may only"
		" be bound to completed castles.",
					box_name(castle));
		return FALSE;
	}

	if (subkind(castle) != sub_castle)
	{
		out(c->who, "%s is not a castle.", c->parse[1]);
		return FALSE;
	}

	if (region(castle) != region(where))
	{
		out(c->who, "%s is not in this region.", box_name(castle));
		return FALSE;
	}

	level = castle_level(castle);
	if (level < 6)
	{
		int count = count_castle_garrisons(castle);
		int allowed = allowed_garrisons(level);

		if (count > allowed)
		{
			wout(c->who, "%s may only support %s garrisons.",
				box_name(castle), nice_num(allowed));

			return FALSE;
		}
	}

	if (!garrison_allowed_here(c->who, where, castle))
		return FALSE;

	if (has_item(c->who, item_soldier) < 10)
	{
		out(c->who, "Must have %s to establish a new garrison.",
					box_name_qty(item_soldier, 10));
		return FALSE;
	}

	new = new_province_garrison(where, castle, item_soldier, 10);

	if (new < 0)
	{
		out(c->who, "Failed to install garrison.");
		return FALSE;
	}

	consume_item(c->who, item_soldier, 10);

	out(c->who, "Installed %s", liner_desc(new));

	return TRUE;
}


int
v_pledge(struct command *c)
{
	int target = c->a;
	int n = 0;

	if (target == c->who)
	{
		wout(c->who, "Can't pledge to yourself.");
		return FALSE;
	}

	if (target == 0)
	{
		p_magic(c->who)->pledge = 0;
		out(c->who, "Pledge cleared.  "
				"Lands will be claimed for ourselves.");
		return TRUE;
	}

	if (kind(target) != T_char)
	{
		out(c->who, "%s is not a character.", c->parse[1]);
		return FALSE;
	}

	if (is_npc(target))
	{
		out(c->who, "May not pledge land to %s.", c->parse[1]);
		return FALSE;
	}

	if (pledged_beneath(c->who, target))
	{
		wout(c->who, "Cannot pledge to %s since %s is pledged to you.",
				box_name(target), just_name(target));
		return FALSE;
	}

	out(c->who, "Lands are now pledged to %s.", box_name(target));

	out(target, "%s pledges to us.", box_name(c->who), add_s(n));

	p_magic(c->who)->pledge = target;

#if 0
/*
 *  Touch all garrisoned locs which we've gained.
 *
 *  We simply redo the whole process; this is somewhat ineffecient,
 *  but a faster approach may not be necessary.
 */

	touch_garrison_locs();
#endif

	return TRUE;
}


void
garrison_gold()
{
	int garr;
	int where;
	int base, remain, spent, has_before, has_now;
	int i;
	int castle, owner;
	extern int gold_provinces;

	clear_temps(T_loc);

	loop_garrison(garr)
	{
		if (default_garrison(garr))
			continue;

/*
 *  Determine tax base of garrisoned province, and remove it
 */

		where = subloc(garr);

		assert(loc_depth(where) == LOC_province);

		base = has_item(where, item_tax_cookie);
		consume_item(where, item_tax_cookie, base);

/*
 *  add as gold to the garrison for the call to the maintenance
 *  cost charger
 */

		gen_item(garr, item_gold, base);
		p_misc(garr)->garr_tax = base;

/*
 *  Find out how much we spent on maintenance.  The garrison may
 *  have had some gold of its own that it had to dip into.
 */

		has_before = has_item(garr, item_gold);
		charge_maint_sup(garr);
		has_now = has_item(garr, item_gold);

		spent = has_before - has_now;

		if (spent >= base)	/* spent entire tax base, or more */
			continue;

		remain = base - spent;
		consume_item(garr, item_gold, remain);

/*
 *  castle gets remaining tax base
 */

		if (province_subloc(where, sub_castle))
		{
			gen_item(where, item_tax_cookie, remain);
			p_misc(garr)->garr_forward = -1;
			continue;
		}

		remain /= 2;		/* 50% of remains go to land owner */

		castle = garrison_castle(garr);

		if (castle && remain > 0)
		{
			bx[castle]->temp += remain;
			p_misc(garr)->garr_forward = remain;
		}
	}
	next_garrison;

	loop_loc(i)
	{
		if (subkind(i) != sub_castle)
			continue;

		owner = building_owner(i);

		if (owner == 0 || bx[i]->temp == 0)
			continue;

		wout(owner, "Collected %s from owned provinces.",
					gold_s(bx[i]->temp));

		gen_item(owner, item_gold, bx[i]->temp);
		gold_provinces += bx[i]->temp;
	}
	next_loc;
}


int
count_castle_garrisons(int castle)
{
	int garr;
	int sum = 0;

	loop_garrison(garr)
	{
		if (garrison_castle(garr) == castle)
			sum++;
	}
	next_garrison;

	return sum;
}


int
allowed_garrisons(int level)
{

	switch (level)
	{
	case 0:	return 5;
	case 1:	return 12;
	case 2:	return 24;
	case 3:	return 37;
	case 4:	return 50;
	case 5:	return 63;

	default:
		return 100000;
	}
}


static int
nprovs_to_rank(int n)
{

	if (n == 0)	return 0;
	if (n <= 5)	return RANK_lord;
	if (n <= 12)	return RANK_knight;
	if (n <= 24)	return RANK_baron;
	if (n <= 37)	return RANK_count;
	if (n <= 50)	return RANK_earl;
	if (n <= 63)	return RANK_marquess;

	return RANK_duke;
}


char *
rank_s(int who)
{
	int n = char_rank(who);

	switch (n)
	{
	case 0:			return "";
	case RANK_lord:		return ", lord";
	case RANK_knight:	return ", knight";
	case RANK_baron:	return ", baron";
	case RANK_count:	return ", count";
	case RANK_earl:		return ", earl";
	case RANK_marquess:	return ", marquess";
	case RANK_duke:		return ", duke";
	case RANK_king:		return ", king";

	default:
		assert(FALSE);
	}
  return 0;
}


static void
find_kings()
{
	int reg;
	int where;
	int ruler;
	int nprovs;

	loop_loc(reg)
	{
		if (loc_depth(reg) != LOC_region)
			continue;

		ruler = -1;
		nprovs = 0;

		loop_here(reg, where)
		{
			if (kind(where) != T_loc)
				continue;

			nprovs++;

			if (ruler == -1)
			{
				ruler = top_ruler(where);
				if (ruler == 0)
					break;		/* fail */
			}
			else
			{
				if (ruler != top_ruler(where))
				{
					ruler = 0;
					break;		/* fail */
				}
			}
		}
		next_here;

		if (ruler && nprovs >= 15)
			p_char(ruler)->rank = RANK_king;
	}
	next_loc;
}


/*
 *  A noble's status is:
 *
 *	min(status by own provinces, lord's status - 1)
 */

static int
det_noble_rank_sup(int who)
{
	int own_rank;
	int pledged_to;

	own_rank = nprovs_to_rank(bx[who]->temp);
	pledged_to = char_pledge(who);

	if (pledged_to == 0)
		return own_rank;

	return min(own_rank, det_noble_rank_sup(pledged_to));
}


void
determine_noble_ranks()
{
	int garr;
	int owner;
	int who;

	stage("determine_noble_ranks()");

	clear_temps(T_char);

	loop_garrison(garr)
	{
		loop_loc_owner(garr, owner)
		{
			bx[owner]->temp++;
		}
		next_loc_owner;
	}
	next_garrison;

	loop_char(who)
	{
		if (char_rank(who))
			rp_char(who)->rank = 0;

		if (bx[who]->temp == 0)
			continue;

		p_char(who)->rank = det_noble_rank_sup(who);
	}
	next_char;

	find_kings();
}


int
garrison_notices(int garr, int target)
{
	struct entity_misc *p;

	if (is_npc(target) ||
	    count_stack_units(target) >= 5 ||
	    count_stack_figures(target) >= 20)
		return TRUE;

	p = rp_misc(garr);

	if (p && ilist_lookup(p->garr_watch, target) >= 0)
		return TRUE;

	return FALSE;
}


int
garrison_spot_check(int garr, int target)
{
	int i;
	int found = FALSE;
	struct entity_misc *p;

	assert(valid_box(garr));

	p = rp_misc(garr);
	if (p == NULL)
		return FALSE;

	loop_stack(target, i)
	{
		if (ilist_lookup(p->garr_watch, i) >= 0)
		{
			found = TRUE;
			break;
		}
	}
	next_stack;

	if (found)
		wout(garr, "Spotted in %s:", box_name(province(garr)));

	return found;
}


static char *
garr_own_s(int garr)
{
	int owner;
	int count = 0;
	int l[5];
	int i;
	static char buf[LEN];

	*buf = '\0';

	for (i = 0; i < 5; i++)
		l[i] = 0;

	loop_loc_owner(garr, owner)
	{
		if (count >= 5)
		{
			l[4] = owner;
			l[3] = -1;
		}
		else
			l[count] = owner;

		count++;
	}
	next_loc_owner;

	if (l[0] == 0)
		return "";

	strcat(buf, box_code_less(l[0]));

	for (i = 1; i < 5 && l[i]; i++)
	{
		strcat(buf, " ");

		if (l[i] == -1)
			strcat(buf, " ...");
		else
			strcat(buf, box_code_less(l[i]));
	}

	return buf;
}


void
garrison_summary(int pl)
{
	int garr;
	static ilist l = NULL;
	int i;
	int forw;

	ilist_clear(&l);

	loop_garrison(garr)
	{
		if (!may_rule_here(pl, garr))
			continue;

		ilist_append(&l, garr);
	}
	next_garrison;

	if (ilist_len(l) == 0)
		return;

	out(pl, "");
	out(pl, "%6s %5s %4s %4s %4s %4s %6s %s",
		"garr", "where", "men", "cost", "tax", "forw", "castle", "rulers");
	out(pl, "%6s %5s %4s %4s %4s %4s %6s %s",
		"----", "-----", "---", "----", "---", "----", "------", "------");

	sort_for_output(l);

	for (i = 0; i < ilist_len(l); i++)
	{
		garr = l[i];

		forw = rp_misc(garr)->garr_forward;

		out(pl, "%6s %5s %4d %4d %4d %4s %6s %s",
				box_code_less(garr),
				box_code_less(subloc(garr)),
				count_stack_figures(garr),
				unit_maint_cost(garr),
				rp_misc(garr)->garr_tax,
				forw == -1 ? "-" : sout("%d", forw),
				box_code_less(garrison_castle(garr)),
				garr_own_s(garr));
	}
}


int
v_decree_watch(struct command *c)
{
	int garr;
	int target = c->a;
	struct entity_misc *p;
	int ncontrol = 0;
	int nordered = 0;
	
	if (kind(target) != T_char)
	{
		wout(c->who, "%s is not a character.", c->parse[1]);
		return FALSE;
	}

	loop_garrison(garr)
	{
		if (!may_rule_here(c->who, garr))
			continue;

		ncontrol++;
		p = p_misc(garr);

		if (ilist_len(p->garr_watch) < 3)
		{
			ilist_append(&p->garr_watch, target);
			wout(garr, "%s orders us to watch for %s.",
					box_name(c->who), box_code(target));

			nordered++;
		}
	}
	next_garrison;

	if (ncontrol == 0)
	{
		wout(c->who, "We rule over no garrisons.");
		return FALSE;
	}

	if (nordered == 0)
	{
		wout(c->who, "Garrisons may only watch for up to three "
						"units per month.");
		return FALSE;
	}

	wout(c->who, "Watch decree given to %s garrison%s.",
					nice_num(nordered), add_s(nordered));

	return TRUE;
}


int
v_decree_hostile(struct command *c)
{
	int garr;
	int target = c->a;
	struct entity_misc *p;
	int ncontrol = 0;
	int nordered = 0;
	
	if (kind(target) != T_char)
	{
		wout(c->who, "%s is not a character.", c->parse[1]);
		return FALSE;
	}

	loop_garrison(garr)
	{
		if (!may_rule_here(c->who, garr))
			continue;

		ncontrol++;
		p = p_misc(garr);

		if (ilist_len(p->garr_host) < 3)
		{
			ilist_append(&p->garr_host, target);
			wout(garr, "%s orders us to attack %s on sight.",
					box_name(c->who), box_code(target));

			nordered++;
		}
	}
	next_garrison;

	if (ncontrol == 0)
	{
		wout(c->who, "We rule over no garrisons.");
		return FALSE;
	}

	if (nordered == 0)
	{
		wout(c->who, "Garrisons may be hostile to at most "
				"three units.");
		return FALSE;
	}

	wout(c->who, "Hostile decree given to %s garrison%s.",
					nice_num(nordered), add_s(nordered));

	return TRUE;
}


static char *decree_tags[] = {
	"watch",	/* 0 */
	"hostile",	/* 1 */
	NULL
};


int
v_decree(struct command *c)
{
	int tag;

	if (numargs(c) < 1)
	{
		wout(c->who, "Must specify what to decree.");
		return FALSE;
	}

	tag = lookup(decree_tags, c->parse[1]);

	if (tag < 0)
	{
		wout(c->who, "Unknown decree '%s'.", c->parse[1]);
		return FALSE;
	}

	cmd_shift(c);

	switch (tag)
	{
	case 0:		return v_decree_watch(c);
	case 1:		return v_decree_hostile(c);

	default:
		assert(FALSE);
	}

	return FALSE;
}


void
ping_garrisons()
{
	int garr;
	int where;

	show_to_garrison = TRUE;

	loop_garrison(garr)
	{
		where = subloc(garr);

		assert(rp_char(garr) != NULL);

		rp_char(garr)->guard = FALSE;

		wout(where, "%s guards %s.",
				liner_desc(garr), box_name(where));

		rp_char(garr)->guard = TRUE;
	}
	next_garrison;

	show_to_garrison = FALSE;
}


int
v_ungarrison(struct command *c)
{
	int garr = c->a;
	int where = subloc(c->who);
	struct item_ent *e;
	int first = TRUE;

	if (garr == 0)
	{
		garr = garrison_here(where);

		if (garr == 0)
		{
			wout(c->who, "There is no garrison here.");
			return FALSE;
		}
	}
	else if (garrison_here(where) != garr)
	{
		wout(c->who, "No garrison %s is here.", c->parse[1]);
		return FALSE;
	}

	if (!may_rule_here(c->who, garr))
	{
		wout(c->who, "%s does not rule over %s.",
				box_name(c->who), box_name(garr));
		return FALSE;
	}


	wout(c->who, "%s disbands.", box_name(garr));

	vector_clear();
	vector_add(garr);
	vector_add(where);
	wout(VECT, "%s is disbanded by %s.", box_name(garr), box_name(c->who));

	loop_inv(garr, e)
	{
		if (first)
		{
			first = FALSE;
			wout(c->who, "Received from %s:", box_name(garr));
			indent += 3;
		}

		wout(c->who, "%s", box_name_qty(e->item, e->qty));

		move_item(garr, c->who, e->item, e->qty);
	}
	next_inv;

	if (!first)
		indent -= 3;

	p_misc(garr)->garr_castle = 0;		/* become silent */
	kill_char(garr, 0);

	return TRUE;
}

