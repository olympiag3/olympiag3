
#include	<stdio.h>
#include	"z.h"
#include	"oly.h"


int tunnel_region = 0;
int under_region = 0;


#define	SUB_SZ	10		/* SZ x SZ is size of Subworld */

static void
create_subworld()
{
	int r, c;
	int map[SUB_SZ+1][SUB_SZ+1];
	int n;
	int north, east, south, west;
	struct entity_loc *p;

/*
 *  Create region wrapper
 */

	under_region = new_ent(T_loc, sub_region);
	set_name(under_region, "Subworld");

	fprintf(stderr, "INIT: creating %s\n", box_name(under_region));

/*
 *  Fill map[row,col] with locations.
 */

	for (r = 0; r <= SUB_SZ; r++)
	{
		for (c = 0; c <= SUB_SZ; c++)
		{
			n = new_ent(T_loc, sub_forest);
			set_name(n, "Subworld");

			map[r][c] = n;
			set_where(n, under_region);

			if (rnd(1,3) == 1)
			{
				int new;

				new = new_ent(T_loc, sub_cave);
				set_where(new, map[r][c]);
				p_loc(new)->hidden = TRUE;
			}

			if (rnd(1,3) == 1)
			{
				int new;

				new = new_ent(T_loc, sub_rocky_hill);
				set_where(new, map[r][c]);
			}
		}
	}

/*
 *  Set the NSEW exit routes for every map location
 */

	for (r = 0; r <= SUB_SZ; r++)
	{
		for (c = 0; c <= SUB_SZ; c++)
		{
			p = p_loc(map[r][c]);

			if (r == 0)
				north = 0;
			else
				north = map[r-1][c];

			if (r == SUB_SZ)
				south = 0;
			else
				south = map[r+1][c];

			if (c == SUB_SZ)
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
}



int
random_subworld_loc()
{
	ilist l = NULL;
	int i;
	int ret;

	loop_loc(i)
	{
		if (region(i) != under_region)
			continue;
		if (subkind(i) != sub_forest)
			continue;

		ilist_append(&l, i);
	}
	next_loc;

	assert(ilist_len(l) > 0);

	ret = l[rnd(0,ilist_len(l)-1)];

	ilist_reclaim(&l);
	return ret;
}


#define		SZ		7	/* 5 x 5 */
#define		MAX_LEVELS	25



static int tun_total_locs;


void
print_map(int map[SZ+2][SZ+2][MAX_LEVELS], int l)
{
	int r, c;

	printf("level %d", l);

	for (r = 0; r < SZ+2; r++) {
		for (c = 0; c < SZ+2; c++) {
			if (map[r][c][l])
				printf("X");
			else
				printf("-");
		}
		printf("\n");
	}
	printf("\n");
}


void
fill_dir_exits(int where)
{
	struct entity_loc *p;

	p = p_loc(where);

	while (ilist_len(p->prov_dest) < 6)
		ilist_append(&p->prov_dest, 0);
}

static int
new_tunnel()
{
	int n;

	n = new_ent(T_loc, sub_tunnel);
	set_where(n, tunnel_region);
	tun_total_locs++;

	fill_dir_exits(n);

	return n;
}


static void
tun_links(int map[SZ+2][SZ+2][MAX_LEVELS], int r, int c, int l)
{

	if (map[r+1][c][l])
	{
		p_loc(map[r][c][l])->prov_dest[DIR_S-1] = map[r+1][c][l];
		p_loc(map[r+1][c][l])->prov_dest[DIR_N-1] = map[r][c][l];
	}

	if (map[r-1][c][l])
	{
		p_loc(map[r][c][l])->prov_dest[DIR_N-1] = map[r-1][c][l];
		p_loc(map[r-1][c][l])->prov_dest[DIR_S-1] = map[r][c][l];
	}

	if (map[r][c+1][l])
	{
		p_loc(map[r][c][l])->prov_dest[DIR_E-1] = map[r][c+1][l];
		p_loc(map[r][c+1][l])->prov_dest[DIR_W-1] = map[r][c][l];
	}

	if (map[r][c-1][l])
	{
		p_loc(map[r][c][l])->prov_dest[DIR_W-1] = map[r][c-1][l];
		p_loc(map[r][c-1][l])->prov_dest[DIR_E-1] = map[r][c][l];
	}
}


static int
filled_locs(int map[SZ+2][SZ+2][MAX_LEVELS], int l, int dir)
{
	int r, c;
	ilist sq = NULL;
	int square;

	for (r = 1; r <= SZ; r++)
		for (c = 1; c <= SZ; c++)
			if (map[r][c][l])
			{
				struct entity_loc *p;

				p = p_loc(map[r][c][l]);

				assert(ilist_len(p->prov_dest) >= 6);

				if (dir == 0 || p->prov_dest[dir-1] == 0)
					ilist_append(&sq, map[r][c][l]);
			}

	assert(ilist_len(sq) > 0);

	ilist_scramble(sq);
	square = sq[0];

	ilist_reclaim(&sq);

	return square;
}


static int
fill_out_level(int map[SZ+2][SZ+2][MAX_LEVELS], int l)
{
	int r, c;
	int n;
	int sum;

	r = rnd(1, SZ);
	c = rnd(1, SZ);

	sum = 0;
	if (map[r+1][c][l])
		sum++;
	if (map[r-1][c][l])
		sum++;
	if (map[r][c+1][l])
		sum++;
	if (map[r][c-1][l])
		sum++;

	if (map[r][c][l] == 0 && sum == 1)
	{
		n = new_tunnel();
		map[r][c][l] = n;

		tun_links(map, r, c, l);

		return 1;
	}

	return 0;
}


static int
add_chamber(int map[SZ+2][SZ+2][MAX_LEVELS], int l)
{
	int dir = rnd(1,4);
	int new;
	int square;

	square = filled_locs(map, l, dir);

	new = new_ent(T_loc, sub_chamber);
	p_loc(new)->hidden = TRUE;
	p_subloc(new)->tunnel_level = l; 
	set_where(new, tunnel_region);
	tun_total_locs++;

	fill_dir_exits(new);

	p_loc(square)->prov_dest[dir-1] = new;
	p_loc(new)->prov_dest[exit_opposite[dir]-1] = square;

	fprintf(stderr, "tunnel chamber accessible from %s\n", box_code_less(square));

	return 0;
}


static int subworld_city;

int
create_tunnel_set(int city, int subworld_link)
{
	int map[SZ+2][SZ+2][MAX_LEVELS];
	int r, c, l;
	int n;
	int sewer;
	int level_size;
	int count;
	int nlevels;
	int ret = 0;
	int clev1, clev2;

	tun_total_locs = 0;

/*
 *  Start with clear map 
 */
	for (r = 0; r < SZ+2; r++)
		for (c = 0; c < SZ+2; c++)
			for (l = 0; l < MAX_LEVELS; l++)
				map[r][c][l] = 0;

/*
 *  Create first loc
 */

	r = rnd(1, SZ);
	c = rnd(1, SZ);
	n = new_tunnel();
	map[r][c][0] = n;

/*
 *  Link this loc to a hidden sewer in the city
 */

	sewer = new_ent(T_loc, sub_sewer);
	p_loc(sewer)->hidden = TRUE;
	set_where(sewer, city);

	fill_dir_exits(sewer);

	p_loc(sewer)->prov_dest[DIR_DOWN-1] = n;
	p_loc(n)->prov_dest[DIR_UP-1] = sewer;

	level_size = rnd(3, 12);

	count = 0;
	while (level_size > 0 && count++ < 500)
		level_size -= fill_out_level(map, 0);

/*
 *  Drop a down from a random loc to the next level
 */

	if (safe_haven(city) || subworld_link)
		nlevels = 11;
	else
		nlevels = rnd(2,5);
	l = 0;

	clev1 = rnd(1,6);
	do {
		clev2 = rnd(1,6);
	} while (clev1 == clev2);

	do {
		do {
			r = rnd(1,SZ);
			c = rnd(1,SZ);
		}
		while (map[r][c][l] == 0);

		l++;

		n = new_tunnel();
		p_loc(n)->hidden = TRUE;
		map[r][c][l] = n;

		p_loc(map[r][c][l])->prov_dest[DIR_UP-1] = map[r][c][l-1];
		p_loc(map[r][c][l-1])->prov_dest[DIR_DOWN-1] = map[r][c][l];

		if (l > 5)
			level_size = rnd(1, 4);
		else
			level_size = rnd(3, 12);

		count = 0;
		while (level_size > 0 && count++ < 500)
			level_size -= fill_out_level(map, l);

		if (l == clev1 || l == clev2)
			add_chamber(map, l);
	}
	while (l < nlevels);

	if (l == 5 && rnd(1,2) == 1)
	{
		int hades;
		int square;

		hades = random_hades_loc();
		square = filled_locs(map, l, 0);

		p_loc(square)->hidden = TRUE;	/* hide view from Hades */
		p_loc(square)->prov_dest[DIR_DOWN-1] = hades;
		fill_dir_exits(hades);
		p_loc(hades)->prov_dest[DIR_UP-1] = square;
		printf("(hades from sewer at %s)\n",
			box_code_less(square));
	}

	if (safe_haven(city))
	{
		ret = filled_locs(map, l, DIR_W);

		subworld_city = new_ent(T_loc, sub_city);
		set_where(subworld_city, random_subworld_loc());
	}

	if (subworld_link)
	{
		int square;

		printf("creating subworld link for city %s, link loc %s\n",
				box_code_less(subworld_city), box_code_less(subworld_link));

		assert(safe_haven(city) == FALSE);

		square = filled_locs(map, l, DIR_E);

		p_loc(square)->prov_dest[DIR_E-1] = subworld_link;
		p_loc(subworld_link)->prov_dest[DIR_W-1] = square;
	}

#if 0
	fprintf(stderr, "%s tunnels: %d levels, %d locs\n",
				box_name(city), nlevels+1, tun_total_locs);
#endif
	print_dot('.');

	return ret;
}


void
create_tunnels()
{
	int city;
	int sum = 0;
	int link;

	tunnel_region = new_ent(T_loc, sub_region);
	set_name(tunnel_region, "Undercity");

	create_subworld();

	fprintf(stderr, "INIT: creating %s\n", box_name(tunnel_region));

	loop_city(city)
	{
		if (greater_region(city) != 0)
			continue;

		if (region(city) == cloud_region)
			continue;

		if (safe_haven(city) || rnd(1,2) == 1)
		{
			link = create_tunnel_set(city, 0);
			sum += tun_total_locs;

			if (link)
			{
				create_tunnel_set(subworld_city, link);
				sum += tun_total_locs;
			}
		}
	}
	next_city;

	fprintf(stderr, "\n%d total tunnel locs\n", sum);
}

