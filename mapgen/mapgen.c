/*
 *  Todo:
 *
 *	1.  Remove generation of special cities, skills for cities
 *	2.  Gate rings might be better made in the engine
 *	3.  Don't forget to implement Summerbridge and Uldim pass in the engine
 *	4.  Make hades bigger to accomodate more graveyards
 *
 */


/*
 *  Olympia map generator
 *
 *  Abandon all hope, ye who enter here.
 */


#include	<stdio.h>
#include	<stdlib.h>
#include	"z.h"


#define		LINE_LEN	256

#define		TRUE		1
#define		FALSE		0

#define		MAX_ROW		100
#define		MAX_COL		100


#define		REGION_OFF	58760	/* where to start numbering regions */

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


#define		DIR_N		1
#define		DIR_E		2
#define		DIR_S		3
#define		DIR_W		4
#define		DIR_NE		5
#define		DIR_SE		6
#define		DIR_SW		7
#define		DIR_NW		8

#define		MAX_DIR		9


#define		MAX_BOX		100000
char alloc_flag[MAX_BOX];


/*
 *  G2 Entity coding system:
 *
 *  range	      extent	use
 *  1-999               999	items
 *  1000-8999	       8000	chars
 *  9000-9999          1000	skills
 *
 *  10,000-19,999    10,000	provinces		(CCNN: AA00-DV99)
 *  20,000-49,999    20,000	more provinces		(CCNN: DW00-ZZ99)
 *  50,000-56,759      6760	player entities		(CCN)
 *  56,760-58,759      2000	lucky locs		(CNN)
 *  58,760-58,999	240	regions			(NNNNN)
 *  59,000-78,999    20,000	sublocs, misc		(CNNN: A000-Z999)
 *  79,000-100,000   21,000     storms			(NNNNN)
 *
 *  Note: restricted alphabet, no vowels (except a) or l:
 *	"abcdfghjkmnpqrstvwxz"
 */

#define		SUBLOC_LOW	59000
#define		SUBLOC_HIGH	78999

#define		CITY_LOW	56760
#define		CITY_HIGH	58759


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
	"battlefield",
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
	char save_char;
	int region;
	char *name;
	int terrain;
	int hidden;
	int city;
	int mark;
	int inside;
	int color;			/* map coloring for */
	int row, col;			/* map tile we're inside */
	int depth;

	int safe_haven;
	int sea_lane;
	int uldim_flag;
	int summerbridge_flag;
	int region_boundary;
	int major_city;

	ilist subs;

	ilist gates_dest;		/* gates from here */
	ilist gates_num;		/* gates from here */
	ilist gates_key;

	struct road **roads;
};


#define	MAX_INSIDE	200		/* max continents/regions */

char *inside_names[MAX_INSIDE];
struct tile **inside_list[MAX_INSIDE];	/* plist of provinces in each region */
int inside_gates_to[MAX_INSIDE];	/* for info gathering only */
int inside_gates_from[MAX_INSIDE];	/* for info gathering only */
int inside_num_cities[MAX_INSIDE];	/* for info gathering only */
int inside_top = 0;

int max_col = 0;
int max_row = 0;

int water_count = 0;			/* count of water provinces */
int land_count = 0;			/* count of land provinces */
int num_islands = 0;

struct tile *map[MAX_ROW][MAX_COL];

struct tile *adjacent_tile_terr();
struct tile *adjacent_tile_water();
struct tile *adjacent_tile_sup();
char *fetch_inside_name();

#define		MAX_SUBLOC	20000

struct tile *subloc[MAX_SUBLOC];
int top_subloc = 0;

void random_province(int *row, int *col, int terr);
void set_province_clumps();
void set_regions();

FILE *loc_fp;
FILE *gate_fp;
FILE *road_fp;

