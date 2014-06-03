/*
 * Add a randomly generated island to the map supplied as stdin.
 * The map is assumed to be one suitable for use by mapgen.
 * A border 2 squares wide is preserved around the edge of the
 * map, but this can be changed with the -b parameter.
 * Existing land masses will have a 3 square continental shelf
 * maintained; this can be changed with the -c parameter.
 * The size of the island can be specified with the -s
 * parameter, but if this is not given then a random size
 * will be chosen using the sum of two poisson distributions
 * with mean 100 each, for a slightly normalised distribution
 * with mean 200.
 * The input map must be square; a simple way to start a new
 * map is to start with a file containing the target size of
 * water characters ("." is nicely visible) and feeding it
 * through a pipeline of this executable, repeated the number
 * of times you want islands.
 * For example:
 *   island < empty_map | island | island | island | island
 */

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>

#define MAX_MAP			120
#define LINE_MAX		200
#define DISTANCE_CAP		9

typedef struct
{
	int x, y;
} location;

typedef struct
{
	char symbol;
	int min, max;
	int target_prob, prob;
} terrain;

terrain terrains[] =
{
	{ 'p', 12, 30, 30, 0 },
	{ 'f',  6, 14, 30, 0 },
	{ 'm',  6, 10, 20, 0 },
	{ 'd', 15, 30, 10, 0 },
	{ 's',  1,  3, 10, 0 }
};

void make_shelf(
	char map[][MAX_MAP],
	int y,
	int x,
	int y_size,
	int x_size,
	int shelf)
{
	if (map[y][x] == '~')
		map[y][x] = '_';
	if (shelf < 1)
		return;
	
	if (y > 0)
		make_shelf(map, y - 1, x, y_size, x_size, shelf - 1);
	if (y < y_size - 1)
		make_shelf(map, y + 1, x, y_size, x_size, shelf - 1);
	if (x > 0)
		make_shelf(map, y, x - 1, y_size, x_size, shelf - 1);
	if (x < x_size - 1)
		make_shelf(map, y, x + 1, y_size, x_size, shelf - 1);

	return;
}

void extend_distance(
	char distance[][MAX_MAP],
	int y,
	int x,
	int y_size,
	int x_size)
{
	if (y > 0 && distance[y - 1][x] > distance[y][x] + 1)
	{
		distance[y - 1][x] = distance[y][x] + 1;
		extend_distance(distance, y - 1, x, y_size, x_size);
	}
	if (y < y_size - 1 && distance[y + 1][x] > distance[y][x] + 1)
	{
		distance[y + 1][x] = distance[y][x] + 1;
		extend_distance(distance, y + 1, x, y_size, x_size);
	}
	if (x > 0 && distance[y][x - 1] > distance[y][x] + 1)
	{
		distance[y][x - 1] = distance[y][x] + 1;
		extend_distance(distance, y, x - 1, y_size, x_size);
	}
	if (x < x_size - 1 && distance[y][x + 1] > distance[y][x] + 1)
	{
		distance[y][x + 1] = distance[y][x] + 1;
		extend_distance(distance, y, x + 1, y_size, x_size);
	}

	return;
}

int gcd(int a, int b)
{
	int temp;

	while (b)
	{
		temp = b;
		b = a % b;
		a = temp;
	}

	return a;
}

int lcm(int a, int b)
{
	return a * b / gcd(a, b);
}

