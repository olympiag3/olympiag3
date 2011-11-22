
/*
 *  Olympia map generator
 *
 *  Abandon all hope, ye who enter here.
 */


#include	<stdio.h>
#include	"z.h"



#define	linehash(t)	(strlen(t) < 2 ? 0 : ((t[0]) << 8 | (t[1])))


#define		LEN		256

#define		TRUE		1
#define		FALSE		0

#define		MAX_ROW		100
#define		MAX_COL		100


#define		INSIDE_OFF	10000	/* where to start allocating regions */
#define		SUBLOC_OFF	20500	/* where to start numbering sublocs */

#if 0
#define		ROAD_OFF	7000	/* where to start numbering roads */
#define		GATE_OFF	8000	/* where to start numbering gates */
#endif

#define		terr_land	1
#define		terr_ocean	2
#define		terr_forest	3
#define		terr_swamp	4
#define		terr_mountain	5
#define		terr_plain	6
#define		terr_desert	7
#define		terr_water	8
#define		terr_island	9
#define		terr_stone_cir	10	/* circle of stones */
#define		terr_grove	11	/* mallorn grove */
#define		terr_bog	12
#define		terr_cave	13
#define		terr_city	14
#define		terr_guild	15
#define		terr_grave	16
#define		terr_ruins	17
#define		terr_battlefield	18
#define		terr_ench_for	19	/* enchanted forest */
#define		terr_rocky_hill	20
#define		terr_tree_cir	21
#define		terr_pits	22
#define		terr_pasture	23
#define		terr_oasis	24
#define		terr_yew_grove	25
#define		terr_sand_pit	26
#define		terr_sac_grove	27	/* sacred grove */
#define		terr_pop_field	28	/* poppy field */
#define		terr_temple	29
#define		terr_lair	30	/* dragon lair */

#define		FIRST_SKILL	120

#define		skill_ship	120
#define		skill_combat	121
#define		skill_stealth	122
#define		skill_beast	123
#define		skill_pers	124
#define		skill_const	125
#define		skill_alchemy	126

#define		skill_basic	160
#define		skill_weather	161
#define		skill_illusion	162
#define		skill_gate	163
#define		skill_art	164
#define		skill_necro	165

int skill_count[100];


#define		DIR_N		1
#define		DIR_NE		2
#define		DIR_E		3
#define		DIR_SE		4
#define		DIR_S		5
#define		DIR_SW		6
#define		DIR_W		7
#define		DIR_NW		8

#define		MAX_DIR		9

int dir_vector[MAX_DIR];

char *terr_s[] = {
	"<null>",
	"land",
	"ocean",
	"forest",
	"swamp",
	"mountain",
	"plain",
	"desert",
	"water",
	"island",
	"ring of stones",
	"mallorn grove",
	"bog",
	"cave",
	"city",
	"guild",
	"graveyard",
	"ruins",
	"field",
	"enchanted forest",
	"rocky hill",
	"circle of trees",
	"pits",
	"pasture",
	"oasis",
	"yew grove",
	"sand pit",
	"sacred grove",
	"poppy field",
	"temple",
	"lair",
	NULL
};

struct road {
	int ent_num;
	char *name;
	int to_loc;
	int hidden;
};

struct tile {
	int region;
	char *name;
	int terrain;
	int hidden;
	int city;
	int safe_haven;
	int mark;
	int inside;
	int color;			/* map coloring for ocean */
	int row, col;			/* map tile we're inside */
	int depth;

	ilist subs;

	ilist gates_dest;		/* gates from here */
	ilist gates_num;		/* gates from here */
	ilist gates_key;

	int label_code;

	ilist teaches;			/* skills location teaches */
	int skill_assign;		/* have skills been given yet */

	struct road **roads;
};


#define	MAX_INSIDE	200		/* max continents/regions */

char *inside_names[MAX_INSIDE];
int inside_code[MAX_INSIDE];
int inside_top = 0;

int max_col = 0;
int max_row = 0;

int water_count = 0;			/* count of water provinces */
int land_count = 0;			/* count of land provinces */
int num_islands = 0;

int subloc_counter = SUBLOC_OFF;

#if 1
#define		road_counter	subloc_counter
#define		gate_counter	subloc_counter
#else
int road_counter = ROAD_OFF;
int gate_counter = GATE_OFF;
#endif

struct tile *map[MAX_ROW][MAX_COL];
char amap[MAX_ROW][MAX_COL];

struct tile *adjacent_tile_terr();
struct tile *adjacent_tile_water();
struct tile *adjacent_tile_sup();
char *fetch_inside_name();

#define		MAX_SUBLOC	20000

struct tile *subloc[MAX_SUBLOC];
int top_subloc = 0;


FILE *loc_fp;
FILE *gate_fp;
FILE *road_fp;

main()
{

	dir_assert();
	open_fps();
	init_random();
	map_init();
	read_map();
	fix_terrain();
	flood_inside();
	make_islands();
	make_special_cities();
	place_sublocations();
	make_gates();
	make_roads();
	assign_skills();
	dump_continents();
	print_map();
	print_sublocs();
	count_continents();
	count_sublocs();
	count_subloc_coverage();
	dump_roads();
	dump_gates();

	fclose(loc_fp);
	fclose(gate_fp);
	fclose(road_fp);

	count_tiles();

	fprintf(stderr, "\n");
	fprintf(stderr, "highest province = %d\n",
				map[max_row][max_col]->region);
	fprintf(stderr, "gate_counter = %d\n", gate_counter);
	fprintf(stderr, "subloc_counter = %d\n", subloc_counter);
	fprintf(stderr, "road_counter = %d\n", road_counter);
	fprintf(stderr, "\n");

/*
 *  If the province allocation spilled into the subloc range,
 *  we have to increase SUBLOC_MAX
 */

	assert(SUBLOC_OFF > map[max_row][max_col]->region);

	ascii_map();
}


open_fps()
{

	loc_fp = fopen("loc", "w");
	if (loc_fp == NULL)
	{
		perror("can't write loc");
		exit(1);
	}

	gate_fp = fopen("gate", "w");
	if (gate_fp == NULL)
	{
		perror("can't write gate");
		exit(1);
	}

	road_fp = fopen("road", "w");
	if (road_fp == NULL)
	{
		perror("can't write road");
		exit(1);
	}
}


map_init()
{
	int i, j;

	for (i = 0; i < MAX_ROW; i++)
		for (j = 0; j < MAX_COL; j++)
			map[i][j] = NULL;

	for (i = 0; i < 100; i++)
		skill_count[i] = 0;
}


read_map()
{
	char buf[LEN];
	int row, col;
	int terrain;
	int skipnext = FALSE;
	int color;
	int i;
	FILE *fp;

	fp = fopen("Map", "r");
	if (fp == NULL)
	{
		perror("can't open Map");
		exit(1);
	}

	row = 0;

	while (fgets(buf, LEN, fp) != NULL)
	{
		for (col = 0; buf[col] && buf[col] != '\n'; col++)
		{
			if (buf[col] == '#')
				continue;

			if (row > max_row)
				max_row = row;

			if (col > max_col)
				max_col = col;

			map[row][col] = my_malloc(sizeof(struct tile));
			map[row][col]->row = row;
			map[row][col]->col = col;
			map[row][col]->region = rc_to_region(row+1, col+1);
			map[row][col]->depth = 2;

			color = 0;

			switch (buf[col])
			{
			case '*':
				terrain = terr_land;
				map[row][col]->city = TRUE;
				map[row][col]->safe_haven = TRUE;
				break;

			case '~':
				terrain = terr_ocean;
				color = 1;
				break;

			case '.':
				terrain = terr_ocean;
				color = 2;
				break;

			case ' ':
				terrain = terr_ocean;
				color = 3;
				break;

			case ',':
				terrain = terr_ocean;
				color = 4;
				break;

			case 'o':
				terrain = terr_plain;
				break;

			case ';':
			case '%':
				terrain = terr_desert;
				break;

			case '^':
			case '@':
				terrain = terr_mountain;
				break;

			case ':':
			case '!':
				terrain = terr_swamp;
				break;

			case '&':
				terrain = terr_forest;
				break;

			case '?':
				map[row][col]->hidden = TRUE;
				terrain = terr_land;
				break;

			default:
				if (!isalpha(buf[col]))
					fprintf(stderr, "unknown terrain %c\n",
							buf[col]);

				if (buf[col] >= 'a' && buf[col] <= 'z')
					terrain = terr_water;
				else
					terrain = terr_land;
			}

			map[row][col]->terrain = terrain;
			map[row][col]->color = color;

			if (terrain == terr_water || terrain == terr_ocean)
				water_count++;
			else
				land_count++;

/*
 *  A letter or number in a continent is an index into the Regions
 *  file for a name for that contininent.  The inside field for every
 *  region in that continent should point to a region named for
 *  the continent.
 *
 *  Lowercase pairs ("aa") designate ocean names, uppercase pairs ("GY")
 *  designate continent names.
 */

			if (isalpha(buf[col]) && buf[col] != 'o' && !skipnext)
			{
				i = alloc_inside((buf[col] << 8) | buf[col+1]);

				map[row][col]->inside = i;
				map[row][col]->label_code =
						(buf[col] << 8) + buf[col+1];

				inside_names[i] =
				fetch_inside_name((buf[col] << 8) | buf[col+1]);
				skipnext = TRUE;
			} else
				skipnext = FALSE;

		}

		row++;
	}

	fclose(fp);
}


