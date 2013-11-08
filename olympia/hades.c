
#include	<stdio.h>
#include	<math.h>
#include	"z.h"
#include	"oly.h"


int hades_region = 0;
int hades_pit = 0;		/* Pit of Hades */
int hades_player = 0;


#define	SZ	100		/* SZ x SZ is the maximum size of Hades */


/*
 *  New Hades configuration
 */


void
create_hades()
{
	int r, c, sz, space;
	int map[SZ][SZ];
	int n;
	int i;
	int north, east, south, west;
	struct entity_loc *p;
	struct entity_subloc *s;
	char *pw;
	ilist graveyards = NULL;
	int base, clear;
	int city, pit;

/*
 *  Create region wrapper for Hades
 */

	hades_region = new_ent(T_loc, sub_region);
	set_name(hades_region, "Hades");

	fprintf(stderr, "INIT: creating %s\n", box_name(hades_region));

/*
 *  Create the King of Hades player
 */

	assert(hades_player == 0);

	hades_player = 205;
	alloc_box(hades_player, T_player, sub_pl_npc);
	set_name(hades_player, "King of Hades");

	/* To override the default password below, create/edit the file "PWD" which contains:

fairy fairypassword
combat combatpassword

	   The string up to the first whitespace contains the keyword used to look up the password below
	   The string after the whitespace contains the password to use instead of the default one
	 */

	pw = read_pw("hades");
	if (pw == NULL)
		pw = "noyoudont";
	p_player(hades_player)->password = pw;

/*
 * Work out how big Hades should be.
 * It will be sized dynamically so that there is approximately one
 * graveyard per 8 Hades provinces, so we need to find out how many
 * graveyards there are...
 */

	loop_loc(i)
	{
		if (subkind(i) == sub_graveyard)
		{
			ilist_append(&graveyards, i);
			set_known(hades_player, i);
		}
	}
	next_loc;

	sz = (int) ceil(sqrt(ilist_len(graveyards) * 8));
	if (sz > SZ)
		sz = SZ;
	fprintf(stderr, "Hades is %dx%d (%d graveyards).\n", sz, sz, ilist_len(graveyards));

/*
 *  Fill map[row,col] with locations.
 */

	// see if there's a contiguous block of provinces so that Hades
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
			if (clear)
			{
				n = 10000 + (base + r) * 100 + c;
				alloc_box(n, T_loc, sub_under);
			}
			else
			{
				n = new_ent(T_loc, sub_under);
			}

			map[r][c] = n;
		}

/*
 *  Set the NESW exit routes for every map location
 */

	for (r = 0; r < sz; r++)
	{
		for (c = 0; c < sz; c++)
		{
			n = map[r][c];
			bx[n]->temp = 0;
			p = p_loc(n);

			set_name(n, "Hades");
			set_where(n, hades_region);
			// 50% of Hades regions are hidden
			if (rnd(0, 1))
			{
				p_loc(n)->hidden = TRUE;

				set_known(hades_player, n);
			}

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
	}

	space = sz * sz;

/*
 *  Place a city in the center of the map, with the Pit of Hades inside
 *  the city.
 */

	n = map[sz/2][sz/2];
	city = new_ent(T_loc, sub_city);
	set_where(city, n);
	set_name(city, "City of the Dead");
	set_known(hades_player, city);
	bx[n]->temp = 1;
	space--;
	
	seed_city(city);

	// s = p_subloc(city);
	// ilist_append(&s->teaches, sk_necromancy);

	pit = new_ent(T_loc, sub_hades_pit);
	set_where(pit, city);
	set_name(pit, "Pit of Hades");
	set_known(hades_player, pit);

	hades_pit = pit;

/*
 *  Put other cities in Hades, as it was too boring
 */

	for (r = 0; r < sz; r++)
		for (c = 0; c < sz; c++)
		{
			if (rnd(0, 60))
				continue;
			n = map[r][c];
			if (bx[n]->temp)
				continue;
			city = new_ent(T_loc, sub_city);
			set_where(city, n);
			set_name(city, "Necropolis");
			set_known(hades_player, city);
			bx[n]->temp = 1;
			space--;
			
			seed_city(city);
		}
/*
 *
 *  Dual-link every graveyard from the world into one of the
 *  Hades locations except the center one containing the pit.
 */

	ilist_scramble(graveyards);

	assert(space > ilist_len(graveyards));

	i = 0;
	while (i < ilist_len(graveyards))
	{
		r = rnd(1, sz) - 1;
		c = rnd(1, sz) - 1;

		if (bx[map[r][c]]->temp)
			continue;

		bx[map[r][c]]->temp = 1;
		space--;

		s = p_subloc(graveyards[i]);
		ilist_append(&s->link_to, map[r][c]);
		s->link_when = -1;
		s->link_open = -1;

		s = p_subloc(map[r][c]);
		ilist_append(&s->link_from, graveyards[i]);

		i++;
	}

	ilist_reclaim(&graveyards);

	printf("hades loc is %s\n", box_name(map[1][1]));
}


