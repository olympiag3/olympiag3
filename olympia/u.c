
#include	<stdio.h>
#include	<string.h>
#include	<time.h>
#include	"z.h"
#include	"oly.h"


/*
 *  u.c -- the useful function junkyard
 */


void
kill_stack_ocean(int who)
{
	static ilist l = NULL;
	int i;
	int where;

	ilist_clear(&l);

	loop_stack(who, i)
	{
		ilist_append(&l, i);
	}
	next_stack;

	for (i = ilist_len(l) - 1; i >= 0; i--)
	{
		kill_char(l[i], 0);

		if (kind(l[i]) == T_char)	/* not dead yet! */
		{
			extract_stacked_unit(l[i]);
			where = find_nearest_land(province(l[i]));

			out(l[i], "%s washed ashore at %s.",
						box_name(l[i]),
						box_name(where));

			log_write(LOG_SPECIAL, "kill_stack_ocean, swam "
				"ashore, who=%s", box_code_less(l[i]));
			move_stack(l[i], where);
		}
	}
}


int
survive_fatal(int who)
{

	if (!has_skill(who, sk_survive_fatal))
	{
		return FALSE;
	}

	if (forget_skill(who, sk_survive_fatal))
	{
		wout(who, "%s would have died, but survived a fatal wound!",
						box_name(who));
		wout(who, "Forgot %s.", box_code(sk_survive_fatal));
		wout(who, "Health is now 100.");

		p_char(who)->health = 100;
		p_char(who)->sick = FALSE;

		return TRUE;
	}

	if (is_magician(who))
	{
		p_magic(who)->cur_aura = 0;
		/* wout(who, "Aura is now 0."); */
	}

	return FALSE;
}


void
char_reclaim(int who)
{

	p_char(who)->melt_me = TRUE;
#if 0
	kill_char(who, MATES);
#else
	kill_char(who, 0);	/* QUIT shouldn't give items to stackmates */
#endif
}


int
v_reclaim(struct command *c)
{
	char *what;

	if (numargs(c) < 1 || c->parse[1] == NULL || *(c->parse[1]) == '\0')
		what = "disperses.";
	else
		what = c->parse[1];

	wout(subloc(c->who), "%s %s", box_name(c->who), what);
	char_reclaim(c->who);
	return TRUE;
}


int
new_char(int sk, int ni, int where, int health, int pl,
					int loy_kind, int loy_lev, char *name)
{
	int new;
	struct entity_char *p;

	new = new_ent(T_char, sk);

	if (new < 0)
		return -1;

	if (name && *name)
		set_name(new, name);
	p = p_char(new);
	p->health = health;
	p->unit_item = ni;
	p->break_point = 50;

	p_char(new)->attack = 60;
	rp_char(new)->defense = 60;

	if (is_loc_or_ship(where))
		set_where(new, where);
	else
		set_where(new, subloc(where));

	set_lord(new, pl, loy_kind, loy_lev);

	if (kind(where) == T_char)
		join_stack(new, where);

	if (beast_capturable(new) || is_npc(new))
		p->break_point = 0;

	return new;
}


int
loc_depth(int n)
{

	switch (subkind(n))
	{
	case sub_region:
				return LOC_region;

	case sub_ocean:
	case sub_forest:
	case sub_plain:
	case sub_mountain:
	case sub_desert:
	case sub_swamp:
	case sub_under:
	case sub_cloud:
	case sub_tunnel:
	case sub_chamber:
				return LOC_province;

	case sub_island:
	case sub_stone_cir:
	case sub_mallorn_grove:
	case sub_bog:
	case sub_cave:
	case sub_city:
	case sub_lair:
	case sub_graveyard:
	case sub_ruins:
	case sub_battlefield:
	case sub_ench_forest:
	case sub_rocky_hill:
	case sub_tree_circle:
	case sub_pits:
	case sub_pasture:
	case sub_oasis:
	case sub_yew_grove:
	case sub_sand_pit:
	case sub_sacred_grove:
	case sub_poppy_field:
	case sub_faery_hill:
	case sub_hades_pit:
				return LOC_subloc;

	case sub_temple:
	case sub_galley:
	case sub_roundship:
	case sub_castle:
	case sub_galley_notdone:
	case sub_roundship_notdone:
	case sub_ghost_ship:
	case sub_temple_notdone:
	case sub_inn:
	case sub_inn_notdone:
	case sub_castle_notdone:
	case sub_mine:
	case sub_mine_notdone:
	case sub_mine_collapsed:
	case sub_tower:
	case sub_tower_notdone:
	case sub_sewer:
				return LOC_build;

	default:
		fprintf(stderr, "subkind is %d\n", subkind(n));
		assert(FALSE);
	}

	return 0;
}


/*
 *  First try to give items to someone below, then to someone above.
 */

int
stackmate_inheritor(int who)
{
	int i;
	int target = 0;

	loop_here(who, i)
	{
		if (kind(i) == T_char && !is_prisoner(i))
		{
			target = i;
			break;
		}
	}
	next_here;

	if (target)
		return target;

	return stack_parent(who);
}