add_road(from, to_loc, hidden, name)
struct tile *from;
int to_loc;
int hidden;
char *name;
{
	struct road *r;

	r = my_malloc(sizeof(*r));
	r->ent_num = ++road_counter;
	r->to_loc = to_loc;
	r->hidden = hidden;
	r->name = name;

	ilist_append((ilist *) &from->roads, (int) r);
}


link_roads(from, to, hidden, name)
struct tile *from;
struct tile *to;
int hidden;
char *name;
{
	int i;
	int n = 0;

/*
 *  If there is a sublocation at an endpoint of the secret road,
 *  move the road to come from the sublocation instead of the province.
 *
 *  Since only 1/3 of the locations have sublocs, this doesn't happen
 *  all the time.  A very few locations will have the route between
 *  two sublocs
 */

	for (i = 1; i <= top_subloc; i++)
		if (subloc[i]->inside == from->region &&
		    subloc[i]->terrain != terr_city)
		{
			n = i;
			break;
		}

	if (n)
	{
		from = subloc[n];
	}

	n = 0;
	for (i = 1; i <= top_subloc; i++)
		if (subloc[i]->inside == to->region &&
		    subloc[i]->terrain != terr_city)
		{
			n = i;
			break;
		}

	if (n)
	{
		to = subloc[n];
	}

	add_road(from, to->region, hidden, name);
	add_road(to, from->region, hidden, name);
}


alloc_inside(c)
int c;
{
	int i;

	for (i = 1; i <= inside_top; i++)
		if (inside_code[i] == c)
			return i;

	inside_top++;
	assert(inside_top < MAX_INSIDE);

	inside_code[inside_top] = c;

	return inside_top;
}


dump_continents()
{
	int i;

	for (i = 1; i <= inside_top; i++)
	{
		fprintf(loc_fp, "%d loc\n", INSIDE_OFF + i);
		if (inside_names[i])
			fprintf(loc_fp, "na %s\n", inside_names[i]);
		print_inside_locs(i);
		fprintf(loc_fp, "LO\n");
		fprintf(loc_fp, " ld 1\n");
		fprintf(loc_fp, "\n");
	}
}


char *
fetch_inside_name(l)
int l;
{
	FILE *fp;
	char *line;
	char *ret = NULL;
	static int dont_call = FALSE;

	if (dont_call)
		return NULL;

	fp = fopen("Regions", "r");
	if (fp == NULL)
	{
		perror("can't open Regions");
		dont_call = TRUE;
		return NULL;
	}

	while (line = getlin_ew(fp))
	{
		if (linehash(line) == l)
		{
			line += 2;
			while (*line && iswhite(*line))
				line++;

			ret = str_save(line);
			break;
		}
	}

	fclose(fp);
	return ret;
}


fix_terrain()
{
	int row, col;
	struct tile *p;

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		if (map[row][col] && map[row][col]->terrain == terr_land)
		{
			p = adjacent_tile_terr(row, col);
			if (p &&
			    p->terrain != terr_land &&
			    p->terrain != terr_ocean)
				map[row][col]->terrain = p->terrain;
			else
			{
	fprintf(stderr, "fix_terrain: could not infer type of (%d,%d)\n",
							row, col);
	fprintf(stderr, "    assuming 'forest'\n");
				map[row][col]->terrain = terr_forest;
			}
		}

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		if (map[row][col] && map[row][col]->terrain == terr_water)
		{
			map[row][col]->terrain = terr_ocean;

			p = adjacent_tile_water(row, col);
			if (p && p->terrain == terr_ocean && p->color)
				map[row][col]->color = p->color;
			else
			{
	fprintf(stderr, "fix_terrain: could not infer color of (%d,%d)\n",
							row, col);
			}
		}

}


print_map()
{
	int row, col;
	int flag;

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		if (map[row][col])
		{
			flag = TRUE;

			fprintf(loc_fp, "%d loc\n", map[row][col]->region);
			if (map[row][col]->name)
				fprintf(loc_fp, "na %s\n",
						map[row][col]->name);

			fprintf(loc_fp, "sk %s\n",
					terr_s[map[row][col]->terrain]);

			if (map[row][col]->inside)
			{
				fprintf(loc_fp, "LI\n");
				flag = FALSE;

				if (map[row][col]->inside)
					fprintf(loc_fp, " wh %d\n",
					map[row][col]->inside + INSIDE_OFF);
			}

			print_inside_sublocs(flag, row, col);

			fprintf(loc_fp, "LO\n");

			if (map[row][col]->hidden)
				fprintf(loc_fp, " hi %d\n",
							map[row][col]->hidden);

			fprintf(loc_fp, " ld %d\n", map[row][col]->depth);
			print_teaches(map[row][col]->teaches);
			fprintf(loc_fp, "\n");
		}
}


print_sublocs()
{
	int i;
	int sl;

	for (i = 1; i <= top_subloc; i++)
	{
		fprintf(loc_fp, "%d loc\n", subloc[i]->region);
		if (subloc[i]->name)
			fprintf(loc_fp, "na %s\n", subloc[i]->name);

		fprintf(loc_fp, "sk %s\n", terr_s[subloc[i]->terrain]);

		assert(subloc[i]->inside);
		fprintf(loc_fp, "LI\n");
		fprintf(loc_fp, " wh %d\n", subloc[i]->inside);
		print_subloc_gates(i);

		fprintf(loc_fp, "LO\n");

		if (subloc[i]->hidden)
			fprintf(loc_fp, " hi %d\n", subloc[i]->hidden);

		fprintf(loc_fp, " ld %d\n", subloc[i]->depth);

		sl = print_teaches(subloc[i]->teaches);

		if (subloc[i]->safe_haven)
		{
			if (!sl)
				fprintf(loc_fp, "SL\n");
			fprintf(loc_fp, " sh 1\n");
		}

		fprintf(loc_fp, "\n");
	}
}


int
teach_comp(a, b)
int *a;
int *b;
{

	return *a - *b;
}


print_teaches(l)
ilist l;
{
	int i;
	int count = 0;

	if (ilist_len(l) > 0)
		qsort(l, ilist_len(l), sizeof(int), teach_comp);

	for (i = 0; i < ilist_len(l); i++)
	{
		count++;
		if (count == 1)
		{
			fprintf(loc_fp, "SL\n");
			fprintf(loc_fp, " te ");
		}

		if (count % 11 == 10)		/* continuation line */
			fprintf(loc_fp, "\\\n\t");

		fprintf(loc_fp, "%d ", l[i]);
	}

	if (count)
		fprintf(loc_fp, "\n");

	return count;
}


print_subloc_gates(n)		/* and inside buildings... */
int n;
{
	int i;
	int count = 0;

	for (i = 0; i < ilist_len(subloc[n]->roads); i++)
	{
		count++;
		if (count == 1)
		{
			fprintf(loc_fp, " hl ");
		}

		if (count % 11 == 10)		/* continuation line */
			fprintf(loc_fp, "\\\n\t");

		fprintf(loc_fp, "%d ", subloc[n]->roads[i]->ent_num);
	}

	for (i = 0; i < ilist_len(subloc[n]->gates_num); i++)
	{
		count++;
		if (count == 1)
		{
			fprintf(loc_fp, " hl ");
		}

		if (count % 11 == 10)		/* continuation line */
			fprintf(loc_fp, "\\\n\t");

		fprintf(loc_fp, "%d ", subloc[n]->gates_num[i]);
	}

	for (i = 0; i < ilist_len(subloc[n]->subs); i++)
	{
		count++;
		if (count == 1)
		{
			fprintf(loc_fp, " hl ");
		}

		if (count % 11 == 10)		/* continuation line */
			fprintf(loc_fp, "\\\n\t");

		fprintf(loc_fp, "%d ", subloc[n]->subs[i]);
	}

	if (count)
		fprintf(loc_fp, "\n");
}