int main(int argc, char *argv[])
{
	char map[MAX_MAP][MAX_MAP];
	char working[MAX_MAP][MAX_MAP];
	char distance[MAX_MAP][MAX_MAP];
	int ids[MAX_MAP][MAX_MAP];
	char buffer[LINE_MAX];
	location *island;
	int opt, x_size, y_size, x, y, max, count, d, island_size, i;
	int GCD, LCM, o, terr, temp, size, cluster_end, id;
	int target_size = 0;
	int border = 2;
	int shelf = 3;

	opterr = 0;
	while ((opt = getopt(argc, argv, "b:c:s:")) != -1)
		switch (opt)
		{
			case 'b':
				border = atoi(optarg);
				break;
			case 'c':
				shelf = atoi(optarg);
				break;
			case 's':
				target_size = atoi(optarg);
				break;
			default:
				fprintf(stderr, "Usage: %s -b border_width -c continental_shelf_width -s size\n", argv[0]);
				exit(1);
		}

	if (border < 0)
		border = 0;

	if (shelf < 0)
		shelf = 0;

	x_size = y_size = 0;

	/* Read the starting map from stdin */
	while (fgets(buffer, LINE_MAX, stdin))
	{
		if (sscanf(buffer, "%[^\r\n]", map[y_size]))
		{
			if (!x_size)
				x_size = strlen(map[y_size]);
			if (strlen(map[y_size]) == x_size)
				y_size++;
		}
	}

	load_seed("randseed");

	/* Divide the map into available and unavailable space */
	/* Only count ,. ' chars as available */
	for (y = 0; y < y_size; y++)
		for (x = 0; x < x_size; x++)
		{
			switch (map[y][x])
			{
				case ',':
				case '.':
				case ' ':
				case '\'':
					working[y][x] = '~';
					break;
				default:
					working[y][x] = 'p';
					break;
			}
			ids[y][x] = 0;
		}
	/* exclude any square within shelf of a land square */
	for (y = 0; y < y_size; y++)
		for (x = 0; x < x_size; x++)
			if (working[y][x] == 'p')
				make_shelf(working, y, x, y_size, x_size, shelf);
	/* exclude border squares around each edge */
	for (y = 0; y < y_size; y++)
		for (x = 0; x < border; x++)
		{
			if (working[y][x] == '~')
				working[y][x] = '_';
			if (working[y][x_size - x - 1] == '~')
				working[y][x_size - x - 1] = '_';
		}
	for (x = 0; x < x_size; x++)
		for (y = 0; y < border; y++)
		{
			if (working[y][x] == '~')
				working[y][x] = '_';
			if (working[y_size - y - 1][x] == '~')
				working[y_size - y - 1][x] = '_';
		}
	/* Calculate distance from land */
	for (y = 0; y < y_size; y++)
		for (x = 0; x < x_size; x++)
		{
			if (working[y][x] == '~')
				distance[y][x] = DISTANCE_CAP;
			else
				distance[y][x] = 0;
		}
	for (y = 0; y < y_size; y++)
		for (x = 0; x < x_size; x++)
			extend_distance(distance, y, x, y_size, x_size);

	if (target_size < 1)
	{
		/* Decide how big to make the island */
		for (i = 0; i < 2; i++)
			while (rnd(0, 99))
				target_size++;
	}
	island = malloc((target_size + 1) * sizeof(*island));
	if (!island) {
		fprintf(stderr, "Unable to allocate memory for island data!\n");
		exit(1);
	}
	/* select a square to start the new island */
	max = 0;
	count = 0;
	island_size = 0;
	for (y = 0; y < y_size; y++)
		for (x = 0; x < x_size; x++)
		{
			if (distance[y][x] > max)
			{
				max = distance[y][x];
				count = 1;
			}
			else if (distance[y][x] == max)
			{
				count++;
			}
		}
	d = rnd(1, count);
	for (y = 0; d > 0 && y < y_size; y++)
		for (x = 0; d > 0 && x < x_size; x++)
			if (distance[y][x] == max)
			{
				d--;
				if (!d)
				{
					island[0].x = x;
					island[0].y = y;
					island_size = 1;
					working[y][x] = 'o';
					map[y][x] = 'o';
					ids[y][x] = 0;
				}
			}
	while (island_size < target_size)
	{
		/* Find all the possible expansions */
		/* Count multiple access paths multiple times to */
		/* make the island more likely to fill interior space */
		count = 0;
		for (i = 0; i < island_size; i++)
		{
			if (island[i].x > 0 && working[island[i].y][island[i].x - 1] == '~')
				count++;
			if (island[i].x < x_size - 1 && working[island[i].y][island[i].x + 1] == '~')
				count++;
			if (island[i].y > 0 && working[island[i].y - 1][island[i].x] == '~')
				count++;
			if (island[i].y < y_size - 1 && working[island[i].y + 1][island[i].x] == '~')
				count++;
		}
		if (count < 1)
		{
			fprintf(stderr, "Not enough room to expand island!\n");
			break;
		}
		d = rnd(0, count - 1);
		for (i = 0; i < island_size; i++)
		{
			if (island[i].x > 0 && working[island[i].y][island[i].x - 1] == '~')
				if (!d--)
				{
					island[island_size].x = island[i].x - 1;
					island[island_size].y = island[i].y;
				}
			if (island[i].x < x_size - 1 && working[island[i].y][island[i].x + 1] == '~')
				if (!d--)
				{
					island[island_size].x = island[i].x + 1;
					island[island_size].y = island[i].y;
				}
			if (island[i].y > 0 && working[island[i].y - 1][island[i].x] == '~')
				if (!d--)
				{
					island[island_size].x = island[i].x;
					island[island_size].y = island[i].y - 1;
				}
			if (island[i].y < y_size - 1 && working[island[i].y + 1][island[i].x] == '~')
				if (!d--)
				{
					island[island_size].x = island[i].x;
					island[island_size].y = island[i].y + 1;
				}
		}
		working[island[island_size].y][island[island_size].x] = 'o';
		map[island[island_size].y][island[island_size].x] = 'o';
		ids[island[island_size].y][island[island_size].x] = island_size;
		island_size++;
	}

	/* Calculate terrain ratios */
	/* Use GCD and LCM to render these down to integer ratios */
	for (i = 0; i < sizeof(terrains) / sizeof(terrains[0]); i++)
	{
		terrains[i].prob = terrains[i].min + terrains[i].max;
		GCD = gcd(terrains[i].target_prob, terrains[i].prob);
		terrains[i].target_prob /= GCD;
		terrains[i].prob /= GCD;
	}
	for (i = 0, LCM = 1; i < sizeof(terrains) / sizeof(terrains[0]); i++)
		LCM = lcm(LCM, terrains[i].prob);
	for (i = 0, count = 0; i < sizeof(terrains) / sizeof(terrains[0]); i++)
	{
		terrains[i].target_prob *= LCM / terrains[i].prob;
		count += terrains[i].target_prob;
	}

	o = island_size;
	size = 0;
	while (o > 0)
	{
		/* pick a random square on the island */
		cluster_end = o;
		o--;
		d = rnd(0, o);
		/* Move that square to the end of the list */
		temp = island[d].x;
		island[d].x = island[o].x;
		island[o].x = temp;
		temp = island[d].y;
		island[d].y = island[o].y;
		island[o].y = temp;
		ids[island[o].y][island[o].x] = o;
		ids[island[d].y][island[d].x] = d;
		if (size < 1)
		{
			/* Pick a random terrain type */
			d = rnd(1, count);
			for (terr = 0; d > terrains[terr].target_prob; terr++)
				d -= terrains[terr].target_prob;
			/* Determine how big this cluster should be */
			size = rnd(terrains[terr].min, terrains[terr].max);
		}
		/* set the first square to this terrain type */
		map[island[o].y][island[o].x] = terrains[terr].symbol;
		working[island[o].y][island[o].x] = terrains[terr].symbol;
		size--;
		if (size > o)
			size = o;
		while (size > 0)
		{
			for (i = o, opt = 0; i < cluster_end; i++)
			{
				if (island[i].y > 0 && working[island[i].y - 1][island[i].x] == 'o')
					opt++;
				if (island[i].y < y_size - 1 && working[island[i].y + 1][island[i].x] == 'o')
					opt++;
				if (island[i].x > 0 && working[island[i].y][island[i].x - 1] == 'o')
					opt++;
				if (island[i].x < x_size - 1 && working[island[i].y][island[i].x + 1] == 'o')
					opt++;
			}
			if (opt < 1)
				break;
			d = rnd(0, opt - 1);
			for (i = o; i < cluster_end; i++)
			{
				if (island[i].y > 0 && working[island[i].y - 1][island[i].x] == 'o')
					if (!d--)
					{
						id = ids[island[i].y - 1][island[i].x];
						break;
					}
				if (island[i].y < y_size - 1 && working[island[i].y + 1][island[i].x] == 'o')
					if (!d--)
					{
						id = ids[island[i].y + 1][island[i].x];
						break;
					}
				if (island[i].x > 0 && working[island[i].y][island[i].x - 1] == 'o')
					if (!d--)
					{
						id = ids[island[i].y][island[i].x - 1];
						break;
					}
				if (island[i].x < x_size - 1 && working[island[i].y][island[i].x + 1] == 'o')
					if (!d--)
					{
						id = ids[island[i].y][island[i].x + 1];
						break;
					}
			}
			o--;
			size--;
			temp = island[id].x;
			island[id].x = island[o].x;
			island[o].x = temp;
			temp = island[id].y;
			island[id].y = island[o].y;
			island[o].y = temp;
			map[island[o].y][island[o].x] = terrains[terr].symbol;
			working[island[o].y][island[o].x] = terrains[terr].symbol;
			ids[island[o].y][island[o].x] = o;
			ids[island[id].y][island[id].x] = id;
		}
	}

	save_seed("randseed");

	fprintf(stderr, "Added island of %d provinces.\n", island_size);

	/* Print the map with the added island */
	for (y = 0; y < y_size; y++)
		printf("%s\n", map[y]);

	return 0;
}

