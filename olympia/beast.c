
#include	<stdio.h>
#include 	<stdlib.h>
#include	"z.h"
#include	"oly.h"


int
v_bird_spy(struct command *c)
{
	int targ = c->a;
	int where = subloc(c->who);
	struct exit_view *v;

	if (is_ship(where))
		where = loc(where);

	if (numargs(c) < 1)
	{
		wout(c->who, "Specify what location the bird should spy on.");
		return FALSE;
	}

	if (!is_loc_or_ship(c->a))
	{
		v = parse_exit_dir(c, where, sout("use %d", sk_bird_spy));

		if (v == NULL)
			return FALSE;

		targ = v->destination;
	}

	if (province(targ) != province(c->who))
	{
		struct exit_view **l;
		int i;
		int okay = FALSE;

		l = exits_from_loc(c->who, province(c->who));

		for (i = 0; i < ilist_len(l); i++)
			if (l[i]->destination == targ)
				okay = TRUE;

		if (!okay)
		{
			wout(c->who, "The location to be spied upon must be "
				"a sublocation in the same province or a "
				"neighboring location.");
			return FALSE;
		}
	}

	c->d = targ;

	return TRUE;
}


int
d_bird_spy(struct command *c)
{
	int targ = c->d;

	if (!is_loc_or_ship(targ))
	{
		wout(c->who, "%s is not a location.", box_code(targ));
		return FALSE;
	}

	wout(c->who, "The bird returns with a report:");
	out(c->who, "");
	show_loc(c->who, targ);

	return TRUE;
}


/*
 *  Default is that species is compatible with itself,
 *  unless an explicit {self, self, 0} is given.
 */

struct breed {
	int i1, i2;
	int result;
}
breed_tbl[] =
{
    {item_peasant,	item_ox,		item_minotaur},
    {item_peasant,	item_wild_horse,	item_centaur},
    {item_wild_horse,	item_wild_horse,	item_wild_horse},
    {item_lion,		item_lizard,		item_chimera},
    {item_peasant,	item_lion,		item_harpie},
    {item_lizard,	item_bird,		item_dragon},
    {item_wild_horse,	item_bird,		item_pegasus},
    {item_peasant,	item_lizard,		item_gorgon},
    {item_rat,		item_spider,		item_ratspider},
    {item_pegasus,	item_dragon,		item_nazgul},
    {0, 0, 0}
};


static int
breed_time(int item)
{

	switch (item)
	{
	case item_nazgul:	return 14;
	case item_harpie:	return 14;
	case item_lion:		return 14;
	case item_chimera:	return 21;
	case item_spider:	return 21;
	case item_hound:	return 21;
	case item_bird:		return 28;
	case item_dragon:	return 45;
	}

	return 7;
}


static int
breed_translate(int item)
{

	switch (item)
	{
	case item_riding_horse:		return item_wild_horse;
	case item_warmount:		return item_wild_horse;
	}

	return item;
}


static int
breed_match(int which, int i1, int i2)
{
	int a[2];
	int b[2];
	int i, j;

	a[0] = i1;
	a[1] = i2;

	b[0] = breed_tbl[which].i1;
	b[1] = breed_tbl[which].i2;

	for (i = 0; i < 2; i++)
		for (j = 0; j < 2; j++)
			if (a[i] == b[j])
			{
				a[i] = 0;
				b[j] = 0;
			}

	for (i = 0; i < 2; i++)
		if (a[i] || b[i])
			return FALSE;

	return TRUE;
}


static int
find_breed(int i1, int i2)
{
	int i;

	i1 = breed_translate(i1);
	i2 = breed_translate(i2);

	for (i = 0; breed_tbl[i].i1; i++)
		if (breed_match(i, i1, i2))
			return breed_tbl[i].result;

	if (item_animal(i1) && i1 == i2)
		return i1;

	return 0;
}