void
take_unit_items(int from, int inherit, int how_many)
{
	int to;
	struct item_ent *e;
	int first = TRUE;
	int qty;
	int silent;
	int i;
	extern int gold_combat;
	extern int gold_combat_indep;

	switch (inherit)
	{
	case 0:
		to = 0;
		silent = TRUE;
		break;

	case MATES:
		to = stackmate_inheritor(from);
		silent = FALSE;
		break;

	case MATES_SILENT:
		to = stackmate_inheritor(from);
		silent = TRUE;
		break;

	default:
		to = inherit;
		silent = FALSE;
	}

	if (how_many == TAKE_NI)
		gen_item(from, noble_item(from), 1);

	loop_inv(from, e)
	{
		qty = e->qty;

		if (e->qty && how_many == TAKE_SOME && rnd(1,2) == 1)
			qty = rnd(0, e->qty);

#if 0
/*
 *  Don't let unique items get dropped this way
 */
		if (qty == 0 && item_unique(e->item))
			qty = 1;
#endif

		if (qty > 0 && !silent && valid_box(to))
		{
			if (first)
			{
				first = FALSE;
				wout(to, "Taken from %s:", box_name(from));
				indent += 3;
			}

			wout(to, "%s", box_name_qty(e->item, qty));
		}

		move_item(from, to, e->item, qty);

		if (e->item == item_gold && player(from) != player(to))
		{
			if (player(from) == indep_player)
				gold_combat_indep += qty;
			else
				gold_combat += qty;
		}

		if (qty != e->qty)
			move_item(from, 0, e->item, e->qty - qty);
	}
	next_inv;


/*
 *  Now give prisoners too
 */

	loop_here(from, i)
	{
		if (kind(i) == T_char && is_prisoner(i))
		{
			if (to > 0)
			{
				if (first && !silent)
				{
				    wout(to, "Taken from %s:", box_name(from));
				    indent += 3;
				    first = FALSE;
				}

				move_prisoner(from, to, i);

				if (!silent)
					wout(to, "%s", liner_desc(i));

				if (player(i) == player(to))
					p_char(i)->prisoner = FALSE;
			}
			else
			{
				p_magic(i)->swear_on_release = FALSE;
				drop_stack(from, i);
			}
		}
	}
	next_here;

	if (!first && !silent)
		indent -= 3;
}


void
add_char_damage(int who, int amount, int inherit)
{
	struct entity_char *p;

	if (amount <= 0)
		return;

	p = p_char(who);

	if (p->health == -1)
	{
		if (amount >= 50)
			kill_char(who, inherit);
		return;
	}

	if (p->health > 0)
	{
		if (amount > p->health)
			amount = p->health;

		p->health -= amount;
		assert(p->health >= 0);

		wout(who, "%s is wounded.  Health is now %d.",
					box_name(who), p->health);
	}

	if (p->health <= 0)
	{
		kill_char(who, inherit);
	}
	else if (!p->sick && rnd(1,100) > p->health)
	{
		p->sick = TRUE;
		wout(who, "%s has fallen ill.", box_name(who));
	}
}


void
put_back_cookie(int who)
{
	struct entity_misc *p;

	p = rp_misc(who);

	if (p == NULL || p->npc_home == 0)
		return;

	gen_item(p->npc_home, p->npc_cookie, 1);
}


#if 0
static int
nearby_grave(int where)
{
	struct entity_loc *p;
	int i;
	static ilist l = NULL;

	where = province(where);
	p = rp_loc(where);

	if (p && p->near_grave)
		return p->near_grave;

	log(LOG_CODE, "%s has no nearby grave", box_name(where));

	ilist_clear(&l);
	loop_subkind(sub_graveyard, i)
	{
		ilist_append(&l, i);
	}
	next_subkind;

	assert(ilist_len(l) > 0);

	ilist_scramble(l);

	return l[rnd(0, ilist_len(l)-1)];
}
#endif


static void
dead_char_body(int pl, int who)
{
	int grave;
	struct entity_item *p;

	grave = province(who);
	if (subkind(grave) == sub_ocean)
		grave = find_nearest_land(grave);

	set_where(who, 0);

/*
 *  npc dead bodies don't stick around
 *  bodies lost at sea get eaten by fish
 */

	if (char_melt_me(who) || is_npc(who) || grave == 0)
	{
		change_box_kind(who, T_deadchar);

		if (subkind(who))
			change_box_subkind(who, 0);
		return;
	}

	change_box_kind(who, T_item);
	change_box_subkind(who, sub_dead_body);

	p_misc(who)->save_name = bx[who]->name;
	bx[who]->name = str_save("dead body");

	rp_misc(who)->old_lord = pl;

	p = p_item(who);
	p->weight = item_weight(item_peasant);
	p->plural_name = str_save("dead bodies");

	hack_unique_item(who, grave);
}


static int
sub_item(int who, int item, int qty)
{
	int i;

	assert(valid_box(who));
	assert(valid_box(item));
	assert(qty >= 0);

	for (i = 0; i < plist_len(bx[who]->items); i++)
		if (bx[who]->items[i]->item == item)
	{
			if (bx[who]->items[i]->qty < qty)
				return FALSE;

			bx[who]->items[i]->qty -= qty;
			return TRUE;
		}

	return FALSE;
}


void
restore_dead_body(int owner, int who)
{
	struct entity_misc *pm;
	struct entity_item *pi;
	struct entity_char *pc;

	log_write(LOG_CODE, "dead body revived: who=%s, owner=%s, player=%s",
			box_code_less(who),
			box_code_less(owner),
			box_code_less(player(owner)));

/*
 *  Stolen from destroy_unique_item:
 */
	{
		int ret;

		ret = sub_item(owner, who, 1);
		assert(ret);

		p_item(who)->who_has = 0;
	}

	change_box_kind(who, T_char);
	change_box_subkind(who, 0);

	pm = p_misc(who);
	pi = p_item(who);
	pc = p_char(who);

	pi->weight = 0;
	my_free(pi->plural_name);
	pi->plural_name = NULL;

	if (pm->save_name && *pm->save_name)
	{
		set_name(who, pm->save_name);
		my_free(pm->save_name);
		pm->save_name = NULL;
	}

	pc->health = 100;
	pc->sick = FALSE;

	set_where(who, subloc(owner));

	if (kind(pm->old_lord) == T_player)
	{
		wout(pm->old_lord, "%s has been brought back to life.",
						box_name(who));

		set_lord(who, pm->old_lord, LOY_UNCHANGED, 0);
	}
	else
	{
		set_lord(who, indep_player, LOY_UNCHANGED, 0);
	}

	pm->old_lord = 0;

	{
		int def = char_defense(who);

		def -= 50;
		if (def < 0)
			def = 0;

		p_char(who)->defense = def;
	}
}