main()
{
	clear_alloc_flag();
	dir_assert();
	open_fps();
	load_seed("randseed");
	map_init();
	read_map();
	fix_terrain_land();
	set_regions();
	set_province_clumps();
	unnamed_province_clumps();
	make_islands();
	make_graveyards();
	place_sublocations();
	make_gates();
	make_roads();
	print_map();
	print_sublocs();
	dump_continents();
	count_cities();
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
	fprintf(stderr, "\n");

	save_seed("randseed");

#if 0
/*
 *  If the province allocation spilled into the subloc range,
 *  we have to increase SUBLOC_MAX
 */

	assert(SUBLOC_LOW > map[max_row][max_col]->region);
#endif
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


clear_alloc_flag()
{
	int i;

	for (i = 0; i < MAX_BOX; i++)
		alloc_flag[i] = 0;
}


int
rnd_alloc_num(low, high)
int low;
int high;
{
        int n;
        int i;
 
        n = rnd(low, high);
 
        for (i = n; i <= high; i++)
                if (alloc_flag[i] == 0)
		{
			alloc_flag[i] = 1;
                        return i;
		}
 
        for (i = low; i < n; i++)
                if (alloc_flag[i] == 0)
		{
			alloc_flag[i] = 1;
                        return i;
		}
 
        fprintf(stderr, "rnd_alloc_num(%d,%d) failed\n", low, high);
 
        return -1;
}


map_init()
{
	int i, j;

	for (i = 0; i < MAX_ROW; i++)
		for (j = 0; j < MAX_COL; j++)
			map[i][j] = NULL;
}


read_map()
{
	char buf[LINE_LEN];
	int row, col;
	int terrain;
	int skipnext = FALSE;
	int color;
	int i;
	FILE *fp;
	int n;

	fp = fopen("Map", "r");
	if (fp == NULL)
	{
		perror("can't open Map");
		exit(1);
	}

	row = 0;

	while (fgets(buf, LINE_LEN, fp) != NULL)
	{
		for (col = 0; buf[col] && buf[col] != '\n'; col++)
		{
			if (buf[col] == '#')	/* hole in map */
				continue;

			if (row > max_row)
				max_row = row;

			if (col > max_col)
				max_col = col;

			map[row][col] = my_malloc(sizeof(struct tile));
			map[row][col]->row = row;
			map[row][col]->col = col;
			map[row][col]->region = rc_to_region(row, col);
			map[row][col]->depth = 2;

			color = 0;

			switch (buf[col])
			{

			case ';':
				map[row][col]->sea_lane = TRUE;
				/* FALLTHROUGH */
			case ',':
				terrain = terr_ocean;
				color = 1;
				break;

			case ':':
				map[row][col]->sea_lane = TRUE;
				/* FALLTHROUGH */
			case '.':
				terrain = terr_ocean;
				color = 2;
				break;

			case '~':
				map[row][col]->sea_lane = TRUE;
				/* FALLTHROUGH */
			case ' ':
				terrain = terr_ocean;
				color = 3;
				break;

			case '"':
				map[row][col]->sea_lane = TRUE;
				/* FALLTHROUGH */
			case '\'':
				terrain = terr_ocean;
				color = 4;
				break;

			case 'p':
				color = 5;
				terrain = terr_plain;
				break;

			case 'P':
				color = 6;
				terrain = terr_plain;
				break;

			case 'd':
				color = 7;
				terrain = terr_desert;
				break;

			case 'D':
				color = 8;
				terrain = terr_desert;
				break;

			case 'm':
				color = 9;
				terrain = terr_mountain;
				break;

			case 'M':
				color = 10;
				terrain = terr_mountain;
				break;

			case 's':
				color = 11;
				terrain = terr_swamp;
				break;

			case 'S':
				color = 12;
				terrain = terr_swamp;
				break;

			case 'f':
				color = 13;
				terrain = terr_forest;
				break;

			case 'F':
				color = 14;
				terrain = terr_forest;
				break;

			case 'o':
				switch (rnd(1,10)) {
				case 1:
				case 2:
				case 3:
					terrain = terr_forest;
					break;

				case 4:
				case 5:
				case 6:
					terrain = terr_plain;
					break;

				case 7:
				case 8:
					terrain = terr_mountain;
					break;

				case 9:
					terrain = terr_swamp;
					break;

				case 10:
					terrain = terr_desert;
					break;
				}

				color = -1;
				break;

#if 0
			case '?':
				map[row][col]->hidden = TRUE;
				terrain = terr_land;
				break;
#endif

/*
 *  Special stuff
 */

			case '^':
				color = 9;		/* was 15, unique */
				terrain = terr_mountain;
				map[row][col]->uldim_flag = 1;
				map[row][col]->region_boundary = TRUE;
				break;

			case 'v':
				color = 9;		/* was 15, unique */
				terrain = terr_mountain;
				map[row][col]->uldim_flag = 2;
				map[row][col]->region_boundary = TRUE;
				break;

			case '{':
				color = 16;
				terrain = terr_mountain;
				map[row][col]->uldim_flag = 3;
				map[row][col]->name = "Uldim pass";
				map[row][col]->region_boundary = TRUE;
				break;

			case '}':
				color = 16;
				terrain = terr_mountain;
				map[row][col]->uldim_flag = 4;
				map[row][col]->name = "Uldim pass";
				map[row][col]->region_boundary = TRUE;
				break;

			case ']':
				terrain = terr_swamp;
				map[row][col]->summerbridge_flag = 1;
				map[row][col]->name = "Summerbridge";
				map[row][col]->region_boundary = TRUE;
				break;

			case '[':
				terrain = terr_swamp;
				map[row][col]->summerbridge_flag = 2;
				map[row][col]->name = "Summerbridge";
				map[row][col]->region_boundary = TRUE;
				break;

			case 'O':
				terrain = terr_mountain;
				color = -1;
				map[row][col]->name = "Mt. Olympus";
				break;

			case '1':
				terrain = terr_forest;
				color = 19;
				map[row][col]->safe_haven = TRUE;

				n = create_a_city(row, col, "Drassa", TRUE);
				subloc[n]->safe_haven = TRUE;

				fprintf(stderr, "Start city #%c %s at (%d,%d)\n",
						buf[col], subloc[n]->name, row, col);
				break;

			case '2':
				terrain = terr_forest;
				color = 19;
				map[row][col]->safe_haven = TRUE;

				n = create_a_city(row, col, "Rimmon", TRUE);
				subloc[n]->safe_haven = TRUE;

				fprintf(stderr, "Start city #%c %s at (%d,%d)\n",
						buf[col], subloc[n]->name, row, col);
				break;

			case '3':
				terrain = terr_forest;
				color = 19;
				map[row][col]->safe_haven = TRUE;

				n = create_a_city(row, col, "Harn", TRUE);
				subloc[n]->safe_haven = TRUE;

				fprintf(stderr, "Start city #%c %s at (%d,%d)\n",
						buf[col], subloc[n]->name, row, col);
				break;

			case '4':
				terrain = terr_forest;
				color = 19;
				map[row][col]->safe_haven = TRUE;

				n = create_a_city(row, col, "Imperial City", TRUE);
				subloc[n]->safe_haven = TRUE;

				fprintf(stderr, "Imperical City #%c %s at (%d,%d)\n",
						buf[col], subloc[n]->name, row, col);
				break;

			case '5':
				terrain = terr_forest;
				color = 19;
				map[row][col]->safe_haven = TRUE;

				n = create_a_city(row, col, "Port Aurnos", TRUE);
				subloc[n]->safe_haven = TRUE;

				fprintf(stderr, "Start city #%c %s at (%d,%d)\n",
						buf[col], subloc[n]->name, row, col);
				break;

			case '6':
				terrain = terr_forest;
				color = 19;
				map[row][col]->safe_haven = TRUE;

				n = create_a_city(row, col, "Greyfell", TRUE);
				subloc[n]->safe_haven = TRUE;

				fprintf(stderr, "Start city #%c %s at (%d,%d)\n",
						buf[col], subloc[n]->name, row, col);
				break;

			case '7':
				terrain = terr_forest;
				color = 19;
				map[row][col]->safe_haven = TRUE;

				n = create_a_city(row, col, "Yellowleaf", TRUE);
				subloc[n]->safe_haven = TRUE;

				fprintf(stderr, "Start city #%c %s at (%d,%d)\n",
						buf[col], subloc[n]->name, row, col);
				break;

			case '8':
				terrain = terr_forest;
				color = 19;

				n = create_a_city(row, col, "Golden City", TRUE);

				fprintf(stderr, "Golden City #%c %s at (%d,%d)\n",
						buf[col], subloc[n]->name, row, col);
				break;

			case '9': /* A starting city with a random name */
			case '0':
				terrain = terr_forest;
				color = 19;
				map[row][col]->safe_haven = TRUE;

				n = create_a_city(row, col, NULL, TRUE);
				subloc[n]->safe_haven = TRUE;

				fprintf(stderr, "Start city #%c %s at (%d,%d)\n",
						buf[col], subloc[n]->name, row, col);
				break;

			case '*':
				terrain = terr_land;
				create_a_city(row, col, NULL, TRUE);
				break;

			case '%':
				terrain = terr_land;
				create_a_city(row, col, NULL, FALSE);
				break;

			default:
				if (!isdigit(buf[col]))
				{
					fprintf(stderr, "unknown terrain %c\n",
							buf[col]);
					assert(FALSE);
				}
				assert(FALSE);
				break;
			}

			map[row][col]->save_char = buf[col];
			map[row][col]->terrain = terrain;
			map[row][col]->color = color;

			if (terrain == terr_water || terrain == terr_ocean)
				water_count++;
			else
				land_count++;
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
	r->ent_num = rnd_alloc_num(SUBLOC_LOW, SUBLOC_HIGH);
	r->to_loc = to_loc;
	r->hidden = hidden;
	r->name = name;

	plist_append((plist *) &from->roads, r);
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


alloc_inside()
{
	int i;

	inside_top++;
	assert(inside_top < MAX_INSIDE);

	return inside_top;
}


dump_continents()
{
	int i;
	int n;

	for (i = 1; i <= inside_top; i++)
	{
		fprintf(loc_fp, "%d loc region\n", REGION_OFF + i);
		if (inside_names[i])
			fprintf(loc_fp, "na %s\n", inside_names[i]);
		print_inside_locs(i);
		fprintf(loc_fp, "\n");
	}
}


print_inside_locs(n)
int n;
{
	int i;
	int count = 0;

	for (i = 0; i < plist_len(inside_list[n]); i++)
	{
		count++;

		if (count == 1)
		{
			fprintf(loc_fp, "LI\n");
			fprintf(loc_fp, " hl ");
		}

		if (count % 11 == 10)		/* continuation line */
			fprintf(loc_fp, "\\\n\t");

		fprintf(loc_fp, "%d ", inside_list[n][i]->region);
	}

	if (count)
		fprintf(loc_fp, "\n");
}


print_map()
{
	int row, col;
	int flag;
	int sl;

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		if (map[row][col])
		{
			flag = TRUE;
			sl = FALSE;

			fprintf(loc_fp, "%d loc %s\n", map[row][col]->region,
					terr_s[map[row][col]->terrain]);

			if (map[row][col]->name &&
			    strcmp(map[row][col]->name, "Unnamed") != 0)
			{
				fprintf(loc_fp, "na %s\n",
						map[row][col]->name);
			}

			if (map[row][col]->uldim_flag)
			{
				if (!sl)
				{
					fprintf(loc_fp, "SL\n");
					sl = TRUE;
				}
				fprintf(loc_fp, " uf %d\n",
						map[row][col]->uldim_flag);
			}

			if (map[row][col]->summerbridge_flag)
			{
				if (!sl)
				{
					fprintf(loc_fp, "SL\n");
					sl = TRUE;
				}
				fprintf(loc_fp, " sf %d\n",
					map[row][col]->summerbridge_flag);
			}

			if (map[row][col]->safe_haven)
			{
				if (!sl)
				{
					fprintf(loc_fp, "SL\n");
					sl = TRUE;
				}
				fprintf(loc_fp, " sh 1\n");
			}

			if (map[row][col]->inside)
			{
				fprintf(loc_fp, "LI\n");
				flag = FALSE;

				if (map[row][col]->inside)
					fprintf(loc_fp, " wh %d\n",
					map[row][col]->inside + REGION_OFF);
			}

			print_inside_sublocs(flag, row, col);

			fprintf(loc_fp, "LO\n");
			fprintf(loc_fp, " pd %d %d %d %d\n",
					prov_dest(map[row][col], DIR_N),
					prov_dest(map[row][col], DIR_E),
					prov_dest(map[row][col], DIR_S),
					prov_dest(map[row][col], DIR_W));

			if (map[row][col]->hidden)
				fprintf(loc_fp, " hi %d\n",
							map[row][col]->hidden);
#if 1	/* untested */
			if (map[row][col]->sea_lane)
				fprintf(loc_fp, " sl 1\n");
#endif

			fprintf(loc_fp, "\n");
		}
}


print_sublocs()
{
	int i;
	int sl;

	for (i = 1; i <= top_subloc; i++)
	{
		sl = FALSE;

		fprintf(loc_fp, "%d loc %s\n", subloc[i]->region,
				terr_s[subloc[i]->terrain]);

		if (subloc[i]->name &&
		    strcmp(subloc[i]->name, "Unnamed") != 0)
		{
			fprintf(loc_fp, "na %s\n", subloc[i]->name);
		}

		assert(subloc[i]->inside);
		fprintf(loc_fp, "LI\n");
		fprintf(loc_fp, " wh %d\n", subloc[i]->inside);
		print_subloc_gates(i);

		fprintf(loc_fp, "LO\n");

		if (subloc[i]->hidden)
			fprintf(loc_fp, " hi %d\n", subloc[i]->hidden);

		if (subloc[i]->major_city)
		{
			if (!sl)
			{
				fprintf(loc_fp, "SL\n");
				sl = TRUE;
			}
			fprintf(loc_fp, " mc %d\n", subloc[i]->major_city);
		}

		if (subloc[i]->safe_haven)
		{
			if (!sl)
			{
				fprintf(loc_fp, "SL\n");
				sl = TRUE;
			}
			fprintf(loc_fp, " sh 1\n");
		}

		fprintf(loc_fp, "\n");
	}
}


print_subloc_gates(n)		/* and inside buildings... */
int n;
{
	int i;
	int count = 0;

	for (i = 0; i < plist_len(subloc[n]->roads); i++)
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

	assert(row >= 0 && row < 100);
	assert(col >= 0 && col < 100);

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
	struct tile *p = NULL;
	int i;

	randomize_dir_vector();

	for (i = 1; !(p && p->terrain == terr_ocean) && i < MAX_DIR; i++)
		p = adjacent_tile_sup(row, col, dir_vector[i]);

	return (i < MAX_DIR) ? p : NULL;
}


struct tile *
adjacent_tile_terr(row, col)
int row;
int col;
{
	struct tile *p = NULL;
	int i;

	randomize_dir_vector();

	for (i = 1; !(p && p->terrain != terr_land && p->terrain != terr_ocean) && i < MAX_DIR; i++)
		p = adjacent_tile_sup(row, col, dir_vector[i]);

	return (i < MAX_DIR) ? p : NULL;
}


/*
 *  Return the region immediately adjacent to <location>
 *  in direction <dir>
 *
 *  Returns 0 if there is no adjacent location in the given
 *  direction.
 */

prov_dest(t, dir)
struct tile *t;
int dir;
{
	int row;
	int col;

	row = t->row;
	col = t->col;

	switch (dir)
	{
	case DIR_N:
		row--;
		break;

	case DIR_E:
		col++;
		break;

	case DIR_S:
		row++;
		break;

	case DIR_W:
		col--;
		break;

	default:
		fprintf(stderr, "location_direction: bad dir %d\n", dir);
		assert(FALSE);
	}

	if (row < 0 || row > 99)
		return 0;

	if (col < 0)
		col = max_col;
	if (col > max_col)
		col = 0;

	if (map[row][col] == NULL)
		return 0;

	return map[row][col]->region;
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

	if (col < 0)
		col = max_col;
	if (col > max_col)
		col = 0;

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
	int i, swap, tmp;

	dir_vector[0] = 0;
	for (i = 1; i < MAX_DIR; i++)
		dir_vector[i] = i;

	for (i = 1; i < MAX_DIR; i++)
	{
		swap = rnd(i, MAX_DIR - 1);

		if (i != swap)
		{
			tmp = dir_vector[i];
			dir_vector[i] = dir_vector[swap];
			dir_vector[swap] = tmp;
		}
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

	for (row = 0; row < max_row; row++)
	    for (col = 0; col < max_col; col++)
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

	if ((n && n->mark) || (s && s->mark) ||
		(e && e->mark) || (w && w->mark) ||
	    (nw && nw->mark) || (sw && sw->mark) ||
		(ne && ne->mark) || (se && se->mark))
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


print_continent(i)
int i;
{
	char coord[50];
	char gates[50];
	char nprovs[50];
	char ncities[50];
	char buf[100];
	struct tile *p;
	char *name;

	p = inside_list[i][0];

	name = inside_names[i];

	if (name == NULL)
	{
		sprintf(buf, "?? (%d,%d)", p->row, p->col);
		name = buf;
	}

	sprintf(coord, "(%d,%d)", p->row, p->col);
	sprintf(gates, "%d/%d", inside_gates_from[i], inside_gates_to[i]);
	sprintf(nprovs, "%d", plist_len(inside_list[i]));
	sprintf(ncities, "%d", inside_num_cities[i]);

	fprintf(stderr, "%-25s  %8s  %6s  %7s  %s\n",
				name,
				coord,
				nprovs,
				ncities,
				gates);
}


count_continents()
{
	int i;
	char *name;
	char buf[100];
	struct tile *p;

	fprintf(stderr, "\nLand regions:\n\n");

	fprintf(stderr, "%-25s  %8s  %6s  %7s  %s\n",
			"name", "coord", "nprovs", "ncities", "gates (out/in)");
	fprintf(stderr, "%-25s  %8s  %6s  %7s  %s\n",
			"----", "-----", "------", "-------", "--------------");

	for (i = 1; i <= inside_top; i++)
	{
		p = inside_list[i][0];

		if (p->terrain != terr_ocean)
			print_continent(i);
	}

	fprintf(stderr, "\n\nOceans:\n\n");

	fprintf(stderr, "%-25s  %8s  %6s  %7s  %s\n",
			"name", "coord", "nprovs", "ncities", "gates (out/in)");
	fprintf(stderr, "%-25s  %8s  %6s  %7s  %s\n",
			"----", "-----", "------", "-------", "--------------");

	for (i = 1; i <= inside_top; i++)
	{
		p = inside_list[i][0];

		if (p->terrain == terr_ocean)
			print_continent(i);
	}

	fprintf(stderr, "\n\n%d continents\n", inside_top);
	fprintf(stderr, "%d land locs\n", land_count);
	fprintf(stderr, "%d water locs\n", water_count);
}


fix_terrain_land()
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
			{
				map[row][col]->terrain = p->terrain;
				map[row][col]->color = p->color;
			}
			else
			{
fprintf(stderr, "fix_terrain: could not infer type of (%d,%d)\n", row, col);
fprintf(stderr, "    assuming 'forest'\n");
				map[row][col]->terrain = terr_forest;
			}
		}
}

void
set_regions()
{
	FILE *fp;
	char buf[LINE_LEN];
	int row, col;
	int ins;
	int land_count = 0;
	int water_count = 0;
	int n;
	char *p;

	fp = fopen("Regions", "r");

	if (fp == NULL)
	{
		perror("can't read Regions");
		return;
	}

	while (fgets(buf, LINE_LEN, fp) != NULL)
	{
		for (p = buf; *p && *p != '\n'; p++)
			;
		*p = '\0';

		if (*buf == '\0')
			continue;

		sscanf(buf, "%d,%d", &row, &col);

		for (p = buf; *p && !iswhite(*p); p++)
			;
		while (iswhite(*p))
			p++;

		if (map[row][col]->inside)
		{
			fprintf(stderr, "collision between %s and %s at (%d,%d)\n",
				p, inside_names[map[row][col]->inside], row, col);
			continue;
		}

		ins = alloc_inside();
		inside_names[ins] = str_save(p);

		if (map[row][col]->terrain == terr_ocean)
		{
			water_count++;
			flood_water_inside(row, col, ins);
		}
		else
		{
			land_count++;
			flood_land_inside(row, col, ins);
		}
	}

	fclose(fp);

	fprintf(stderr,
		"set_regions: named %d land regions, %d water regions\n",
				land_count, water_count);

/*
 *  Now locate unnamed regions
 */

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		if (map[row][col] && map[row][col]->inside == 0)
		{
			ins = alloc_inside();

			if (map[row][col]->terrain == terr_ocean)
			{
				n = flood_water_inside(row, col, ins);
				fprintf(stderr, "\tunnamed sea at  %d,%d (%d locs)\n", row, col, n);
			}
			else
			{
				n = flood_land_inside(row, col, ins);
				fprintf(stderr, "\tunnamed land at %d,%d (%d locs)\n", row, col, n);
			}
		}

/*
 *  Now collect the list of provinces in each region
 */

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		if (map[row][col] && map[row][col]->inside)
		{
		    plist_append((plist *) &inside_list[map[row][col]->inside],
					map[row][col]);
		}
}


flood_land_inside(row, col, ins)
int row, col;
int ins;
{
	int dir;
	struct tile *p;
	int count = 1;

	map[row][col]->inside = ins;

	if (map[row][col]->region_boundary)
		return count;

	for (dir = 1; dir < MAX_DIR; dir++)
	{
		p = adjacent_tile_sup(row, col, dir);

		if (!p || p->terrain == terr_ocean)
			continue;

		if (p->inside == ins)
			continue;		/* already been here */

		if (p->inside)
		{
			fprintf(stderr, "flood_land_inside(%d,%d,%s) error\n",
				row, col, inside_names[ins]);
			assert(FALSE);
		}

		count += flood_land_inside(p->row, p->col, ins);
	}

	return count;
}


flood_water_inside(row, col, ins)
int row, col;
int ins;
{
	int dir;
	struct tile *p;
	int count = 1;

	map[row][col]->inside = ins;

	for (dir = 1; dir < MAX_DIR; dir++)
	{
		p = adjacent_tile_sup(row, col, dir);

		if (!p || p->color == -1 || p->color != map[row][col]->color)
			continue;

		if (p->inside == ins)
			continue;		/* already been here */

		if (p->inside)
		{
			fprintf(stderr, "flood_water_inside(%d,%d,%s) error\n",
				row, col, inside_names[ins]);
			assert(FALSE);
		}

		count += flood_water_inside(p->row, p->col, ins);
	}

	return count;
}


/*
 *  Name groups of provinces
 */

void
set_province_clumps()
{
	FILE *fp;
	char buf[LINE_LEN];
	int row, col;
	int count = 0;
	char *p;
	char type;

	fp = fopen("Land", "r");

	if (fp == NULL)
	{
		perror("can't read Land");
		return;
	}

	while (fgets(buf, LINE_LEN, fp) != NULL)
	{
		for (p = buf; *p && *p != '\n'; p++)
			;
		*p = '\0';
		sscanf(buf, "%d,%d %c", &row, &col, &type);

		for (p = buf; *p && !iswhite(*p); p++)
			;
		while (*p && iswhite(*p))
			p++;

		while (*p && !iswhite(*p))
			p++;
		while (iswhite(*p))
			p++;

		if (map[row][col]->save_char != type)
		{
			fprintf(stderr, "Land '%s' expects '%c' at (%d,%d), got '%c'\n",
				p, type, row, col, map[row][col]->save_char);
		}

		if (map[row][col]->name)
		{
			fprintf(stderr, "clump collision between %s and %s at (%d,%d)\n",
				p, map[row][col]->name, row, col);
		}

		flood_land_clumps(row, col, str_save(p));
		count++;
	}

	fclose(fp);

	fprintf(stderr,
		"set_province_clumps: named %d areas\n", count);
}


unnamed_province_clumps()
{
	int row, col;
	int n;

	fprintf(stderr, "Unnamed areas\n\n");

	for (row = 0; row < MAX_ROW; row++)
	    for (col = 0; col < MAX_COL; col++)
		if (map[row][col] && map[row][col]->name == NULL)
		{
			if (map[row][col]->terrain == terr_ocean)
				continue;

			n = flood_land_clumps(row, col, str_save("Unnamed"));

			if (map[row][col]->save_char != 'o')
				fprintf(stderr, "%d,%d\t%c\t%d unnamed\n",
					row, col, map[row][col]->save_char, n);
		}

	fprintf(stderr, "\n");
}


flood_land_clumps(row, col, name)
int row, col;
char *name;
{
	int dir;
	struct tile *p;
	int count = 1;

	map[row][col]->name = name;

	for (dir = 1; dir < MAX_DIR; dir++)
	{
		p = adjacent_tile_sup(row, col, dir);

		if (!p || p->terrain == terr_ocean || p->color == -1 ||
		    p->color != map[row][col]->color)
			continue;

		if (p->name == name)
			continue;		/* already been here */

		if (p->name)
		{
			fprintf(stderr, "flood_land_clumps(%d,%d,%s) error\n",
				row, col, name);
			assert(FALSE);
		}

		count += flood_land_clumps(p->row, p->col, name);
	}

	return count;
}



print_inside_sublocs(flag, row, col)
int flag;
int row;
int col;
{
	int i;
	int count = 0;

	for (i = 0; i < plist_len(map[row][col]->roads); i++)
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
	if (kind == terr_city)
		subloc[top_subloc]->region = rnd_alloc_num(CITY_LOW, CITY_HIGH);
	else
		subloc[top_subloc]->region = rnd_alloc_num(SUBLOC_LOW, SUBLOC_HIGH);
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
	subloc[top_subloc]->region = rnd_alloc_num(SUBLOC_LOW, SUBLOC_HIGH);
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
		    for (j = 0; j < plist_len(map[row][col]->roads); j++)
		    {
			fprintf(road_fp, "%d road 0\n",
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
		for (j = 0; j < plist_len(subloc[i]->roads); j++)
		{
			fprintf(road_fp, "%d road 0\n",
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
			fprintf(gate_fp, "%d gate 0\n",
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
			fprintf(gate_fp, "%d gate 0\n",
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


struct tile **
random_tile_from_each_region()
{
	static struct tile **l = NULL;
	int i, j;

	plist_clear((plist *) &l);

	for (i = 1; i <= inside_top; i++)
	{
		if (inside_list[i][0]->terrain == terr_ocean)
			continue;

		if (strcmp(inside_names[i], "Impassable Mountains") == 0)
			continue;

		do
		{
			j = rnd(0, plist_len(inside_list[i])-1);
		}
		while (inside_list[i][j]->safe_haven);

		plist_append((plist *) &l, inside_list[i][j]);
	}

	plist_scramble((plist) l);

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

	plist_clear((plist *) &other);

	for (i = 0; i < plist_len(l); i++)
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

		if (q == NULL || q->terrain == terr_ocean || q->safe_haven)
		{
			fprintf(stderr, "couldn't shift tour (%d,%d)\n",
						l[i]->row, l[i]->col);
			plist_append((plist *) &other, l[i]);
		} else
			plist_append((plist *) &other, q);
	}

	return other;
}


/*
 *  Gate laying plan
 *
 *	province->island->province	(gate_province_islands)
 *	province->province		(random_province_gates)
 *	continental tour		(gate_continental_tour)
 *
 *	each region gets a stone ring	(gate_stone_circles)
 *	each with links to 2 other
 *	rings, and 5 random provinces
 *
 *	7 rings of 5-12 provinces	(gate_land_ring)
 *	12 rings of 6-12 islands	(gate_link_islands)
 */

make_gates()
{

	gate_province_islands((land_count + 199) / 200);
	random_province_gates((land_count + 199) / 200);
	gate_continental_tour();
	gate_stone_circles();

	gate_land_ring((land_count + 749) / 750);
	gate_link_islands((num_islands + 149) / 150);		/* disjoint */
	gate_link_islands(num_islands / 450);		/* might cross */

#if 0
	show_gate_coverage();
#endif
}


new_gate(from, to, key)
struct tile *from;
struct tile *to;
int key;
{
	int gate_num;

	gate_num = rnd_alloc_num(SUBLOC_LOW, SUBLOC_HIGH);

	ilist_append(&from->gates_num, gate_num);
	ilist_append(&from->gates_dest, to->region);
	ilist_append(&from->gates_key, key);

/*
 *  Gather statistics:
 */

	inside_gates_from[map[from->row][from->col]->inside]++;
	inside_gates_to[map[to->row][to->col]->inside]++;
}


#if 0
show_gate_coverage()
{
	int i;

	fprintf(stderr, "\nGate coverage:  (in/out)\n");

	for (i = 1; i <= inside_top; i++)
	{
		if (inside_list[i][0]->terrain == terr_ocean)
			continue;

		fprintf(stderr, "\t%d/%d\t%s\n",
				inside_gates_to[i],
				inside_gates_from[i],
				inside_names[i]);
	}
}
#endif


random_province_gates(n)
int n;
{
	int r1, c1, r2, c2;
	int i;

	clear_province_marks();
	mark_bad_locs();

	for (i = 0; i < n; i++)
	{
		random_province(&r1, &c1, 0);
		random_province(&r2, &c2, 0);

		new_gate(map[r1][c1], map[r1][c2], 0);
	}
}


gate_province_islands(times)
int times;
{
	int i, j;
	int isle;
	int r1, c1, r2, c2;

	clear_province_marks();
	mark_bad_locs();
	clear_subloc_marks();

	for (j = 1; j <= times; j++)
	{
		random_province(&r1, &c1, 0);
		isle = random_island();
		random_province(&r2, &c2, 0);

		new_gate(map[r1][c1], subloc[isle], 0);
		new_gate(subloc[isle], map[r2][c2], 0);
	}
}


gate_continental_tour()
{
	int i;
	struct tile **l;
	struct tile **m;

	l = random_tile_from_each_region();
	m = shift_tour_endpoints(l);

	assert(plist_len(l) == plist_len(m));

	fprintf(stderr, "\nContinental gate tour:\n");

	for (i = 0; i < plist_len(l)-1; i++)
	{
		fprintf(stderr, "\t(%2d,%2d) -> (%2d,%2d)\n",
			l[i]->row, l[i]->col, m[i+1]->row, m[i+1]->col);

		new_gate(l[i], m[i+1], 0);
	}

	fprintf(stderr, "\t(%2d,%2d) -> (%2d,%2d)\n\n",
		l[i]->row, l[i]->col, m[0]->row, m[0]->col);

	new_gate(l[i], m[0], rnd(111,333));
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
		num = rnd(5, 10);

		first = random_island();
		n = first;

		for (i = 1; i < num; i++)
		{
			next = random_island();
			new_gate(subloc[n], subloc[next], 0);
			n = next;
		}

		new_gate(subloc[n], subloc[first], 0);
	}
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
	mark_bad_locs();

	for (j = 1; j <= rings; j++)
	{
		num = rnd(5, 10);
		random_province(&r_first, &c_first, 0);

		r_n = r_first;
		c_n = c_first;

		for (i = 1; i < num; i++)
		{
			random_province(&r_next, &c_next, 0);
			new_gate(map[r_n][c_n], map[r_next][c_next], 0);

			r_n = r_next;
			c_n = c_next;
		}

		new_gate(map[r_n][c_n], map[r_first][c_first], 0);
	}
}


struct tile *
choose_random_stone_circle(l, avoid1, avoid2)
struct tile **l;
struct tile *avoid1;
struct tile *avoid2;
{
	int i;

	do
	{
		i = rnd(0, plist_len(l)-1);
	}
	while (l[i] == avoid1 || l[i] == avoid2);

	return l[i];
}


/*
 *  every region gets a hidden stone circle
 *  Each stone circle has two gates to other stone circles,
 *	chosen at random, and five gates to random provinces
 */

gate_stone_circles()
{
	struct tile **l;
	int i, j;
	int n;
	static struct tile **circs = NULL;
	struct tile *first;
	struct tile *second;
	int row, col;

	l = random_tile_from_each_region();
	plist_clear((plist *) &circs);

	fprintf(stderr, "\nRing of stones:\n");

	for (i = 0; i < plist_len(l); i++)
	{
		n = create_a_subloc(l[i]->row, l[i]->col, 1, terr_stone_cir);
		plist_append((plist *) &circs, subloc[n]);

		fprintf(stderr, "	(%2d,%2d) in %s\n",
				l[i]->row, l[i]->col,
				inside_names[l[i]->inside]);
	}

	for (i = 0; i < plist_len(circs); i++)
	{
		first = choose_random_stone_circle(circs, circs[i], NULL);
		second = choose_random_stone_circle(circs, circs[i], first);

		new_gate(circs[i], first, rnd(111,333));
		new_gate(circs[i], second, rnd(111,333));
	}

	clear_province_marks();
	mark_bad_locs();

	for (i = 0; i < plist_len(circs); i++)
	{
		for (j = 1; j <= 5; j++)
		{
			random_province(&row, &col, 0);
			new_gate(circs[i], map[row][col],
					rnd(0,1) ? 0 : rnd(111,333));
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


mark_bad_locs()
{
	int i, j, r, c;

	for (i = 1; i <= inside_top; i++)
		if (strcmp(inside_names[i], "Impassable Mountains") == 0)
		{
			for (j = 0; j < plist_len(inside_list[i]); j++)
				inside_list[i][j]->mark = 1;
		}

	/* don't put gates in locations where magic can't be used */
	for (r = 0; r <= max_row; r++)
		for (c = 0; c < max_col; c++)
			if (map[r][c] && map[r][c]->safe_haven)
				map[r][c]->mark = 1;
}


/*
 *  The 'not' refers to not desert and not swamp
 *  We don't want to make any cities there
 *  (Except for the lost city and the city of the ancients)
 */

void
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

void
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


struct {
	int skill;
	int weight;
	char *name;
} guild_names[] = {

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

	{terr_battlefield,3,	"Old battlefield"},
	{terr_battlefield,1,	"Ancient battlefield"},
	{terr_battlefield,1,	NULL},

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


count_cities()
{
	int i;
	int row, col, ins;

	for (i = 1; i <= top_subloc; i++)
		if (subloc[i]->terrain == terr_city)
		{
			row = subloc[i]->row;
			col = subloc[i]->col;
			ins = map[row][col]->inside;

			inside_num_cities[ins]++;
		}
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


struct {
	int terr;		/* terrain appropriate */
	int kind;		/* what to make there */
	int weight;		/* weight given to selection */
	int hidden;		/* 0=no, 1=yes, 2=rnd(0,1) */
} loc_table[] = {

	{terr_desert,		terr_cave,			10,	1},
	{terr_desert,		terr_oasis,			10,	2},
	{terr_desert,		terr_sand_pit,		10,	2},

	{terr_mountain,		terr_ruins,			10,	1},
	{terr_mountain,		terr_cave,			10,	1},
	{terr_mountain,		terr_yew_grove,		8,	2},
	{terr_mountain,		terr_tree_cir,		5,	1},
	{terr_mountain,		terr_pits,			5,	1},
	{terr_mountain,		terr_lair,			10,	2},
	{terr_mountain,		terr_battlefield,	6,	2},

	{terr_swamp,		terr_bog,			10,	2},
	{terr_swamp,		terr_pits,			10,	2},
	{terr_swamp,		terr_battlefield,	6,	2},
	{terr_swamp,		terr_lair,			5,	2},

	{terr_forest,		terr_ruins,			10,	1},
	{terr_forest,		terr_rocky_hill,	10,	0},
	{terr_forest,		terr_tree_cir,		10,	1},
	{terr_forest,		terr_ench_for,		8,	1},
	{terr_forest,		terr_pasture,		5,	0},
	{terr_forest,		terr_yew_grove,		10,	2},
	{terr_forest,		terr_cave,			10,	1},
	{terr_forest,		terr_grove,			9,	1},
	{terr_forest,		terr_battlefield,	6,	2},
	{terr_forest,		terr_lair,			3,	1},

	{terr_plain,		terr_ruins,			10,	1},
	{terr_plain,		terr_pasture,		10,	0},
	{terr_plain,		terr_rocky_hill,	10,	0},
	{terr_plain,		terr_sac_grove,		6,	2},
	{terr_plain,		terr_tree_cir,		6,	1},
	{terr_plain,		terr_pop_field,		10,	0},
	{terr_plain,		terr_cave,			10,	1},
	{terr_plain,		terr_battlefield,	6,	2},

	{0,0,0,0}
};

void
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


create_a_graveyard(row, col)
int row;
int col;
{
	int n;
	char *s;

	n = create_a_subloc(row, col, rnd(0,1), terr_grave);
	s = name_guild(terr_grave);

	if (s && *s)
		subloc[n]->name = str_save(s);
}


make_graveyards()
{
	int i, j;
	struct tile *p;
	int n;
	struct tile **l;

	for (i = 1; i <= inside_top; i++)
	{
		p = inside_list[i][0];

		if (p->terrain == terr_ocean)
			continue;

		n = plist_len(inside_list[i]);

		if (n < 10)
			continue;

		l = (struct tile **) plist_copy((plist) inside_list[i]);
		plist_scramble((plist) l);

		for (j = 0; j < n/10; j++)
			create_a_graveyard(l[j]->row, l[j]->col);

		plist_reclaim((plist *) &l);
	}
}


create_a_city(row, col, name, major)
int row;
int col;
char *name;
int major;
{
	int n;
	static FILE *fp = NULL;
	int dont_try = FALSE;
	char *line;

	if (!dont_try && fp == NULL)
	{
		fp = fopen("Cities", "r");
		if (fp == NULL)
		{
			perror("can't open Cities");
			dont_try = TRUE;
		}
	}

	if (name == NULL && fp && (line = getlin(fp)))
		name = str_save(line);

	n = create_a_subloc(row, col, 0, terr_city);

	subloc[n]->name = name;
	subloc[n]->major_city = major;

	return n;
}


#if 1
place_sublocations()
{
	int row, col;
	int n;
	int i;

	ilist l = NULL;

	for (row = 0; row < MAX_ROW; row++)
		for (col = 0; col < MAX_COL; col++)
			if (map[row][col] && map[row][col]->terrain != terr_ocean)
				ilist_append((ilist *) &l, row*1000+col);

	ilist_scramble(l);

	for (i = 0; i < ilist_len(l); i++)
	{
		row = l[i] / 1000;
		col = l[i] % 1000;

	/*
	 *  Put a city everywhere there is a * or every 1 in 15 locs,
	 *  randomly.  Don't put one where there already is a city (city != 2).
	 */
		if (map[row][col]->city == 1 || (rnd(1,15) == 1 && map[row][col]->city != 2))
		{
			create_a_city(row, col, NULL, FALSE);
		}

		if (rnd(1,100) <= 35)
			make_appropriate_subloc(row, col, 0);
		if (rnd(1,100) <= 35)
			make_appropriate_subloc(row, col, 0);
		if (rnd(1,100) <= 35)
			make_appropriate_subloc(row, col, 0);
	}

	ilist_reclaim((ilist *) &l);
}
#else
place_sublocations()
{
    int row, col;
    int n;
    int i;

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
			create_a_city(row, col, NULL, FALSE);
		}

		if (rnd(1,3) == 1)
			make_appropriate_subloc(row, col, 0);
		if (rnd(1,3) == 1)
			make_appropriate_subloc(row, col, 0);
	    }
}
#endif


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

