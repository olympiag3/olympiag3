
#include	<stdio.h>
#include	"z.h"
#include	"oly.h"


/* region production routines */


#define		MOUNTAIN_STONE	50
#define		POPPY_OPIUM	25


struct {
	int terr;		/* terrain type */
	int item;		/* good produced by location */
	int qty;		/* amount produced */
}
terr_prod[] =
{
	{sub_forest,		item_lumber,		30},
	{sub_sacred_grove,	item_lumber,		5},
	{sub_tree_circle,	item_lumber,		5},

	{sub_mountain,		item_stone,		MOUNTAIN_STONE},
	{sub_rocky_hill,	item_stone,		MOUNTAIN_STONE},
	{sub_desert,		item_stone,		10},

	{sub_cave,		item_farrenstone,	2},
	{sub_plain,		item_wild_horse,	5},
	{sub_pasture,		item_wild_horse,	5},
	{sub_ocean,		item_fish,		50},

	{sub_mallorn_grove,	item_avinia_leaf,	2},
	{sub_mallorn_grove,	item_mallorn_wood,	2},

	{sub_bog,		item_spiny_root,	4},
	{sub_pits,		item_spiny_root,	4},
	{sub_swamp,		item_spiny_root,	1},

	{sub_yew_grove,		item_yew,		5},
	{sub_graveyard,		item_corpse,		15},
	{sub_tree_circle,	item_lana_bark,		3},
	{sub_sand_pit,		item_pretus_bones,	1},
	{sub_poppy_field,	item_opium,		POPPY_OPIUM},

	{sub_forest,		item_peasant,		10},
	{sub_mountain,		item_peasant,		10},
	{sub_plain,		item_peasant,		10},
	{sub_city,		item_peasant,		10},

	{0, 0, 0}
};


struct {
	int iron;
	int gold;
	int mithril;
} mine_prod[] = {
/*        iron  gold  mithril   */
/*        ----  ----  -------   */
/* 0 */   {0,    0,    0},
/* 1 */   {15,   25,   0},
/* 2 */   {12,   100,  0},
/* 3 */   {10,   200,  0},
/* 4 */   {8,    500,  1},
/* 5 */   {5,    500,  0},
/* 6 */   {3,    400,  0},
/* 7 */   {0,    150,  0},
/* 8 */   {0,    50,   1},
/* 9 */   {0,    0,    2},
/* 10 */  {0,    0,    8},
/* 11 */  {0,    0,    5},
/* 12 */  {0,    0,    0},
/* 13 */  {10,   10,   0},
/* 14 */  {10,   50,   2},
/* 15 */  {0,    10,   1},
/* 16 */  {0,    0,    0},
/* 17 */  {10,   0,    1},
/* 18 */  {0,    500,  0},
/* 19 */  {0,    0,    0},
/* 20 */  {0,    0,    0}
};

#define		MINE_MAX	20


static void
replenish(int where, int item, int qty)
{
	int n;

	n = has_item(where, item);
	if (n < qty)
		gen_item(where, item, qty - n);
}


void
mine_production(int where)
{
	int depth;

	depth = mine_depth(where);

	assert(depth > 0);

	if (depth > MINE_MAX)
		depth = MINE_MAX;

	replenish(where, item_iron, mine_prod[depth].iron);
	replenish(where, item_gold, mine_prod[depth].gold);
	replenish(where, item_mithril, mine_prod[depth].mithril);
}


void
location_production()
{
    int where;
    int i;
    int terr; 

    loop_loc(where)
    {
	terr = subkind(where);

	if (terr == sub_mine)
	{
	    mine_production(where);
	}
	else
	{
	    for (i = 0; terr_prod[i].terr; i++)
		if (terr_prod[i].terr == terr)
		{
		    replenish(where, terr_prod[i].item, terr_prod[i].qty);
		}
	}

/*
 *  First limit poppy fields to normal production level.
 *  Then double opium if poppy field was specially tended.
 */

	if (terr == sub_poppy_field)
	{
		int n;

		n = has_item(where, item_opium);
		if (n > POPPY_OPIUM)
			consume_item(where, item_opium, n - POPPY_OPIUM);

		if (rp_misc(where) && rp_misc(where)->opium_double)
		{
		    rp_misc(where)->opium_double = FALSE;
		    gen_item(where, item_opium, has_item(where, item_opium));
		}
	}

	if (terr == sub_island ||
	   (loc_depth(where) == LOC_province && has_ocean_access(where)))
		replenish(where, item_flotsam, 30);
    }
    next_loc;
}


