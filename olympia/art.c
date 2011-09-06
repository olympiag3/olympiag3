
#include	<stdio.h>
#include	"z.h"
#include	"oly.h"


int
has_auraculum(int who)
{
	int ac;

	ac = char_auraculum(who);

	if (ac && valid_box(ac) && has_item(who, ac) > 0)
		return ac;

	return 0;
}


/*
 *  Maximum aura, innate plus the auraculum bonus
 */

int
max_eff_aura(int who)
{
	int a;				/* aura */
	int ac;				/* auraculum */

	a = char_max_aura(who);
	if (ac = has_auraculum(who))
		a += item_aura(ac);

	{
		struct item_ent *e;
		int n;

		loop_inv(who, e)
		{
			if (n = item_aura_bonus(e->item))
				a += n;
		}
		next_inv;
	}

	return a;
}


void
limit_cur_aura(int who)
{
	if (char_cur_aura(who) > MAX_CURRENT_AURA(who))
		p_magic(who)->cur_aura = MAX_CURRENT_AURA(who);
}


int
v_forge_palantir(struct command *c)
{

	if (!check_aura(c->who, 8))
		return FALSE;

	wout(c->who, "Attempt to create a palantir.");
	return TRUE;
}


int
d_forge_palantir(struct command *c)
{
	int new;
	struct entity_item *p;
	struct item_magic *pm;

	if (!charge_aura(c->who, 8))
		return FALSE;

	new = create_unique_item(c->who, sub_palantir);

	if (new < 0)
	{
		wout(c->who, "Spell failed.");
		return FALSE;
	}

	set_name(new, "Palantir");
	p = p_item(new);
	p->weight = 2;

	pm = p_item_magic(new);

	pm->use_key = use_palantir;
	pm->creator = c->who;
	pm->region_created = province(c->who);

	wout(c->who, "Created %s.", box_name(new));

	log_write(LOG_SPECIAL, "%s created %s.", box_name(c->who), box_name(new));

	return TRUE;
}


int
v_use_palantir(struct command *c)
{
	int item = c->a;
	int target = c->b;
	struct item_magic *p;

	if (!is_loc_or_ship(target))
	{
		wout(c->who, "%s is not a location.", box_code(target));
		return FALSE;
	}

	p = rp_item_magic(item);

	if (p && p->one_turn_use)
	{
		wout(c->who, "The palantir may only be used once per month.");
		return FALSE;
	}

	wout(c->who, "Will attempt to view %s with the palantir.",
					box_code(target));

	c->wait = 7;

	return TRUE;
}


int
d_use_palantir(struct command *c)
{
	int item = c->a;
	int target = c->b;

	if (!is_loc_or_ship(target))
	{
		wout(c->who, "%s is not a location.", box_code(target));
		return FALSE;
	}

	if (loc_shroud(target) || diff_region(c->who, target))
	{
		log_write(LOG_CODE, "Murky palantir result, who=%s, targ=%s",
				box_code_less(c->who), box_code_less(target));
		wout(c->who, "Only murky, indistinct images are seen in "
							"the palantir.");
		return FALSE;
	}

	log_write(LOG_CODE, "Palantir scry, who=%s, targ=%s",
				box_code_less(c->who), box_code_less(target));

	p_item_magic(item)->one_turn_use++;

	wout(c->who, "A vision of %s appears:", box_name(target));
	out(c->who, "");
	show_loc(c->who, target);

	alert_palantir_scry(c->who, target);

	return TRUE;
}


static int
destroyable_item(int item)
{

	switch (subkind(item))
	{
	case sub_palantir:
	case sub_auraculum:
		return TRUE;
	}

	return FALSE;
}


int
v_destroy_art(struct command *c)
{
	int item = c->a;

	if (!valid_box(item) || (has_item(c->who, item) < 1))
	{
		wout(c->who, "%s does not have %s.",
					box_name(c->who),
					box_code(item));
		return FALSE;
	}

	if (!destroyable_item(item))
	{
		wout(c->who, "Cannot destroy %s with this spell.",
					box_name(item));
		return FALSE;
	}

	if (!check_aura(c->who, 2))
		return FALSE;

	wout(c->who, "Attempt to destroy %s.", box_name(item));
	return TRUE;
}