void
kill_char(int who, int inherit)
{
	int where = subloc(who);
	int pl = player(who);

	assert(kind(who) == T_char);

	if (!char_melt_me(who) && survive_fatal(who))
		return;

	p_char(who)->prisoner = FALSE;
	wout(who, "*** %s has %s ***", just_name(who),
				char_melt_me(who) ? "vanished" : "died");

	{
		int sp = stack_parent(who);

		if (sp)
			wout(sp, "%s has %s.", box_name(who),
				char_melt_me(who) ? "vanished" : "died");
	}

	p_char(who)->prisoner = TRUE;		/* suppress output */

	log_write(LOG_DEATH, "%s %s in %s.", box_name(who),
				char_melt_me(who) ? "melted" : "died",
				char_rep_location(who));

	take_unit_items(who, inherit, TAKE_SOME);

	extract_stacked_unit(who);
	{
		int i;

		loop_here(who, i)
		{
			fprintf(stderr, "i=%s\n", box_name_kind(i));
			assert(FALSE);
		}
		next_here;
	}

	flush_unit_orders(player(who), who);
	interrupt_order(who);

	if (is_magician(who))
		p_magic(who)->cur_aura = 0;

	if (!char_melt_me(who) && has_skill(who, sk_transcend_death))
	{
		int hades_point = random_hades_loc();

		p_char(who)->prisoner = FALSE;

		log_write(LOG_SPECIAL, "%s transcends death", box_name(who));
		log_write(LOG_SPECIAL, "...%s moved to %s",
				box_name(who), box_name(hades_point));
		p_char(who)->prisoner = FALSE;
		p_char(who)->sick = FALSE;
		p_char(who)->health = 100;
		move_stack(who, hades_point);
		wout(who, "%s appears at %s.",
				box_name(who), box_name(hades_point));

		return;
	}

	{
		int token_item = our_token(who);
		struct entity_player *token_pl;
		int who_has;

		if (token_item)
		{
			who_has = item_unique(token_item);
			token_pl = p_player(token_item);

			ilist_rem_value(&token_pl->units, who);

			if (!char_melt_me(who))
				p_item_magic(token_item)->token_num--;

			if (item_token_num(token_item) <= 0)
			{
				if (player(who_has) == sub_pl_regular)
					wout(who_has, "%s vanishes.",
						box_name(token_item));
				destroy_unique_item(who_has, token_item);
			}
		}
	}

	unit_deserts(who, 0, TRUE, LOY_UNCHANGED, 0);

	put_back_cookie(who);
	p_char(who)->death_time = sysclock;	/* record time of death */
	p_char(who)->prisoner = FALSE;
	dead_char_body(pl, who);
}


/*
 *  Has a contacted b || has b found a?
 */

int
contacted(int a, int b)
{
	struct entity_char *p;

	p = p_char(a);

	if (ilist_lookup(p->contact, b) >= 0)
		return TRUE;

	if (ilist_lookup(p->contact, player(b)) >= 0)
		return TRUE;

	return FALSE;
}


int
char_where(int where, int who, int target)
{
	int pl;

	if (where != subloc(target))
		return FALSE;

	if (char_really_hidden(target) ||
		(loc_depth(where) == LOC_province &&
		weather_here(where, sub_fog)))
	{
		pl = player(who);
		if (pl == player(target))
			return TRUE;

		if (target == garrison_here(where))
			return TRUE;

		if (contacted(target, who))
			return TRUE;

		if (stack_leader(target) == stack_leader(who))
			return TRUE;

		return FALSE;
	}

	return TRUE;
}


int
char_here(int who, int target)
{
	int where = subloc(who);

	return char_where(where, who, target);
}


int
check_char_where(int where, int who, int target)
{

	if (target == garrison_magic)
	{
		wout(who, "There is no garrison here.");
		return FALSE;
	}

	if (kind(target) != T_char)
	{
		wout(who, "%s is not a character.", box_code(target));
		return FALSE;
	}

	if (!char_where(where, who, target))
	{
		wout(who, "%s cannot be seen here.", box_code(target));
		return FALSE;
	}

	return TRUE;
}


int
check_char_here(int who, int target)
{
	int where = subloc(who);

	return check_char_where(where, who, target);
}


int
check_char_gone(int who, int target)
{

	if (target == garrison_magic)
	{
		wout(who, "There is no garrison here.");
		return FALSE;
	}

	if (kind(target) != T_char)
	{
		wout(who, "%s is not a character.", box_code(target));
		return FALSE;
	}

	if (!char_here(who, target))
	{
		wout(who, "%s cannot be seen here.", box_code(target));
		return FALSE;
	}

	if (char_gone(target))
	{
		wout(who, "%s has left.", box_name(target));
		return FALSE;
	}

	return TRUE;
}


