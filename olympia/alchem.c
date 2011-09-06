#include	<stdio.h>
#include <stdlib.h>
#include	"z.h"
#include	"oly.h"


int
new_potion(int who)
{
	char *s;
	int new;
	struct item_magic *p;

	new = create_unique_item(who, 0);

	if (new < 0)
		return -1;

	switch (rnd(1,2))
	{
	case 1:
		s = "Magic potion";
		break;

	case 2:
		s = "Strange potion";
		break;

	default:
		assert(FALSE);
	}

	set_name(new, s);
	p = p_item_magic(new);
	p->creator = who;
	p->region_created = province(who);
	p_item(new)->weight = 1;

	wout(who, "Produced one %s", box_name(new));

	return new;
}


int
d_brew_slave(struct command *c)
{
	int new;

	new = new_potion(c->who);

	if (new < 0)
	{
		wout(c->who, "Attempt to brew potion failed.");
		return FALSE;
	}

	p_item_magic(new)->use_key = use_slave_potion;

	return TRUE;
}


int
d_brew_death(struct command *c)
{
	int new;

	new = new_potion(c->who);

	if (new < 0)
	{
		wout(c->who, "Attempt to brew potion failed.");
		return FALSE;
	}

	p_item_magic(new)->use_key = use_death_potion;

	return TRUE;
}


int
v_brew(struct command *c)
{

	return TRUE;
}


int
d_brew_heal(struct command *c)
{
	int new;

	new = new_potion(c->who);

	if (new < 0)
	{
		wout(c->who, "Attempt to brew potion failed.");
		return FALSE;
	}

	p_item_magic(new)->use_key = use_heal_potion;

	return TRUE;
}


int
v_use_heal(struct command *c)
{
	int item = c->a;

	assert(kind(item) == T_item);

	wout(c->who, "%s drinks the potion...", just_name(c->who));

	if (char_health(c->who) == 100)
	{
		wout(c->who, "Nothing happens.");
		destroy_unique_item(c->who, item);
		return TRUE;
	}

	p_char(c->who)->sick = FALSE;
	wout(c->who, "%s has been cured of illness.", just_name(c->who));

#if 0
	if (char_sick(c->who)) {
		p_char(c->who)->sick = FALSE;
		wout(c->who, "%s has been cured of illness.",
				just_name(c->who));
	}

	{
		int bonus = rnd(0,3) * 10;

		if (bonus)
		{
			p_char(c->who)->health += bonus;
			if (char_health(c->who) > 100)
				p_char(c->who)->health = 100;
			wout(c->who, "Health is now %d.",
				char_health(c->who));
		}
	}

	wout(c->who, "%s is immediately healed of all wounds!",
				just_name(c->who));

	p_char(c->who)->sick = FALSE;
	rp_char(c->who)->health = 100;
#endif

	destroy_unique_item(c->who, item);

	return TRUE;
}


int
v_use_death(struct command *c)
{
	int item = c->a;

	assert(kind(item) == T_item);

	wout(c->who, "%s drinks the potion...", just_name(c->who));
	destroy_unique_item(c->who, item);

	wout(c->who, "It's poison!");

	p_char(c->who)->sick = TRUE;

	// by Cappinator:
	// This should damage the noble with 100 damage,
	// instead of 50.
	add_char_damage(c->who, 100, MATES);

	return TRUE;
}


int
v_use_slave(struct command *c)
{
	int item = c->a;
	int creator;
	int nps;

	assert(kind(item) == T_item);
	creator = item_creator(item);

	log(LOG_SPECIAL, "%s drinks a slavery potion to %s\n",
				box_name(c->who), box_name(creator));

	wout(c->who, "%s drinks the potion...", just_name(c->who));

	destroy_unique_item(c->who, item);

	if (rnd(1,100) <= 33)
	{
		kill_char(c->who, MATES);
		return TRUE;
	}

	nps = char_np_total(c->who);

	if (!valid_box(creator)) {
		wout(c->who, "Nothing happens.");
		return TRUE;
	}

	if (kind(creator) != T_char) {
		wout(c->who, "Nothing happens.");
		return TRUE;
	}

	if (!valid_box(player(creator))) {
		wout(c->who, "Nothing happens.");
		return TRUE;
	}

	if (player_np(player(creator)) < nps) {
		wout(c->who, "Nothing happens.");
		return TRUE;
	}

	if ((c->who == creator) || (player(c->who) == player(creator))) {
		wout(c->who, "Nothing happens.");
		return TRUE;
	}

	wout(c->who, "%s is suddenly overcome with an irresistible "
				"desire to serve %s.",
				just_name(c->who),
				box_name(creator));

	unit_deserts(c->who, creator, TRUE, LOY_contract, 250);
	return TRUE;
}


int
v_lead_to_gold(struct command *c)
{
	int amount = c->a;
	int qty;

	if (has_item(c->who, item_farrenstone) < 1)
	{
	    wout(c->who, "Requires %s.", box_name_qty(item_farrenstone, 1));
	    return FALSE;
	}

	qty = has_item(c->who, item_lead);

	if (amount == 0)
		amount = qty;

	if (amount > qty)
		amount = qty;

	qty = min(qty, 20);

	if (qty == 0)
	{
		wout(c->who, "Don't have any %s.", box_name(item_lead));
		return FALSE;
	}

	c->d = qty;

	return TRUE;
}


int
d_lead_to_gold(struct command *c)
{
	int qty = c->d;
	int has = has_item(c->who, item_lead);
	extern int gold_lead_to_gold;

	if (has_item(c->who, item_farrenstone) < 1)
	{
	    wout(c->who, "Requires %s.", box_name_qty(item_farrenstone, 1));
	    return FALSE;
	}

	if (has < qty)
		qty = has;

	if (qty == 0)
	{
		wout(c->who, "Don't have any %s.", box_name(item_lead));
		return FALSE;
	}

	wout(c->who, "Turned %s into %s.", just_name_qty(item_lead, qty),
				just_name_qty(item_gold, qty*10));

	consume_item(c->who, item_lead, qty);

	// by Cappinator:
	// Removed the code that consumes the farrentstone
	// Catalysts are never consumed!

	gen_item(c->who, item_gold, qty * 10);
	gold_lead_to_gold += qty * 10;

	return TRUE;
}