static int
destroy_palantir(struct command *c, int item)
{

	wout(c->who, "Destroyed %s.", box_name(item));

	gen_item(c->who, item_gate_crystal, 1);

	wout(c->who, "Received one %s from the shattered palantir.",
				box_name(item_gate_crystal));

	return TRUE;
}


static int
destroy_auraculum(struct command *c, int item)
{
	int creator;
	int reg_created;

fprintf(stderr, "destroy_auraculum: c->who=%d, item=%d\n",
			c->who, item);

	reg_created = item_creat_loc(item);

	if (province(c->who) != reg_created) {
		wout(c->who, "%s was not created here.  The spell fails.",
				box_name(item));
		return FALSE;
	}

fprintf(stderr, "destroy_auraculum: loc check ok\n");

	creator = item_creator(item);

	if (creator == c->who)
	{
fprintf(stderr, "destroy_auraculum: suicide disallowed\n");

		wout(c->who, "Can't destroy one's own auraculum.");
		return FALSE;
	}

	if (valid_box(creator) && alive(creator))
	{
fprintf(stderr, "destroy_auraculum: killed the owner %s\n", box_name(creator));

		wout(creator, "The auraculum %s has been destroyed!",
						box_name(item));

		wout(c->who, "For a brief instant, a vision of %s being consumed by fire appears, then fades away.", box_name(creator));

		kill_char(creator, MATES);
	}

fprintf(stderr, "destroy_auraculum: return true\n");
	return TRUE;
}



static int
destroy_item(struct command *c, int item)
{
	int ret;
	int aura;

	switch (subkind(item))
	{
	case sub_palantir:
		ret = destroy_palantir(c, item);
		break;

	case sub_auraculum:
		ret = destroy_auraculum(c, item);
		break;

	default:
		assert(FALSE);
	}

	if (ret)
	{
		aura = item_aura(item);

		if (aura > 0)
		{
			p_magic(c->who)->cur_aura += aura;
			wout(c->who, "Gained %s current aura.",
						nice_num(aura));
		}

		log_write(LOG_SPECIAL, "%s destroyed %s (%s, creator=%s)",
				box_name(c->who), box_name(item),
				subkind_s[subkind(item)],
				box_name(item_creator(item)));

		destroy_unique_item(c->who, item);
		return TRUE;
	}

	return FALSE;
}


int
d_destroy_art(struct command *c)
{
	int item = c->a;

	if (has_item(c->who, item) < 0)
	{
		wout(c->who, "%s does not have %s.",
					box_name(c->who),
					box_code(item));
		return FALSE;
	}

	if (!destroyable_item(item))
	{
		wout(c->who, "Cannot destroy %s with this spell.",
					box_name(item));
		return FALSE;
	}

	if (!charge_aura(c->who, 2))
		return FALSE;

	return destroy_item(c, item);
}


int
v_show_art_creat(struct command *c)
{
	int item = c->a;
	int aura;

	if (has_item(c->who, item) < 1)
	{
		wout(c->who, "%s has no %s.", box_name(c->who),
						box_code(item));
		return FALSE;
	}

	if (c->b < 1)
		c->b = 1;
	aura = c->b;

	if (!check_aura(c->who, aura))
		return FALSE;

	wout(c->who, "Attempt to learn the creator of %s.", box_name(item));

	return TRUE;
}


int
d_show_art_creat(struct command *c)
{
	int item = c->a;
	int aura = c->b;
	int n;

	if (has_item(c->who, item) < 1)
	{
		wout(c->who, "%s has no %s.", box_name(c->who),
						box_code(item));
		return FALSE;
	}

	if (!charge_aura(c->who, aura))
		return FALSE;

	if (aura <= item_creat_cloak(item))
	{
		wout(c->who, "A magical shroud hinders inspection of %s.",
							box_name(item));
		return FALSE;
	}

	n = item_creator(item);

	if (!valid_box(n))
	{
		wout(c->who, "The imprint of the maker's presence has "
			"faded from %s.  It is not possible to learn who "
			"created it.", box_name(item));
		return FALSE;
	}

	wout(c->who, "%s created %s.", box_name(n), box_name(item));
	return TRUE;
}


int
v_show_art_reg(struct command *c)
{
	int item = c->a;
	int aura;

	if (has_item(c->who, item) < 1)
	{
		wout(c->who, "%s has no %s.", box_name(c->who),
						box_code(item));
		return FALSE;
	}

	if (c->b < 1)
		c->b = 1;
	aura = c->b;

	if (!check_aura(c->who, aura))
		return FALSE;

	wout(c->who, "Attempt to learn where %s was created.", box_name(item));

	return TRUE;
}


