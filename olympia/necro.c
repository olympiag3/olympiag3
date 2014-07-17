
#include	<stdio.h>
#include	<stdlib.h>
#include	"z.h"
#include	"oly.h"



static int
keep_undead_check(struct command *c, int check_bond)
{
	int target = c->a;
	int where = c->d;

	if (kind(target) != T_char || subkind(target) != sub_undead)
	{
	    wout(c->who, "%s is not a demon lord.", box_code(target));
	    return FALSE;
	}

	if (subloc(target) != where)
	{
		if (subloc(c->who) == where)
			wout(c->who, "%s is not here.", box_code(target));
		else
			wout(c->who, "%s is not in %s.", box_code(target),
					box_code(where));
		return FALSE;
	}

	if (check_bond && loyal_kind(target) != LOY_summon)
	{
		wout(c->who, "%s is no longer bonded.", box_code(target));
		return FALSE;
	}

	return TRUE;
}


int
v_keep_undead(struct command *c)
{
	int target = c->a;
	c->d = subloc(c->who);

	if (!keep_undead_check(c, TRUE))
		return FALSE;

	if (!check_aura(c->who, 3))
		return FALSE;

	return TRUE;
}


int
d_keep_undead(struct command *c)
{
	int target = c->a;
	int loy;

	if (!keep_undead_check(c, TRUE))
		return FALSE;

	if (!charge_aura(c->who, 3))
		return FALSE;

	set_loyal(target, LOY_summon, max(loyal_rate(target) + 4, 8));

	wout(c->who, "%s will remain for %d months.",
				box_code(target),
				loyal_rate(target));
	return TRUE;
}


int
v_undead_lord(struct command *c)
{
	int where = subloc(c->who);
	int aura = c->a;

	if (aura < 3)
		c->a = aura = 3;
	if (aura > 8)
		c->a = aura = 8;

	if (!may_cookie_npc(c->who, where, item_undead_cookie))
		return FALSE;

	if (!check_aura(c->who, aura))
		return FALSE;

	return TRUE;
}


int
d_undead_lord(struct command *c)
{
	int where = subloc(c->who);
	int aura = c->a;
	int undead;
	int ret;
	int rating;

	if (!may_cookie_npc(c->who, where, item_undead_cookie))
		return FALSE;

	if (!charge_aura(c->who, aura))
		return FALSE;

	undead = do_cookie_npc(c->who, where, item_undead_cookie, c->who);

	if (undead == 0)
	{
		log_write(LOG_CODE, "d_undead_lord: why not?");
		wout(c->who, "Unable to summon a demon lord.");
		return FALSE;
	}

	switch (aura)
	{
	case 3:		rating = 100;	break;
	case 4:		rating = 150;	break;
	case 5:		rating = 190;	break;
	case 6:		rating = 220;	break;
	case 7:		rating = 240;	break;
	case 8:		rating = 250;	break;

	default:
		assert(FALSE);
	}

	p_char(undead)->attack = rating;
	p_char(undead)->defense = rating;

	set_loyal(undead, LOY_summon, 5);

	wout(c->who, "Summoned %s.", box_name(undead));
	wout(where, "%s has summoned %s.",
				box_name(c->who),
				liner_desc(undead));

	return TRUE;
}


int
v_banish_undead(struct command *c)
{
	if (!check_aura(c->who, 6))
		return FALSE;

	c->d = reset_cast_where(c->who);
	if (!keep_undead_check(c, FALSE))
		return FALSE;

	return TRUE;
}


int
d_banish_undead(struct command *c)
{
	int target = c->a;
	int where = c->d;
	int head;

	if (!charge_aura(c->who, 6))
		return FALSE;

	if (!keep_undead_check(c, FALSE))
		return FALSE;

	head = stack_leader(target);

	wout(head, "%s banishes %s!", box_name(c->who), box_name(target));
	wout(where, "%s banishes %s!", box_name(c->who), box_name(target));

	extract_stacked_unit(target);
	kill_char(target, 0);

	return TRUE;
}


int
v_eat_dead(struct command *c)
{
	int body = c->a;

	if (kind(body) != T_item || subkind(body) != sub_dead_body)
	{
		wout(c->who, "%s is not the dead body of a noble.",
						box_code(body));
		return FALSE;
	}

	if (has_item(c->who, body) == 0)
	{
		wout(c->who, "Don't have %s.", box_code(body));
		return FALSE;
	}

	if (has_item(c->who, item_ratspider_venom) == 0)
	{
		wout(c->who, "Requires %s.",
				box_name_qty(item_ratspider_venom, 1));
		return FALSE;
	}

	if (!check_aura(c->who, 5))
		return FALSE;

	return TRUE;
}


static void
get_some_skills(int who, int body, int chance)
{
	struct skill_ent *e;
	int parent;
	ilist to_learn = NULL;
	int i;
	int nps = 0;

/*
 *  First do category skills
 */

	loop_char_skill_known(body, e)
	{
		parent = skill_school(e->skill);
		if (parent != e->skill)
			continue;

		// Prevent Advanced Sorcery to be learned this way
		if (e->skill == 920)
			continue;

		if (has_skill(who, e->skill))
			continue;

		if (rnd(1,100) > chance)
			continue;

		ilist_append(&to_learn, e->skill);
	}
	next_char_skill_known;

/*
 *  Now do subskills
 *  Must know parent in order to pick up a subskill
 */

	loop_char_skill_known(body, e)
	{
		parent = skill_school(e->skill);
		if (parent == e->skill)
			continue;

		if (has_skill(who, e->skill))
			continue;

		if (has_skill(who, parent) == 0 &&
		    ilist_lookup(to_learn, parent) < 0)
			continue;

		if (rnd(1,100) > chance)
			continue;

		ilist_append(&to_learn, e->skill);
	}
	next_char_skill_known;

	if (ilist_len(to_learn) == 0)
	{
		wout(who, "No new skills can be learned from this brain.");
		ilist_reclaim(&to_learn);
		return;
	}

	for (i = 0; i < ilist_len(to_learn); i++)
		nps += skill_np_req(to_learn[i]);

	if (player_np(player(who)) < nps)
	{
		wout(who, "Don't have the required %d NP%s.",
				nps, add_s(nps));
		ilist_reclaim(&to_learn);
		return;
	}

	deduct_np(player(who), nps);

	for (i = 0; i < ilist_len(to_learn); i++)
		learn_skill(who, to_learn[i]);

	ilist_reclaim(&to_learn);
}


