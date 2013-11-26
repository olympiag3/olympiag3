
#include	<stdio.h>
#include	<stdlib.h>
#include	"z.h"
#include	"oly.h"


/*
 *  10%  9
 *  40%  6
 *  40%  3
 *  10%  0
 */

static int
choose_city_prominence(int city)
{
	int n;

	if (safe_haven(city) || major_city(city))
		return 3;

	if (loc_hidden(city) || loc_hidden(province(city)))
		return 0;

	n = rnd(1,100);

	if (n <= 10)
		return 0;
	if (n <= 50)
		return 1;
	if (n <= 90)
		return 2;
	return 3;
}


static void
add_near_city(int where, int city)
{
	struct entity_subloc *p;

	p = p_subloc(where);

	ilist_append(&p->near_cities, city);
}


void
prop_city_near_list(int city)
{
	int prom;
	int m;
	int i;
	int n;
	int dest;
	int where;
	struct exit_view **l;

	clear_temps(T_loc);

	bx[province(city)]->temp = 1;
	prom = choose_city_prominence(city);
	p_subloc(city)->prominence = prom;
	prom *= 3;

	for (m = 1; m < prom; m++)
	{
		loop_loc(where)
		{
			if (bx[where]->temp != m)
				continue;

			l = exits_from_loc_nsew(0, where);

			for (i = 0; i < ilist_len(l); i++)
			{
				dest = l[i]->destination;

				if (loc_depth(dest) != LOC_province)
					continue;

				if (bx[dest]->temp == 0)
				{
					bx[dest]->temp = m + 1;
					if (n = city_here(dest))
						add_near_city(n, city);
				}
			}
		}
		next_loc;
	}
}


void
seed_city_near_lists()
{
	int city;

	stage("INIT: seed_city_near_lists()");

	loop_city(city)
	{
		ilist_clear(&p_subloc(city)->near_cities);
	}
	next_city;

	loop_city(city)
	{
		prop_city_near_list(city);
	}
	next_city;
}


void
seed_mob_cookies()
{
	int i;

	loop_loc(i)
	{
		if (subkind(i) != sub_city && loc_depth(i) != LOC_province)
			continue;

		if (subkind(i) == sub_ocean)
			continue;

		gen_item(i, item_mob_cookie, 1);
	}
	next_loc;
}


void
seed_undead_cookies()
{
	int i;

	loop_loc(i)
	{
		if (subkind(i) != sub_graveyard)
			continue;

		gen_item(i, item_undead_cookie, 1);
	}
	next_loc;
}


void
seed_weather_cookies()
{
	int i;

	loop_loc(i)
	{
		switch (subkind(i))
		{
		case sub_forest:
			gen_item(i, item_rain_cookie, 1);
			gen_item(i, item_fog_cookie, 1);
			break;

		case sub_plain:
		case sub_desert:
		case sub_mountain:
			gen_item(i, item_wind_cookie, 1);
			break;

		case sub_swamp:
			gen_item(i, item_fog_cookie, 1);
			break;

		case sub_ocean:
		case sub_cloud:
			gen_item(i, item_fog_cookie, 1);
			gen_item(i, item_wind_cookie, 1);
			gen_item(i, item_rain_cookie, 1);
			break;
		}
	}
	next_loc;
}


void
seed_cookies()
{

	stage("INIT: seed_cookies()");

	seed_mob_cookies();
	seed_undead_cookies();
	seed_weather_cookies();
}


/*
 *  Could be speeded up by saving the return from province_gate_here()
 *  in some temp field.  But this routine is only run once, when a new
 *  database is first read in, so it probably doesn't matter.
 */

