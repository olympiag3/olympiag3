
#include	<stdio.h>
#include	"z.h"
#include	"oly.h"



int
v_spy_inv(struct command *c)
{
	int target = c->a;

	if (!check_char_here(c->who, target))
		return FALSE;

	return TRUE;
}


int
d_spy_inv(struct command *c)
{
	int target = c->a;

	if (!check_still_here(c->who, target))
		return FALSE;

	wout(c->who, "Discovered the inventory of %s:", box_name(target));
	show_char_inventory(c->who, target);

	return TRUE;
}


int
v_spy_skills(struct command *c)
{
	int target = c->a;

	if (!check_char_here(c->who, target))
		return FALSE;

	return TRUE;
}


int
d_spy_skills(struct command *c)
{
	int target = c->a;

	if (!check_still_here(c->who, target))
		return FALSE;

	wout(c->who, "Learned the skills of %s:", box_name(target));
	list_skills(c->who, target);

	return TRUE;
}


int
v_spy_lord(struct command *c)
{
	int target = c->a;

	if (!check_char_here(c->who, target))
		return FALSE;

	return TRUE;
}


int
d_spy_lord(struct command *c)
{
	int target = c->a;
	int parent;

	if (!check_still_here(c->who, target))
		return FALSE;

	if (cloak_lord(target))
	{
		wout(c->who, "Failed to learn the lord of %s.",
					box_code(target));
		return FALSE;
	}

	parent = player(target);

	assert(valid_box(parent));

	wout(c->who, "%s is sworn to %s.", box_name(target),
				box_name(parent));

	return TRUE;
}


int
v_hide(struct command *c)
{
	int flag = c->a;

	if (!check_skill(c->who, sk_hide_self))
		return FALSE;

	if (flag && !char_alone(c->who))
	{
		wout(c->who, "Must be alone to hide.");
		return FALSE;
	}

	if (!flag)
	{
		p_magic(c->who)->hide_self = FALSE;
		wout(c->who, "No longer hidden.");

		c->wait = 0;
		c->inhibit_finish = TRUE;
		return TRUE;
	}

	return TRUE;
}


int
d_hide(struct command *c)
{

	if (!char_alone(c->who))
	{
		wout(c->who, "Must be alone to hide.");
		return FALSE;
	}

	p_magic(c->who)->hide_self = TRUE;

	wout(c->who, "Now hidden.");
	return TRUE;
}


int
v_sneak(struct command *c)
{
	struct exit_view *v;
	int where = subloc(c->who);
	int outside = subloc(where);
	int dest;

	if (!char_alone(c->who))
	{
		wout(c->who, "Must be alone in order to sneak.");
		return FALSE;
	}

	if (numargs(c) > 0)
	{
		v = parse_exit_dir(c, where, "sneak");

		if (v == NULL)
			return FALSE;

		dest = v->destination;
	}
	else
		dest = outside;

	if (dest == outside)
	{
		if (loc_depth(where) != LOC_build)
		{
			wout(c->who, "Not in a structure.");
			return FALSE;
		}

		if (subkind(outside) == sub_ocean)
		{
			wout(c->who, "May not leave while on the ocean.");
			return FALSE;
		}

		return TRUE;
	}

	if (loc_depth(v->destination) != LOC_build)
	{
		wout(c->who, "May only sneak into buildings and ships.");
		return FALSE;
	}

	if (v->impassable)
	{
		wout(c->who, "That route is impassable.");
		return FALSE;
	}

	if (v->in_transit)
	{
		wout(c->who, "%s is underway.  Boarding is not "
				"possible.", box_name(v->destination));
		return FALSE;
	}

	return TRUE;
}


int
d_sneak(struct command *c)
{
	struct exit_view *v;
	int where = subloc(c->who);
	int outside = subloc(where);
	int dest;

	if (!char_alone(c->who))
	{
		wout(c->who, "Must be alone in order to sneak.");
		return FALSE;
	}

	if (numargs(c) > 0)
	{
		v = parse_exit_dir(c, where, "sneak");

		if (v == NULL)
			return FALSE;

		dest = v->destination;
	}
	else
		dest = outside;

	if (dest == outside)
	{
		if (loc_depth(where) != LOC_build)
		{
			wout(c->who, "Not in a structure.");
			return FALSE;
		}

		if (subkind(outside) == sub_ocean)
		{
			wout(c->who, "May not leave while on the ocean.");
			return FALSE;
		}

		move_stack(c->who, outside);
		wout(c->who, "Now outside of %s.", box_name(where));

		return TRUE;
	}

	if (loc_depth(v->destination) != LOC_build)
	{
		wout(c->who, "May only sneak into buildings and ships.");
		return FALSE;
	}

	if (v->impassable)
	{
		wout(c->who, "That route is impassable.");
		return FALSE;
	}

	if (v->in_transit)
	{
		wout(c->who, "%s is underway.  Boarding is not "
				"possible.", box_name(v->destination));
		return FALSE;
	}

	move_stack(c->who, v->destination);
	wout(c->who, "Now inside %s.", box_name(v->destination));
	bark_dogs(v->destination);

	return TRUE;
}