/*
 *  The entity number of a region determines where it is on the map.
 *
 *  Here is how:
 *
 *	  (r,c)
 *	--------------------
 *	| (1,1)		(1,99)
 *	|
 *	|
 *	| (n,1)		(n,99)
 *
 *
 *  Entity 10101 corresponds to (1,1).  [10199] corresponds to (1,99).
 *
 *  Note that for player convenience an alternate method of representing
 *  location entity numbers may be used, i.e. 'aa'-> 101, 'ab' -> 102,
 *  so [aa01] -> [10101], [ab53] -> [10253].
 */

int
region_row(int where)
{
	int row;

	row = (where / 100) % 100;

	return row;
}


int
region_col(int where)
{
	int col;

	col = where % 100;

	return col;
}


int
rc_to_region(int row, int col)
{
	int reg;

	assert(row > 0 && row < 100);
	assert(col > 0 && col < 100);

	reg = 10000 + (row * 100) + col;

	return reg;
}


dir_assert()
{
	int row, col, reg;

	row = 1;
	col = 1;
	reg = rc_to_region(row, col);
	assert(reg == 10101);
	assert(region_row(reg) == 1);
	assert(region_col(reg) == 1);

	row = 99;
	col = 99;
	reg = rc_to_region(row, col);
	assert(reg == 19999);
	assert(region_row(reg) == 99);
	assert(region_col(reg) == 99);

	row = 57;
	col = 63;
	reg = rc_to_region(row, col);
	assert(reg == 15763);
	assert(region_row(reg) == 57);
	assert(region_col(reg) == 63);
}


struct tile *
adjacent_tile_water(row, col)
int row;
int col;
{
	struct tile *p;

	randomize_dir_vector();

	p = adjacent_tile_sup(row, col, dir_vector[1]);

	if (!p || p->terrain != terr_ocean)
		p = adjacent_tile_sup(row, col, dir_vector[2]);
	if (!p || p->terrain != terr_ocean)
		p = adjacent_tile_sup(row, col, dir_vector[3]);
	if (!p || p->terrain != terr_ocean)
		p = adjacent_tile_sup(row, col, dir_vector[4]);
	if (!p || p->terrain != terr_ocean)
		p = adjacent_tile_sup(row, col, dir_vector[5]);
	if (!p || p->terrain != terr_ocean)
		p = adjacent_tile_sup(row, col, dir_vector[6]);
	if (!p || p->terrain != terr_ocean)
		p = adjacent_tile_sup(row, col, dir_vector[7]);
	if (!p || p->terrain != terr_ocean)
		p = adjacent_tile_sup(row, col, dir_vector[8]);

	return p;
}


struct tile *
adjacent_tile_terr(row, col)
int row;
int col;
{
	struct tile *p;

	randomize_dir_vector();

	p = adjacent_tile_sup(row, col, dir_vector[1]);
	if (!p || p->terrain == terr_land || p->terrain == terr_ocean)
		p = adjacent_tile_sup(row, col, dir_vector[2]);
	if (!p || p->terrain == terr_land || p->terrain == terr_ocean)
		p = adjacent_tile_sup(row, col, dir_vector[3]);
	if (!p || p->terrain == terr_land || p->terrain == terr_ocean)
		p = adjacent_tile_sup(row, col, dir_vector[4]);
	if (!p || p->terrain == terr_land || p->terrain == terr_ocean)
		p = adjacent_tile_sup(row, col, dir_vector[5]);
	if (!p || p->terrain == terr_land || p->terrain == terr_ocean)
		p = adjacent_tile_sup(row, col, dir_vector[6]);
	if (!p || p->terrain == terr_land || p->terrain == terr_ocean)
		p = adjacent_tile_sup(row, col, dir_vector[7]);
	if (!p || p->terrain == terr_land || p->terrain == terr_ocean)
		p = adjacent_tile_sup(row, col, dir_vector[8]);

	return p;
}


/*
 *  Return the region immediately adjacent to <location>
 *  in direction <dir>
 *
 *  Returns 0 if there is no adjacent location in the given
 *  direction.
 */

struct tile *
adjacent_tile_sup(row, col, dir)
int row;
int col;
int dir;
{

	switch (dir)
	{
	case DIR_N:
		row--;
		break;

	case DIR_NE:
		row--;
		col++;
		break;

	case DIR_E:
		col++;
		break;

	case DIR_SE:
		row++;
		col++;
		break;

	case DIR_S:
		row++;
		break;

	case DIR_SW:
		row++;
		col--;
		break;

	case DIR_W:
		col--;
		break;

	case DIR_NW:
		row--;
		col--;
		break;

	default:
		fprintf(stderr, "location_direction: bad dir %d\n", dir);
		assert(FALSE);
	}

	if (row < 0 || row > 99 || col < 0 || col > 99)
		return NULL;	/* off the map */

	return map[row][col];
}


is_port_city(row, col)
int row;
int col;
{
	struct tile *n, *s, *e, *w;

	n = adjacent_tile_sup(row, col, DIR_N);
	s = adjacent_tile_sup(row, col, DIR_S);
	e = adjacent_tile_sup(row, col, DIR_E);
	w = adjacent_tile_sup(row, col, DIR_W);

	if (n && n->terrain == terr_ocean ||
	    s && s->terrain == terr_ocean ||
	    e && e->terrain == terr_ocean ||
	    w && w->terrain == terr_ocean)
		return TRUE;

	return FALSE;
}


randomize_dir_vector()
{
	int i;
	int one, two, tmp;

	dir_vector[0] = 0;
	for (i = 1; i < MAX_DIR; i++)
		dir_vector[i] = i;

	for (i = 1; i < 20; i++)
	{
		one = rnd(1, MAX_DIR-1);
		two = rnd(1, MAX_DIR-1);

		tmp = dir_vector[one];
		dir_vector[one] = dir_vector[two];
		dir_vector[two] = tmp;
	}
}


bridge_map_hole_sup(row, col)
int row;
int col;
{
	struct tile *n, *s, *e, *w;
	struct tile *nw, *sw, *ne, *se;
	static ilist l = NULL;
	int i;
	char *name;
	static int road_name_cnt = 0;

	ilist_clear(&l);

/*
 *  Find all squares neighboring the hole
 */

	n = adjacent_tile_sup(row, col, DIR_N);
	s = adjacent_tile_sup(row, col, DIR_S);
	e = adjacent_tile_sup(row, col, DIR_E);
	w = adjacent_tile_sup(row, col, DIR_W);
	nw = adjacent_tile_sup(row, col, DIR_NW);
	sw = adjacent_tile_sup(row, col, DIR_SW);
	ne = adjacent_tile_sup(row, col, DIR_NE);
	se = adjacent_tile_sup(row, col, DIR_SE);

	if (n->mark || s->mark || e->mark || w->mark ||
	    nw->mark || sw->mark || ne->mark || se->mark)
		return FALSE;
/*
 *  For every path across the hole, add it to the list of possibilities
 *  if it's land-land and we haven't already used one of the destination
 *  tiles for another hole-crossing.
 */

	if (n && s &&
	    n->terrain != terr_ocean && s->terrain != terr_ocean &&
	    map[n->row][n->col]->mark + map[s->row][s->col]->mark == 0)
		ilist_append(&l, 1);

	if (e && w &&
	    e->terrain != terr_ocean && w->terrain != terr_ocean &&
	    map[e->row][e->col]->mark + map[w->row][w->col]->mark == 0)
		ilist_append(&l, 2);

	if (ne && sw &&
	    ne->terrain != terr_ocean && sw->terrain != terr_ocean &&
	    map[ne->row][ne->col]->mark + map[sw->row][sw->col]->mark == 0)
		ilist_append(&l, 3);

	if (se && nw &&
	    se->terrain != terr_ocean && nw->terrain != terr_ocean &&
	    map[se->row][se->col]->mark + map[nw->row][nw->col]->mark == 0)
		ilist_append(&l, 4);

	i = ilist_len(l);

	if (i <= 0)
		return FALSE;

	switch (road_name_cnt++ % 3)
	{
	case 0:
		name = "Secret pass";
		break;

	case 1:
		name = "Secret route";
		break;

	case 2:
		name = "Old road";
		break;

	default:
		assert(FALSE);
	}

/*
 *  The horror, the horror
 */

	if (n) n->mark += rnd(0,1);
	if (e) e->mark += rnd(0,1);
	if (w) w->mark += rnd(0,1);
	if (s) s->mark += rnd(0,1);
	if (nw) nw->mark += rnd(0,1);
	if (ne) ne->mark += rnd(0,1);
	if (sw) sw->mark += rnd(0,1);
	if (se) se->mark += rnd(0,1);

	i = rnd(0,i-1);

	switch (l[i])
	{
	case 1:
		link_roads(n, s, 1, name);
		n->mark = 1;
		s->mark = 1;
		break;

	case 2:
		link_roads(e, w, 1, name);
		e->mark = 1;
		w->mark = 1;
		break;

	case 3:
		link_roads(ne, sw, 1, name);
		ne->mark = 1;
		sw->mark = 1;
		break;

	case 4:
		link_roads(se, nw, 1, name);
		se->mark = 1;
		nw->mark = 1;
		break;

	default:
		assert(FALSE);
	}

	return l[i];
}