int
d_eat_dead(struct command *c)
{
	int body = c->a;

	if (kind(body) != T_item || subkind(body) != sub_dead_body)
	{
		wout(c->who, "%s is not the dead body of a noble.",
						box_code(body));
		return FALSE;
	}

	if (has_item(c->who, body) == 0)
	{
		wout(c->who, "Don't have %s.", box_code(body));
		return FALSE;
	}

	if (subkind(body) != sub_dead_body)
	{
		wout(c->who, "%s is not the dead body of a noble.",
						box_code(body));
		return FALSE;
	}

	if (consume_item(c->who, item_ratspider_venom, 1) == 0)
	{
		wout(c->who, "Requires %s.",
				box_name_qty(item_ratspider_venom, 1));
		return FALSE;
	}

	if (!charge_aura(c->who, 5))
		return FALSE;

	wout(c->who, "Consumed %s.", box_name(body));

	if (rnd(1,100) <= 33)
	{
		destroy_unique_item(c->who, body);
		kill_char(c->who, MATES);
		return TRUE;
	}

	get_some_skills(c->who, body, 100);
	destroy_unique_item(c->who, body);

	if (rnd(1,100) <= 25 && !char_sick(c->who))
	{
		p_char(c->who)->sick = TRUE;
		wout(c->who, "%s has fallen ill.", box_name(c->who));
	}

	return TRUE;
}


static int
random_body_here(int where)
{
	struct item_ent *e;
	static ilist l = NULL;

	ilist_clear(&l);

	loop_inv(where, e)
	{
		if (subkind(e->item) == sub_dead_body &&
		    sysclock.turn > p_char(e->item)->death_time.turn)
		{
			ilist_append(&l, e->item);
		}
	}
	next_inv;

	if (ilist_len(l) == 0)
		return 0;

	ilist_scramble(l);

	return l[0];
}


void
auto_undead(int who)
{
	int master;
	int where = subloc(who);

	master = npc_summoner(who);

	if (master && subloc(who) == subloc(master))
	{
		queue(who, "attack %s", box_code_less(master));
		p_misc(who)->summoned_by = 0;
		return;
	}

	if (loc_depth(where) != LOC_province || rnd(1,2) == 1)
	{
		npc_move(who);
		return;
	}

	queue(who, "pillage 1");
}


int
v_aura_blast(struct command *c)
{
	int target = c->a;
	int aura = c->b;
	int have_left = c->c;
	int where;

	if (in_safe_now(c->who))
	{
		wout(c->who, "Not allowed in a safe haven.");
		return FALSE;
	}

	where = reset_cast_where(c->who);
	if (!check_char_where(where, c->who, target)) {
		return FALSE;
	}

/*
	if (diff_region(c->who, target))
	{
		wout(c->who, "%s is too far away.", box_code(c->who));
		return FALSE;
	}
*/

	c->d = where;

	return TRUE;
}


int
d_aura_blast(struct command *c)
{
	int target = c->a;
	int aura = c->b;
	int have_left = c->c;
	int where = c->d;

	if (!check_char_where(where, c->who, target))
		return FALSE;

	if (in_safe_now(c->who) || in_safe_now(target))
	{
		wout(c->who, "Not allowed in a safe haven.");
		return FALSE;
	}

/*
	if (diff_region(c->who, target))
	{
		wout(c->who, "%s is too far away.", box_code(c->who));
		return FALSE;
	}
*/

	if (aura < 1)
		aura = char_cur_aura(c->who);
	
	if (have_left)
	{
		int m = max(char_cur_aura(c->who) - have_left, 0);

		if (aura > m)
			aura = m;
	}

	if (aura == 0)
	{
		wout(c->who, "No aura available for blast.");
		return FALSE;
	}

	if (!charge_aura(c->who, aura))
		return FALSE;

	vector_clear();
	vector_add(c->who);
	vector_add(target);
	vector_add(where);

	wout(VECT, "%s blasts %s with a burst of aura!",
			box_name(c->who), box_name(target));

	log_write(LOG_SPECIAL, "%s blasts %s with a burst of aura!",
			box_name(c->who), box_name(target));

	if (has_skill(target, sk_absorb_blast))
	{
		if (reflect_blast(target))
		{
			wout(VECT, "%s reflected the blast back to %s!",
					just_name(target), just_name(c->who));

			add_char_damage(c->who, aura * 2, MATES);
		}
		else
		{
			wout(VECT, "%s absorbed the blast!", just_name(target));

			p_magic(target)->cur_aura += aura/2;

			limit_cur_aura(target);

			wout(target, "Current aura is now %d.",
					rp_magic(target)->cur_aura);
		}
	}
	else
		add_char_damage(target, aura * 2, MATES);

	return TRUE;
}


int
v_aura_reflect(struct command *c)
{
	int flag = c->a;

	p_magic(c->who)->aura_reflect = flag;

	if (flag)
		wout(c->who, "Will reflect aura blasts back at the attacker.");
	else
		wout(c->who, "Will absorb aura blasts.");

	return TRUE;
}