int
d_show_art_reg(struct command *c)
{
	int item = c->a;
	int aura = c->b;
	int n;

	if (has_item(c->who, item) < 1)
	{
		wout(c->who, "%s has no %s.", box_name(c->who),
						box_code(item));
		return FALSE;
	}

	if (!charge_aura(c->who, aura))
		return FALSE;

	if (aura <= item_creat_cloak(item))
	{
		wout(c->who, "A magical shroud hinders inspection of %s.",
							box_name(item));
		return FALSE;
	}

	n = item_creat_loc(item);

	if (!valid_box(n))
	{
		wout(c->who, "The location of creation is not recorded in %s.",
						box_name(item));
		return FALSE;
	}

	wout(c->who, "%s was created in %s.",
				box_name(item),
				char_rep_location(n));
	return TRUE;
}


int
v_rem_art_cloak(struct command *c)
{
	int item = c->a;

	if (has_item(c->who, item) < 1)
	{
		wout(c->who, "%s does not have %s.",
					box_name(c->who),
					box_code(item));
		return FALSE;
	}

	if (!check_aura(c->who, 8))
		return FALSE;

	wout(c->who, "Attempt to remove all cloaking spells from %s.",
					box_name(item));
	return FALSE;
}


int
d_rem_art_cloak(struct command *c)
{
	int item = c->a;
	struct item_magic *im;

	if (has_item(c->who, item) < 1)
	{
		wout(c->who, "%s does not have %s.",
					box_name(c->who),
					box_code(item));
		return FALSE;
	}

	im = rp_item_magic(item);

	if (im == NULL)
	{
		wout(c->who, "%s is not cloaked in any way.", box_name(item));
		return FALSE;
	}

	if (!charge_aura(c->who, 8))
		return FALSE;

	im->cloak_creator = 0;
	im->cloak_region = 0;

	wout(c->who, "Cloaking spells removed from %s.", box_name(item));

	return TRUE;
}


int
v_cloak_creat(struct command *c)
{
	int item = c->a;
	int aura;

	if (c->b < 1)
		c->b = 1;
	aura = c->b;

	if (!valid_box(item) || has_item(c->who, item) < 1)
	{
		wout(c->who, "%s does not have %s.",
					box_name(c->who),
					box_code(item));
		return FALSE;
	}

	wout(c->who, "Attempt to conceal the identity of the creator of %s.",
						box_name(item));

	return TRUE;
}


int
d_cloak_creat(struct command *c)
{
	int item = c->a;
	int aura = c->b;
	struct item_magic *im;

	if (has_item(c->who, item) < 1)
	{
		wout(c->who, "%s does not have %s.",
					box_name(c->who),
					box_code(item));
		return FALSE;
	}

	if (!charge_aura(c->who, aura))
		return FALSE;

	im = p_item_magic(item);
	im->cloak_creator += aura;

	wout(c->who, "Creator cloaking in %s now %d.", box_name(item),
				im->cloak_creator);

	return TRUE;
}


int
v_cloak_reg(struct command *c)
{
	int item = c->a;
	int aura;

	if (c->b < 1)
		c->b = 1;
	aura = c->b;

	if (!valid_box(item) || (has_item(c->who, item) < 1))
	{
		wout(c->who, "%s does not have %s.",
					box_name(c->who),
					box_code(item));
		return FALSE;
	}

	wout(c->who, "Attempt to conceal the region of creation for %s.",
				box_name(item));

	return TRUE;
}


int
d_cloak_reg(struct command *c)
{
	int item = c->a;
	int aura = c->b;
	struct item_magic *im;

	if (has_item(c->who, item) < 1)
	{
		wout(c->who, "%s does not have %s.",
					box_name(c->who),
					box_code(item));
		return FALSE;
	}

	if (!charge_aura(c->who, aura))
		return FALSE;

	im = p_item_magic(item);
	im->cloak_region += aura;

	wout(c->who, "Region cloaking in %s now %d.", box_name(item),
				im->cloak_region);

	return TRUE;
}