char *bridge_dir_s[] = {
	"-invalid-",
	"  n-s",
	"  e-w",
	"ne-sw",
	"nw-se"
};


/*
 *  Bridge a # map hole with a secret road
 *
 *  Won't put two roads in the same square
 */

bridge_map_holes()
{
	int row, col;
	int n;

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		if (map[row][col] == NULL)
			if ((n = bridge_map_hole_sup(row, col)))
				fprintf(stderr,
					"%s map hole bridge at (%d,%d)\n",
						bridge_dir_s[n],
							row, col);

	fprintf(stderr, "\n");
}


bridge_corner_sup(row, col)
int row;
int col;
{
	struct tile *n, *s, *e, *w;
	struct tile *nw, *sw, *ne, *se;
	struct tile *from, *to;
	int i;
	char *name;
	static int road_name_cnt = 0;
	static ilist l = NULL;

	ilist_clear(&l);

/*
 *  Find all squares neighboring the hole
 */

	n = adjacent_tile_sup(row, col, DIR_N);
	s = adjacent_tile_sup(row, col, DIR_S);
	e = adjacent_tile_sup(row, col, DIR_E);
	w = adjacent_tile_sup(row, col, DIR_W);
	nw = adjacent_tile_sup(row, col, DIR_NW);
	sw = adjacent_tile_sup(row, col, DIR_SW);
	ne = adjacent_tile_sup(row, col, DIR_NE);
	se = adjacent_tile_sup(row, col, DIR_SE);

	if (n->mark || s->mark || e->mark || w->mark ||
	    nw->mark || sw->mark || ne->mark || se->mark)
		return FALSE;

	switch (road_name_cnt++ % 3)
	{
	case 0:
		name = "Secret pass";
		break;

	case 1:
		name = "Secret route";
		break;

	case 2:
		name = "Old road";
		break;

	default:
		assert(FALSE);
	}

	if (nw && nw->terrain != terr_ocean)
		ilist_append(&l, 1);
	if (ne && ne->terrain != terr_ocean)
		ilist_append(&l, 2);
	if (se && se->terrain != terr_ocean)
		ilist_append(&l, 3);
	if (sw && sw->terrain != terr_ocean)
		ilist_append(&l, 4);

	i = ilist_len(l);

	if (i <= 0)
		return FALSE;

/*
 *  The horror, the horror
 */

	if (n) n->mark += rnd(0,1);
	if (e) e->mark += rnd(0,1);
	if (w) w->mark += rnd(0,1);
	if (s) s->mark += rnd(0,1);
	if (nw) nw->mark += rnd(0,1);
	if (ne) ne->mark += rnd(0,1);
	if (sw) sw->mark += rnd(0,1);
	if (se) se->mark += rnd(0,1);

	i = rnd(0,i-1);

	switch (l[i])
	{
	case 1:
		link_roads(map[row][col], nw, 1, name);

		map[row][col]->mark = 1;
		nw->mark = 1;
		break;

	case 2:
		link_roads(map[row][col], ne, 1, name);

		map[row][col]->mark = 1;
		ne->mark = 1;
		break;

	case 3:
		link_roads(map[row][col], se, 1, name);

		map[row][col]->mark = 1;
		se->mark = 1;
		break;

	case 4:
		link_roads(map[row][col], sw, 1, name);

		map[row][col]->mark = 1;
		sw->mark = 1;
		break;

	default:
		assert(FALSE);
	}

	return l[i];
}



bridge_caddy_corners()
{
	int row, col;

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		if (map[row][col] && map[row][col]->terrain != terr_ocean &&
		    rnd(1,35) == 35)
			bridge_corner_sup(row, col);
}


bridge_mountain_sup(row, col)
int row;
int col;
{
	struct tile *from;
	struct tile *to;
	char *name;

	from = map[row][col];
	to = adjacent_tile_water(row, col);

	assert(to->terrain == terr_ocean);

	switch (rnd(1,3))
	{
	case 1:
		name = "Narrow channel";
		break;

	case 2:
		name = "Rocky channel";
		break;

	case 3:
		name = "Secret sea route";
		break;

	default:
		assert(FALSE);
	}

	add_road(from, to->region, 1, name);
	add_road(to, from->region, 1, name);

	fprintf(stderr, "secret sea route at (%d,%d)\n", from->row, from->col);
}


bridge_mountain_ports()
{
	int row, col;

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		if (map[row][col] &&
		    map[row][col]->terrain == terr_mountain &&
		    is_port_city(row, col) &&
		    rnd(1,7) == 7)
			bridge_mountain_sup(row, col);
}


make_roads()
{

	clear_province_marks();
	bridge_map_holes();
	bridge_caddy_corners();
	bridge_mountain_ports();
}


int cont_count;

count_continents()
{
	int row, col;
	int sum = 0;
	char *name;
	char buf[100];

	clear_province_marks();

	fprintf(stderr, "Land regions:\n\n");

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		if (map[row][col] &&
		    map[row][col]->terrain != terr_ocean &&
		    map[row][col]->mark == FALSE)
		{
			sum++;
			cont_count = 0;
			mark_continent(row, col);

			name = inside_names[map[row][col]->inside];

			if (name == NULL)
			{
				sprintf(buf, "?? (%d,%d)", row, col);
				name = buf;
			}

			fprintf(stderr, "%d\t %s\n", cont_count, name);
		}

	fprintf(stderr, "\n\nOceans:\n\n");

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		if (map[row][col] &&
		    map[row][col]->terrain == terr_ocean &&
		    map[row][col]->mark == FALSE)
		{
			cont_count = 0;
			mark_continent_ocean(row, col);

			name = inside_names[map[row][col]->inside];

			if (name == NULL)
			{
				sprintf(buf, "?? (%d,%d)", row, col);
				name = buf;
			}

			fprintf(stderr, "%d\t %s\n", cont_count, name);
		}

	fprintf(stderr, "\n\n%d continents\n", sum);
	fprintf(stderr, "%d land locs\n", land_count);
	fprintf(stderr, "%d water locs\n", water_count);
}


mark_continent(row, col)
int row, col;
{
	int dir;
	struct tile *p;

	map[row][col]->mark = TRUE;
	cont_count++;

	for (dir = 1; dir < MAX_DIR; dir++)
	{
		p = adjacent_tile_sup(row, col, dir);
		if (p && p->terrain != terr_ocean && p->mark == FALSE)
			mark_continent(p->row, p->col);
	}
}


mark_continent_ocean(row, col)
int row, col;
{
	int dir;
	struct tile *p;

	map[row][col]->mark = TRUE;
	cont_count++;

	for (dir = 1; dir < MAX_DIR; dir++)
	{
		p = adjacent_tile_sup(row, col, dir);
		if (p && p->terrain == terr_ocean && p->mark == FALSE &&
		    p->color == map[row][col]->color)
			mark_continent_ocean(p->row, p->col);
	}
}


flood_inside()
{
	int row, col;
	int kind;

	clear_province_marks();

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		if (map[row][col] &&
		    map[row][col]->inside &&
		    map[row][col]->mark == FALSE)
		{
		    if (map[row][col]->terrain == terr_ocean)
			kind = terr_ocean;
		    else
			kind = terr_land;

		    flood_inside_sup(row, col, map[row][col]->inside,
					kind, map[row][col]->color);
		}
}


flood_inside_sup(row, col, ins, kind, color)
int row, col;
int ins;
int kind;
int color;
{
	int dir;
	struct tile *p;

	map[row][col]->mark = TRUE;
	map[row][col]->inside = ins;

	for (dir = 1; dir < MAX_DIR; dir++)
	{
		p = adjacent_tile_sup(row, col, dir);

		if (!p || p->mark)
			continue;

		if (kind == terr_land && p->terrain == terr_ocean)
			continue;

		if (kind == terr_ocean && p->terrain != terr_ocean)
			continue;

		if (kind == terr_ocean && color != p->color)
			continue;

		flood_inside_sup(p->row, p->col, ins, kind, color);
	}
}