void
clear_contacts(int stack)
{
	int i;

	if (kind(stack) == T_char)
	{
		loop_stack(stack, i)
		{
			ilist_clear(&(p_char(i)->contact));
		}
		next_stack;
	}
}


static void
add_contact(int a, int b)
{

	assert(kind(a) == T_char);

	ilist_append(&(p_char(a)->contact), b);
}


int
v_contact(struct command *c)
{

	while (numargs(c) > 0)
	{
		if (kind(c->a) != T_char && kind(c->a) != T_player)
		{
			wout(c->who, "%s is not a character or player entity.",
					c->parse[1]);
		}
		else
			ilist_append(&p_char(c->who)->contact, c->a);

		cmd_shift(c);
	}

	return TRUE;
}


int
v_seek(struct command *c)
{
	int target = c->a;

	if (target)	/* target specified */
	{
		if (kind(target) != T_char)
		{
			wout(c->who, "%s is not a character.", box_code(target));
			return FALSE;
		}

		if (char_here(c->who, target))
		{
			wout(c->who, "%s is here.", box_name(target));
			add_contact(target, c->who);

			c->wait = 0;
			c->inhibit_finish = TRUE;
			return TRUE;
		}
	}

	return TRUE;
}


int
d_seek(struct command *c)
{
	int target = c->a;
	int i;

	if (target)		/* target specified */
	{
		if (kind(target) != T_char)
		{
			wout(c->who, "%s is not a character.", box_code(target));
			return FALSE;
		}

		if (char_here(c->who, target))
		{
			wout(c->who, "%s is here.", box_name(target));
			add_contact(target, c->who);

			c->wait = 0;
			c->inhibit_finish = TRUE;	/* don't call d_wait */
			return TRUE;
		}

		if ((subloc(c->who) == subloc(target)) && (rnd(1,10) == 1))
		{
			add_contact(target, c->who);
			wout(c->who, "Found %s.", box_name(target));

			c->wait = 0;
			c->inhibit_finish = TRUE;	/* don't call d_wait */
			return TRUE;
		}

		return TRUE;
	}

/*
 *  5% chance of finding any hidden noble present
 */
	loop_here(subloc(c->who), i)
	{
		if (kind(i) != T_char || char_here(c->who, i))
			continue;

		if (rnd(1,100) > 5)
			continue;

		add_contact(i, c->who);
		wout(c->who, "Found %s.", box_name(i));

		break;
	}
	next_here;

	return TRUE;
}


static void
add_fill(int where, ilist *l, int max_depth, int depth)
{
	int i;
	struct entity_loc *p;

	assert(loc_depth(where) == LOC_province);

	if (ilist_lookup(*l, where) >= 0)
		return;

	ilist_append(l, where);

	p = rp_loc(where);
	if (p == NULL)
		return;

	if (depth >= max_depth)
		return;

	for (i = 0; i < ilist_len(p->prov_dest); i++)
		if (p->prov_dest[i])
			add_fill(p->prov_dest[i], l, max_depth, depth + 1);
}


int
v_find_rich(struct command *c)
{
	int where = subloc(c->who);

	if (subkind(where) != sub_inn)
	{
		wout(c->who, "May only be used in an inn.");
		return FALSE;
	}

	return TRUE;
}


int
d_find_rich(struct command *c)
{
	static ilist l = NULL;
	int pl = player(c->who);
	int max_gold = 500;
	int who_gold = 0;
	int i, j, n;
	int where = subloc(c->who);
	char *s;

	if (subkind(where) != sub_inn)
	{
		wout(c->who, "May only be used in an inn.");
		return FALSE;
	}

	ilist_clear(&l);

	add_fill(province(where), &l, 3, 1);

	for (i = 0; i < ilist_len(l); i++)
	{
		loop_all_here(l[i], j)
		{
			if (kind(j) != T_char || player(j) == pl)
				continue;

			n = has_item(j, item_gold);
			if (n >= max_gold)
			{
				max_gold = n;
				who_gold = j;
			}
		}
		next_all_here;
	}

	if (who_gold == 0)
		wout(c->who, "No weathy nobles are rumored to be nearby.");
	else
	{
		if (max_gold <= 1000)
			s = "large sum";
		else if (max_gold <= 2000)
			s = "considerable amount";
		else
			s = "vast quantity";

		wout(c->who, "Rumors claim that one %s is nearby, and "
			"possesses a %s of gold.", box_name(who_gold), s);
	}

	return TRUE;
}