void
compute_dist_gate()
{
	int where;
	struct exit_view **l;
	int set_one;
	int i;
	int dest;
	int m;

	clear_temps(T_loc);

	loop_province(where)
	{
		if (!province_gate_here(where))
			continue;

		l = exits_from_loc_nsew(0, where);

		for (i = 0; i < ilist_len(l); i++)
		{
			if (loc_depth(l[i]->destination) != LOC_province)
				continue;

			if (!province_gate_here(l[i]->destination))
			{
				bx[l[i]->destination]->temp = 1;
			}
		}
	}
	next_province;

	m = 1;

	do
	{
		set_one = FALSE;

		loop_province(where)
		{
			if (province_gate_here(where) || bx[where]->temp != m)
				continue;

			l = exits_from_loc_nsew(0, where);

			for (i = 0; i < ilist_len(l); i++)
			{
				dest = l[i]->destination;

				if (loc_depth(dest) != LOC_province)
					continue;

				if (!province_gate_here(dest) && bx[dest]->temp == 0)
				{
					bx[dest]->temp = m + 1;
					set_one = TRUE;
				}
			}
		}
		next_province;

		m++;
	}
	while (set_one);

	loop_province(where)
	{
		if (!province_gate_here(where) &&
			bx[where]->temp < 1 &&
			greater_region(where) == 0)
			fprintf(stderr, "2: error on %d reg=%d\n",
				where, region(where));
	}
	next_province;
}


void
compute_dist()
{
	int i;

	stage("INIT: compute_dist()");

	compute_dist_gate();

	loop_province(i)
	{
		p_loc(i)->dist_from_gate = bx[i]->temp;
	}
	next_province;
}


int
int_comp(void *a, void *b)
{

	return *(int *)a - *(int *)b;
}


static void
seed_city_skill(int where)
{
	int terr = subkind(province(where));
	struct entity_subloc *p;
	int common, magic;

	p = p_subloc(where);

	ilist_clear(&p->teaches);

/*
 *  Skills taught everywhere
 */

	ilist_append(&p->teaches, sk_combat);
	ilist_append(&p->teaches, sk_construction);

/*
 *  Skills based on location
 */

	if (safe_haven(where))
	{
		common = 1;
		magic = 1;
	}
	else
	{
		common = rnd(1, 4);
		magic = rnd(1, 8);
	}

	if (in_faery(where))
	{
		common = rnd(2, 4);
		magic = 2;
	}
	else if (in_clouds(where))
	{
		magic = 3;
	}
	else if (in_hades(where))
	{
		common = 4;
		magic = 4;
		if (!rnd(0, 2))
			ilist_append(&p->teaches, sk_artifact);
	}

	switch (common)
	{
		case 1: ilist_append(&p->teaches, sk_trade); break;
		case 2: ilist_append(&p->teaches, sk_stealth); break;
		case 3: ilist_append(&p->teaches, sk_persuasion); break;
		default: break;
	}
	switch (magic)
	{
		case 1: ilist_append(&p->teaches, sk_gate); break;
		case 2: ilist_append(&p->teaches, sk_scry); break;
		case 3: ilist_append(&p->teaches, sk_weather); break;
		case 4: ilist_append(&p->teaches, sk_necromancy); break;
		case 5: ilist_append(&p->teaches, sk_artifact); break;
		case 6: ilist_append(&p->teaches, sk_basic); break;
		case 7: ilist_append(&p->teaches, sk_alchemy); break;
		default: break;
	}
	if (magic < 6)
		ilist_append(&p->teaches, sk_basic);

	if (is_port_city(where))
		ilist_append(&p->teaches, sk_shipcraft);

	switch (terr)
	{
		case sub_plain: ilist_append(&p->teaches, sk_beast); break;
		case sub_mountain: ilist_append(&p->teaches, sk_mining); break;
		case sub_forest: ilist_append(&p->teaches, sk_forestry); break;
	}

	if (ilist_len(p->teaches) > 0)
		qsort(p->teaches, ilist_len(p->teaches), sizeof(int), int_comp);
}