print_inside_locs(n)
int n;
{
	int row, col;
	int count = 0;

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		{
		    if (map[row][col] && map[row][col]->inside == n)
		    {
			count++;
			if (count == 1)
			{
				fprintf(loc_fp, "LI\n");
				fprintf(loc_fp, " hl ");
			}

			if (count % 11 == 10)		/* continuation line */
				fprintf(loc_fp, "\\\n\t");

			fprintf(loc_fp, "%d ", map[row][col]->region);
		    }
		}

	if (count)
		fprintf(loc_fp, "\n");
}


print_inside_sublocs(flag, row, col)
int flag;
int row;
int col;
{
	int i;
	int count = 0;

	for (i = 0; i < ilist_len(map[row][col]->roads); i++)
	{
		count++;
		if (count == 1)
		{
			if (flag)
				fprintf(loc_fp, "LI\n");
			fprintf(loc_fp, " hl ");
		}

		if (count % 11 == 10)		/* continuation line */
			fprintf(loc_fp, "\\\n\t");

		fprintf(loc_fp, "%d ", map[row][col]->roads[i]->ent_num);
	}

	for (i = 0; i < ilist_len(map[row][col]->gates_num); i++)
	{
		count++;
		if (count == 1)
		{
			if (flag)
				fprintf(loc_fp, "LI\n");
			fprintf(loc_fp, " hl ");
		}

		if (count % 11 == 10)		/* continuation line */
			fprintf(loc_fp, "\\\n\t");

		fprintf(loc_fp, "%d ", map[row][col]->gates_num[i]);
	}

	for (i = 0; i < ilist_len(map[row][col]->subs); i++)
	{
		count++;
		if (count == 1)
		{
			if (flag)
				fprintf(loc_fp, "LI\n");
			fprintf(loc_fp, " hl ");
		}

		if (count % 11 == 10)		/* continuation line */
			fprintf(loc_fp, "\\\n\t");

		fprintf(loc_fp, "%d ", map[row][col]->subs[i]);
	}

	if (count)
		fprintf(loc_fp, "\n");
}


make_islands()
{
	int i;
	int row, col;

	num_islands = water_count / 3;

	for (i = 1; i <= num_islands; i++)
	{
		row = rnd(0, max_row);
		col = rnd(0, max_col);

		if (map[row][col] && map[row][col]->terrain == terr_ocean &&
		    island_allowed(row, col))
			create_a_subloc(row, col, rnd(0,1), terr_island);
		else
			i--;
	}
}


island_allowed(row, col)
int row;
int col;
{
	int inside;
	char *p;

	inside = map[row][col]->inside;

	if (inside == 0)
		return TRUE;

	for (p = inside_names[inside]; *p; p++)
		if (strncmp(p, "Deep", 4) == 0)
			return FALSE;

	return TRUE;
}


create_a_subloc(row, col, hidden, kind)
int row;
int col;
int hidden;
int kind;
{

	top_subloc++;
	assert(top_subloc < MAX_SUBLOC);

	subloc[top_subloc] = my_malloc(sizeof(struct tile));
	subloc[top_subloc]->region = ++subloc_counter;
	subloc[top_subloc]->inside = map[row][col]->region;
	subloc[top_subloc]->row = row;
	subloc[top_subloc]->col = col;
	subloc[top_subloc]->hidden = hidden;
	subloc[top_subloc]->terrain = kind;
	subloc[top_subloc]->depth = 3;

	if (kind == terr_city)
		map[row][col]->city = 2;

	ilist_append(&map[row][col]->subs, subloc[top_subloc]->region);

	return top_subloc;
}


create_a_building(sl, hidden, kind)
int sl;
int hidden;
int kind;
{

	top_subloc++;
	assert(top_subloc < MAX_SUBLOC);

	subloc[top_subloc] = my_malloc(sizeof(struct tile));
	subloc[top_subloc]->region = ++subloc_counter;
	subloc[top_subloc]->inside = subloc[sl]->region;

	subloc[top_subloc]->row = subloc[sl]->row;
	subloc[top_subloc]->col = subloc[sl]->col;

	subloc[top_subloc]->hidden = hidden;
	subloc[top_subloc]->terrain = kind;
	subloc[top_subloc]->depth = 4;

	ilist_append(&subloc[sl]->subs, subloc[top_subloc]->region);

	return top_subloc;
}


count_sublocs()
{
	int row, col;
	int i;
	int count[100];

	fprintf(stderr, "\nsubloc counts:\n");

	for (i = 0; i < 100; i++)
		count[i] = 0;

	clear_province_marks();

	for (i = 1; i <= top_subloc; i++)
		if (subloc[i]->terrain == terr_island)
		{
			row = subloc[i]->row;
			col = subloc[i]->col;
			map[row][col]->mark++;
		}

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		if (map[row][col] && map[row][col]->terrain == terr_ocean)
			count[map[row][col]->mark]++;

	for (i = 99; i >= 0; i--)
		if (count[i] != 0)
			break;
		else
			count[i] = -1;

	for (i = 0; i < 100; i++)
	{
		if (count[i] == -1)
			break;

		fprintf(stderr, "%6d %s %d island%s (%d%%)\n",
				count[i],
				count[i] == 1 ? "loc has" : "locs have",
				i, i == 1 ? " " : "s",
				count[i] * 100 / water_count);
	}
}


dump_roads()
{
	int row, col;
	int i, j;

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		if (map[row][col])
		    for (j = 0; j < ilist_len(map[row][col]->roads); j++)
		    {
			fprintf(road_fp, "%d road\n",
					map[row][col]->roads[j]->ent_num);
			if (map[row][col]->roads[j]->name)
				fprintf(road_fp, "na %s\n",
					map[row][col]->roads[j]->name);
			fprintf(road_fp, "LI\n");
			fprintf(road_fp, " wh %d\n", map[row][col]->region);
			fprintf(road_fp, "GA\n");
			fprintf(road_fp, " tl %d\n",
					map[row][col]->roads[j]->to_loc);
			if (map[row][col]->roads[j]->hidden)
				fprintf(road_fp, " rh %d\n",
					map[row][col]->roads[j]->hidden);
			fprintf(road_fp, "\n");
		    }

	for (i = 1; i <= top_subloc; i++)
		for (j = 0; j < ilist_len(subloc[i]->roads); j++)
		{
			fprintf(road_fp, "%d road\n",
					subloc[i]->roads[j]->ent_num);
			if (subloc[i]->roads[j]->name)
				fprintf(road_fp, "na %s\n",
					subloc[i]->roads[j]->name);
			fprintf(road_fp, "LI\n");
			fprintf(road_fp, " wh %d\n", subloc[i]->region);
			fprintf(road_fp, "GA\n");
			fprintf(road_fp, " tl %d\n",
					subloc[i]->roads[j]->to_loc);
			if (subloc[i]->roads[j]->hidden)
				fprintf(road_fp, " rh %d\n",
					subloc[i]->roads[j]->hidden);
			fprintf(road_fp, "\n");
		}
}


dump_gates()
{
	int row, col;
	int i, j;

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		if (map[row][col])
		    for (j = 0; j < ilist_len(map[row][col]->gates_dest); j++)
		    {
			fprintf(gate_fp, "%d gate\n",
						map[row][col]->gates_num[j]);
			fprintf(gate_fp, "LI\n");
			fprintf(gate_fp, " wh %d\n", map[row][col]->region);
			fprintf(gate_fp, "GA\n");
			fprintf(gate_fp, " tl %d\n",
						map[row][col]->gates_dest[j]);
			if (map[row][col]->gates_key[j])
				fprintf(gate_fp, " sk %d\n",
						map[row][col]->gates_key[j]);
			fprintf(gate_fp, "\n");
		    }

	for (i = 1; i <= top_subloc; i++)
		for (j = 0; j < ilist_len(subloc[i]->gates_num); j++)
		{
			fprintf(gate_fp, "%d gate\n",
						subloc[i]->gates_num[j]);
			fprintf(gate_fp, "LI\n");
			fprintf(gate_fp, " wh %d\n", subloc[i]->region);
			fprintf(gate_fp, "GA\n");
			fprintf(gate_fp, " tl %d\n",
						subloc[i]->gates_dest[j]);
			if (subloc[i]->gates_key[j])
				fprintf(gate_fp, " sk %d\n",
						subloc[i]->gates_key[j]);
			fprintf(gate_fp, "\n");
		}
}


/*
 *  A rough plan for a gate-making strategy
 *
 *  I do not promise to stick to this plan in the code below
 *
 *
 *	10 links from provinces to islands, 10 from each island
 *	back to another province
 *
 *	three rings of 10 islands each
 *
 *	ring the places-of-power/stonehenge things
 *
 *	the continental tour, a ring landing on every single continent
 *		too easy?
 *
 *	strings of short, random length provinces
 *
 *	star fan-out from a nifty place
 *		to every continent?
 */