int
check_still_here(int who, int target)
{

	if (target == garrison_magic)
	{
		wout(who, "There is no garrison here.");
		return FALSE;
	}

	if (kind(target) != T_char)
	{
		wout(who, "%s is not a character.", box_code(target));
		return FALSE;
	}

	if (!char_here(who, target))
	{
		wout(who, "%s can no longer be seen here.", box_name(target));
		return FALSE;
	}

#if 0
	if (char_gone(target))
	{
		wout(who, "%s is no longer here.", box_name(target));
		return FALSE;
	}
#endif

	return TRUE;
}


int
check_skill(int who, int skill)
{

	if (has_skill(who, skill) < 1)
	{
		wout(who, "Requires %s.", box_name(skill));
		return FALSE;
	}

	return TRUE;
}


static void
sink_ship(int ship)
{
	int who;
	struct entity_subloc *p;
	int i;
	int storm;
	int where = subloc(ship);

	log_write(LOG_SPECIAL, "%s has sunk in %s.", box_name(ship),
					box_name(subloc(ship)));

	wout(ship, "%s has sunk!", box_name(ship));
	wout(subloc(ship), "%s has sunk!", box_name(ship));

	if (subkind(where) == sub_ocean)
	{
		loop_here(ship, who)
		{
			if (kind(who) == T_char)
				kill_stack_ocean(who);
		}
		next_here;
	}
	else
	{
		loop_here(ship, who)
		{
			if (kind(who) == T_char)
				move_stack(who, where);
			else
				set_where(who, where);
		}
		next_here;
	}

/*
 *  Unbind any storms bound to this ship
 */

	p = rp_subloc(ship);
	if (p)
	{
		for (i = 0; i < plist_len(p->bound_storms); i++)
		{
			storm = p->bound_storms[i];
			if (kind(storm) == T_storm)
				p_misc(storm)->bind_storm = 0;
		}

		ilist_clear(&p->bound_storms);
	}

	set_where(ship, 0);
	delete_box(ship);
}


void
get_rid_of_collapsed_mine(int fort)
{
	int who;
	int where = subloc(fort);

	assert(subkind(fort) == sub_mine_collapsed);

/*
 *  Move anything inside, out, just in case
 */

	loop_here(fort, who)
	{
		if (kind(who) == T_char)
			move_stack(who, where);
		else
			set_where(who, where);
	}
	next_here;

	set_where(fort, 0);
	delete_box(fort);
}


static void
building_collapses(int fort)
{
	int who;
	int where = subloc(fort);

	log_write(LOG_SPECIAL, "%s collapsed in %s.", box_name(fort),
					box_name(where));

	vector_char_here(fort);
	vector_add(where);
	wout(VECT, "%s collapses!", box_name(fort));

	loop_here(fort, who)
	{
		if (kind(who) == T_char)
			move_stack(who, where);
		else
			set_where(who, where);
	}
	next_here;

	if (subkind(fort) == sub_mine)
	{
		change_box_subkind(fort, sub_mine_collapsed);
		p_misc(fort)->mine_delay = 8;
		return;
	}

	if (subkind(fort) == sub_castle)
	{
		int i;

		loop_garrison(i)
		{
			if (garrison_castle(i) == fort)
				p_misc(i)->garr_castle = 0;
		}
		next_garrison;
	}

	set_where(fort, 0);
	delete_box(fort);
}


int
add_structure_damage(int fort, int damage, int can_destroy)
{
	struct entity_subloc *p;

	assert(damage >= 0);

	p = p_subloc(fort);

	if (p->damage + damage > 100)
		p->damage = 100;
	else
		p->damage += damage;

	if (p->damage < 100)
		return FALSE;

	if (!can_destroy)
	{
		p->damage = 99;
		return FALSE;
	}
/*
 *  Completely destroyed
 */

	if (is_ship(fort))
		sink_ship(fort);
	else
		building_collapses(fort);

	return TRUE;
}


int
count_man_items(int who)
{
	struct item_ent *e;
	int sum = 1;

#if 1
	if (subkind(who) == sub_garrison)
		sum = 0;
#endif

	loop_inv(who, e)
	{
		if (man_item(e->item))
			sum += e->qty;
	}
	next_inv;

	return sum;
}


int
count_stack_units(int who)
{
	int i;
	int sum = 1;

	loop_char_here(who, i)
	{
		sum++;
	}
	next_char_here;

	return sum;
}

int
count_stack_move_nobles(int who)
{
	int i;
	int sum = 1;
	int pl = player(who);

	loop_char_here(who, i)
	{
		if (player(i) == pl || (!is_npc(i) && !is_prisoner(i)))
			sum++;
	}
	next_char_here;

	return sum;
}

int
count_stack_figures(int who)
{
	int i;
	int sum = 0;

	loop_stack(who, i)
	{
		sum += count_any(i);
	}
	next_stack;

	return sum;
}

int
count_stack_any(int who)
{
	int i;
	int sum = 0;

	loop_stack(who, i)
	{
		sum += count_any(i);
	}
	next_stack;

	return sum;
}


#if 0
int
count_figures_here(int where)
{
	int who;
	int sum = 0;

	loop_here(where, who)
	{
		if (kind(who) == T_char)
			sum += count_stack_figures(who);
	}
	next_here;

	return sum;
}
#endif


int
count_fighters(int who)
{
	struct item_ent *e;
	int sum = 0;

	loop_inv(who, e)
	{
		if (is_fighter(e->item))
			sum += e->qty;
	}
	next_inv;

	return sum;
}


int
count_stack_fighters(int who)
{
	int i;
	int sum = 0;

	loop_stack(who, i)
	{
		sum += count_fighters(i);
	}
	next_stack;

	return sum;
}

