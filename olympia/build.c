
#include	<stdio.h>
#include 	<string.h>
#include	"z.h"
#include	"oly.h"


int
fort_default_defense(int sk)
{

	switch (sk)
	{
	case sub_castle:	return 50;
	case sub_tower:		return 40;
	case sub_galley:	return 20;
	case sub_roundship:	return 10;
	case sub_temple:	return 10;
	case sub_mine:		return 10;
	case sub_inn:		return 10;
	}

	return 0;
}


static int
ship_loc_okay(struct command *c, int where)
{

	if (!has_ocean_access(where))
	{
		wout(c->who, "%s is not an ocean port location.",
				box_name(where));
		return FALSE;
	}

	if (subkind(where) != sub_city)
	{
		wout(c->who, "Ships may only be built in cities.");
		return FALSE;
	}

	return TRUE;
}


static int
temple_loc_okay(struct command *c, int where)
{

	if (safe_haven(where))
	{
		wout(c->who, "Building is not permitted in safe havens.");
		return FALSE;
	}

	if (loc_depth(where) != LOC_build)
		return TRUE;

	wout(c->who, "A temple may not be built inside another building.");

	return FALSE;
}


static int
tower_loc_okay(struct command *c, int where)
{
	int ld = loc_depth(where);

#if 0
	if (safe_haven(where))
	{
		wout(c->who, "Building is not permitted in safe havens.");
		return FALSE;
	}
#endif

	if (ld != LOC_province &&
	    ld != LOC_subloc &&
	    subkind(where) != sub_castle &&
	    subkind(where) != sub_castle_notdone)
	{
		wout(c->who, "A tower may not be built here.");
		return FALSE;
	}

	if (ld == LOC_build &&
	    count_loc_structures(where, sub_tower, sub_tower_notdone) >= 6)
	{
	    wout(c->who, "Six towers at most can be built within a %s.",
					subkind_s[subkind(where)]);
	    return FALSE;
	}

	return TRUE;
}


static int
mine_loc_okay(struct command *c, int where)
{

	if (subkind(where) != sub_mountain && subkind(where) != sub_rocky_hill)
	{
		wout(c->who, "Mines may only be built in mountain provinces "
						"and rocky hills.");
		return FALSE;
	}

	if (safe_haven(where))
	{
		wout(c->who, "Building is not permitted in safe havens.");
		return FALSE;
	}

	if (count_loc_structures(where, sub_mine, sub_mine_notdone))
	{
		wout(c->who, "A location may not have more than one mine.");
		return FALSE;
	}

	if (count_loc_structures(where, sub_mine_collapsed, 0))
	{
		wout(c->who, "Another mine may not be built here until the "
			"collapsed mine vanishes.");
		return FALSE;
	}

	return TRUE;
}


static int
inn_loc_okay(struct command *c, int where)
{

	if (safe_haven(where))
	{
		wout(c->who, "Building is not permitted in safe havens.");
		return FALSE;
	}

	if (loc_depth(where) != LOC_province && subkind(where) != sub_city)
	{
	    wout(c->who, "Inns may only be built in cities and provinces.");
	    return FALSE;
	}

	return TRUE;
}


/*
 *  Return a sublocation if it can be found in the province or in
 *  the city.
 */

int
province_subloc(int where, int sk)
{
	int city;
	int prov;
	int n;

	prov = province(where);
	city = city_here(prov);

	if (n = subloc_here(prov, sk))
		return n;

	if (city)
		return subloc_here(city, sk);

	return 0;
}


static int
castle_loc_okay(struct command *c, int where)
{

	if (safe_haven(where))
	{
		wout(c->who, "Building is not permitted in safe havens.");
		return FALSE;
	}

	if (loc_depth(where) != LOC_province &&
		subkind(where) != sub_city)
	{
	    wout(c->who, "A castle must be built in a province or a city.");
	    return FALSE;
	}

	if (province_subloc(where, sub_castle) ||
	    province_subloc(where, sub_castle_notdone))
	{
		wout(c->who, "This province already contains a castle.  "
				"Another may not be built here.");
		return FALSE;
	}

	return TRUE;
}