int
v_curse_noncreat(struct command *c)
{
	int item = c->a;
	int aura;

	if (c->b < 1)
		c->b = 1;
	aura = c->b;

	if (has_item(c->who, item) < 1)
	{
		wout(c->who, "%s does not have %s.",
					box_name(c->who),
					box_code(item));
		return FALSE;
	}

/*
 *  Only let forged artifacts be cursed, not just anything
 */

	if (!destroyable_item(item))
	{
		wout(c->who, "The curse can not be applied to %s.",
					box_name(item));
		return FALSE;
	}

	wout(c->who, "Attempt to cast a noncreator possession curse on %s.",
				box_name(item));

	return TRUE;
}


int
d_curse_noncreat(struct command *c)
{
	int item = c->a;
	int aura = c->b;
	struct item_magic *im;

	if (has_item(c->who, item) < 1)
	{
		wout(c->who, "%s does not have %s.",
					box_name(c->who),
					box_code(item));
		return FALSE;
	}

	if (!charge_aura(c->who, aura))
		return FALSE;

	im = p_item_magic(item);
	im->curse_loyalty += aura;

	wout(c->who, "Noncreator curse on %s now %d.",
			box_name(item), im->curse_loyalty);

	return TRUE;
}


int
v_forge_aura(struct command *c)
{
	int aura;

	if (char_auraculum(c->who))
	{
		wout(c->who, "%s may only be used once.",
					box_name(c->use_skill));
		return FALSE;
	}

	if (c->a < 1)
		c->a = 1;
	aura = c->a;

	if (!check_aura(c->who, aura))
		return FALSE;

	if (aura > char_max_aura(c->who))
	{
		wout(c->who, "The specified amount of aura exceeds the "
			"maximum aura level of %s.", box_name(c->who));
		return FALSE;
	}

	wout(c->who, "Attempt to forge an auraculum.");
	return TRUE;
}



static void
notify_others_auraculum(int who, int item)
{
	int n;

	loop_char(n)
	{
		if (n != who && is_magician(n) && has_auraculum(n))
			wout(n, "Another auraculum has come into existence.");
	}
	next_char;

	log_write(LOG_SPECIAL, "%s created %s, %s.",
					box_name(who),
					box_name(item),
					subkind_s[subkind(item)]);
}


int
d_forge_aura(struct command *c)
{
	int aura = c->a;
	char *new_name;
	int new;
	struct entity_item *p;
	struct item_magic *pm;
	struct char_magic *cm;

	if (aura > char_max_aura(c->who))
	{
		wout(c->who, "The specified amount of aura exceeds the "
			"maximum aura level of %s.", box_name(c->who));
		return FALSE;
	}

	if (!charge_aura(c->who, aura))
		return FALSE;

	if (numargs(c) < 2)
	{
		switch (rnd(1,3))
		{
		case 1:
			new_name = "Gold ring";
			break;
		case 2:
			new_name = "Wooden staff";
			break;
		case 3:
			new_name = "Jeweled crown";
			break;
		default:
			assert(FALSE);
		}
	}
	else
		new_name = str_save(c->parse[2]);

	new = create_unique_item(c->who, sub_auraculum);

	if (new < 0)
	{
		wout(c->who, "Spell failed.");
		return FALSE;
	}

	set_name(new, new_name);

	p = p_item(new);
	p->weight = rnd(1,3);

	pm = p_item_magic(new);

	pm->creator = c->who;
	pm->region_created = province(c->who);
	pm->aura = aura * 2;

	cm = p_magic(c->who);

	cm->auraculum = new;
	cm->max_aura -= aura;

	wout(c->who, "Created %s.", box_name(new));
	notify_others_auraculum(c->who, new);

	learn_skill(c->who, sk_adv_sorcery);

	return TRUE;
}


int
new_orb(int who)
{
	int new;

	new = create_unique_item(who, 0);

	if (new < 0)
	{
		wout(who, "Orb creation failed.");
		return 0;
	}

	set_name(new, "Orb");

	p_item(new)->weight = 1;
	p_item_magic(new)->use_key = use_orb;
	p_item_magic(new)->lore = lore_orb;
	p_item_magic(new)->orb_use_count = rnd(1,4)*2+1;

	return new;
}


static ilist orb_used_this_month = NULL;


