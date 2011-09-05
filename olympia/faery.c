
#include	<stdio.h>
#include	<unistd.h>
#include	"z.h"
#include	"oly.h"


int faery_region = 0;
int faery_player = 0;


#define	SZ_row	6		/* 6 rows tall */
#define	SZ_col	937		/* 1000 columns wide */

#define	NCITIES	100		/* num cities to make in Faery */


void
create_faery()
{
	int r, c;
	int map[SZ_row+1][SZ_col+1];
	int n;
	int i;
	int north, east, south, west;
	struct entity_loc *p;
	int sk;


/*
 *  Create region wrapper for Faery
 */

	faery_region = new_ent(T_loc, sub_region);
	set_name(faery_region, "Faery");

	fprintf(stderr, "INIT: creating %s\n", box_name(faery_region));

/*
 *  Fill map[row,col] with locations.
 *  Capped on the two ends with ocean
 *  top and bottom edges wrap
 */

	for (r = 0; r <= SZ_row; r++)
	{
		for (c = 0; c <= SZ_col; c++)
		{
			if (c == 0 || c == SZ_col)
				sk = sub_ocean;
			else
				sk = sub_forest;

			n = new_ent(T_loc, sk);
			map[r][c] = n;
			set_where(n, faery_region);
		}
	}

/*
 *  Set the NSEW exit routes for every map location
 */

	for (r = 0; r <= SZ_row; r++)
	{
		for (c = 0; c <= SZ_col; c++)
		{
			p = p_loc(map[r][c]);

			n = (r == 0 ? SZ_row : r-1);
			north = map[n][c];

			n = (r == SZ_row ? 0 : r+1);
			south = map[n][c];

			n = (c == SZ_col ? 0 : c+1);
			east = map[r][n];

			n = (c == 0 ? SZ_col : c-1);
			west = map[r][n];

			ilist_append(&p->prov_dest, north);
			ilist_append(&p->prov_dest, east);
			ilist_append(&p->prov_dest, south);
			ilist_append(&p->prov_dest, west);
		}
	}

/*
 *  Make a ring of stones
 *  Randomly place it in Faery
 *  link with a gate to a Ring of Stones in the outside world
 */

	{
		int gate;
		int ring;
		struct loc_info *li;
		int randloc;
		ilist l = NULL;
		int i;
		int other_ring;

		loop_loc(i)
		{
			if (subkind(i) == sub_stone_cir)
				ilist_append(&l, i);
		}
		next_loc;

		assert(ilist_len(l) > 0);
		ilist_scramble(l);
		other_ring = l[0];

		li = rp_loc_info(faery_region);
		assert(li && ilist_len(li->here_list) > 0);

		randloc = li->here_list[rnd(0, ilist_len(li->here_list)-1)];

		ring = new_ent(T_loc, sub_stone_cir);
		set_where(ring, randloc);

		gate = new_ent(T_gate, 0);
		set_where(gate, ring);

		p_gate(gate)->to_loc = other_ring;
		rp_gate(gate)->seal_key = rnd(111,999);

		ilist_reclaim(&l);
	}


/*
 *  Make a faery hill for every region on the map (except Faery itself).
 *  Place them randomly within Faery.
 *  Link them with the special road to a random location within the region.
 */

	loop_loc(i)
	{
		struct loc_info *li;
		int randloc;
		struct entity_subloc *sl;

		if (loc_depth(i) != LOC_region || i == faery_region)
			continue;

		li = rp_loc_info(i);

		if (li == NULL || ilist_len(li->here_list) < 1)
		{
			fprintf(stderr, "warning: loc info for %s is NULL\n",
					box_name(i));
			continue;
		}

		randloc = li->here_list[rnd(0, ilist_len(li->here_list)-1)];

		if (subkind(randloc) == sub_ocean)
			continue;

		r = rnd(0,SZ_row-1);
		c = rnd(1,SZ_col-2);

		n = new_ent(T_loc, sub_faery_hill);
		set_where(n, map[r][c]);

		sl = p_subloc(n);
		ilist_append(&sl->link_to, randloc);
		sl->link_when = rnd(0, NUM_MONTHS-1);

		sl = p_subloc(randloc);
		ilist_append(&sl->link_from, n);

		bx[map[r][c]]->temp = 1;
	}
	next_loc;

/*
 *  Create some Faery cities.  Faery cities have markets which sell
 *  rare items.
 */

	{
		ilist l = NULL;
		int i;
		int new;

		for (r = 1; r < SZ_row; r++)
		    for (c = 1; c < SZ_col; c++)
		    {
			if (bx[map[r][c]]->temp == 0)
				ilist_append(&l, map[r][c]);
		    }

		if (ilist_len(l) < NCITIES)
			fprintf(stderr, "\twarning: space for Faery cities "
					"only %d\n", ilist_len(l));

		ilist_scramble(l);

		for (i = 0; i < ilist_len(l) && i < NCITIES; i++)
		{
			new = new_ent(T_loc, sub_city);
			set_where(new, l[i]);
			set_name(new, "Faery city");
			seed_city(new);
		}

		ilist_reclaim(&l);
	}

/*
 *  Create the Faery player
 */

	assert(faery_player == 0);

	faery_player = 204;
	alloc_box(faery_player, T_player, sub_pl_npc);
	set_name(faery_player, "Faery player");
	p_player(faery_player)->password = str_save("noyoudont");

	printf("faery loc is %s\n", box_name(map[1][1]));
}