int
count_any(int who)
{
	struct item_ent *e;
	int sum = 1;

#if 1
	if (subkind(who) == sub_garrison)
		sum = 0;
#endif

	loop_inv(who, e)
	{
		if (man_item(e->item) || is_fighter(e->item))
			sum += e->qty;
	}
	next_inv;

	return sum;
}


int
count_loc_char_item(int where, int item)
{
	int i;
	int sum = 0;

	loop_char_here(where, i)
	{
		sum += has_item(i, item);
	}
	next_char_here;

	return sum;
}


void
clear_temps(int kind)
{
	int i;

	loop_kind(kind, i)
	{
		bx[i]->temp = 0;
	}
	next_kind;
}


void
olytime_increment(olytime *p)
{

	p->days_since_epoch++;
	p->day++;
}


/*
 *  Ready counter for next turn
 *  Must be followed by an olytime_increment
 */

void
olytime_turn_change(olytime *p)
{

	p->day = 0;
	p->turn++;
}


int
max(int a, int b)
{

	if (a > b)
		return a;
	return b;
}


int
min(int a, int b)
{

	if (a < b)
		return a;
	return b;
}


/*
 *  Olympian weight system
 *
 *	Each item has three fields related to weight and carrying
 *	capacity:
 *
 *		weight			fetch with item_weight(item)
 *		land capacity		fetch with land_cap(item)
 *		ride capacity		fetch with ride_cap(item)
 *
 *	Weight is the complete weight of the item, such as 100 for
 *	men, or 1,000 for oxen.
 *
 *	land capacity is how much the item can carry walking,
 *	not counting its own weight.
 *
 *	ride capacity is how much the item can carry on horseback,
 *	not counting its own weight.
 *
 *	if the item can carry itself riding or walking, but can not
 *	carry any extra goods, set the capacity to -1.  This is because
 *	0 represents "not set" instead of a value.
 *
 *	For example, a wild horse can walk and ride, but cannot be laden
 *	with rider or inventory.  Therefore, its land_cap is -1, and its
 *	ride_cap is -1.
 *
 *	An ox can carry great loads: land_cap 1500.  Perhaps it can trot
 *	alongside horses, but can carry no inventory if doing so.
 *	ride_cap -1.
 *
 */


static void
add_item_weight(int item, int qty, struct weights *w)
{
	int wt, lc, rc, fc;

	wt = item_weight(item) * qty;
	lc = item_land_cap(item);
	rc = item_ride_cap(item);
	fc = item_fly_cap(item);

	if (lc)
		w->land_cap += max(lc, 0) * qty;
	else
		w->land_weight += wt;

	if (rc)
		w->ride_cap += max(rc, 0) * qty;
	else
		w->ride_weight += wt;

	if (fc)
		w->fly_cap += max(fc, 0) * qty;
	else
		w->fly_weight += wt;

	w->total_weight += wt;

	if (item_animal(item))
		w->animals += qty;
}


void
determine_unit_weights(int who, struct weights *w)
{
	struct item_ent *e;
	int unit_base;

	assert(kind(who) == T_char);

	memset(w, '\0', sizeof (*w));

	unit_base = noble_item(who);
	if (unit_base == 0)
		unit_base = item_peasant;

	add_item_weight(unit_base, 1, w);

	loop_inv(who, e)
	{
		add_item_weight(e->item, e->qty, w);
	}
	next_inv;
}


void
determine_stack_weights(int who, struct weights *w)
{
	struct weights v;
	int i;

	determine_unit_weights(who, w);

	loop_all_here(who, i)
	{
		determine_unit_weights(i, &v);
		w->total_weight += v.total_weight;
		w->land_cap += v.land_cap;
		w->ride_cap += v.ride_cap;
		w->fly_cap += v.fly_cap;
		w->land_weight += v.land_weight;
		w->ride_weight += v.ride_weight;
		w->fly_weight += v.fly_weight;
		w->animals += v.animals;
	}
	next_all_here;
}


int weight_display_flag = 0;

int
ship_weight(int ship)
{
	int i;
	int sum = 0;
	struct weights w;

	assert(kind(ship) == T_ship);

	loop_char_here(ship, i)
	{
		determine_unit_weights(i, &w);
		sum += w.total_weight;

#if 0
		if (weight_display_flag)
		{
			indent += 3;
			wout(weight_display_flag, "weight of %s is %d",
					box_name(i), w.total_weight);
			indent -= 3;
		}
#endif
	}
	next_char_here;

	return sum;
}


int
lookup(char *table[], char *s)
{
	int i = 0;

	while (table[i] != NULL)
		if (i_strcmp(s, table[i]) == 0)
			return i;
		else
			i++;

	return -1;
}


char *
loyal_s(int who)
{
	char *s;

	switch (loyal_kind(who))
	{
	case 0:
		s = "unsworn";
		break;

	case LOY_contract:
		s = "contract";
		break;

	case LOY_oath:
		s = "oath";
		break;

	case LOY_fear:
		s = "fear";
		break;

	case LOY_npc:
		s = "npc";
		break;

	case LOY_summon:
		s = "summon";
		break;

	default:
		assert(FALSE);
	}

	return sout("%s-%d", s, loyal_rate(who));
}


char *
gold_s(int n)
{

	return sout("%s~gold", comma_num(n));
}


char *
weeks(int n)
{

	if (n == 0)
		return "0~days";

	if (n % 7 == 0)
	{
		n = n / 7;
		return sout("%s~week%s", nice_num(n), add_s(n));
	}

	return sout("%s~day%s", nice_num(n), add_s(n));
}