int
v_breed(struct command *c)
{
	int i1 = c->a;
	int i2 = c->b;
	int exp;
	int offspring;
	int delay;

	if (!has_skill(c->who, sk_breed_beasts))
	{
		wout(c->who, "Requires %s.", box_name(sk_breed_beasts));
		return FALSE;
	}

	if (numargs(c) < 2)
	{
		wout(c->who, "Usage: breed <item> <item>");
		return FALSE;
	}

	if (kind(i1) != T_item)
	{
		wout(c->who, "%s is not an item.", c->parse[1]);
		return FALSE;
	}

	if (kind(i2) != T_item)
	{
		wout(c->who, "%s is not an item.", c->parse[2]);
		return FALSE;
	}

	if (has_item(c->who, i1) < 1)
	{
		wout(c->who, "Don't have any %s.", box_code(i1));
		return FALSE;
	}

	if (has_item(c->who, i2) < 1)
	{
		wout(c->who, "Don't have any %s.", box_code(i2));
		return FALSE;
	}

	if (i1 == i2 && has_item(c->who, i1) < 2)
	{
		wout(c->who, "Don't have two %s.", box_code(i1));
		return FALSE;
	}

	offspring = find_breed(i1, i2);

	delay = breed_time(offspring);

	exp = max(has_skill(c->who, sk_breed_beasts) - 1, 0);
	if (exp)
		delay--;

	c->wait = delay;

	wout(c->who, "Breed attempt will take %d days.", delay);

/* 
 *  Hack to fold experience_use_speedup into this skill
 *  if they use BREED instead of USE xxxx
 */

	return TRUE;
}


#define	BREED_ACCIDENT		10


int
d_breed(struct command *c)
{
	int i1 = c->a;
	int i2 = c->b;
	int offspring;
	int breed_accident = BREED_ACCIDENT;
	int killed = FALSE;

	if (kind(i1) != T_item)
	{
		wout(c->who, "%s is not an item.", c->parse[1]);
		return FALSE;
	}

	if (kind(i2) != T_item)
	{
		wout(c->who, "%s is not an item.", c->parse[2]);
		return FALSE;
	}

	if (has_item(c->who, i1) < 1)
	{
		wout(c->who, "Don't have any %s.", box_code(i1));
		return FALSE;
	}

	if (has_item(c->who, i2) < 1)
	{
		wout(c->who, "Don't have any %s.", box_code(i2));
		return FALSE;
	}

	if (i1 == i2 && has_item(c->who, i1) < 2)
	{
		wout(c->who, "Don't have two %s.", box_code(i1));
		return FALSE;
	}

	p_skill(sk_breed_beasts)->use_count++;

/*
 *  The following isn't quite right -- there is no chance of
 *  killing both the breeders if they are of the same type.
 */

	offspring = find_breed(i1, i2);

	if (i1 == i2)
		breed_accident *= 2;

	if (i1 && rnd(1,100) <= breed_accident)
	{
		wout(c->who, "%s was killed in the breeding attempt.",
					cap(box_name_qty(i1, 1)));
		consume_item(c->who, i1, 1);
		killed = TRUE;
	}

	if (i2 && rnd(1,100) <= breed_accident && i1 != i2)
	{
		wout(c->who, "%s was killed in the breeding attempt.",
					cap(box_name_qty(i2, 1)));
		consume_item(c->who, i2, 1);
		killed = TRUE;
	}

	if (offspring == 0 || (killed && rnd(1,2) == 1))
	{
		wout(c->who, "No offspring was produced.");
		return FALSE;
	}

	wout(c->who, "Produced %s.", box_name_qty(offspring, 1));

	gen_item(c->who, offspring, 1);
	add_skill_experience(c->who, sk_breed_beasts);

	return TRUE;
}


int
v_breed_hound(struct command *c)
{
	return TRUE;
}


int
d_breed_hound(struct command *c)
{

	gen_item(c->who, item_hound, 1);
	wout(c->who, "Bred and trained %s.", box_name_qty(item_hound, 1));
	return TRUE;
}