make_gates()
{

	gate_link_islands(3);		/* three disjoint */
	gate_link_islands(1);		/* one might cross */
	gate_province_islands(15);
	gate_land_ring(5);
	gate_continental_tour();

/*
 *  Make five circles of stones.  Connect them together.  Create
 *  a fan out set of gates such that each circle leads to about
 *  1/5 of the continents.  Also connect the stone together.
 */

	{
		int one, two, three, four, five;
		int i;

		clear_province_marks();
		one = place_random_subloc(terr_stone_cir, 0, terr_plain);
		two = place_random_subloc(terr_stone_cir, 0, terr_plain);
		three = place_random_subloc(terr_stone_cir, 0, terr_plain);
		four = place_random_subloc(terr_stone_cir, 0, terr_plain);
		five = place_random_subloc(terr_stone_cir, 0, terr_plain);

		gate_fan(one, two, three, four, five);

/* 
 *  Make some more stone circles, just for fun
 */

		for (i = 1; i <= 10; i++)
			place_random_subloc(terr_stone_cir, 0, terr_plain);
	}

/*
 *  Make 21 mallorn groves
 *  Each grove has a 50% chance of being gate linked to the previous
 *  grove.  Each gate has a 30% chance of being sealed.
 */

	{
		int last = 0;
		int cur;
		int i;

		clear_province_marks();
		for (i = 1; i <= 21; i++)
		{
			cur = place_random_subloc(terr_grove, 1, terr_forest);
			if (last > 0 && rnd(0,1) == 1)
			    gate_subloc(cur, last, rnd(0,2)?0:rnd(111,333));

			last = cur;
		}
	}

/*
 *  Make 21 more
 */

	{
		int last = 0;
		int cur;
		int i;

		for (i = 1; i <= 21; i++)
		{
			cur = place_random_subloc(terr_grove, 1, terr_forest);
			if (last > 0 && rnd(0,1) == 1)
			    gate_subloc(cur, last, rnd(0,2)?0:rnd(111,333));

			last = cur;
		}
	}
}


clear_province_marks()
{
	int row;
	int col;

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		if (map[row][col])
			map[row][col]->mark = 0;
}


clear_subloc_marks()
{
	int i;

	for (i = 1; i <= top_subloc; i++)
		subloc[i]->mark = 0;
}


/*
 *  The 'not' refers to not desert and not swamp
 *  We don't want to make any cities there
 *  (Except for the lost city and the city of the ancients)
 */

not_random_province(row, col)		/* oh, hack upon hack ... */
int *row;
int *col;
{
	int n;
	int r, c;
	int sum = 0;

	for (r = 0; r <= max_row; r++)
	    for (c = 0; c < max_col; c++)
		if (map[r][c] && map[r][c]->terrain != terr_ocean &&
		    map[r][c] && map[r][c]->terrain != terr_swamp &&
		    map[r][c] && map[r][c]->terrain != terr_desert &&
			map[r][c]->mark == 0)
		    sum++;

	n = rnd(1, sum);

	for (r = 0; r <= max_row; r++)
	    for (c = 0; c < max_col; c++)
		if ((map[r][c] && map[r][c]->terrain != terr_ocean &&
		    map[r][c] && map[r][c]->terrain != terr_swamp &&
		    map[r][c] && map[r][c]->terrain != terr_desert &&
		        map[r][c]->mark == 0) && (--n <= 0))
		{
			*row = r;
			*col = c;
			map[r][c]->mark = TRUE;
			return;
		}

	assert(FALSE);
}


not_place_random_subloc(kind, hidden)
int kind;
int hidden;
{
	int row, col;

	not_random_province(&row, &col);
	return create_a_subloc(row, col, hidden, kind);
}


random_province(row, col, terr)
int *row;
int *col;
int terr;
{
	int n;
	int r, c;
	int sum = 0;

	if (terr == 0)
	{
	    for (r = 0; r <= max_row; r++)
	        for (c = 0; c < max_col; c++)
			if (map[r][c] && map[r][c]->terrain != terr_ocean &&
				map[r][c]->mark == 0)
			    sum++;
	}
	else
	{
	    for (r = 0; r <= max_row; r++)
	        for (c = 0; c < max_col; c++)
			if (map[r][c] && map[r][c]->terrain == terr &&
				map[r][c]->mark == 0)
			    sum++;
	}

	n = rnd(1, sum);

	if (terr == 0)
	{
	    for (r = 0; r <= max_row; r++)
	        for (c = 0; c < max_col; c++)
		    if ((map[r][c] && map[r][c]->terrain != terr_ocean &&
		        map[r][c]->mark == 0) && (--n <= 0))
		    {
			*row = r;
			*col = c;
			map[r][c]->mark = TRUE;
			return;
		    }
	}
	else
	{
	    for (r = 0; r <= max_row; r++)
	        for (c = 0; c < max_col; c++)
		    if ((map[r][c] && map[r][c]->terrain == terr &&
		        map[r][c]->mark == 0) && (--n <= 0))
		    {
			*row = r;
			*col = c;
			map[r][c]->mark = TRUE;
			return;
		    }
	}

	assert(FALSE);
}


place_random_subloc(kind, hidden, terr)
int kind;
int hidden;
int terr;
{
	int row, col;

	random_province(&row, &col, terr);
	return create_a_subloc(row, col, hidden, kind);
}


random_island()
{
	int n;
	int i;

	do {
		n = rnd(1, num_islands);
		i = 1;

		while (i <= top_subloc)
		{	
			if (subloc[i]->terrain == terr_island)
				if (--n <= 0)
					break;
			i++;
		}

		assert(n <= top_subloc);
	}
	while (subloc[i]->mark);

	subloc[i]->mark = TRUE;

	return i;
}


gate_land_ring(rings)
int rings;
{
	int i, j;
	int r_first, c_first;
	int r_next, c_next;
	int r_n, c_n;
	int num;

	clear_province_marks();

	for (j = 1; j <= rings; j++)
	{
		num = rnd(5, 12);
		random_province(&r_first, &c_first, 0);

		r_n = r_first;
		c_n = c_first;

		for (i = 1; i < num; i++)
		{
			random_province(&r_next, &c_next, 0);

			gate_counter++;
			ilist_append(&map[r_n][c_n]->gates_num, gate_counter);
			ilist_append(&map[r_n][c_n]->gates_dest,
						map[r_next][c_next]->region);
			ilist_append(&map[r_n][c_n]->gates_key, 0);

			r_n = r_next;
			c_n = c_next;
		}

		gate_counter++;
		ilist_append(&map[r_n][c_n]->gates_num, gate_counter);
		ilist_append(&map[r_n][c_n]->gates_dest,
					map[r_first][c_first]->region);
		ilist_append(&map[r_n][c_n]->gates_key, 0);
	}
}


gate_link_islands(rings)
int rings;
{
	int i, j;
	int first, next, n;
	int num;

	clear_subloc_marks();

	for (j = 1; j <= rings; j++)
	{
		num = rnd(6, 12);

		first = random_island();
		n = first;

		for (i = 1; i < num; i++)
		{
			gate_counter++;

			next = random_island();
			ilist_append(&subloc[n]->gates_num, gate_counter);
			ilist_append(&subloc[n]->gates_dest,
						subloc[next]->region);
			ilist_append(&subloc[n]->gates_key, 0);

			n = next;
		}

		gate_counter++;
		ilist_append(&subloc[n]->gates_num, gate_counter);
		ilist_append(&subloc[n]->gates_dest, subloc[first]->region);
		ilist_append(&subloc[n]->gates_key, 0);
	}
}


gate_province_islands(times)
int times;
{
	int i, j;
	int isle;
	int r1, c1, r2, c2;

	clear_province_marks();
	clear_subloc_marks();

	for (j = 1; j <= times; j++)
	{
		random_province(&r1, &c1, 0);
		isle = random_island();
		random_province(&r2, &c2, 0);

		gate_counter++;
		ilist_append(&map[r1][c1]->gates_num, gate_counter);
		ilist_append(&map[r1][c1]->gates_dest, subloc[isle]->region);
		ilist_append(&map[r1][c1]->gates_key, 0);

		gate_counter++;
		ilist_append(&subloc[isle]->gates_num, gate_counter);
		ilist_append(&subloc[isle]->gates_dest, map[r2][c2]->region);
		ilist_append(&subloc[isle]->gates_key, 0);
	}
}


struct tile **
collect_labeled_provinces()
{
	static struct tile **l = NULL;
	int row, col;
	int i;

	if (l)
		return l;

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		if (map[row][col] && map[row][col]->terrain != terr_ocean &&
			map[row][col]->label_code)
		{
			for (i = 0; i < ilist_len(l); i++)
				if (l[i]->inside == map[row][col]->inside)
					continue;
			ilist_append((ilist *) &l, (int) map[row][col]);
		}

	return l;
}