char *
more_weeks(int n)
{

	if (n == 0)
		return "0~more days";

	if (n % 7 == 0)
	{
		n = n / 7;
		return sout("%d~more week%s", n, add_s(n));
	}

	return sout("%d~more day%s", n, add_s(n));
}


char *
comma_num(int n)
{
	int ones;
	int thousands;
	int millions;
	int further;

	further = n / 1000000000;
	n = n % 1000000000;

	millions = n / 1000000;
	n = n % 1000000;

	thousands = n / 1000;
	ones = n % 1000;

	if (further == 0 && millions == 0 && thousands == 0)
		return sout("%d", ones);
	else if (further == 0 && millions == 0)
		return sout("%d,%.3d", thousands, ones);
	else if (further == 0)
		return sout("%d,%.3d,%.3d", millions, thousands, ones);
	else
		return sout("%d,%.3d,%.3d,%.3d", further, millions,
						thousands, ones);
}


static char *num_s[] = { "zero", "one", "two", "three", "four", "five",
				"six", "seven", "eight", "nine", "ten" };


char *
nice_num(int n)
{

	if (n > 10 || n < 0)
		return sout("%s", comma_num(n));
	else
		return num_s[n];
}


char *
knum(int n, int nozero)
{

	if (n == 0 && nozero)
		return "";

	if (n < 9999)
		return sout("%d", n);

	if (n < 1000000)
		return sout("%dk", n/1000);

	return sout("%dM", n/1000000);
}


char *
ordinal(int n)
{

	if (n >= 10 && n <= 19)
		return sout("%sth", comma_num(n));

	switch (n % 10) {
	case 1:		return sout("%sst", comma_num(n));
	case 2:		return sout("%snd", comma_num(n));
	case 3:		return sout("%srd", comma_num(n));
	default:	return sout("%sth", comma_num(n));
	}
}

char *
cap(char *s)				/* return a capitalized copy of s */
{
	char *t;

	if (s == NULL || *s == '\0')
		return s;

	t = sout("%s", s);
	*t = toupper(*t);

	return t;
}


int
deduct_np(int pl, int num)
{
	struct entity_player *p;

	assert(kind(pl) == T_player);

	p = p_player(pl);

	if (p->noble_points < num)
		return FALSE;

	p->noble_points -= num;
	p->np_spent += num;
	return TRUE;
}


void
add_np(int pl, int num)
{
	struct entity_player *p;

	assert(kind(pl) == T_player);

	p = p_player(pl);
	p->noble_points += num;
	p->np_gained += num;
}


int
deduct_aura(int who, int amount)
{
	struct char_magic *p;

	p = rp_magic(who);

	if (p == NULL || p->cur_aura < amount)
		return FALSE;

	p->cur_aura -= amount;
	return TRUE;
}


int
charge_aura(int who, int amount)
{

	if (!deduct_aura(who, amount))
	{
		wout(who, "%s aura required, current level is %s.",
			cap(nice_num(amount)), nice_num(char_cur_aura(who)));
		return FALSE;
	}

	return TRUE;
}


int
check_aura(int who, int amount)
{

	if (char_cur_aura(who) < amount)
	{
		wout(who, "%s aura required, current level is %s.",
			cap(nice_num(amount)), nice_num(char_cur_aura(who)));
		return FALSE;
	}

	return TRUE;
}


int
has_item(int who, int item)
{
	int i;

	assert(valid_box(who));
#if 0
	if (!valid_box(item)) {
		fprintf(stderr, "has_item(who=%s, item=%s) failure\n",
				box_code_less(who), box_code_less(item));
		fprintf(stderr, "player(who) = %s\n", box_code_less(player(who)));
		fprintf(stderr, "c->line '%s'\n", bx[who]->cmd->line);
		assert(FALSE);
	}
#endif
	assert(valid_box(item));

	for (i = 0; i < plist_len(bx[who]->items); i++)
		if (bx[who]->items[i]->item == item)
			return bx[who]->items[i]->qty;

	return 0;
}


static void
add_item(int who, int item, int qty)
{
	struct item_ent *new;
	int i;
	int old;

	assert(valid_box(who));
	assert(valid_box(item));
	assert(qty >= 0);

	{
		int lore;

		lore = item_lore(item);

		if (lore && kind(who) == T_char && !test_known(who, item))
		{
			queue_lore(who, item, FALSE);
		}
	}

	for (i = 0; i < plist_len(bx[who]->items); i++)
		if (bx[who]->items[i]->item == item)
		{
			old = bx[who]->items[i]->qty;

			bx[who]->items[i]->qty += qty;

			investigate_possible_trade(who, item, old);
			return;
		}

	new = my_malloc(sizeof(*new));
	new->item = item;
	new->qty = qty;

	plist_append((plist *) &bx[who]->items, new);

	investigate_possible_trade(who, item, 0);
}


void
gen_item(int who, int item, int qty)
{

	assert(!item_unique(item));
	add_item(who, item, qty);
}


int
consume_item(int who, int item, int qty)
{

	assert(!item_unique(item));
	return sub_item(who, item, qty);
}


/*
 *  Move item from one unit to another
 *  Destination=0 means discard the items
 */

int
move_item(int from, int to, int item, int qty)
{

	if (qty <= 0)
		return TRUE;

	if (to == 0)
		return drop_item(from, item, qty);

	if (sub_item(from, item, qty))
	{
		add_item(to, item, qty);

		if (item_unique(item))
		{
			assert(qty == 1);
			p_item(item)->who_has = to;

			if (subkind(item) == sub_npc_token)
				move_token(item, from, to);
		}

		return TRUE;
	}

	return FALSE;
}