struct build_ent
{
	char *what;				/* what are we building? */
	int skill_req, skill2;			/* one or the other */
	int kind;

	int (*loc_ok)(struct command *c, int where);

	int unfinished_subkind;
	int finished_subkind;

	int min_workers;			/* min # of workers to begin */

	int worker_days;			/* time to complete */
	int min_days;				/* soonest can be completed */

	int req_item;
	int req_qty;

	int capacity;
	char *default_name;
}
build_tbl[] =
{
    {
	"galley",
	sk_shipbuilding, 0,
	T_ship,
	ship_loc_okay,				/* can we build here? */
	sub_galley_notdone, sub_galley,		/* ship types */
	3,					/* minimum # of workers */
	250,					/* worker-days to complete */
	1,					/* at least n days */
	item_lumber, 10,			/* required item, 1/5 qty */
	5000,					/* structure capacity */
	"New galley"				/* default name */
    },
    {
	"roundship",
	sk_shipbuilding, 0,
	T_ship,
	ship_loc_okay,				/* can we build here? */
	sub_roundship_notdone, sub_roundship,	/* ship types */
	3,					/* minimum # of workers */
	500,					/* worker-days to complete */
	1,					/* at least n days */
	item_lumber, 20,			/* required item, 1/5 qty */
	25000,					/* structure capacity */
	"New roundship"				/* default name */
    },
    {
	"raft",
	0, 0,
	T_ship,
	ship_loc_okay,				/* can we build here? */
	sub_raft_notdone, sub_raft,		/* ship types */
	0,					/* minimum # of workers */
	45,					/* worker-days to complete */
	1,					/* at least n days */
	item_flotsam, 5,			/* required item, 1/5 qty */
	2500,					/* structure capacity */
	"New raft"				/* default name */
    },
    {
	"temple",
	sk_construction, 0,
	T_loc,
	temple_loc_okay,			/* can we build here? */
	sub_temple_notdone, sub_temple,
	3,					/* minimum # of workers */
	1000,					/* worker-days to complete */
	1,					/* at least n days */
	item_stone, 10,				/* required item, 1/5 qty */
	0,					/* structure capacity */
	"New temple"				/* default name */
    },
    {
	"inn",
	sk_construction, 0,
	T_loc,
	inn_loc_okay,				/* can we build here? */
	sub_inn_notdone, sub_inn,
	3,					/* minimum # of workers */
	300,					/* worker-days to complete */
	1,					/* at least n days */
	item_lumber, 15,			/* required item, 1/5 qty */
	0,					/* structure capacity */
	"New inn"				/* default name */
    },
    {
	"castle",
	sk_construction, 0,
	T_loc,
	castle_loc_okay,			/* can we build here? */
	sub_castle_notdone, sub_castle,
	3,					/* minimum # of workers */
	10000,					/* worker-days to complete */
	1,					/* at least n days */
	item_stone, 100,			/* required item, 1/5 qty */
	0,					/* structure capacity */
	"New castle"				/* default name */
    },
    {
	"mine",
	sk_construction, sk_mining,
	T_loc,
	mine_loc_okay,				/* can we build here? */
	sub_mine_notdone, sub_mine,
	3,					/* minimum # of workers */
	500,					/* worker-days to complete */
	1,					/* at least n days */
	item_lumber, 5,				/* required item, 1/5 qty */
	0,					/* structure capacity */
	"New mine"				/* default name */
    },
    {
	"tower",
	sk_construction, 0,
	T_loc,
	tower_loc_okay,				/* can we build here? */
	sub_tower_notdone, sub_tower,
	3,					/* minimum # of workers */
	2000,					/* worker-days to complete */
	1,					/* at least n days */
	item_stone, 20,				/* required item, 1/5 qty */
	0,					/* structure capacity */
	"New tower"				/* default name */
    },
    {
	NULL,
	0, 0,
	0,
	NULL,
	0, 0,
	0,
	0,
	0,
	0, 0,
	0,
	NULL
    }
};


