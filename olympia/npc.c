
#include	<stdio.h>
#include	"z.h"
#include	"oly.h"


int
controlled_humans_here(int where)
{
	int i;
	int ret = FALSE;

	loop_all_here(where, i)
	{
		if (kind(i) == T_char &&
		    subkind(i) == 0 &&
		    loyal_kind(i) != LOY_unsworn)
		{
			ret = TRUE;
			break;
		}
	}
	next_all_here;

	return ret;
}


struct exit_view *
get_exit_dir(struct exit_view **l, int dir)
{
	int i;

	for (i = 0; i < ilist_len(l); i++)
		if (l[i]->direction == dir)	/* && l[i]->hidden == FALSE? */
			return l[i];

	return NULL;
}


struct exit_view *
choose_npc_direction(int who, int where, int dir)
{
	struct exit_view *e;
	struct exit_view **l;

	l = exits_from_loc_nsew_select(who, where, LAND, RAND);

	if (ilist_len(l) == 0)
		return NULL;

/*
 *  There is a 90% chance an NPC will keep going in the same
 *  direction, if it can.
 */

	if (dir && rnd(1,10) < 10)
		if (e = get_exit_dir(l, dir))
			return e;

	return l[0];	/* order of l has already been randomized */
}


void
npc_move(int who)
{
	struct exit_view *e;
	int where = subloc(who);

	if (loc_depth(where) != LOC_province)
	{
		queue(who, "move out");
		return;
	}

	e = choose_npc_direction(who, where, npc_last_dir(who));

	if (e != NULL)
	{
		p_misc(who)->npc_dir = e->direction;

		queue(who, "move %s", full_dir_s[e->direction]);
	}
}


static void
auto_unsworn(int who)
{
	int n;
	int where = subloc(who);

	if (loc_depth(where) == LOC_build)
		return;

	if (rnd(1,2) == 1)
	{
		if ((n = city_here(where)) && rnd(1,2)==1)
			queue(who, "move %s", box_code_less(n));
		else
			npc_move(who);
	}
}


static void
auto_mob(int who)
{
	struct entity_misc *p;

	p = rp_misc(who);

	if (p == NULL)
	{
	    fprintf(stderr, "warning: mob's rp_misc is NULL, who=%d\n", who);
	    return;
	}

/*
 *  Disperse if unstacked and not at home.
 *  50% chance of dispersing each turn after five turns guarding
 *
 *  Since auto npc orders are only queued at the beginning of a turn,
 *  a mob unstacked will appear in the end of turn location report.
 *  Someone may try to rally the mob, so give them a chance before
 *  dispersing the mob.
 */

	if ((subloc(who) != p->npc_home) ||
	   (sysclock.turn - p->npc_created >= 5 && rnd(1,2) == 1))
	{
		queue(who, "wait time %d", rnd(10,20));
		queue(who, "reclaim \"disperses.\"");
		return;
	}
}


static int
create_hades_bandit(int where)
{
	int new;
	char *name = NULL;
	int item;

	switch (rnd(1,5))
	{
	case 1:
		item = item_spirit;
		break;

	case 2:
		item = item_corpse;
		break;

	case 3:
		item = item_savage;
		break;

	case 4:
		item = item_skeleton;
		break;

	case 5:
		item = item_gorgon;
		break;

	default:
		assert(FALSE);
	}

	new = new_char(sub_ni, item, where, -1, indep_player,
							LOY_npc, 0, name);

	p_char(new)->break_point = 0;
	rp_char(new)->npc_prog = PROG_bandit;

	if (new < 0)
		return -1;

	gen_item(new, item, rnd(4,24));

	wout(where, "%s appear.", box_name(new));

	return new;
}


static int
create_faery_bandit(int where)
{
	int new;
	char *name = NULL;
	int item;

	switch (rnd(1,3))
	{
	case 1:
	case 2:
		item = item_faery;
		break;

	case 3:
		item = item_elf;
		break;

	default:
		assert(FALSE);
	}

	new = new_char(sub_ni, item, where, -1, indep_player,
							LOY_npc, 0, name);

	p_char(new)->break_point = 0;
	rp_char(new)->npc_prog = PROG_bandit;

	if (new < 0)
		return -1;

	gen_item(new, item, rnd(4,24));

	wout(where, "%s appear.", box_name(new));

	gen_item(new, item_gold, rnd(1,25));

	return new;
}


void
hades_attack_check(int who, int where)
{
	int new;

	if (rnd(1,100) > 6)
		return;

	if (is_npc(who) ||
	    kind(who) != T_char ||
	    char_really_hidden(who))
		return;

	new = create_hades_bandit(where);

	if (new < 0)
		return;

	queue(new, "wait time 0");
	init_load_sup(new);   /* make ready to execute commands immediately */

	if (rnd(1,2) == 1)
	{
		queue(new, "attack %s", box_code_less(who));
	}
}


void
faery_attack_check(int who, int where)
{
	int new;

	if (rnd(1,100) > 6)
		return;

	if (is_npc(who) ||
	    kind(who) != T_char ||
	    char_really_hidden(who))
		return;

	new = create_faery_bandit(where);

	if (new < 0)
		return;

	queue(new, "wait time 0");
	init_load_sup(new);   /* make ready to execute commands immediately */

	if (rnd(1,2) == 1)
	{
		queue(new, "attack %s", box_code_less(who));
	}
}


