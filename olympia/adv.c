/*
 *  Advanced sorcery
 */

#include	<stdio.h>
#include	"z.h"
#include	"oly.h"


int
v_trance(struct command *c)
{

	if (has_skill(c->who, sk_trance) < 1)
	{
		wout(c->who, "Requires knowledge of %s.", box_name(sk_trance));
		return FALSE;
	}

	return TRUE;
}


int
d_trance(struct command *c)
{
	struct char_magic *p;

	p = p_magic(c->who);

	p->cur_aura = max(p->cur_aura, max_eff_aura(c->who) * 2 / 3);

	wout(c->who, "Current aura is now %d.", p->cur_aura);

	if (char_health(c->who) < 100 || char_sick(c->who))
	{
		p_char(c->who)->sick = FALSE;
		rp_char(c->who)->health = 100;

		wout(c->who, "%s is fully healed.", box_name(c->who));
	}

	return TRUE;
}


int
v_teleport_item(struct command *c)
{

	return TRUE;
}


/*
 *  give <who> <what> [qty] [have-left]
 */

int
d_teleport_item(struct command *c)
{
	int target = c->a;
	int item = c->b;
	int qty = c->c;
	int have_left = c->d;
	int ret;
	int aura;

	if (kind(target) != T_char)
	{
		wout(c->who, "%s is not a character.", box_code(target));
		return FALSE;
	}

	if (is_prisoner(target))
	{
		wout(c->who, "Prisoners may not be given anything.");
		return FALSE;
	}

	if (kind(item) != T_item)
	{
		wout(c->who, "%s is not an item.", box_code(item));
		return FALSE;
	}

	if (diff_region(c->who, target))
	{
		wout(c->who, "%s is too far away to teleport items to.",
			box_code(target));
		return FALSE;
	}

	if (has_item(c->who, item) < 1)
	{
		wout(c->who, "%s does not have any %s.", box_name(c->who),
						box_code(item));
		return FALSE;
	}

	qty = how_many(c->who, c->who, item, qty, have_left);

	if (qty <= 0)
		return FALSE;

	aura = 3 + item_weight(item) * qty / 50;

	if (!check_aura(c->who, aura))
		return FALSE;

	if (!will_accept(target, item, c->who, qty))
		return FALSE;

	charge_aura(c->who, aura);

	ret = move_item(c->who, target, item, qty);
	assert(ret);

	wout(c->who, "Teleported %s to %s.",
				just_name_qty(item, qty),
				box_name(target));

	wout(target, "%s teleported %s to us.",
				box_name(c->who),
				just_name_qty(item, qty));

	return TRUE;
}