static int fuzzy_build_match;


static struct build_ent *
find_build(char *s)
{
	int i;

	fuzzy_build_match = FALSE;

	for (i = 0; build_tbl[i].what; i++)
		if (i_strcmp(build_tbl[i].what, s) == 0)
			return &build_tbl[i];

	fuzzy_build_match = TRUE;

	for (i = 0; build_tbl[i].what; i++)
		if (fuzzy_strcmp(build_tbl[i].what, s))
			return &build_tbl[i];

	return NULL;
}


static int
build_materials_check(struct command *c, struct build_ent *bi)
{

	if (bi->skill_req)		/* if a skill is required... */
	{
		if (bi->skill2)		/* either one of two skills */
		{
			if (has_skill(c->who, bi->skill_req) < 1 &&
			    has_skill(c->who, bi->skill2) < 1)
			{
				wout(c->who, "Building a %s requires either "
						"%s or %s.",
						bi->what,
						box_name(bi->skill_req),
						box_name(bi->skill2));
				return FALSE;
			}
		}
		else			/* single skill requirement */
		{
			if (has_skill(c->who, bi->skill_req) < 1)
			{
				wout(c->who, "Building a %s requires %s.",
						bi->what,
						box_name(bi->skill_req));
				return FALSE;
			}
		}
	}

/*
 *  Materials check
 */

	if (bi->req_item > 0 && has_item(c->who, bi->req_item) < bi->req_qty)
	{
		wout(c->who, "Need %s to start.",
				box_name_qty(bi->req_item, bi->req_qty));
		return FALSE;
	}

	if (has_item(c->who, item_worker) < bi->min_workers)
	{
		wout(c->who, "Need at least %s for construction.",
				box_name_qty(item_worker, bi->min_workers));
		return FALSE;
	}

/*
 *  Materials deduct
 */

	if (bi->req_item > 0)
		consume_item(c->who, bi->req_item, bi->req_qty);

	return TRUE;
}


static void
create_new_building(struct command *c, struct build_ent *bi, int where)
{
	struct entity_subloc *p;

	change_box_subkind(where, bi->finished_subkind);

	p = p_subloc(where);
	p->effort_given = 0;
	p->effort_required = 0;
	p->defense = fort_default_defense(bi->finished_subkind);
	p->damage = 0;
	p->build_materials = 0;

	if (bi->finished_subkind == sub_mine)
	{
		p->shaft_depth = 3;
		if (rnd(1,5) == 1)
			gen_item(where, item_gate_crystal, 1);
		mine_production(where);
	}

	if (bi->finished_subkind == sub_temple)
		ilist_append(&p->teaches, sk_religion);

	if (bi->skill_req)
		add_skill_experience(c->who, bi->skill_req);
}