void
hack_unique_item(int item, int owner)
{

	p_item(item)->who_has = owner;
	add_item(owner, item, 1);
}


int
create_unique_item(int who, int sk)
{
	int new;

	new = new_ent(T_item, sk);

	if (new < 0)
		return -1;

	p_item(new)->who_has = who;
	add_item(who, new, 1);

	return new;
}


int
create_unique_item_alloc(int new, int who, int sk)
{

	alloc_box(new, T_item, sk);

	p_item(new)->who_has = who;
	add_item(who, new, 1);

	return new;
}


void
destroy_unique_item(int who, int item)
{
	int ret;

	assert(kind(item) == T_item);
	assert(item_unique(item));

	if (subkind(item) == sub_dead_body)
	{
		int pl = body_old_lord(item);

		if (pl == indep_player)
			pl = p_char(item)->prev_lord;

		if (kind(pl) == T_player)
		{
			int nps = char_np_total(item);

			out(pl, "%s~%s has passed on.  Gained %d NP%s.",
				rp_misc(item)->save_name, box_code(item),
				nps, add_s(nps));
			add_np(pl, nps);
		}
	}

	ret = sub_item(who, item, 1);
	assert(ret);

	delete_box(item);
}


int
find_nearest_land(int where)
{
	int i;
	int orig_where = where;
	int dir;
	int check;
	int ret = 0;
	int try_one, try_two;

	try_two = 100;
	while (try_two-- > 0) {
		dir = rnd(1,4);

		try_one = 1000;
		while (try_one-- > 0)
		{
			if (subkind(where) != sub_ocean)
				return where;

			loop_here(where, i)
			{
				if (subkind(i) == sub_island)
				{
					assert(kind(i) == T_loc);

					ret = i;
					break;
				}
			}
			next_here;

			if (ret)
				return ret;

			where = location_direction(where, dir);

			check = 0;
			while (where == 0)
			{
				where = orig_where;
				dir = (dir % 4) + 1;
				check++;
				assert(check <= 4);
				where = location_direction(where, dir);
			}
		}

		if (try_two == 99)
			log_write(LOG_CODE, "find_nearest_land: Plan B");
	}

	log_write(LOG_CODE, "find_nearest_land: Plan C");

	{
		ilist l = NULL;
		int i;
		int ret;

		loop_loc(i)
		{
			if (region(i) != region(orig_where))
				continue;
			if (loc_depth(i) != LOC_province)
				continue;
			if (subkind(i) == sub_ocean)
				continue;

			ilist_append(&l, i);
		}
		next_loc;

		if (ilist_len(l) < 1)
			return 0;

		ret = l[rnd(0,ilist_len(l)-1)];

		ilist_reclaim(&l);
		return ret;
	}

	return 0;
}


/*
 *  Simply throw away non-unique items
 *  Put unique items into the province to be found with EXPLORE
 *  If we're at sea, look for a nearby island or shore to move
 *  the item to.
 */

int
drop_item(int who, int item, int qty)
{
	int who_gets;

	if (!item_unique(item))
		return consume_item(who, item, qty);

	who_gets = province(who);

	if (subkind(item) == sub_dead_body)
	{
		if (who_gets == 0)
		{
			destroy_unique_item(who, item);
			return TRUE;
		}
	}

	if (subkind(who_gets) == sub_ocean)
		who_gets = find_nearest_land(who_gets);

	if (who_gets == 0)
		who_gets = province(who);	/* oh well */

	log_write(LOG_CODE, "drop_item: %s from %s to %s", box_name(item),
					box_name(subloc(who)),
					box_name(who_gets));

	return move_item(who, who_gets, item, qty);
}


int
can_pay(int who, int amount)
{

	return has_item(who, item_gold) >= amount;
}


int
charge(int who, int amount)
{
	return sub_item(who, item_gold, amount);
}


int
stack_has_item(int who, int item)
{
	int i;
	int sum = 0;
	int head = stack_leader(who);

	loop_stack(head, i)
	{
		if (player(i) != player(who))	/* friendly with us */
			continue;

		sum += has_item(i, item);
	}
	next_stack;

	return sum;
}


int
has_use_key(int who, int key)
{
	struct item_ent *e;
	int ret = 0;
	struct item_magic *p;

	loop_inv(who, e)
	{
		p = rp_item_magic(e->item);

		if (p && p->use_key == key)
			ret = e->item;

		if (ret)
			break;
	}
	next_inv;

	return ret;
}


int
stack_has_use_key(int who, int key)
{
	int i;
	int ret = 0;
	int head = stack_leader(who);

	loop_stack(head, i)
	{
		if (player(i) != player(who))	/* friendly with us */
			continue;

		ret = has_use_key(i, key);
		if (ret)
			break;
	}
	next_stack;

	return ret;
}

/*
 *  Subtract qty of item from a stack
 *  Take it from who first, then take it from
 *  anyone else in the stack who has it, starting from
 *  the stack leader and working down.
 *
 *  Return FALSE if the stack doesn't have qty of item.
 */