static int
item_gen_here(int terr, int item)
{
	int i;

	for (i = 0; terr_prod[i].terr; i++)
		if (terr_prod[i].terr == terr && terr_prod[i].item == item)
			return TRUE;

	return FALSE;
}


int
start_generic_mine(struct command *c, int item)
{
	int where = subloc(c->who);
	int nworkers;

	if (subkind(where) != sub_mine)
	{
		wout(c->who, "Must be in a mine to extract %s.",
					just_name(item));
		return FALSE;
	}

	nworkers = has_item(c->who, item_worker);
	if (nworkers < 10)
	{
		wout(c->who, "Mining activity requires at least ten workers.");
		return FALSE;
	}

	wout(c->who, "Will mine %s for the next %s days.",
					just_name(item),
					nice_num(c->wait));

	return TRUE;
}


int
finish_generic_mine(struct command *c, int item)
{
	int where = subloc(c->who);
	int has;
	struct entity_subloc *p;
	int nworkers;
	int qty;
	int depth;

	if (subkind(where) != sub_mine)
	{
		wout(c->who, "%s is no longer in a mine.", box_name(c->who));
		return FALSE;
	}

	nworkers = has_item(c->who, item_worker);
	if (nworkers < 10)
	{
		wout(c->who, "%s no longer has ten workers.",
							box_name(c->who));
		return FALSE;
	}

	depth = mine_depth(where);
	p = p_subloc(where);

	p->shaft_depth++;

	if (depth >= 4 && rnd(1,5) == 1 && has_item(where, item_gate_crystal))
	{
		wout(c->who, "A gate crystal was found while mining!");
		move_item(where, c->who, item_gate_crystal, 1);
	}

	has = has_item(where, item);

	if (has <= 0)
	{
		wout(c->who, "Mining yielded no %s.", just_name(item));
		return FALSE;
	}

	qty = has;

	move_item(where, c->who, item, qty);

	wout(c->who, "Mining yielded %s.", box_name_qty(item, qty));
	return TRUE;
}


int
v_mine_iron(struct command *c)
{

	return start_generic_mine(c, item_iron);
}


int
d_mine_iron(struct command *c)
{

	return finish_generic_mine(c, item_iron);
}


int
v_mine_gold(struct command *c)
{

	return start_generic_mine(c, item_gold);
}


int
d_mine_gold(struct command *c)
{

	return finish_generic_mine(c, item_gold);
}


int
v_mine_mithril(struct command *c)
{

	return start_generic_mine(c, item_mithril);
}


int
d_mine_mithril(struct command *c)
{

	return finish_generic_mine(c, item_mithril);
}