int
start_build(struct command *c, struct build_ent *bi, int new)
{
	char *new_name;
	struct entity_subloc *p;
	int pl = player(c->who);
	int instant_build = FALSE;	/* build takes 1 day and no effort */
	int where = subloc(c->who);

#if 0
/*
 *  First tower a player builds take no workers, materials or time
 */

	if (bi->finished_subkind == sub_tower &&
	    p_player(pl)->first_tower == FALSE)
	{
		instant_build = TRUE;
		p_player(pl)->first_tower = TRUE;
	}
#endif

	if (!instant_build && !build_materials_check(c, bi))
		return FALSE;

	change_box_kind(new, bi->kind);
	change_box_subkind(new, bi->unfinished_subkind);

	set_where(new, where);

	if (numargs(c) < 2 || c->parse[2] == NULL || *(c->parse[2]) == '\0')
		new_name = bi->default_name;
	else
		new_name = c->parse[2];

	if (strlen(new_name) > 25)
	{
		wout(c->who, "The name you gave is too long.  Place names "
			"must be 25 characters or less.  Please use the "
			"NAME order to set a shorter name next turn.");

		new_name = bi->default_name;
	}

	set_name(new, new_name);

	p = p_subloc(new);

	p->effort_required = bi->worker_days * 100;
	p->damage = 0;
	p->build_materials = 0;
	p->capacity = bi->capacity;

	if (instant_build)
	{
		change_box_subkind(new, bi->finished_subkind);
		wout(c->who, "Built %s.", box_name_kind(new));

		show_to_garrison = TRUE;
		wout(where, "%s built %s in %s.",
						box_name(c->who),
						box_name_kind(new),
						box_name(where));
		show_to_garrison = FALSE;
	}
	else
	{
		wout(c->who, "Created %s.", box_name_kind(new));

		show_to_garrison = TRUE;
		wout(where, "%s began construction of %s in %s.",
						box_name(c->who),
						box_name_kind(new),
						box_name(where));
		show_to_garrison = FALSE;
	}

	move_stack(c->who, new);

	if (instant_build)
	{
		create_new_building(c, bi, new);

		c->wait = 1;
		c->inhibit_finish = TRUE;
		return TRUE;
	}

	return TRUE;
}


int
daily_build(struct command *c, struct build_ent *bi)
{
	int nworkers;
	int inside = subloc(c->who);
	struct entity_subloc *p;
	int bonus;
	int effort_given;

/*
 *  NOTYET:  apply building energy to repair structure if damaged
 *		currently, damage figure gets erased when
 *		the structure is completed.
 */

	if (subkind(inside) == bi->finished_subkind)
	{
		wout(c->who, "%s is finished!",
					box_name(inside));
		c->wait = 0;
		return TRUE;
	}

	if (subkind(inside) != bi->unfinished_subkind)
	{
		wout(c->who, "%s is no longer in a %s.  Construction halts.",
				just_name(c->who), bi->what);
		return FALSE;
	}

	if (bi->min_workers == 0)
		nworkers = 1;
	else
		nworkers = has_item(c->who, item_worker);

	if (nworkers <= 0)
	{
		wout(c->who, "%s has no workers.  Construction halts.",
					just_name(c->who));
		return FALSE;
	}

	p = p_subloc(inside);

/*
 *  Give a 5% speed bonus for each experience level of the
 *  construction skill
 */

	bonus = 5 * c->use_exp * nworkers;
	effort_given = nworkers * 100 + bonus;

/*
 *  Materials check
 */

	// by Cappinator:
	// TODO: Fix bug: if 2/5 or more of the effort is given
	// in one day, only 1/5 of the materials is deducted.

	if (bi->req_item > 0)
	{
	    int fifth = (p->effort_given + effort_given) * 5 /
						p->effort_required;

	    while (fifth < 5 && p->build_materials < fifth)
	    {
		if (!consume_item(c->who, bi->req_item, bi->req_qty))
		{
			wout(c->who, "Need another %s to continue work.  "
					"Construction halted.",
				box_name_qty(bi->req_item, bi->req_qty));
			return FALSE;
		}

		p->build_materials++;
	    }
	}

	p->effort_given += effort_given;

	if (p->effort_given < p->effort_required ||
	    command_days(c) < bi->min_days)
		return TRUE;

/*
 *  It's done
 */

	create_new_building(c, bi, inside);

	wout(c->who, "%s is finished!", box_name(inside));

	c->wait = 0;
	return TRUE;
}


int
build_structure(struct command *c, struct build_ent *bi, int new)
{
	int who = c->who;
	int where = subloc(who);

	if (loc_depth(where) == LOC_build)
	{
		if (subkind(where) == bi->finished_subkind)
		{
			wout(who, "%s is already finished.", box_name(where));
			return FALSE;
		}

		if (subkind(where) == bi->unfinished_subkind)
		{
			wout(who, "Continuing work on %s.", box_name(where));
			return TRUE;
		}
	}

	if (subkind(where) == sub_ocean)
	{
		wout(who, "Construction may not take place at sea.");
		return FALSE;
	}

	if (subkind(where) == sub_tunnel)
	{
		wout(c->who, "Building is not permitted underground.");
		return FALSE;
	}

	if (!(*bi->loc_ok)(c, where))
		return FALSE;

	return start_build(c, bi, new);
}


