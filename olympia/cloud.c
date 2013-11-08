
#include	<stdio.h>
#include 	<string.h>
#include	"z.h"
#include	"oly.h"


int cloud_region = 0;


#define	SZ	4


/*
 *  New Cloudlands for G2
 *
 *
 *	 12345
 *      +-----
 *     1|  c
 *     2|  b
 *     3|  a
 *     4|  d
 *     5|  e
 *
 *
 *
 *
 *        fghij
 *         |  |
 *      6789 abcd
 *        |  |  |
 *        42135 e
 *          |
 *
 *
 *  Configuration:
 *
 *	123456*8
 *
 *
 *	  8
 *	  7
 *   	12*34
 *	  5dc
 *	  6ab
 *
 *	* = contains Cloud City
 *	Cloud City has gate to ring of stones somewhere
 *	Cloud City has sealed gate to ring of stones
 *	other locs all have incoming gates from random locs elsewhere
 */


/*
 *	%%%%
 *	%*%%	*=Nimbus
 *	%^%*	*=Aerovia	^=Mt. Olympus link
 *	*%%%	*=Stratos
 */
	

void
create_cloudlands()
{
	int r, c, clear, base;
	int map[SZ+1][SZ+1];
	int n;
	int i;
	int north, east, south, west;
	struct entity_loc *p;

/*
 *  Create region wrapper
 */

	cloud_region = new_ent(T_loc, sub_region);
	set_name(cloud_region, "Cloudlands");

	fprintf(stderr, "INIT: creating %s\n", box_name(cloud_region));

/*
 *  Fill map[row,col] with locations.
 */

	clear = 0;
	for (base = 0; base < 400 - SZ; base += 20)
	{
		n = 10000 + base * 100;
		if (bx[n] == NULL)
		{
			clear = 1;
			for (r = 0; clear && r <= SZ; r++)
				for (c = 0; clear && c <= SZ; c++)
				{
					n = 10000 + (base + r) * 100 + c;
					if (bx[n] != NULL)
						clear = 0;
				}
			break;
		}
	}
	for (r = 0; r <= SZ; r++)
	{
		for (c = 0; c <= SZ; c++)
		{
			if (clear)
			{
				n = 10000 + (base + r) * 100 + c;
				alloc_box(n, T_loc, sub_cloud);
			}
			else
			{
				n = new_ent(T_loc, sub_cloud);
			}
			map[r][c] = n;
			set_name(n, "Cloud");
			set_where(n, cloud_region);
		}
	}

/*
 *  Set the NSEW exit routes for every map location
 */

	for (r = 0; r <= SZ; r++)
	{
		for (c = 0; c <= SZ; c++)
		{
			p = p_loc(map[r][c]);

			if (r == 0)
				north = 0;
			else
				north = map[r-1][c];

			if (r == SZ)
				south = 0;
			else
				south = map[r+1][c];

			if (c == SZ)
				east = 0;
			else
				east = map[r][c+1];

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

	{
		int nimbus, aerovia, stratos;

		nimbus = new_ent(T_loc, sub_city);
		set_where(nimbus, map[1][1]);
		set_name(nimbus, "Nimbus");
		seed_city(nimbus);

		aerovia = new_ent(T_loc, sub_city);
		set_where(aerovia, map[2][3]);
		set_name(aerovia, "Aerovia");
		seed_city(aerovia);

		stratos = new_ent(T_loc, sub_city);
		set_where(stratos, map[3][0]);
		set_name(stratos, "Stratos");
		seed_city(stratos);
	}

/*
 *  Create gates to rings of stones at the four corners of the Cloudlands
 */

	{
		ilist l = NULL;
		int i;
		int gate1, gate2, gate3, gate4;

		loop_loc(i)
		{
			if (subkind(i) == sub_stone_cir)
				ilist_append(&l, i);
		}
		next_loc;

		assert(ilist_len(l) >= 4);
		ilist_scramble(l);

		gate1 = new_ent(T_gate, 0);
		set_where(gate1, map[0][0]);
		p_gate(gate1)->to_loc = l[0];
		rp_gate(gate1)->seal_key = rnd(111,999);

		gate2 = new_ent(T_gate, 0);
		set_where(gate2, map[SZ][0]);
		p_gate(gate2)->to_loc = l[1];
		rp_gate(gate2)->seal_key = rnd(111,999);

		gate3 = new_ent(T_gate, 0);
		set_where(gate3, map[0][SZ]);
		p_gate(gate3)->to_loc = l[2];
		rp_gate(gate3)->seal_key = rnd(111,999);

		gate4 = new_ent(T_gate, 0);
		set_where(gate4, map[SZ][SZ]);
		p_gate(gate4)->to_loc = l[3];
		rp_gate(gate4)->seal_key = rnd(111,999);

		ilist_reclaim(&l);
	}

	printf("Aerovia is in %s\n", box_name(map[2][3]));

/*
 *  Link a cloud to a Mt. Olympus below
 */

	{
		int i;
		int olympus = 0;
		struct entity_loc *p;

		loop_mountain(i)
		{
			if (strcmp(name(i), "Mt. Olympus") == 0)
			{
				olympus = i;
				break;
			}
		}
		next_mountain;

		if (olympus == 0)
		{
			fprintf(stderr,
				"ERROR: Can't find mountain 'Mt. Olympus'\n");
			return;
		}

		p = p_loc(map[2][1]);
		while (ilist_len(p->prov_dest) < DIR_DOWN)
			ilist_append(&p->prov_dest, 0);
		p->prov_dest[DIR_DOWN-1] = olympus;

		p = p_loc(olympus);
		while (ilist_len(p->prov_dest) < DIR_UP)
			ilist_append(&p->prov_dest, 0);
		p->prov_dest[DIR_UP-1] = map[2][1];
	}
}

