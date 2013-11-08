
#include	<stdio.h>
#include	<math.h>
#include	"z.h"
#include	"oly.h"


int faery_region = 0;
int faery_player = 0;


#define	SZ	100		/* SZ x SZ is the maximum size of faery */


void
create_faery()
{
	int r, c, hills, total, sz, space, base, clear;
	int map[SZ][SZ];
	int n;
	int i;
	int north, east, south, west;
	struct entity_loc *p;
	struct loc_info *li;
	int sk, new;
	char *pw;


/*
 *  Create region wrapper for Faery
 */

	faery_region = new_ent(T_loc, sub_region);
	set_name(faery_region, "Faery");

	fprintf(stderr, "INIT: creating %s\n", box_name(faery_region));

/*
 * Size Faery dynamically to fit the number of faery hills we want
 */

	total = 0;
	loop_loc(i)
	{
		if (loc_depth(i) != LOC_region || i == faery_region)
			continue;

		li = rp_loc_info(i);

		if (li == NULL || ilist_len(li->here_list) < 1)
			continue;

		hills = ilist_len(li->here_list) / 50;
		if (hills < 1)
			hills = 1;

		total += hills;
	}
	next_loc;

	sz = (int) ceil(sqrt(total * 16)) + 2;
	if (sz > SZ)
		sz = SZ;
	
	fprintf(stderr, "Faery is %dx%d (max %d hills)\n", sz, sz, total);

/*
 *  Fill map[row,col] with locations.
 *  Capped on all edges with ocean
 */

	// see if there's a contiguous block of provinces so that Faery
	// map coords can follow the same pattern as the surface
	clear = 0;
	for (base = 0; base < 400 - sz; base += 20)
	{
		n = 10000 + base * 100;
		if (bx[n] == NULL)
		{
			clear = 1;
			for (r = 0; clear && r < sz; r++)
				for (c = 0; clear && c < sz; c++)
				{
					n = 10000 + (base + r) * 100 + c;
					if (bx[n] != NULL)
						clear = 0;
				}
			break;
		}
	}
	for (r = 0; r < sz; r++)
		for (c = 0; c < sz; c++)
		{
			if (c == 0 || c == sz - 1 || r == 0 || r == sz - 1)
				sk = sub_ocean;
			else
				sk = sub_forest;
			if (clear)
			{
				n = 10000 + (base + r) * 100 + c;
				alloc_box(n, T_loc, sk);
			}
			else
			{
				n = new_ent(T_loc, sk);
			}

			map[r][c] = n;
			set_where(n, faery_region);
		}

/*
 *  Set the NSEW exit routes for every map location
 */

	for (r = 0; r < sz; r++)
		for (c = 0; c < sz; c++)
		{
			p = p_loc(map[r][c]);

			if (r == 0)
				north = 0;
			else
				north = map[r-1][c];

			if (r < sz - 1)
				south = map[r+1][c];
			else
				south = 0;

			if (c < sz - 1)
				east = map[r][c+1];
			else
				east = 0;

			if (c == 0)
				west = 0;
			else
				west = map[r][c-1];

			ilist_append(&p->prov_dest, north);
			ilist_append(&p->prov_dest, east);
			ilist_append(&p->prov_dest, south);
			ilist_append(&p->prov_dest, west);
		}

	clear_temps(T_loc);
	space = sz * sz;

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
		bx[randloc]->temp = 1;
		space--;

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

		if (subkind(li->here_list[0]) == sub_ocean)
			continue;

		hills = ilist_len(li->here_list) / 50;
		if (hills < 1)
			hills = 1;

		while (space > 0 && hills > 0)
		{
			hills--;
			/* 50% chance of a hill for each 50 provinces
			 * in a region, but at least one
			 */
			if (hills && rnd(0,1))
				continue;
			do
			{
				randloc = li->here_list[rnd(0, ilist_len(li->here_list)-1)];
				r = rnd(1, sz - 2);
				c = rnd(1, sz - 2);
			}
			while (bx[randloc]->temp || bx[map[r][c]]->temp);

			n = new_ent(T_loc, sub_faery_hill);
			set_where(n, map[r][c]);

			sl = p_subloc(n);
			ilist_append(&sl->link_to, randloc);
			sl->link_when = rnd(0, NUM_MONTHS-1);

			sl = p_subloc(randloc);
			ilist_append(&sl->link_from, n);

			bx[map[r][c]]->temp = 1;
			bx[randloc]->temp = 1;
			space--;
		}
	}
	next_loc;

/*
 *  Create some Faery cities.  Faery cities have markets which sell
 *  rare items.
 */

	new = 0;
	for (r = 2; space > 0 && r < sz - 2; r++)
		for (c = 2; space > 0 && c < sz - 2; c++)
		{
			if (bx[map[r][c]]->temp)
				continue;
			if (rnd(0, 30))
				continue;
			new = new_ent(T_loc, sub_city);
			set_where(new, map[r][c]);
			set_name(new, "Faery city");
			seed_city(new);
			bx[map[r][c]]->temp = 1;
			space--;
		}
	
	while (!new && space > 0)
	{
		r = rnd(2, sz - 3);
		c = rnd(2, sz - 3);
		if (bx[map[r][c]]->temp)
			continue;
		new = new_ent(T_loc, sub_city);
		set_where(new, map[r][c]);
		set_name(new, "Faery city");
		seed_city(new);
		bx[map[r][c]]->temp = 1;
		space--;
	}

/*
 *  Create the Faery player
 */

	assert(faery_player == 0);

	faery_player = 204;
	alloc_box(faery_player, T_player, sub_pl_npc);
	set_name(faery_player, "Faery player");

	/* To override the default password below, create/edit the file "PWD" which contains:

fairy fairypassword
combat combatpassword

	   The string up to the first whitespace contains the keyword used to look up the password below
	   The string after the whitespace contains the password to use instead of the default one
	 */

	pw = read_pw("faery");
	if (pw == NULL)
		pw = "noyoudont";
	p_player(faery_player)->password = pw;

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
	log_write(LOG_SPECIAL, "Faery hunt warned %s.", box_name(targ));
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

	log_write(LOG_CODE, "Swapped %s and %s in %s", box_name(l[0]), box_name(l[1]), box_name(reg));
}
#endif