int
v_torture(struct command *c)
{
	int target = c->a;

	if (!has_skill(c->who, sk_torture))
	{
		wout(c->who, "Requires %s.", box_name(sk_torture));
		return FALSE;
	}

	if (!is_prisoner(target) ||
	    stack_leader(target) != stack_leader(c->who))
	{
		wout(c->who, "%s is not a prisoner of %s.",
			box_code(target), box_name(c->who));
		return FALSE;
	}

	if (is_npc(target) || loyal_kind(target) == LOY_npc ||
	    loyal_kind(target) == LOY_summon)
	{
		wout(c->who, "NPC's cannot be tortured.");
		return FALSE;
	}

	return TRUE;
}


int
d_torture(struct command *c)
{
	int target = c->a;
	int chance;

	if (!is_prisoner(target) ||
	    stack_leader(target) != stack_leader(c->who))
	{
		wout(c->who, "%s is not a prisoner of %s.",
			box_code(target), box_name(c->who));
		return FALSE;
	}

	if (is_npc(target) || loyal_kind(target) == LOY_npc ||
	    loyal_kind(target) == LOY_summon)
	{
		wout(c->who, "NPC's cannot be tortured.");
		return FALSE;
	}

	add_char_damage(target, 50, c->who);

	if (!alive(target))
	{
		wout(c->who, "%s died under torture.", box_name(target));
		return FALSE;
	}

	switch (loyal_kind(target))
	{
	case LOY_oath:
		if (loyal_rate(target) == 1)
			chance = 10;
		else
			chance = 0;
		break;

	case LOY_contract:
		chance = 50;
		break;

	case LOY_fear:
		chance = 100;
		break;

	default:
		chance = 0;
	}

	if (rnd(1,100) > chance)
	{
		wout(c->who, "The prisoner refused to talk.");
		return FALSE;
	}

	add_skill_experience(c->who, sk_torture);

	wout(c->who, "%s belongs to faction %s.",
				box_name(target),
				box_name(player(target)));

	return TRUE;
}


int
cloak_lord(int n)
{

	return has_skill(n, sk_hide_lord);
}


int
v_petty_thief(struct command *c)
{
	int where = subloc(c->who);

	if (subkind(where) != sub_city)
	{
		wout(c->who, "Must be in a city.");
		return FALSE;
	}

	if (loc_pillage(where))
	{
		wout(c->who, "This city has recently been pillaged; there "
			"are no opportunities for thievery.");
		return FALSE;
	}

/*
 *  NOTYET:  problem with this is that if the command is interrupted,
 *	     the cookie isn't put back.
 */

	if (!consume_item(where, item_petty_thief, 1))
	{
	    wout(c->who, "A petty thief has already worked here this month.");
	    return FALSE;
	}

	return TRUE;
}


int
d_petty_thief(struct command *c)
{
	int where = subloc(c->who);
	int amount;
	char *self, *third;
	extern int gold_petty_thief;

	if (loc_pillage(where))
	{
		wout(c->who, "This city has recently been pillaged.  There "
			"are no opportunities for thievery.");
		return FALSE;
	}

	if (rnd(1,100) <= 5)
	{
		show_to_garrison = TRUE;
		vector_clear();
		vector_add(where);
		vector_add(c->who);

		switch (rnd(1,3))
		{
		case 1:
			wout(VECT, "%s was caught trying to steal from the "
					"city merchants, and given a beating.",
					box_name(c->who));
			break;

		case 2:
			wout(VECT, "%s was caught trying to pick pockets in "
			"the town square, and flogged by the townsfolk.",
					box_name(c->who));
			break;

		case 3:
			wout(VECT, "%s was caught stealing, and given a "
						"beating.", box_name(c->who));
			break;

		default:
			assert(FALSE);
		}

		show_to_garrison = FALSE;

		add_char_damage(c->who, rnd(5,15), MATES);
		return FALSE;
	}

	amount = rnd(50,150);
	consume_item(where, item_tax_cookie, amount);
	gen_item(c->who, item_gold, amount);
	gold_petty_thief += amount;

	switch (rnd(1,3))
	{
	case 1:
		self = " stealing from merchants";
		third = sout("%s merchants complain that they were robbed "
					"by a thief.",
					rnd(0,1) ? "Several" :
					cap(nice_num(rnd(2,3))));
		break;

	case 2:
		self = " picking pockets";
		third = sout("%s townspeople complain that their pockets were "
				"picked in the town square.",
					rnd(0,1) ? "Several" :
					cap(nice_num(rnd(2,3))));
		break;

	case 3:
		self = "";
		switch (rnd(1,3))
		{
		case 1:
			third = "There are rumors that a thief is loose in "
							"the city.";
			break;

		case 2:
			third = "There are rumors that a thief has been "
							"working the city.";
			break;

		case 3:
			third = "Reports of thievery are heard throughout "
							"the city.";
			break;

		default:
			assert(FALSE);
		}
		break;

	default:
		assert(FALSE);
	}

	wout(c->who, "Earned %s%s.", gold_s(amount), self);

	if (third)
	{
		show_to_garrison = TRUE;
		wout(where, "%s", third);
		show_to_garrison = FALSE;
	}

	return TRUE;
}