static char *
unfinished_building(int who)
{
	int where = subloc(who);

	switch (subkind(where))
	{
	case sub_castle_notdone:	return "castle";
	case sub_tower_notdone:		return "tower";
	case sub_temple_notdone:	return "temple";
	case sub_galley_notdone:	return "galley";
	case sub_roundship_notdone:	return "roundship";
	case sub_inn_notdone:		return "inn";
	case sub_mine_notdone:		return "mine";
	}

	return NULL;
}


int
v_build(struct command *c)
{
	struct build_ent *t;
	int days = c->c;
	char *s;
	int new = c->d;
	struct entity_player *p;

	if (numargs(c) < 1 && (s = unfinished_building(c->who)))
	{
		int ret;

		wout(c->who, "(assuming you meant 'build %s')", s);

		ret = oly_parse(c, sout("build %s", s));
		assert(ret);

		return v_build(c);
	}

	if (numargs(c) < 1)
	{
		wout(c->who, "Must specify what to build.");
		return FALSE;
	}

	t = find_build(c->parse[1]);

	if (t == NULL)
	{
		wout(c->who, "Don't know how to build '%s'.",
					c->parse[1]);
		return FALSE;
	}

	if (fuzzy_build_match)
		wout(c->who, "(assuming you meant 'build %s')", t->what);

	if (days)
		c->wait = days;

	p = p_player(player(c->who));

	if (new)
	{
		if (kind(new) != T_unform ||
		    ilist_lookup(p->unformed, new) < 0)
		{
			wout(c->who, "%s is not a valid unformed "
				"entity code.", box_code(new));

			new = 0;
		}
	}

#if 0
	if (new == 0 && ilist_len(p->unformed) > 0)
		new = p->unformed[0];
#endif

	if (new == 0)
		new = new_ent(T_unform, 0);

	if (new < 0)
	{
		wout(c->who, "Out of new entity codes.");
		return FALSE;
	}

	ilist_rem_value(&p->unformed, new);

	return build_structure(c, t, new);
}


int
d_build(struct command *c)
{
	struct build_ent *t;

	t = find_build(c->parse[1]);

	if (t == NULL)
	{
		log_write(LOG_CODE, "d_build: t is NULL (%s)", c->parse[1]);
		out(c->who, "Internal error.");
		return FALSE;
	}

	return daily_build(c, t);
}


/*
 *  Worker-days to repair a structure, for different structures
 */

static int
repair_points(int k)
{

	switch (k)
	{
	case sub_castle:		return 3;
	case sub_castle_notdone:	return 3;
	case sub_tower:			return 2;
	case sub_tower_notdone:		return 2;
	case sub_temple:		return 2;
	case sub_temple_notdone:	return 2;
	case sub_inn:			return 2;
	case sub_inn_notdone:		return 2;
	case sub_mine:			return 2;
	case sub_mine_notdone:		return 2;
	case sub_galley:		return 1;
	case sub_galley_notdone:	return 1;
	case sub_roundship:		return 1;
	case sub_roundship_notdone:	return 1;

	default:
		return 0;
	}

	assert(FALSE);
}