struct tile **
shift_tour_endpoints(l)
struct tile **l;
{
	static struct tile **other = NULL;
	int i;
	struct tile *p;
	struct tile *q;

	for (i = 0; i < ilist_len(l); i++)
	{
		p = adjacent_tile_terr(l[i]->row, l[i]->col);

		if (p == NULL)
		{
			p = l[i];
		}

		q = adjacent_tile_terr(p->row, p->col);
		if (q == l[i])	/* doubled back, retry */
		{
			q = adjacent_tile_terr(p->row, p->col);
		}

		if (q == NULL || q->terrain == terr_ocean)
		{
			fprintf(stderr, "couldn't shift tour (%d,%d)\n",
						l[i]->row, l[i]->col);
			ilist_append((ilist *) &other, (int) l[i]);
		} else
			ilist_append((ilist *) &other, (int) q);
	}

	return other;
}


gate_continental_tour()
{
	int i;
	struct tile **l;
	struct tile **m;

	l = collect_labeled_provinces();
	m = shift_tour_endpoints(l);

	assert(ilist_len(l) == ilist_len(m));

	fprintf(stderr, "\nContinental gate tour:\n");

	for (i = 0; i < ilist_len(l)-1; i++)
	{
		fprintf(stderr, "\t(%2d,%2d) -> (%2d,%2d)\n",
			l[i]->row, l[i]->col, m[i+1]->row, m[i+1]->col);
		gate_counter++;
		ilist_append(&map[l[i]->row][l[i]->col]->gates_num,
							gate_counter);
		ilist_append(&map[l[i]->row][l[i]->col]->gates_dest,
				map[m[i+1]->row][m[i+1]->col]->region);
		ilist_append(&map[l[i]->row][l[i]->col]->gates_key, 0);
	}

	fprintf(stderr, "\t(%2d,%2d) -> (%2d,%2d)\n\n",
		l[i]->row, l[i]->col, m[0]->row, m[0]->col);

	gate_counter++;
	ilist_append(&map[l[i]->row][l[i]->col]->gates_num, gate_counter);
	ilist_append(&map[l[i]->row][l[i]->col]->gates_dest,
			map[m[0]->row][m[0]->col]->region);
	ilist_append(&map[l[i]->row][l[i]->col]->gates_key, rnd(111,333));
}


gate_subloc(a, b, key)
int a;
int b;
int key;
{

	gate_counter++;
	ilist_append(&subloc[a]->gates_num, gate_counter);
	ilist_append(&subloc[a]->gates_dest, subloc[b]->region);
	ilist_append(&subloc[a]->gates_key, key);
}


gate_fan_sup(where, start, end, l)
int where;
int start;
int end;
struct tile **l;
{
	int i;

	for (i = start; i <= end; i++)
	{
		gate_counter++;
		ilist_append(&subloc[where]->gates_num, gate_counter);
		ilist_append(&subloc[where]->gates_dest, l[i]->region);
		ilist_append(&subloc[where]->gates_key, 0);
	}
}


gate_fan(one, two, three, four, five)
int one;
int two;
int three;
int four;
int five;
{
	int i;
	struct tile **l;
	int n, part;

	assert(one != two);
	assert(two != three);
	assert(three != four);
	assert(four != five);

	gate_subloc(one, two, rnd(111, 333));
	gate_subloc(two, three, rnd(111, 333));
	gate_subloc(three, four, rnd(111, 333));
	gate_subloc(four, five, rnd(111, 333));
	gate_subloc(five, one, rnd(111, 333));

	gate_subloc(five, three, 0);
	gate_subloc(three, one, 0);

	l = collect_labeled_provinces();

	n = ilist_len(l);
	part = n / 5;

	gate_fan_sup(one, 0, part-1, l);
	gate_fan_sup(two, part, part*2-1, l);
	gate_fan_sup(three, part*2, part*3-1, l);
	gate_fan_sup(four, part*3, part*4-1, l);
	gate_fan_sup(five, part*4, n-1, l);
}

struct {
	int skill;
	int weight;
	char *name;
} guild_names[] = {

	{skill_ship,	  3,	"Guild of Shipwrights"},
	{skill_ship,	  3,	"Academy of Shipbuilding"},
	{skill_ship,	  1,	"School of Naval Science and Engineering"},

	{skill_combat,	  3,	"Warriors Guild"},
	{skill_combat,	  3,	"Warriors Tower"},
	{skill_combat,	  3,	"School of Combat"},
	{skill_combat,	  3,	"Guild of Combat"},
	{skill_combat,	  2,	"Tower of the Sword and Axe"},

	{skill_stealth,	  3,	"Guild of Thieves"},
	{skill_stealth,	  3,	"Thieves Guild"},
	{skill_stealth,	  2,	"Brotherhood of Night"},
	{skill_stealth,	  2,	"Thieves Tower"},

	{skill_beast,	  1,	"Beastmasters Tower"},
	{skill_beast,	  1,	"Beastmasters Guild"},

	{skill_pers,	  1,	"Speakers Guild"},
	{skill_pers,	  1,	"School of Persuasion"},
	{skill_pers,	  1,	"School of Oratory"},

	{skill_const,	  1,	"School of Engineering"},
	{skill_const,	  1,	"Engineers Guild"},

	{skill_alchemy,	  1,	"Society of Alchemists"},
	{skill_alchemy,	  1,	"Alchemists Guild"},

	{skill_basic,	  1,	"Guild of Magicians"},
	{skill_basic,	  1,	"Magicians Guild"},

	{skill_weather,	  1,	"Weathermasters Guild"},
	{skill_illusion,  1,	"Illusionists Guild"},
	{skill_gate,	  1,	"Gatecrafters Guild"},
	{skill_art,	  1,	"Guild of Artifact Sorcery"},

	{skill_necro,	  1,	"Necromancers Guild"},
	{skill_necro,	  1,	"Order of the Necromancer"},

	{terr_stone_cir,  1,	NULL},
	{terr_grove,	  1,	NULL},
	{terr_bog,	  1,	NULL},
	{terr_cave,	  1,	NULL},

	{terr_grave,	  20,	NULL},
	{terr_grave,	  1,	"Barrows"},
	{terr_grave,	  1,	"Barrow Downs"},
	{terr_grave,	  1,	"Barrow Hills"},
	{terr_grave,	  1,	"Cairn Hills"},
	{terr_grave,	  1,	"Catacombs"},
	{terr_grave,	  1,	"Grave Mounds"},
	{terr_grave,	  1,	"Place of the Dead"},
	{terr_grave,	  1,	"Cemetary Hill"},
	{terr_grave,	  1,	"Fields of Death"},

	{terr_ruins,	  1,	NULL},

	{terr_battlefield,1,	"Ancient battlefield"},
	{terr_battlefield,1,	"Old battlefield"},

	{terr_ench_for,	  1,	NULL},
	{terr_rocky_hill, 1,	NULL},
	{terr_tree_cir,	  1,	NULL},
	{terr_pits,	  1,	"Cursed Pits"},
	{terr_pasture,	  1,	NULL},
	{terr_pasture,	  1,	"Grassy field"},
	{terr_sac_grove,  1,	NULL},
	{terr_oasis,	  1,	NULL},
	{terr_pop_field,  1,	NULL},
	{terr_sand_pit,	  1,	NULL},
	{terr_yew_grove,  1,	NULL},

	{terr_temple,     1,	NULL},
	{terr_lair,       1,	NULL},

	{0, 0, NULL}
};


char *
name_guild(skill)
int skill;
{
	int i;
	int sum = 0;
	int n;

	for (i = 0; guild_names[i].skill; i++)
		if (guild_names[i].skill == skill)
			sum += guild_names[i].weight;

	assert(sum > 0);

	n = rnd(1, sum);

	for (i = 0; guild_names[i].skill; i++)
		if (guild_names[i].skill == skill)
		{
			n -= guild_names[i].weight;

			if (n <= 0)
				return guild_names[i].name;
		}

	assert(FALSE);
}


add_city_skill(sl, skill, hidden)
int sl;
int skill;
int hidden;
{

	ilist_append(&subloc[sl]->teaches, skill);
	skill_count[skill - FIRST_SKILL]++;
}


add_city_guild(sl, skill, hidden)
int sl;
int skill;
int hidden;
{
	int n;

	n = create_a_building(sl, hidden, terr_guild);
	ilist_append(&subloc[n]->teaches, skill);
	subloc[n]->name = name_guild(skill);

	skill_count[skill - FIRST_SKILL]++;

	if (skill == skill_stealth)
	{
		struct tile *dest;
		char *s;

		dest = adjacent_tile_terr(subloc[n]->row, subloc[n]->col);
		if (dest == NULL || dest->terrain == terr_ocean)
			dest = map[subloc[n]->row][subloc[n]->col];

		if (rnd(0,1) == 0)
			s = "Secret passage";
		else
			s = "Underground passage";

		add_road(subloc[n], dest->region, 1, s);
	}
}