void
link_opener(int who, int where, int sk)
{
	struct entity_subloc *p, *pp;
	int i;
	int set_something = FALSE;

	p = rp_subloc(where);

	if (p == NULL)
	{
		wout(who, "Nothing happens.");
		return;
	}

	if (subkind(where) == sk && ilist_len(p->link_to) > 0)
	{
		if (p->link_open < 2 && p->link_open >= 0)
			p->link_open = 2;

		for (i = 0; i < ilist_len(p->link_to); i++)
			out(who, "A gateway to %s is here.",
					box_name(p->link_to[i]));

		set_something = TRUE;
	}

	for (i = 0; i < ilist_len(p->link_from); i++)
	{
		if (subkind(p->link_from[i]) != sk)
			continue;

		pp = rp_subloc(p->link_from[i]);
		assert(pp);

		if (pp->link_open < 2)
			pp->link_open = 2;

		out(who, "A gateway to %s is here.",
					box_name(p->link_from[i]));

		set_something = TRUE;
	}

	if (!set_something)
		wout(who, "Nothing happens.");
}


int
v_use_faery_stone(struct command *c)
{

	link_opener(c->who, subloc(c->who), sub_faery_hill);
	return TRUE;
}


static void
create_elven_hunt()
{
	int new;
	struct loc_info *p;
	int where;

	p = rp_loc_info(faery_region);
	assert(p);

	do
	{
		where = p->here_list[rnd(0,ilist_len(p->here_list)-1)];
	}
	while (subkind(where) == sub_ocean);

	new = new_char(sub_ni, item_elf, where, 100, faery_player,
						LOY_npc, 0, "Faery Hunt");

	if (new < 0)
		return;

	gen_item(new, item_elf, rnd(25,100));

	queue(new, "wait time 0");
	init_load_sup(new);   /* make ready to execute commands immediately */
}


static void
warn_human(int who, int targ)
{

	queue(who, "message 1 %s", box_code_less(targ));
	queue(who, "You are not welcome in Faery.  Leave, "
				"or you will be killed.");
	log(LOG_SPECIAL, "Faery hunt warned %s.", box_name(targ));
}


static void
auto_faery_sup(int who)
{
	int i;
	int where = subloc(who);
	struct entity_misc *p;
	int queued_something = FALSE;

	p = p_misc(player(who));

	loop_here(where, i)
	{
		if (kind(i) != T_char || subkind(player(i)) != sub_pl_regular)
			continue;

		if (stack_has_use_key(i, use_faery_stone))
			continue;

		queued_something = TRUE;

		if (!test_bit(p->npc_memory, i))
		{
			warn_human(who, i);
			set_bit(&p->npc_memory, i);
			continue;
		}

		queue(who, "attack %s", box_code_less(i));
	}
	next_here;

	if (!queued_something)
		npc_move(who);
}


void
auto_faery()
{
	int i;
	int n_faery = 0;

	loop_units(faery_player, i)
	{
		n_faery++;
	}
	next_unit;

	while (n_faery < 15)
	{
		create_elven_hunt();
		n_faery++;
	}

	loop_units(faery_player, i)
	{
		auto_faery_sup(i);
	}
	next_unit;
}


#if 0
void
swap_region_locs(int reg)
{
	ilist l = NULL;
	int i;
	int j;
	int who;
	int skip;

	loop_loc(i)
	{
		if (region(i) != reg)
			continue;

		if (loc_depth(i) != LOC_province)
			continue;

		skip = FALSE;
		loop_char_here(i, who)
		{
			if (char_moving(who) && player(who) == sub_pl_regular)
				skip = TRUE;
		}
		next_char_here;

		if (skip)
			continue;

		ilist_append(&l, i);
	}
	next_loc;

	if (ilist_len(l) < 2)
	{
		fprintf(stderr, "can't find two swappable locs for %s\n", box_name(reg));
		ilist_reclaim(&l);
		return;
	}

	ilist_scramble(l);

	loop_loc(i)
	{
		struct entity_loc *p;

		if (loc_depth(i) != LOC_province)
			continue;

		p = rp_loc(i);
		if (p == NULL)
			continue;

		for (j = 0; j < ilist_len(p->prov_dest); j++)
		{
			if (p->prov_dest[j] == l[0])
				p->prov_dest[j] = l[1];
			else if (p->prov_dest[j] == l[1])
				p->prov_dest[j] = l[0];
		}
	}
	next_loc;

	{
		ilist tmp;
		struct entity_loc *p1;
		struct entity_loc *p2;

		p1 = p_loc(l[0]);
		p2 = p_loc(l[1]);

		tmp = p1->prov_dest;
		p1->prov_dest = p2->prov_dest;
		p2->prov_dest = tmp;
	}

	log(LOG_CODE, "Swapped %s and %s in %s", box_name(l[0]), box_name(l[1]), box_name(reg));
}
#endif