int
v_use_orb(struct command *c)
{
	int item = c->a;
	int target = c->b;
	int where = 0;
	int owner;
	struct item_magic *p;

	if (ilist_lookup(orb_used_this_month, item) >= 0)
	{
	    wout(c->who, "The orb may only be used once per month.");
	    wout(c->who, "Only murky, indistinct images are seen in the orb.");
	    return FALSE;
	}

	ilist_append(&orb_used_this_month, item);

	if (rnd(1,3) == 1)
	{
	    wout(c->who, "Only murky, indistinct images are seen in the orb.");
	    return FALSE;
	}

	switch (kind(target))
	{
	case T_loc:
	case T_ship:
		where = province(target);
		break;

	case T_char:
		where = province(target);
		break;

	case T_item:
		if (owner = item_unique(target))
			where = province(owner);
		break;
	}

	if (where == 0)
	{
		wout(c->who, "The orb is unsure what location is meant to be scried.");
	}
	else if (diff_region(where, c->who))
	{
		wout(c->who, "Only murky, indistinct images are seen.");
	}
	else if (loc_shroud(where))
	{
		wout(c->who, "The orb is unable to penetrate a shroud over %s.",
					box_name(where));
	}
	else
	{
		wout(c->who, "A vision of %s appears:", box_name(where));
		show_loc(c->who, where);
		alert_scry_generic(c->who, where);
	}

	p = p_item_magic(item);

	p->orb_use_count--;
	if (p->orb_use_count <= 0)
	{
		wout(c->who, "After the vision fades, the orb grows "
				"dark, and shatters.  The orb is gone");
		destroy_unique_item(c->who, item);
	}

	return TRUE;
}


/*
 *  Artifact:  npc token (small npc group controller)
 *
 *  recognized by sub_npc_token
 *  units under token control: p_player(item)->units
 *  p_misc(unit)->cmd_allow = 'r' to restrict control
 *  loyalty is LOY_npc
 *
 *  hook in move_item to swear token units to new owner or
 *  to indep_player if owner is not a controlled char
 *
 *  check that every token artifact has its number of
 *  units at the beginning and end of each turn
 *
 *  show char wearing the token in display
 *  perhaps triggered by use?  then need a token active field.  naw
 *
 *  item_magic->token_ni	item for the controlled units
 *  item_magic->token_num	max number of controlled units
 *
 */


int
token_player(int owner)
{
	int pl;

	if (kind(owner) != T_char)
		return indep_player;

	pl = player(owner);
	if (subkind(pl) != sub_pl_regular)
		return indep_player;

	return pl;
}


static void
swear_token_units(int item, int target)
{
	int i;

	log_write(LOG_MISC, "%s got npc token %s",
				box_name(target), box_name(item));

	assert(subkind(item) == sub_npc_token);

	loop_units(item, i)
	{
		if (subkind(i) == T_char)
		{
			log_write(LOG_MISC, "   swearing %s", box_name_kind(i));
			set_lord(i, target, LOY_UNCHANGED, 0);
		}
	}
	next_unit;
}


static void
melt_token_units(int item)
{
	int who;
	int first = TRUE;

	assert(subkind(item) == sub_npc_token);

	loop_units(item, who)
	{
		if (kind(who) != T_char)
			continue;

		if (first)
		{
			first = FALSE;
			log_write(LOG_MISC, "Melting token units for %s.", box_name(item));
		}

		wout(subloc(who), "%s melts into the ground and vanishes.", box_name(who));
		char_reclaim(who);
	}
	next_unit;

	if (item_token_num(item) > 1)
		p_item_magic(item)->token_num = 1;
}


static void
add_token_unit_sup(int item)
{
	int new;
	int owner;
	int where;
	struct entity_player *p;

	owner = item_unique(item);
	assert(owner);

	where = province(owner);

	if (subkind(where) == sub_ocean)
		where = subloc(owner);

	new = new_char(sub_ni, item_token_ni(item), where, -1,
				token_player(owner), LOY_npc, 0, NULL);

	if (new < 0)
	{
		log_write(LOG_CODE, "  FAILed to add unit to token %s",
				box_code_less(item));
		return;
	}

	log_write(LOG_MISC, "  adding %s to %s", box_name(new), box_name(item));

	if (beast_capturable(new))
		p_char(new)->break_point = 0;
	p_misc(new)->cmd_allow = 'r';
	p_magic(new)->token = item;
#if 0
	gen_item(new, item_token_ni(item), rnd(3,15));
#endif

	p = p_player(item);
	ilist_append(&p->units, new);

	wout(where, "%s appears.", box_name(new));
}