int
stack_sub_item(int who, int item, int qty)
{
	int n;
	int head;
	int i;

	if (stack_has_item(who, item) < qty)
		return FALSE;

	n = min(has_item(who, item), qty);

	if (n > 0)
	{
		qty -= n;
		sub_item(who, item, n);
	}

	assert(qty >= 0);

	if (qty == 0)
		return TRUE;

/*
 *  Try to borrow what we need from friendly stackmates
 */

	head = stack_leader(who);

	loop_stack(head, i)
	{
		if (qty <= 0)
			break;

		if (player(i) != player(who))	/* friendly with us */
			continue;

		n = min(has_item(i, item), qty);

		if (n > 0)
		{
			qty -= n;
			sub_item(i, item, n);

#if 0
			if (show_day)
			{
				wout(who, "Borrowed %s from %s.",
					box_item_desc(item, n),
					box_name(i));

				wout(i, "%s borrowed %s.",
					box_name(who),
					box_item_desc(item, n));
			}
#endif
		}
	}
	next_stack;

	assert(qty == 0);	/* or else stack_has_item above lied */

	return TRUE;
}


int
autocharge(int who, int amount)
{

	return stack_sub_item(who, item_gold, amount);
}


int
test_bit(sparse kr, int i)
{

	if (ilist_lookup(kr, i) == -1)
		return FALSE;

	return TRUE;
}


void
set_bit(sparse *kr, int i)
{

	if (ilist_lookup(*kr, i) == -1)
		ilist_append(kr, i);
}


void
clear_know_rec(sparse *kr)
{

	ilist_clear(kr);
}


int
test_known(int who, int i)
{
	struct entity_player *ep;
	int pl;

	if (who == 0)
		return FALSE;

	assert(valid_box(who));
	assert(valid_box(i));

	pl = player(who);
	ep = rp_player(pl);

	if (ep && test_bit(ep->known, i))
		return TRUE;

	return FALSE;
}


void
set_known(int who, int i)
{
	int pl;

	if (!valid_box(who) || !valid_box(i)) {
		return;
	}
	assert(valid_box(who));
	assert(valid_box(i));

	pl = player(who);

	if (!valid_box(pl))
		return;

	assert(valid_box(pl));

	set_bit(&(p_player(pl)->known), i);
}


static int dot_count = 0;

void
print_dot(int c)
{

	if (dot_count == 0)
	{
		fprintf(stderr, "   ");
		dot_count++;
	}

	if (++dot_count % 60 == 0)
		fprintf(stderr, "\n   ");
	fputc(c, stderr);
	fflush(stderr);
}


int
first_char_here(int where)
{
	int i;
	int first = 0;

	loop_here(where, i)
	{
		if (kind(i) == T_char)
		{
			first = i;
			break;
		}
	}
	next_here;

	return first;
}


char *
entab(int pl)
{

	if (player_notab(pl))
		return "";

#ifdef _WIN32
	return "entab | ";
#else
	return "unexpand -t8 | ";
#endif
}


int
loc_hidden(int n)
{

#if 0
	if (loc_depth(n) > LOC_province && weather_here(n, sub_fog))
		return TRUE;
#endif

	return (rp_loc(n) ? rp_loc(n)->hidden : 0);
}


char *
rest_name(struct command *c, int a)
{
	char *s;
	int i;

	if (numargs(c) < a)
		return NULL;

	s = c->parse[a];

	for (i = a+1; i <= numargs(c); i++)
	{
		s = sout("%s %s", s, c->parse[i]);
	}

	return s;
}


int
nprovinces()
{
	static int nprov = 0;
	int i;

	if (nprov)
		return nprov;

	loop_province(i)
	{
		nprov++;
	}
	next_province;

	return nprov;
}


int
my_prisoner(int who, int pris)
{

	if (kind(pris) != T_char)
		return FALSE;

	if (!is_prisoner(pris))
		return FALSE;

	if (loc(pris) != who)
		return FALSE;

	return TRUE;
}


int
beast_capturable(int who)
{
	int ni;

	if (subkind(who) != sub_ni)
		return FALSE;

	ni = noble_item(who);

	if (item_capturable(ni))
		return TRUE;

	return FALSE;
}


void
stage(char *s)
{
	extern int time_self;
	static time_t old = 0;
	static time_t first = 0;
	time_t t;

	if (!time_self)
	{
		if (s)
			fprintf(stderr, "%s\n", s);
		return;
	}

	time(&t);

	if (old)
	{
		fprintf(stderr, "\t%d sec\n", (int)(t-old));
	}
	else
		first = t;
	old = t;

	if (s)
	{
		fprintf(stderr, "%s", s);
	}
	else
		fprintf(stderr, "%d seconds\n", (int)(t - first));
}


int
ship_cap(int ship)
{
	int sc = ship_cap_raw(ship);
	int dam = loc_damage(ship);

	sc -= sc * dam / 100;

	return sc;
}

int
greater_region(int who)
{
	int reg;

	reg = region(who);

	if (reg != faery_region &&
	    reg != hades_region &&
	    reg != nowhere_region &&
	    reg != cloud_region &&
	    reg != tunnel_region &&
	    reg != under_region)
		reg = 0;

	return reg;
}

int
diff_region(int a, int b)
{
	return greater_region(a) != greater_region(b);
}


void
bark_dogs(int where)
{
	int who;
	int sum = 0;
	int bark = 0;
	int n, i;

   vector_char_here(where);
   vector_add(where);

	loop_here(where, who)
	{
		if (kind(who) != T_char)
			continue;

		n = stack_has_item(who, item_hound);
		sum += n;
		for (i = 1; i <= n; i++)
			if (rnd(1,2) == 1)
				bark++;
	}
	next_here;

	if (bark == 1 && sum == 1)
		wout(VECT, "The hound is barking.");
	else if (bark == 1)
		wout(VECT, "A hound is barking.");
	else if (bark > 1)
		wout(VECT, "The hounds are barking.");
}