void
seed_city_trade(int where)
{
	int qty = 0;
	int cst = 0;
	int prov = province(where);
	int prov_kind = subkind(prov);
	struct entity_subloc *p = rp_subloc(where);

	clear_all_trades(where);

	if (in_hades(where))
	{
		return;
	}

	if (in_clouds(where))
	{
		add_city_trade(where, CONSUME, item_basket, 30, 4, 0);
		add_city_trade(where, PRODUCE, item_pegasus, 1, 1000, 0);

		loc_trade_sup(where, TRUE);
		return;
	}

	if (rnd(1,2) == 1)
		add_city_trade(where, CONSUME, item_pot, 17, 7, 0);
	else
		add_city_trade(where, CONSUME, item_basket, 30, 4, 0);

	if (in_faery(where))		/* seed Faery city trade */
	{
		if (rnd(1,2) == 1)
			add_city_trade(where, PRODUCE, item_lana_bark, 3, 50, 0);
		else
			add_city_trade(where, PRODUCE, item_avinia_leaf, 10, 35, 0);

		if (rnd(1,2) == 1)
			add_city_trade(where, PRODUCE, item_yew, 5, 100, 0);
		else
			add_city_trade(where, PRODUCE, item_mallorn_wood, 5, 200, 0);


		add_city_trade(where, CONSUME, item_mithril, 10, 500, 0);

		if (rnd(1,2) == 1)
			add_city_trade(where, CONSUME, item_gate_crystal, 2, 1000, 0);

		if (rnd(1,2) == 1)
			add_city_trade(where, PRODUCE, item_pegasus, 1, 1000, 0);

		loc_trade_sup(where, TRUE);
		return;
	}

	if (is_port_city(where))
	{
		add_city_trade(where, CONSUME, item_fish, 100, 2, 0);
		add_city_trade(where, PRODUCE, item_glue, 10, 50, 0);
	}

	if (prov_kind == sub_plain)
	{
		add_city_trade(where, PRODUCE, item_ox, 5, 100, 0);
		qty = rnd(2,3);
		cst = rnd(20,30);
		add_city_trade(where, PRODUCE, item_riding_horse, qty, cst * 5, 0);
		add_city_trade(where, CONSUME, item_riding_horse, qty, cst * 5 / 2, 0);
	}
	else if (rnd(1,3) == 1)
		add_city_trade(where, CONSUME, item_hide, rnd(3,6),
						rnd(125,135), 0);

	if (prov_kind == sub_mountain)
	{
		qty = rnd(1,2);
		cst = rnd(25,30);
		add_city_trade(where, PRODUCE, item_iron, qty, cst, 0);
		add_city_trade(where, CONSUME, item_iron, qty, cst / 2, 0);
	}

	if (prov_kind == sub_forest)
	{
		cst = rnd(11,15);
		add_city_trade(where, PRODUCE, item_lumber, 25, cst, 0);
		add_city_trade(where, CONSUME, item_lumber, 25, cst / 2, 0);
	}

	if (p && ilist_lookup(p->teaches, sk_alchemy) >= 0)
		add_city_trade(where, PRODUCE, item_lead, 50, 1, 0);

	loc_trade_sup(where, TRUE);
}


void
seed_city(int where)
{

	seed_city_skill(where);
	seed_city_trade(where);
}


void
seed_initial_locations()
{

	int i;

	loop_city(i)
	{
		seed_city(i);
	}
	next_city;

	loop_city(i)
	{
		loc_trade_sup(i, TRUE);
	}
	next_city;
}


static void
add_city_garrisons()
{
	int where;
	int garr;

	loop_city(where)
	{
		if (safe_haven(where) || greater_region(where) != 0)
			continue;

		garr = new_province_garrison(where, 0, item_pikeman, rnd(25,150));
		p_magic(garr)->default_garr = TRUE;
	}
	next_city;
}


void
seed_phase_two()
{
	compute_dist();
	seed_city_near_lists();
	seed_cookies();
	add_city_garrisons();
	close_logfile();
}


void
seed_taxes()
{
	int where;
	int base;
	int pil;

	loop_loc(where)
	{
		if (loc_depth(where) != LOC_province &&
			subkind(where) != sub_city)
			continue;

		if (subkind(where) == sub_ocean)
			continue;

		if (subkind(where) == sub_city)
		{
			consume_item(where, item_petty_thief,
					has_item(where, item_petty_thief));

			gen_item(where, item_petty_thief, 1);
		}

/*
 *  Magician menial labor cookies
 */

		consume_item(where, item_mage_menial,
					has_item(where, item_mage_menial));

		consume_item(where, item_tax_cookie,
					has_item(where, item_tax_cookie));

		gen_item(where, item_mage_menial,
			loc_civ(province(where)) * 5);

		assert(has_item(where, item_tax_cookie) == 0);

/*
 *  Tax base of province is equal to civilization level there
 */

		if (subkind(where) == sub_city)
			base = 100;
		else
			base = 50 + loc_civ(province(where)) * 50;

/*
 *  Each point of loc_opium reduces tax base by 10%
 */

		base -= base/10 * loc_opium(where);

		assert(base > 0);

		if (pil = loc_pillage(where))
			base /= (pil + 1);

		gen_item(where, item_tax_cookie, base);
	}
	next_loc;
}