static void
add_token_units(int item)
{
	int l;
	struct entity_player *p;

	log_write(LOG_MISC, "add_token_units(%s)", box_name(item));

	assert(subkind(item) == sub_npc_token);

	p = p_player(item);
	l = ilist_len(p->units);

	while (l < item_token_num(item))
	{
		add_token_unit_sup(item);
		l++;
	}
}


/*
 *  Called when an NPC token is moved from one owner to another. 
 *  Swear any units controlled by the token to the new owner, if
 *  the owner belongs to a different faction.  If a player just
 *  acquired the token (not from another player, but from an indep,
 *  npc, or via explore), then create the token units on the spot
 *  (instead of waiting for the end of the turn).
 */

void
move_token(int item, int from, int to)
{
	int to_pl = token_player(to);

	log_write(LOG_MISC, "Token %s moved from %s (%s) to %s (%s)",
			box_name(item),
			box_name(from),
			box_name(token_player(from)),
			box_name(to),
			box_name(to_pl));

	if (token_player(from) == indep_player &&
	    ilist_len(p_player(item)->units) == 0)
	{
		log_write(LOG_SPECIAL, "token %s from %d to 1.",
					box_code_less(item),
					item_token_num(item));
		p_item_magic(item)->token_num = 1;
	}

	if (token_player(from) != to_pl)
	{
		swear_token_units(item, to_pl);

#if 0
		if (to_pl == indep_player)
			melt_token_units(item);
		else
			add_token_units(item);
#else
		if (to_pl != indep_player)
			add_token_units(item);
/*
 *  We don't want to melt the units if the artifact goes indep
 *  until later, since we might be in the middle of executing
 *  a command, AND one of the token units itself might have
 *  been the cause of the move.
 */
#endif
	}
}


void
check_token_units()
{
	int item;
	int owner, pl;
	int unit;

	loop_subkind(sub_npc_token, item)
	{
/*
 *  If NPC token is held by a real player, replace any units which were
 *  killed this turn.  Otherwise, melt away any units controlled by the
 *  token until it is once again held by a player.
 */

		owner = item_unique(item);
		assert(owner);

		pl = token_player(owner);

		if (pl == indep_player)
			melt_token_units(item);
		else
			add_token_units(item);

		loop_units(item, unit)
		{
			if (kind(unit) != T_char)
			{
				log_write(LOG_CODE, "%s holds unit %s which is %s, player(unit) = %s, owner = %s",
					box_code(unit), box_code(unit),
					kind_s[kind(unit)],
					box_code_less(player(unit)),
					box_code_less(owner));
				continue;
			}

			if (player(unit) != pl &&
			    player_np(pl) >= char_np_total(unit))
			{
				fprintf(stderr, "fixing token owner for %s (%s to %s)\n",
						box_name_kind(unit),
						box_code_less(player(unit)),
						box_code_less(pl));

				if (player(unit) > 0)
					wout(player(unit), "%s renounces loyalty.", box_name(unit));
				wout(pl, "%s swears loyalty.", box_name(unit));
				set_lord(unit, pl, LOY_UNCHANGED, 0);
			}
		}
		next_unit;
	}
	next_subkind;
}


int
create_npc_token(int who)
{
	int new;
	int ni;
	char *name;
	int lore;

	new = create_unique_item(who, sub_npc_token);

	switch (rnd(1,5))
	{
	case 1:
		ni = item_barbarian;
		name = "Crown of the Barbarians";
		lore = lore_barbarian_npc_token;
		break;

	case 2:
		ni = item_savage;
		name = "Horn of the Savages";
		lore = lore_savage_npc_token;
		break;

	case 3:
		ni = item_corpse;
		name = "Crown of the Undead lord";
		lore = lore_undead_npc_token;
		break;

	case 4:
		ni = item_orc;
		name = "Golden idol of the Orcs";
		lore = lore_orc_npc_token;
		break;

	case 5:
		ni = item_skeleton;
		name = "Banner of the Skeletons";
		lore = lore_skeleton_npc_token;
		break;

	default:
		assert(FALSE);
	}

	set_name(new, name);

	p_item_magic(new)->token_num = 1;
	p_item_magic(new)->token_ni = ni;
	p_item_magic(new)->lore = lore;

#if 0
	add_token_units(new);
#endif

	return new;
}