static void
create_hades_nasty()
{
	int new;
	struct loc_info *p;
	int where;

	p = rp_loc_info(hades_region);
	assert(p);

	where = p->here_list[rnd(0,ilist_len(p->here_list)-1)];

	switch (rnd(1,4))
	{
	case 1:
		new = new_char(sub_ni, item_spirit, where, 100, hades_player,
					LOY_npc, 0, "Tortured spirits");

		if (new < 0)
			return;

		gen_item(new, item_spirit, rnd(25,75));
		break;

	case 2:
		new = new_char(0, 0, where, 100, hades_player,
					LOY_npc, 0, "Ghostly presence");

		if (new < 0)
			return;

		p_char(new)->attack = 100;
		rp_char(new)->defense = 100;
		break;

	case 3:
		new = new_char(0, 0, where, 100, hades_player,
					LOY_npc, 0, "Lesser Demon");

		if (new < 0)
			return;

		p_char(new)->attack = 250;
		rp_char(new)->defense = 250;
		gen_item(new, item_spirit, rnd(50,150));
		break;

	case 4:
		new = new_char(0, 0, where, 100, hades_player,
					LOY_npc, 0, "Greater Demon");

		if (new < 0)
			return;

		p_char(new)->attack = 500;
		rp_char(new)->defense = 500;

		gen_item(new, item_spirit, rnd(100,250));
		break;

	default:
		assert(FALSE);
	}

	queue(new, "wait time 0");
	init_load_sup(new);   /* make ready to execute commands immediately */
}


static void
auto_hades_sup(int who)
{
	int i;
	int where = subloc(who);
	int queued_something = FALSE;

	loop_here(where, i)
	{
		if (kind(i) != T_char || subkind(player(i)) != sub_pl_regular)
			continue;

		if (has_skill(i, sk_transcend_death) && char_alone(i))
			continue;

		queued_something = TRUE;

		queue(who, "attack %s", box_code_less(i));
	}
	next_here;

	if (!queued_something)
		npc_move(who);
}


void
auto_hades()
{
	int i;
	int n_hades = 0;

	loop_units(hades_player, i)
	{
		n_hades++;
	}
	next_unit;

	while (n_hades < 25)
	{
		create_hades_nasty();
		n_hades++;
	}

	loop_units(hades_player, i)
	{
		auto_hades_sup(i);
	}
	next_unit;
}


int
random_hades_loc()
{
	ilist l = NULL;
	int i;
	int ret;

	loop_loc(i)
	{
		if (region(i) != hades_region)
			continue;
		if (subkind(i) != sub_under)
			continue;

		ilist_append(&l, i);
	}
	next_loc;

	if (ilist_len(l) < 1)
		return 0;

	ret = l[rnd(0,ilist_len(l)-1)];

	ilist_reclaim(&l);
	return ret;
}