struct harvest
{
	int item;
	int vis_item;		/* replace item with this when generated */
	int mult;		/* multiply vis_item by this when gen'ing */
	int skill;
	int worker;
	int chance;		/* chance to get one each day, if nonzero */
	char *got_em;
	char *none_now;
	char *none_ever;
	char *task_desc;
	int public;		/* 3rd party view, yes/no */
}
harv_tbl[] = {
    {
	item_peasant, 0, 0,
	0,
	0,
	0,
	"recruited",
	"There are no more peasants here to recruit.",
	"Peasants must be recruited in provinces.",
	"recruit peasants",
	TRUE
    },
    {
	item_corpse, 0, 0,
	0,
	0,
	0,
	"raised",
	"There are no more corpses here to raise.",
	"Corpses are found in graveyards.",
	"raise corpses",
	FALSE
    },
    {
	item_mallorn_wood, 0, 0,
	sk_harvest_mallorn,
	0,
	20,
	"cut",
	"All mallorn wood ready this month has been cut here.",
	"Mallorn wood is found only in mallorn groves.",
	"cut mallorn wood",
	TRUE
    },
    {
	item_opium, 0, 0,
	sk_harvest_opium,
	0,
	0,
	"harvested",
	"All opium  ready this month has been harvested.",
	"Opium is harvested only in poppy fields.",
	"harvest opium",
	TRUE
    },
    {
	item_stone, 0, 0,
	sk_quarry_stone,
	item_worker,
	0,
	"quarried",
	"No further stone may be quarried here this month.",
	"Stone must be quarried in mountain provinces.",
	"quarry stone",
	TRUE
    },
    {
	item_fish, 0, 0,
	sk_fishing,
	item_sailor,
	50,
	"caught",
	"No further fish may be caught here this month.",
	"Fish must be caught in ocean provinces.",
	"catch fish",
	TRUE
    },
    {
	item_lumber, 0, 0,
	sk_harvest_lumber,
	item_worker,
	0,
	"cut",
	"All ready timber has already been cut this month.",
	"Wood must be cut in forest provinces.",
	"cut timber",
	TRUE
    },
    {
	item_yew, 0, 0,
	sk_harvest_yew,
	item_worker,
	0,
	"cut",
	"All yew available this month has already been cut.",
	"Yew must be cut in yew groves",
	"cut yew",
	TRUE
    },
    {
	item_wild_horse, 0, 0,
	sk_catch_horse,
	0,
	50,
	"caught",
	"No wild horses can be found roaming here now.",
	"Wild horses are found on the plains and in pastures.",
	"catch horses",
	TRUE
    },
    {
	item_avinia_leaf, 0, 0,
	sk_collect_foliage,
	0,
	20,
	"collected",
	"All of the avinia leaves here have been collected.",
	"Avinia leaves are found in mallorn groves.",
	"collect avinia leaves",
	TRUE
    },
    {
	item_spiny_root, 0, 0,
	sk_collect_foliage,
	0,
	25,
	"collected",
	"All of the spiny roots here have been collected.",
	"Avinia leaves are found in swamps, pits and bogs.",
	"collect spiny roots",
	TRUE
    },
    {
	item_lana_bark, 0, 0,
	sk_collect_foliage,
	0,
	50,
	"collected",
	"All of the lana bark here has been collected.",
	"Lana bark is found in circles of trees.",
	"collect lana bark",
	TRUE
    },
    {
	item_farrenstone, 0, 0,
	sk_collect_elem,
	0,
	100,
	"collected",
	"This cave's supply of farrenstone for this month has been exhausted.",
	"Farrenstone is found in caves.",
	"collect farrenstone",
	TRUE
    },
    {
	item_pretus_bones, 0, 0,
	sk_collect_elem,
	0,
	100,
	"collected",
	"No pretus bones can be found.",
	"Pretus bones are found in sand pits.",
	"collect pretus bones",
	TRUE
    },
    {
	item_mage_menial, item_gold, 10,
	sk_mage_menial,
	0,
	100,
	"earned",
	"No work at common magic can be found here.",
	"No work at common magic can be found here.",
	"work at common magic",
	TRUE
    },
    {
	0, 0, 0,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	0
    }
};


static struct harvest *
find_harv(int k)
{
	int i;

	for (i = 0; harv_tbl[i].item; i++)
		if (harv_tbl[i].item == k)
			return &harv_tbl[i];

	return NULL;
}


static ilist collectors = NULL;


void
init_collect_list()
{
	int i;
	struct command *c;
	int cmd_collect;

	cmd_collect = find_command("collect");
	assert(cmd_collect > 0);

	loop_char(i)
	{
		c = rp_command(i);

		if (c && c->state == STATE_RUN && c->cmd == cmd_collect)
			ilist_append(&collectors, i);
	}
	next_char;
}