int
v_forge_art_x(struct command *c)
{
	int aura = c->a;
	int rare_item;

	if (aura < 1)
		c->a = aura = 1;
	if (aura > 20)
		c->a = aura = 20;

	if (!check_aura(c->who, aura))
		return FALSE;

	if (!can_pay(c->who, 500))
	{
		wout(c->who, "Requires %s.", gold_s(500));
		return FALSE;
	}

	switch (c->use_skill)
	{
	case sk_forge_weapon:
	case sk_forge_armor:
		rare_item = item_mithril;
		break;

	case sk_forge_bow:
		rare_item = item_mallorn_wood;
		break;

	default:
		assert(FALSE);
	}
	c->d = rare_item;

	if (!has_item(c->who, rare_item) >= 1)
	{
		wout(c->who, "Requires %s.", box_name_qty(rare_item, 1));
		return FALSE;
	}

	return TRUE;
}


int
d_forge_art_x(struct command *c)
{
	int new;
	int aura = c->a;
	int rare_item = c->d;
	char *new_name;
	struct item_magic *pm;

	if (!check_aura(c->who, aura))
		return FALSE;

	if (!charge(c->who, 500))
	{
		wout(c->who, "Requires %s.", gold_s(500));
		return FALSE;
	}

	if (!has_item(c->who, rare_item) >= 1)
	{
		wout(c->who, "Requires %s.", box_name_qty(rare_item, 1));
		return FALSE;
	}

	charge_aura(c->who, aura);

	// by Cappinator:
	// Removed duplicate charge of 500 gold

	consume_item(c->who, rare_item, 1);

	new = create_unique_item(c->who, 0);
	pm = p_item_magic(new);

	switch (c->use_skill)
	{
	case sk_forge_weapon:
		pm->attack_bonus = aura * 5;
		new_name = "enchanted sword";
		break;

	case sk_forge_armor:
		pm->defense_bonus = aura * 5;
		new_name = "enchanted armor";
		break;

	case sk_forge_bow:
		pm->missile_bonus = aura * 5;
		new_name = "enchanted bow";
		break;

	default:
		assert(FALSE);
	}

	if (numargs(c) >= 2 && c->parse[2] && *(c->parse[2]))
		new_name = c->parse[2];

	set_name(new, new_name);
	p_item(new)->weight = 10;
	pm->creator = c->who;
	pm->region_created = province(c->who);

	wout(c->who, "Created %s.", box_name(new));

	return TRUE;
}


int
new_suffuse_ring(int who)
{
	int new;
	int ni;
	int lore;

	new = create_unique_item(who, sub_suffuse_ring);

	switch (rnd(1,5))
	{
	case 1:
		ni = use_barbarian_kill;
		lore = lore_barbarian_kill;
		break;

	case 2:
		ni = use_savage_kill;
		lore = lore_savage_kill;
		break;

	case 3:
		ni = use_corpse_kill;
		lore = lore_undead_kill;
		break;

	case 4:
		ni = use_orc_kill;
		lore = lore_orc_kill;
		break;
	case 5:
		ni = use_skeleton_kill;
		lore = lore_skeleton_kill;
		break;

	default:
		assert(FALSE);
	}

	set_name(new, "Golden ring");

	p_item(new)->weight = 1;
	p_item_magic(new)->use_key = ni;
	p_item_magic(new)->lore = lore;

	return new;
}


int
v_suffuse_ring(struct command *c, int kind)
{
	int item = c->use_skill;
	int where = province(subloc(c->who));
	int num;
	struct item_ent *t;

	log_write(LOG_SPECIAL, "Golden ring %s used by %s",
			box_code_less(item), box_code_less(player(c->who)));

	if (rnd(1,3) == 1)
	{
		wout(c->who, "Nothing happens.");
	}
	else
	{
		wout(c->who, "A golden glow suffuses the province.");
		wout(where, "A golden glow suffuses the province.");

		loop_all_here(where, num)
		{
			wout(num, "A golden glow suffuses the province.");

			loop_inv(num, t)
			{
				if (t->item == kind)
				{
					wout(num, "%s vanished!", box_name_qty(t->item, t->qty));
					consume_item(num, t->item, t->qty);
				}
			}
			next_inv;

			if (subkind(num) == sub_ni && noble_item(num) == kind)
			{
				kill_char(num, MATES);
			}
		}
		next_all_here;
	}

	wout(c->who, "%s vanishes.", box_name(item));
	destroy_unique_item(c->who, item);

	return TRUE;
}