int
v_repair(struct command *c)
{
	int days = c->a;
	int where = subloc(c->who);
	int workers;
	int req_item = 0;
	int fort_def = fort_default_defense(subkind(where));

	if (days < 1)
		days = -1;

	if (loc_depth(where) != LOC_build)
	{
		wout(c->who, "%s may not be repaired.", box_name(where));
		return FALSE;
	}

	if (repair_points(subkind(where)) == 0)
	{
		wout(c->who, "%s may not be repaired.", box_name(where));
		return FALSE;
	}

	if (loc_damage(where) < 1 && loc_defense(where) >= fort_def)
	{
		wout(c->who, "%s is not damaged.", box_name(where));
		return FALSE;
	}

	workers = has_item(c->who, item_worker);

	if (workers < 1)
	{
		wout(c->who, "Need at least one %s.", box_name(item_worker));
		return FALSE;
	}

	switch (subkind(where))
	{
	case sub_galley:
	case sub_roundship:
		req_item = item_glue;
		break;
	}

	if (req_item && !consume_item(c->who, req_item, 1))
	{
		wout(c->who, "%s repair requires %s.",
					cap(subkind_s[subkind(where)]),
					box_name_qty(req_item, 1));
		return FALSE;
	}

	c->d = 0;		/* remainder */
	c->e = 0;
	c->f = 0;

	c->wait = days;
	return TRUE;
}


int
d_repair(struct command *c)
{
	int where = subloc(c->who);
	int workers;
	int per_point;
	struct entity_subloc *p;
	int points;
	int fort_def = fort_default_defense(subkind(where));

	if (loc_depth(where) != LOC_build)
	{
		wout(c->who, "No longer in a repairable structure.");
		return FALSE;
	}

	if (loc_damage(where) < 1 && loc_defense(where) >= fort_def)
	{
		wout(c->who, "%s has been fully repaired.", box_name(where));
		c->wait = 0;
		return TRUE;
	}

	workers = has_item(c->who, item_worker);

	if (workers < 1)
	{
		wout(c->who, "No longer have at least one %s.",
					box_name(item_worker));
		return FALSE;
	}

	per_point = repair_points(subkind(where));

	workers += c->d;
	c->d = workers % per_point;
	points = workers / per_point;

	p = p_subloc(where);

	if (p->damage > 0)
	{
		if (points > p->damage)
		{
			points -= p->damage;
			c->e += p->damage;
			p->damage = 0;
		}
		else
		{
			p->damage -= points;
			c->e += points;
			points = 0;
		}
	}

	if (points > 0 && p->defense < fort_def)
	{
		if (points > fort_def - p->defense)
		{
			c->f += fort_def - p->defense;
			p->defense = fort_def;
		}
		else
		{
			p->defense += points;
			c->f += points;
		}
	}

	if (p->damage < 1 && p->defense >= fort_def)
	{
		wout(c->who, "%s has been fully repaired.", box_name(where));
		i_repair(c);
		c->wait = 0;
		return TRUE;
	}

	if (c->wait == 0)
		i_repair(c);

	return TRUE;
}


int
i_repair(struct command *c)
{
	int where = subloc(c->who);

	vector_char_here(where);

	if (c->e && c->f)
		wout(VECT, "%s repaired %s damage and %s defense for %s.",
					box_name(c->who),
					nice_num(c->e), nice_num(c->f),
					box_name(where));
	else if (c->e)
		wout(VECT, "%s repaired %s damage to %s.",
					box_name(c->who),
					nice_num(c->e), box_name(where));
	else if (c->f)
		wout(VECT, "%s repaired %s defense for %s.",
					box_name(c->who),
					nice_num(c->f), box_name(where));
}


int
v_raze(struct command *c)
{
	int target = c->a;
	int where = subloc(c->who);
	int men;
	int per_point;

	if (loc_depth(where) != LOC_build)
	{
		wout(c->who, "Not in a building.");
		return FALSE;
	}

	if (target && where != target)
	{
		wout(c->who, "Not in %s.", c->parse[1]);
		return FALSE;
	}

	if (building_owner(where) != c->who)
	{
		wout(c->who, "Must be the owner of a structure to RAZE.");
		return FALSE;
	}

	per_point = repair_points(subkind(where));
	men = count_man_items(c->who) + 1;

	if (per_point < 1) {
		wout(c->who, "Can't raze this location.");
		return FALSE;
	}

#if 0
	if (men < per_point)
	{
		wout(c->who, "Need at least %d men to harm a %s.",
					per_point, subkind_s[subkind(where)]);
		return FALSE;
	}
#endif

	c->d = 0;		/* remainder */

	return TRUE;
}