static void
bump_other_collectors(int where, struct harvest *t)
{
	int i;
	struct command *c;
	int wh2;
	ilist l;

	l = ilist_copy(collectors);

	for (i = 0; i < ilist_len(l); i++)
	{
		c = rp_command(l[i]);
		assert(c);

		if (c->a != t->item)
			continue;

		wh2 = subloc(c->who);

		if (t->item == item_fish && is_ship(wh2))
			wh2 = loc(wh2);

		if (where != wh2)
			continue;

		interrupt_order(c->who);
	}

	ilist_reclaim(&l);
}


int
v_generic_harvest(struct command *c, int number, int days, struct harvest *t)
{
	int where = subloc(c->who);
	int workers;
	int avail;

	if (t->item == item_fish && is_ship(where))
		where = loc(where);

	if (t->skill && has_skill(c->who, t->skill) < 1)
	{
		wout(c->who, "Requires %s.", box_name(t->skill));
		return FALSE;
	}

	if (days < 1)
		days = -1;	/* as long as it takes to get number */

	c->c = number;		/* number desired; 0 means all possible */
	c->d = 0;		/* number we have obtained so far */

	avail = has_item(where, t->item);

	if (avail <= 0)
		return i_generic_harvest(c, t);

	if (t->worker)
	{
		workers = has_item(c->who, t->worker);

		if (workers < 1)
		{
			wout(c->who, "Need at least one %s to %s.",
						box_name(t->worker),
						t->task_desc);
			return FALSE;
		}
	}

	ilist_append(&collectors, c->who);

	c->wait = days;
	return TRUE;
}


int
d_generic_harvest(struct command *c, struct harvest *t)
{
	int where = subloc(c->who);
	int number = c->c;
	int qty;
	int workers;

	if (t->item == item_fish && is_ship(where))
		where = loc(where);

	qty = has_item(where, t->item);

	if (t->worker)
	{
		workers = has_item(c->who, t->worker);
		qty = min(qty, workers);

		if (number > 0 && (c->d + qty > number))
			qty = number - c->d;

		assert(qty >= 0);
	}
	else
	{
		qty = min(qty, 1);
	}

	if (qty > 0)
	{
		if (t->chance && rnd(1,100) > t->chance)
		{
			if (c->wait == 0)
				return i_generic_harvest(c, t);
			return TRUE;
		}

		if (t->vis_item)
		{
			consume_item(where, t->item, qty);
			gen_item(c->who, t->vis_item, qty * t->mult);
			c->d += qty * t->mult;
		}
		else
		{
			move_item(where, c->who, t->item, qty);
			c->d += qty;
		}

/*
 *  There's no point spending an extra day to find out that the
 *  resource is depleted.  If there are none left, terminate the
 *  command now, rather than next evening.
 *
 *  We also want to bump any other units collecting out, so they
 *  won't waste an extra evening just finding out that there's no
 *  more to collect.
 */

		if (has_item(where, t->item) == 0)
		{
			int ret = i_generic_harvest(c, t);
			bump_other_collectors(where, t);
			return ret;
		}

		if (c->wait != 0 && !(number > 0 && c->d >= number))
			return TRUE;		/* not done yet */
	}

	return i_generic_harvest(c, t);
}


static char *
mage_menial_how()
{

	switch (rnd(1,9))
	{
	case 1:		return " curing runny noses";
	case 2:		return " dowsing for water";
	case 3:		return " selling love potions";
	case 4:		return " selling good luck charms";
	case 5:		return " predicting the future";
	case 6:		return " reading palms";

	case 7:
	case 8:
	case 9:		return "";
	default:
		assert(FALSE);
	}

	return 0;
}