assign_skills()
{
    int i;

     for (i = 1; i <= top_subloc; i++)
         if (subloc[i]->terrain == terr_city && subloc[i]->skill_assign == 0)
	 {
		subloc[i]->skill_assign = 1;

		if (rnd(1,100) <= 25)
			add_city_skill(i, skill_basic, rnd(0,1));

		if (rnd(1,100) <= 25)
			add_city_skill(i, skill_alchemy, rnd(0,1));

		if (rnd(1,100) <= 50)
			add_city_skill(i, skill_combat, 0);

		if (rnd(1,100) <= 25)
			add_city_skill(i, skill_pers, 0);

		if (rnd(1,100) <= 25)
			add_city_skill(i, skill_stealth, rnd(0,1));

		if (subloc[i]->terrain == terr_city &&
		    is_port_city(subloc[i]->row, subloc[i]->col))
			add_city_skill(i, skill_ship);

		switch (map[subloc[i]->row][subloc[i]->col]->terrain)
		{
		case terr_plain:
			if (rnd(1,100) <= 100)
				add_city_skill(i, skill_beast, 0);
			break;

		case terr_mountain:
			if (rnd(1,100) <= 100)
				add_city_skill(i, skill_const, 0);
			break;

		case terr_forest:
			if (rnd(1,100) <= 50)
				add_city_skill(i, skill_const, 0);
			break;
		}
	}

	fprintf(stderr, "n_skill_ship     %d\n", skill_count[0]);
	fprintf(stderr, "n_skill_combat   %d\n", skill_count[1]);
	fprintf(stderr, "n_skill_stealth  %d\n", skill_count[2]);
	fprintf(stderr, "n_skill_beast    %d\n", skill_count[3]);
	fprintf(stderr, "n_skill_pers     %d\n", skill_count[4]);
	fprintf(stderr, "n_skill_const    %d\n", skill_count[5]);
	fprintf(stderr, "n_skill_alchemy  %d\n", skill_count[6]);
}


count_subloc_coverage()
{
	int row, col;
	int i;
	int count[100];

	clear_province_marks();

	for (i = 1; i <= top_subloc; i++)
		if (subloc[i]->depth == 3)
		{
			map[subloc[i]->row][subloc[i]->col]->mark++;

			if (map[subloc[i]->row][subloc[i]->col]->mark >= 5)
			fprintf(stderr, "(%d,%d) has %d sublocs (region %d)\n",
				subloc[i]->row, subloc[i]->col,
			map[subloc[i]->row][subloc[i]->col]->mark,
			map[subloc[i]->row][subloc[i]->col]->region);
		}

	fprintf(stderr, "\nsubloc coverage:\n");

	for (i = 0; i < 100; i++)
		count[i] = 0;

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		if (map[row][col] && map[row][col]->terrain != terr_ocean)
			count[map[row][col]->mark]++;

	for (i = 99; i >= 0; i--)
		if (count[i] != 0)
			break;
		else
			count[i] = -1;

	for (i = 0; i < 100; i++)
	{
		if (count[i] == -1)
			break;

		fprintf(stderr, "%6d %s %d subloc%s (%d%%)\n",
				count[i],
				count[i] == 1 ? "loc has" : "locs have",
				i, i == 1 ? " " : "s",
				count[i] * 100 / land_count);
	}
}


make_special_cities()
{
	int n;
	int n1, n2;

	clear_province_marks();

	n1 = place_random_subloc(terr_city, 1, terr_swamp);
	subloc[n1]->name = str_save("City of the Lost");
	subloc[n1]->skill_assign = 1;

	n2 = place_random_subloc(terr_city, 1, terr_desert);
	subloc[n2]->name = str_save("City of the Ancients");
	subloc[n2]->skill_assign = 1;

	gate_subloc(n1, n2, rnd(111,333));
	gate_subloc(n2, n1, rnd(111,333));

	n = not_place_random_subloc(terr_city, 1);
	subloc[n]->name = str_save("City of Magicians");
	add_city_skill(n, skill_basic);

	add_city_skill(n, skill_weather, 0);
	add_city_skill(n, skill_illusion, 0);
	add_city_skill(n, skill_gate, 0);
	add_city_skill(n, skill_art, 0);
	add_city_skill(n, skill_necro, 0);
	subloc[n]->skill_assign = 1;
}


struct {
	int terr;		/* terrain appropriate */
	int kind;		/* what to make there */
	int weight;		/* weight given to selection */
	int hidden;		/* 0=no, 1=yes, 2=rnd(0,1) */
} loc_table[] = {

	{terr_desert,		terr_cave,		10,	1},
	{terr_desert,		terr_oasis,		10,	2},
	{terr_desert,		terr_sand_pit,		10,	2},

	{terr_mountain,		terr_grave,		10,	2},
	{terr_mountain,		terr_ruins,		10,	1},
	{terr_mountain,		terr_cave,		10,	1},
	{terr_mountain,		terr_yew_grove,		10,	2},
	{terr_mountain,		terr_lair,		10,	2},

	{terr_swamp,		terr_bog,		10,	2},
	{terr_swamp,		terr_pits,		10,	2},

	{terr_forest,		terr_grave,		10,	2},
	{terr_forest,		terr_ruins,		10,	1},
	{terr_forest,		terr_tree_cir,		10,	1},
	{terr_forest,		terr_ench_for,		10,	1},
	{terr_forest,		terr_yew_grove,		10,	2},

	{terr_plain,		terr_grave,		10,	2},
	{terr_plain,		terr_ruins,		10,	1},
	{terr_plain,		terr_pasture,		10,	0},
	{terr_plain,		terr_rocky_hill,	10,	0},
	{terr_plain,		terr_sac_grove,		10,	2},
	{terr_plain,		terr_pop_field,		10,	0},

	{0,0,0,0}
};


make_appropriate_subloc(row, col)
int row;
int col;
{
	int terr;
	int sum = 0;
	int i;
	int n;
	int hid;
	char *s;

	terr = map[row][col]->terrain;

	for (i = 0; loc_table[i].kind; i++)
	    if (loc_table[i].terr == terr)
	    {
		sum += loc_table[i].weight;
	    }

	if (sum <= 0)
	{
		fprintf(stderr, "no subloc appropriate for (%d,%d)\n",
					row, col);
		return;
	}

	n = rnd(1, sum);

	for (i = 0; loc_table[i].kind; i++)
	    if (loc_table[i].terr == terr)
	    {
		n -= loc_table[i].weight;

		if (n <= 0)
		{
			if (loc_table[i].kind < 0)
				break;

			if (loc_table[i].hidden == 2)
				hid = rnd(0,1);
			else
				hid = loc_table[i].hidden;

			n = create_a_subloc(row, col, hid, loc_table[i].kind);
			s = name_guild(loc_table[i].kind);
			if (s && *s)
				subloc[n]->name = str_save(s);
			break;
		}
	    }
}


place_sublocations()
{
    int row, col;
    FILE *city_fp;
    int n;
    int i;
    char *line;

    city_fp = fopen("Cities", "r");

    for (row = 0; row < MAX_ROW; row++)
        for (col = 0; col < MAX_COL; col++)
	    if (map[row][col] && map[row][col]->terrain != terr_ocean)
	    {
/*
 *  Put a city everywhere there is a * or every 1 in 16 locs,
 *  randomly.  Don't put one where there already is a city (city != 2).
 */
		if (map[row][col]->city == 1 ||
			(rnd(1,16) == 1 && map[row][col]->city != 2))
		{
			n = create_a_subloc(row, col, 0, terr_city);

			if (city_fp && (line = getlin(city_fp)))
				subloc[n]->name = str_save(line);

			if (map[row][col]->safe_haven)
				subloc[n]->safe_haven = TRUE;
		}

		if (rnd(1,3) == 1)
			make_appropriate_subloc(row, col, 0);
	    }

    fclose(city_fp);
}


count_tiles()
{
	int r, c;
	int i;
	int count[1000];

	for (i = 0; i < 1000; i++)
		count[i] = 0;

	for (r = 0; r < MAX_ROW; r++)
	    for (c = 0; c < MAX_COL; c++)
		if (map[r][c])
			count[map[r][c]->terrain]++;

	for (i = 1; i <= top_subloc; i++)
		count[subloc[i]->terrain]++;

	for (i = 1; terr_s[i]; i++)
		fprintf(stderr, "%-30s %d\n", terr_s[i], count[i]);
}