static void
auto_bandit(int who)
{
	int where = subloc(who);
	int i;
	int victim = 0;

	loop_here(who, i)
	{
		queue(who, "unstack %s", box_code_less(i));
	}
	next_here;

	loop_here(where, i)
	{
		if (kind(i) == T_char &&
		    !is_npc(i) &&
		    !char_really_hidden(i))
		{
			victim = i;
			break;
		}
	}
	next_here;

	if (victim)
		queue(who, "attack %s", box_code_less(victim));
	else
		npc_move(who);
}


#define	PROV_OR_CITY	-1

struct cookie_monster_tbl
{
	int cookie;
	int kind, sk, ni;
	int terrain;
	int man_kind, low, high;
	char *not_here;
	char *no_cookies;
}
cookie_monster[] =
{
    {
	item_mob_cookie,
	T_char, sub_ni, item_angry_peasant,
	PROV_OR_CITY,
	item_angry_peasant, 12, 36,
	"Mobs can only be raised in provinces and cities.",
	"A mob has already been raised from this place.",
    },
    {
	item_undead_cookie,
	T_char, sub_undead, 0,
	sub_graveyard,
	item_corpse, 15, 25,
	"Demon lords may only be summoned in graveyards.",
	"A demon lord has already been summoned from this graveyard.",
    },
    {
	item_rain_cookie,
	T_storm, sub_rain, 0,
	0,
	0, 0, 0,
	"Rain may not be summoned here.",
	"A storm has already been summoned from this province.",
    },
    {
	item_wind_cookie,
	T_storm, sub_wind, 0,
	0,
	0, 0, 0,
	"Wind may not be summoned here.",
	"A storm has already been summoned from this province.",
    },
    {
	item_fog_cookie,
	T_storm, sub_fog, 0,
	0,
	0, 0, 0,
	"Fog may not be summoned here.",
	"A storm has already been summoned from this province.",
    },
    {
	0,
	0, 0, 0,
	0,
	0, 0, 0,
	NULL, NULL
    }
};


static struct cookie_monster_tbl *
find_cookie(int k)
{
	int i;

	assert(kind(k) == T_item);

	for (i = 0; cookie_monster[i].cookie; i++)
		if (cookie_monster[i].cookie == k)
			return &cookie_monster[i];

	return NULL;
}


int
may_cookie_npc(int who, int where, int cookie)
{
	struct cookie_monster_tbl *t;
	int bad_place = FALSE;

	t = find_cookie(cookie);
	assert(t);

	if (t->terrain > 0 && subkind(where) != t->terrain)
		bad_place = TRUE;

	if (t->terrain == PROV_OR_CITY &&
	    subkind(where) != sub_city && loc_depth(where) != LOC_province)
		bad_place = TRUE;

	if (bad_place)
	{
		if (who)
			wout(who, "%s", t->not_here);
		return FALSE;
	}

	if (has_item(where, cookie) == 0)
	{
		if (who)
			wout(who, "%s", t->no_cookies);
		return FALSE;
	}

	return TRUE;
}


int
do_cookie_npc(int who, int where, int cookie, int place)
{
	struct cookie_monster_tbl *t;
	struct entity_misc *p;
	int new;

	if (!may_cookie_npc(who, where, cookie))
		return 0;

	t = find_cookie(cookie);
	assert(t);

	if (t->kind == T_char)
	{
		new = new_char(t->sk, t->ni, place, 100, indep_player,
							LOY_npc, 0, NULL);
	}
	else
	{
		new = new_ent(t->kind, t->sk);

		if (new > 0)
			set_where(new, place);
	}

	if (new <= 0)
		return 0;

	if (t->sk == sub_ni)
		p_char(new)->health = -1;

	p = p_misc(new);
	p->npc_home = where;
	p->npc_cookie = cookie;
	p->summoned_by = who;
	p->npc_created = sysclock.turn;

	if (t->man_kind)
		gen_item(new, t->man_kind, rnd(t->low, t->high));

	consume_item(where, cookie, 1);

	return new;
}


int
create_peasant_mob(int where)
{
	int new;

	new = do_cookie_npc(0, where, item_mob_cookie, where);

	if (new <= 0)
		return 0;

	set_name(new, rnd(1,2) == 1 ? "Mob" : "Crowd");

	queue(new, "guard 1");
	init_load_sup(new);   /* make ready to execute commands immediately */

	return new;
}


void
queue_npc_orders()
{
	int who;

	stage("queue_npc_orders()");

	init_savage_attacks();
	auto_hades();

	loop_units(indep_player, who)
	{
		if (loyal_kind(who) == LOY_summon)
			continue;

		if (is_prisoner(who))
			continue;

		if (rp_command(who) && rp_command(who)->state != DONE)
			continue;		/* running an order */

		if (top_order(indep_player, who))
			continue;		/* orders already queued */

		switch (npc_program(who))
		{
		case 0:
			switch (subkind(who))
			{
			case 0:
				auto_unsworn(who);
				break;

			case sub_undead:
				auto_undead(who);
				break;

			case sub_ni:
				switch (noble_item(who))
				{
				case item_savage:
					auto_savage(who);
					break;

				case item_peasant:
				case item_angry_peasant:
					auto_mob(who);
					break;
				}
				break;
			}
			break;

		case PROG_bandit:
			auto_bandit(who);
			break;

		case PROG_subloc_monster:
			break;

		case PROG_npc_token:
			npc_move(who);
			break;

		default:
			assert(FALSE);
		}
	}
	next_unit;
}