int
i_generic_harvest(struct command *c, struct harvest *t)
{
	int where = subloc(c->who);

	if (t->item == item_fish && is_ship(where))
		where = loc(where);

	if (c->d == 0)
	{
		if (item_gen_here(subkind(where), t->item))
			out(c->who, t->none_now);
		else
			out(c->who, t->none_ever);
	}
	else
	{
		int item;

		item = t->vis_item ? t->vis_item : t->item;

		if (t->item == item_mage_menial)
			wout(c->who, "Earned %s%s.", gold_s(c->d),
						mage_menial_how());
		else
			out(c->who, "%s %s.", cap(t->got_em),
				just_name_qty(item, c->d));


		if (t->public)
		{
			show_to_garrison = TRUE;

			if (t->item == item_mage_menial)
				wout(where, "%s earned %s working at common magic.",
					box_name(c->who), gold_s(c->d));
			else
				out(where, "%s %s %s.",
					box_name(c->who),
					t->got_em,
					just_name_qty(item, c->d));

			show_to_garrison = FALSE;
		}

		if (t->skill)
			add_skill_experience(c->who, t->skill);
	}

	ilist_rem_value(&collectors, c->who);

	c->wait = 0;
	if (c->d > 0 && c->d >= c->c)
		return TRUE;
	return FALSE;
}


int
v_collect(struct command *c)
{
	int item = c->a;
	int number = c->b;
	int days = c->c;
	struct harvest *t;

	t = find_harv(item);

	if (t == NULL)
	{
		wout(c->who, "Don't know how to collect %s.",
				box_code(item));
		return FALSE;
	}

	return v_generic_harvest(c, number, days, t);
}


int
d_collect(struct command *c)
{
	int item = c->a;
	struct harvest *t;

	t = find_harv(item);

	if (t == NULL)
	{
		out(c->who, "Internal error.");
		log_write(LOG_CODE, "d_collect: t is NULL, who=%d", c->who);
		return FALSE;
	}

	return d_generic_harvest(c, t);
}


int
i_collect(struct command *c)
{
	int item = c->a;
	struct harvest *t;

	t = find_harv(item);

	if (t == NULL)
	{
		out(c->who, "Internal error.");
		log_write(LOG_CODE, "i_collect: t is NULL, who=%d", c->who);
		return FALSE;
	}

	return i_generic_harvest(c, t);
}


int
v_quarry(struct command *c)
{
	int ret;

	ret = oly_parse(c, sout("collect %d %d %d", item_stone, c->a, c->b));
	assert(ret);

	return v_collect(c);
}


int
v_recruit(struct command *c)
{
	int ret;

	ret = oly_parse(c, sout("collect %d %d %d", item_peasant, c->a, c->b));
	assert(ret);

	return v_collect(c);
}


int
v_raise_corpses(struct command *c)
{
	int ret;

	ret = oly_parse(c, sout("collect %d %d %d", item_corpse, c->a, c->b));
	assert(ret);

	return v_collect(c);
}


int
v_fish(struct command *c)
{
	int ret;

	ret = oly_parse(c, sout("collect %d %d %d", item_fish, c->a, c->b));
	assert(ret);

	return v_collect(c);
}


int
v_wood(struct command *c)
{
	int ret;

	ret = oly_parse(c, sout("collect %d %d %d", item_lumber, c->a, c->b));
	assert(ret);

	return v_collect(c);
}


int
v_opium(struct command *c)
{
	int ret;

	ret = oly_parse(c, sout("collect %d %d %d", item_opium, c->a, c->b));
	assert(ret);

	return v_collect(c);
}


int
v_mallorn(struct command *c)
{
	int ret;

	ret = oly_parse(c, sout("collect %d %d %d",
					item_mallorn_wood, c->a, c->b));
	assert(ret);

	return v_collect(c);
}


int
v_yew(struct command *c)
{
	int ret;

	ret = oly_parse(c, sout("collect %d %d %d", item_yew, c->a, c->b));
	assert(ret);

	return v_collect(c);
}


int
v_catch(struct command *c)
{
	int ret;

	ret = oly_parse(c,
		sout("collect %d %d %d", item_wild_horse, c->a, c->b));
	assert(ret);

	return v_collect(c);
}


int
v_mage_menial(struct command *c)
{
	int ret;

	ret = oly_parse(c,
		sout("collect %d 0 %d", item_mage_menial, c->a));
	assert(ret);

	return v_collect(c);
}