int
d_raze(struct command *c)
{
	int target = c->a;
	int where = subloc(c->who);
	int men;
	int per_point;
	int points;

	if (loc_depth(where) != LOC_build)
	{
		wout(c->who, "No longer in a building.");
		return FALSE;
	}

	if (target && where != target)
	{
		wout(c->who, "No longer in %s.", c->parse[1]);
		return FALSE;
	}

	if (building_owner(where) != c->who)
	{
		wout(c->who, "Must be the owner of a structure to RAZE.");
		return FALSE;
	}

	per_point = repair_points(subkind(where));
	men = count_man_items(c->who) + 1;

	if (per_point < 1) {
		wout(c->who, "Can't raze this location.");
		return FALSE;
	}

	men += c->d;
	c->d = men % per_point;
	points = men / per_point;

	if (points > 100)
		points = 100;

/*
 *  NOTYET:  first erode defense points before going on to structure
 *	     damage, as with combat damage against structures?
 */

	if (add_structure_damage(where, points))
		c->wait = 0;
	return TRUE;
}


/*
 *	level	stone	worker-days
 *	-----	-----	-----------
 *	  1 	  50	   1000
 *	  2	  60	   1250
 *	  3	  70	   1500
 *	  4	  80	   1750
 *	  5	  90	   2000
 *	  6	 100	   2500
 */

static int improve_stone[] = {50, 60, 70, 80, 90, 100};
static int improve_work[] = {1000, 1250, 1500, 1750, 2000, 2500};

int
v_improve(struct command *c)
{
	int where = subloc(c->who);
	int days = c->a;
	struct entity_subloc *p;

	if (subkind(where) != sub_castle)
	{
		wout(c->who, "Not in a castle.");
		return FALSE;
	}

	if (castle_level(where) >= 6)
	{
		wout(c->who, "No further castle improvement is possible.");
		return FALSE;
	}

	if (has_item(c->who, item_worker) < 1)
	{
		wout(c->who, "Need at least one %s.", box_name(item_worker));
		return FALSE;
	}

	p = p_subloc(where);

	if (p->effort_given == 0)
	{
		int stone = improve_stone[castle_level(where)];

		if (has_item(c->who, item_stone) < stone)
		{
			wout(c->who, "%s required to proceed with improvement to level %s.",
					box_name_qty(item_stone, stone),
					nice_num(castle_level(where)+1));
			return FALSE;
		}

		consume_item(c->who, item_stone, stone);
		wout(c->who, "Applied %s to begin work toward level %s.",
					box_name_qty(item_stone, stone),
					nice_num(castle_level(where)+1));

		p->effort_given = improve_work[castle_level(where)];
	}

	if (days == 0)
		days = -1;

	c->wait = days;
	return TRUE;
}


int
d_improve(struct command *c)
{
	int where = subloc(c->who);
	int workers;
	struct entity_subloc *p;

	if (subkind(where) != sub_castle)
	{
		wout(c->who, "No longer in a castle.");
		return FALSE;
	}

	workers = has_item(c->who, item_worker);

	if (workers < 1)
	{
		wout(c->who, "No longer have at least one %s.",
					box_name(item_worker));
		return FALSE;
	}

	p = p_subloc(where);

	p->effort_given -= workers;
	if (p->effort_given < 0)
		p->effort_given = 0;

	if (p->effort_given == 0)
	{
		vector_clear();
		vector_add(c->who);
		vector_add(where);

		p->castle_lev++;
		p->defense += 5;

		out(where, "%s is now at improvement level %s, defense %d",
					box_name(where),
					nice_num(castle_level(where)),
					p->defense);

		c->wait = 0;
	}

	return TRUE;
}

