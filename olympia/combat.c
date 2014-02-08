
/*
 *  ideas:  ->inside indicates slot of fort we're in
 *	siege engines would have to choose a fort
 *
 *	problem is with combat among allied ships
 *	simply disallow?  two ships can't stack and attack
 *	then we disallow a castle defending a person
 */

#include	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include	"z.h"
#include	"oly.h"



#define	allied(a,b)	(is_defend((a),(b)))

#define		A_WON	1
#define		B_WON	2
#define		TIE	3

static int combat_swampy = FALSE;	/* bad for horses */
static int combat_rain = FALSE;		/* bad for archers */
static int combat_wind = FALSE;		/* poor for archers */
static int combat_fog = FALSE;
static int combat_sea = FALSE;		/* naval combat */


#define		FK_fort		-1	/* structure */
#define		FK_noble	-2


struct fight {
	int unit;		/* what unit are we in */
	int kind;		/* item type or FK_xxx */
	int sav_num;		/* original health or count */
	int num;		/* num of item, health or fort rating */
				/* (will be 1 for nobles, since they */
				/* may only take 1 hit) */
				/* (damage for locs is stored in num) */
	int new_health;		/* health after battle is over */

	int attack;
	int defense;

	int behind;		/* how behind we are, 0=front */
	int missile;		/* missile attack (0=can't fight from behind)*/

	int protects;		/* fighter struct we stand in front of */
	int nprot;		/* number of fighter structs protecting us */
	int ally;		/* unit pulled into fight by ally order */

	int prisoner;		/* unit lost and is candidate for prisoner */
	int inside;		/* fighter is inside a structure */

	int seize_slot;		/* take loser's place if we win */
	int survived_fatal;	/* survived a fatal wound with [9101] */

	int attack_item;	/* bonus item this character is wielding */
	int defense_item;
	int missile_item;
};


struct wield {
	int attack;
	int defense;
	int missile;
};



static int
cannot_take_prisoners(int who)
{

	if (only_defeatable(who))
		return TRUE;

	if (subkind(who) == sub_garrison)
		return TRUE;

#if 0
	if (is_npc(who))
		return TRUE;
#endif

	return FALSE;
}


int
cannot_take_booty(int who)
{

#if 0
	if (is_npc(who))
		return TRUE;
#endif

	if (subkind(who) == sub_garrison)
		return TRUE;

	return FALSE;
}


int
v_behind(struct command *c)
{
	int num = c->a;
	char *s = "";

	if (num < 0)
		num = 0;
	if (num > 9)
		num = 9;

	p_char(c->who)->behind = num;

	if (num == 0)
		s = " (front unit)";

	wout(c->who, "Behind flag set to %d%s.", num, s);

	return TRUE;
}


static void
execute_prisoner(int who, int pris)
{

	vector_stack(who, TRUE);
	wout(VECT, "%s executes %s!", box_name(who), box_name(pris));

	if (survive_fatal(pris))
	{
		wout(VECT, "%s miraculously is still alive!", box_name(pris));
		prisoner_escapes(pris);
	}
	else
	{
		kill_char(pris, who);

#if 0
		/* Check if they are really dead */
		if ((kind(pris) == T_item) &&
		    (subkind(pris) == sub_dead_body) &&
		    valid_box(subloc(pris)))
		{
			/* If so, give the body to the executioner */
			move_item(subloc(pris), who, pris, 1);
		}
#endif
	}
}


int
v_execute(struct command *c)
{
	int pris = c->a;
	int first = TRUE;

	if (numargs(c) < 1)
	{
		loop_here(c->who, pris)
		{
			if (is_prisoner(pris))
			{
				execute_prisoner(c->who, pris);
				first = FALSE;
			}
		}
		next_here;

		if (first)
			out(c->who, "No prisoners to execute.");
		return TRUE;
	}

	if (!has_prisoner(c->who, pris))
	{
		wout(c->who, "Don't have a prisoner %s.", box_code(pris));
		return FALSE;
	}

	execute_prisoner(c->who, pris);
	return TRUE;
}


static int
fort_covers(int n)
{
	switch (subkind(n))
	{
	case sub_castle:		return 500;
	case sub_castle_notdone:	return 500;
	case sub_tower:			return 100;
	case sub_tower_notdone:		return 100;
	case sub_galley:		return 50;
	case sub_galley_notdone:	return 50;
	case sub_roundship:		return 50;
	case sub_roundship_notdone:	return 50;
	case sub_temple:		return 50;
	case sub_temple_notdone:	return 50;
	case sub_inn:			return 50;
	case sub_inn_notdone:		return 50;
	case sub_mine:			return 50;
	case sub_mine_notdone:		return 50;
	case sub_sewer:			return 50;

	default:
		fprintf(stderr, "subkind is %s\n", subkind_s[subkind(n)]);
		assert(FALSE);
	}
}


static int
is_siege_engine(int item)
{

	switch (item)
	{
	case item_battering_ram:
	case item_catapult:
	case item_siege_tower:
		return TRUE;
	}

	return FALSE;
}


static int
siege_engine_useful(struct fight **l)
{

	assert(plist_len(l) > 0);

	if (l[0]->kind == FK_fort && l[0]->num > 0)
		return TRUE;

	return FALSE;
}


static int
lead_char_pos(struct fight **l)
{

	assert(plist_len(l) > 0);

	if (l[0]->kind == FK_noble)
		return 0;

	assert(plist_len(l) > 0);

	if (l[1]->kind == FK_noble)
		return 1;

	assert(FALSE);

	// To force VS to stop complaining
	return 0;
}


static int
lead_char(struct fight **l)
{
	int i;

	i = lead_char_pos(l);

	return l[i]->unit;
}


/*
 *  Assumptions:
 *
 *	l[0] will be a fort, if there is one
 *	l[1] will be the lead noble in this case.
 *	Otherwise, l[0] is the lead noble.
 *
 *	Fighters inside a structure (those with ->inside set) must
 *	be contiguous in the array, and start from the beginning.
 *
 *	(Defenders not in the structure will only be allies from the
 *	outside, which are added after the primary defenders).
 */


static void
dump_fighters(struct fight **l)
{
	int i;
	char *s;

	out(combat_pl, "side:  %s", box_name(lead_char(l)));

	for (i = 0; i < plist_len(l); i++)
	{
		s = sout("bh=%d ms=%d ins=%d pris=%d sav=%d at=%d df=%d", 
			l[i]->behind, l[i]->missile, l[i]->inside,
			l[i]->prisoner, l[i]->sav_num, l[i]->attack,
			l[i]->defense);

		out(combat_pl, "   %s.%d n=%d prt=%d nprt=%d al=%d %s",
			box_code_less(l[i]->unit), l[i]->kind, l[i]->num,
			l[i]->protects, l[i]->nprot, l[i]->ally, s);
	}

	out(combat_pl, "");
}


/*
 *  Backup from a fighter slot until the previous noble
 *  they are part of is found. 
 */

static int
who_protects(struct fight **l, int pos)
{
	int i = pos;

	while (i >= 0 && l[i]->kind != FK_noble)
		i--;

	assert(i >= 0);
	assert(l[i]->kind == FK_noble);
	assert(l[i]->unit == l[pos]->unit);

	return i;
}


static void
init_prot(struct fight **l)
{
	int i;

	for (i = 0; i < plist_len(l); i++)
	{
	    if (l[i]->num > 0)
	    {
		switch (l[i]->kind)
		{
		case FK_fort:
			l[i]->protects = -1;
			break;

		case FK_noble:
			l[i]->protects = lead_char_pos(l);

/*
 *  Don't set protect field to point to ourself
 */

			if (l[i]->protects == i)
				l[i]->protects = -1;

#if 0
			l[i]->protects = find_unit(l, stack_parent(l[i]->unit));
#endif
			break;

		default:

/*
 *  Siege engines shouldn't count to protect the noble.
 *  The attacker would rather claim them as booty than destroy them, too.
 */

			if (is_siege_engine(l[i]->kind))
				l[i]->protects = -1;
			else
				l[i]->protects = who_protects(l, i);

			break;
		}
	    }
	}

	for (i = 0; i < plist_len(l); i++)
		l[i]->nprot = 0;

	for (i = 0; i < plist_len(l); i++)
		if (l[i]->protects >= 0)
			l[ l[i]->protects ]->nprot++;
}


/*
 *  Determine what the character is wielding and wearing, if
 *  anything.  There may be up to three items; an attack weapon,
 *  a missile weapon, and some sort of defensive garment.
 *
 *  If f is not NULL, add in the bonuses to a fight struct.
 *
 *  Returns TRUE if the character is wearing or wielding something.
 */

int
find_wield(struct wield *w, int who, struct fight *f)
{
	struct item_ent *e;
	int n;
	int attack_max = -1;
	int defense_max = -1;
	int missile_max = -1;
	struct wield _w;

	if (w == NULL)
		w = &_w;

	w->attack = 0;
	w->defense = 0;
	w->missile = 0;

	loop_inv(who, e)
	{
		if ((n = item_attack_bonus(e->item)) && (n > attack_max))
		{
			attack_max = n;
			w->attack = e->item;
		}

		if ((n = item_defense_bonus(e->item)) && (n > defense_max))
		{
			defense_max = n;
			w->defense = e->item;
		}

		if ((n = item_missile_bonus(e->item)) && (n > missile_max))
		{
			missile_max = n;
			w->missile = e->item;
		}
	}
	next_inv;

	if (f)
	{
		if (w->attack)
			f->attack += attack_max;

		if (w->defense)
			f->defense += defense_max;

		if (w->missile)
			f->missile += missile_max;
	}

	if (w->attack || w->defense || w->missile)
		return TRUE;

	return FALSE;
}


char *
wield_s(int who)
{
	static char buf[LEN];
	struct wield w;

	if (!find_wield(&w, who, NULL))
		return "";

	*buf = '\0';

/*
 *  Clear out multiple copies of the same item.
 *  This would happen if one weapon had multiple bonuses.
 *  We don't want to say "Wielding foo and foo, wearing foo."
 */

	if (w.attack == w.missile)
		w.missile = 0;

	if (w.attack == w.defense)
		w.defense = 0;

	if (w.attack || w.missile)
	{
		if (w.attack == 0)
			strcpy(buf, sout(", wielding %s",
				box_name(w.missile)));
		else if (w.missile == 0)
			strcpy(buf, sout(", wielding %s",
				box_name(w.attack)));
		else
			strcpy(buf, sout(", wielding %s and %s",
				box_name(w.attack),
				box_name(w.missile)));
	}

	if (w.defense)
		strcat(buf, sout(", wearing %s", box_name(w.defense)));

	return buf;
}



static void
init_attack_defense(struct fight **l)
{
	int i;
	struct fight *f;
	int mk;

	for (i = 0; i < plist_len(l); i++)
	{
		f = l[i];

		switch (f->kind)
		{
		case FK_fort:
			f->attack = 0;
			f->defense = loc_defense(f->unit);
			f->missile = 0;
			f->behind = 0;
			break;

		case FK_noble:
			mk = noble_item(f->unit);
			if (mk == 0)
			{
				f->attack = char_attack(f->unit);
				f->defense = char_defense(f->unit);
				f->missile = char_missile(f->unit);
			}
			else
			{
				f->attack = item_attack(mk);
				f->defense = item_defense(mk);
				f->missile = item_missile(mk);
			}

			f->behind = char_behind(f->unit);
/*
 *  Add in combat bonuses from items the noble is carrying
 */

			find_wield(NULL, f->unit, f);

			break;

		default:
			f->attack = item_attack(f->kind);
			f->defense = item_defense(f->kind);
			f->missile = item_missile(f->kind);;
			f->behind = char_behind(f->unit);

			if ((combat_swampy || combat_sea) &&
			    (f->kind == item_elite_guard ||
			     f->kind == item_knight))
			{
				f->attack -= 25;
				f->defense -= 25;
				assert(f->attack > 0);
				assert(f->defense > 0);
			}

			if (combat_fog) {
				f->missile /= 4;
			} else if (combat_wind || combat_rain) {
				if (f->kind == item_archer)
					f->missile /= 2;
				else if (f->kind == item_elite_arch)
					f->missile /= 2;
				else if (f->kind == item_crossbowman)
					f->missile /= 4;
			}

			if (f->kind == item_pirate)
			{
				int where = subloc(f->unit);

				if (is_ship(where) || is_ship_notdone(where))
				{
					f->attack *= 3;
					f->defense *= 3;
				}
			}

			break;
		}
	}
}


static void
add_to_fight_list(struct fight ***l, int unit, int kind, int num,
						int ally, int inside)
{
	struct fight *new;

/*
 *  Siege engines not engaged for naval combat
 */

	if (combat_sea && is_siege_engine(kind))
		return;

	new = my_malloc(sizeof(*new));

	new->unit = unit;
	new->kind = kind;
	new->sav_num = num;

	if (kind == FK_noble)
		new->num = 1;
	else
		new->num = num;

	new->ally = ally;
	new->inside = inside;

	plist_append((plist *) l, new);
}


static void
add_fighters(struct fight ***l, int who, int ally, int inside, int is_defender)
{
	struct item_ent *e;
	int use_beasts = FALSE;
	int can_add;

	assert(kind(who) == T_char);

	if (is_prisoner(who))
		return;

	add_to_fight_list(l, who, FK_noble, char_health(who), ally, inside);

	if (is_npc(who) || has_skill(who, sk_use_beasts))
		use_beasts = TRUE;

	loop_inv(who, e)
	{
	    if (!use_beasts && item_animal(e->item))
		continue;

	    if (subkind(e->item) == sub_dead_body)
		continue;

	    can_add = is_defender || (e->item != item_peasant &&
		e->item != item_worker && e->item != item_sailor);

	    if (can_add && is_fighter(e->item))
		add_to_fight_list(l, who, e->item, e->qty, ally, inside);
	}
	next_inv;
}


static void
add_fight_stack(struct fight ***l, int who, int ally, int is_defender)
{
	int i;
	int inside = FALSE;

	assert(kind(who) == T_char);

	if (plist_len(*l) > 0 && (*l)[0]->kind == FK_fort &&
	    somewhere_inside((*l)[0]->unit, who))
		inside = TRUE;

	add_fighters(l, who, ally, inside, is_defender);

	loop_char_here(who, i)
	{
		add_fighters(l, i, ally, inside, is_defender);
	}
	next_char_here;
}


/*
 *  Look for any characters in where that are allied to def1 or def2
 */

static void
look_for_allies(struct fight ***l, int where, int def1, int def2, int attacker)
{
	int i;

	loop_here(where, i)
	{
		if (kind(i) == T_char &&
		    i != def1 && i != def2 &&
		    (allied(i, def1) || allied(i, def2)) &&
		    !char_gone(i) &&
		    stack_leader(i) == i &&
		    attacker != i &&
		    player(attacker) != player(i))
		{
			add_fight_stack(l, i, TRUE, TRUE);
		}
	}
	next_here;
}


/*
 *  Given the target, fill in the fighter array with the
 *  members of the side's fighting force.
 *
 *  A protecting building will be the first element of the
 *  array.  Nobles and each kind of fighter each get their
 *  own slot.
 *
 *  Then fill in attack and defense and protection values
 *  for the slots.
 */

static struct fight **
construct_fight_list(int target, int attacker, int add_allies, int is_defender)
{
	struct fight **l = NULL;
	int who;

/*
 *  If target is a location, add the protecting structure as
 *  the first element of the array, then start filling in the
 *  rest with the location owner, and work down through the stack.
 *
 *  If there is no location owner, return an empty array, since
 *  there is no one to fight.
 *
 *  If there is no protecting location, the target will be first
 *  in the array, then men and stackmates follow.
 */

	if (is_loc_or_ship(target))
	{
		if (loc_depth(target) == LOC_build)
			who = building_owner(target);
		else
			who = first_character(target);

		if (who == 0)
			return NULL;

		if (loc_depth(target) == LOC_build)
		{
			int rating = 100 - loc_damage(target);

			assert(rating > 0);

			add_to_fight_list(&l, target, FK_fort, rating,
					FALSE, FALSE);
		}
	}
	else
		who = target;

	add_fight_stack(&l, who, FALSE, is_defender);

	if (add_allies)
	{
		look_for_allies(&l, subloc(who), target, who, attacker);

		if (is_loc_or_ship(target))
			look_for_allies(&l, subloc(target), target, who, attacker);
	}

	return l;
}


static struct fight **
construct_guard_fight_list(int target, int attacker, struct fight **l_a)
{
	struct fight **l = NULL;
	int i;
	int where = subloc(target);

	loop_here(where, i)
	{
		if (kind(i) != T_char || !char_guard(i))
			continue;

/*
 *  Don't count a guarding unit if they are stacked with the
 *  pillagers, or if they are part of the pillager's faction.
 */

		if (player(i) == player(attacker))
			continue;

		if (plist_lookup((plist) l_a, i) >= 0)
			continue;

		add_fight_stack(&l, i, FALSE, TRUE);
	}
	next_here;

	look_for_allies(&l, subloc(target), target, 0, attacker);

	return l;
}


static void
ready_fight_list(struct fight **l)
{

	init_prot(l);
	init_attack_defense(l);
}


static void
reclaim_fight_list(struct fight ***l)
{
	int i;

	for (i = 0; i < plist_len(*l); i++)
		my_free((*l)[i]);

	plist_reclaim((plist *) l);
}


static int
advance_behind(struct fight **l)
{
	int i;
	int least = 0;

	for (i = 0; i < plist_len(l); i++)
		if (l[i]->behind &&
		    l[i]->kind != FK_fort &&
		    (l[i]->behind < least || least == 0))
		{
			least = l[i]->behind;
		}

	if (least)
	{
		for (i = 0; i < plist_len(l); i++)
			if (l[i]->behind == least)
			{
				l[i]->behind = 0;

				out(combat_pl, "    advancing unit %s.%d",
					box_code_less(l[i]->unit), l[i]->kind);

				dump_fighters(l);
			}
	}

	return least;
}


static int
num_attackers(struct fight *f, struct fight **enemy)
{

	if (f->kind == FK_fort)
		return 0;

	if (f->behind && f->missile == 0)
		return 0;

	if (f->missile == 0 && f->attack == 0)
		return 0;

	if (is_siege_engine(f->kind) && !siege_engine_useful(enemy))
		return 0;

	return f->num;
}


static int
total_attackers(struct fight **l, struct fight **enemy)
{
	int i;
	int sum = 0;

	for (i = 0; i < plist_len(l); i++)
		sum += num_attackers(l[i], enemy);

	if (sum == 0 && advance_behind(l))
	{
		for (i = 0; i < plist_len(l); i++)
			sum += num_attackers(l[i], enemy);
	}

	return sum;
}


static int
num_targets(struct fight *f, struct fight **enemy)
{

	if (f->kind == FK_fort && f->num > 0)
		return 1;

	if (is_siege_engine(f->kind) && !siege_engine_useful(enemy))
		return 0;

	return f->num;
}


static int
num_valid_targets(struct fight *f, struct fight **enemy)
{

	if (f->nprot > 0 || f->behind)
		return 0;

	return num_targets(f, enemy);
}


static int
total_valid_targets(struct fight **l, struct fight **enemy)
{
	int i;
	int sum = 0;
	int count = 0;

	for (i = 0; i < plist_len(l); i++)
		sum += num_valid_targets(l[i], enemy);

	while (sum == 0 || (sum == 1 && l[0]->kind == FK_fort &&
	       num_targets(l[0], enemy) > 0))
	{
		if (advance_behind(l))
		{
			sum = 0;
			for (i = 0; i < plist_len(l); i++)
				sum += num_valid_targets(l[i], enemy);
		}

		count++;
		assert(count < 10000);
	}

	return sum;
}


static int
num_non_damage(struct fight *f)
{

	if (f->kind == FK_fort)
		return 0;

	return f->num;
}


static int
total_non_damage(struct fight **l)
{
	int i;
	int sum = 0;

	for (i = 0; i < plist_len(l); i++)
		sum += num_non_damage(l[i]);

	return sum;
}


static int
combat_sum(struct fight *f)
{

	if (f->kind == FK_fort || is_siege_engine(f->kind))
		return 0;

	assert(f->num >= 0);

	return (max(f->attack, f->missile) + f->defense) * f->num;
}


static int
total_combat_sum(struct fight **l)
{
	int i;
	int sum = 0;

	for (i = 0; i < plist_len(l); i++)
		sum += combat_sum(l[i]);

	return sum;
}


/*
 *  Register a hit on a fighter
 *
 *  l		side list for g
 *  attacker	fighter doing the hitting
 *  g		fighter that has been hit
 */

static void
decrement_num(struct fight **l, struct fight *attacker, struct fight *g)
{
	int hit;

	assert(g->num > 0);

/*
 *  How can we be hit if we're supposed to be protected?
 */
	assert(g->nprot == 0);

	switch (g->kind)
	{
	case FK_noble:
		g->num--;
		break;

	case FK_fort:
		if (is_siege_engine(attacker->kind))
			hit = rnd(5, 10);
		else
			hit = 1;

		if (g->defense)
		{
			g->defense -= hit;
			if (g->defense < 0)
			{
				g->num += g->defense;
				g->defense = 0;
			}
		}
		else
			g->num -= hit;

		if (g->num < 0)
			g->num = 0;
		break;

	case item_blessed_soldier:
		if (rnd(1,2) == 1)	/* 50% chance of surviving a hit */
			g->num--;
		break;

	default:
		g->num--;
	}

	assert(g->num >= 0);

	if (g->num <= 0 && g->protects >= 0)
	{
		assert(l[g->protects]->nprot > 0);

		l[g->protects]->nprot--;

		out(combat_pl, "    %s.%d no longer protects %s",
				box_code_less(g->unit),
				g->kind,
				box_code_less(l[g->protects]->unit));

		dump_fighters(l);
	}
}


static void
resolve_hit(struct fight **l, struct fight *f, struct fight *g, int man)
{
	int n;
	int defense = g->defense;
	int attack;

	if (g->inside && man <= fort_covers(l[0]->unit))
	{
		out(combat_pl, "%s.%d gets fort bonus of %d",
			box_code_less(g->unit), g->kind, l[0]->defense);
		defense += l[0]->defense;
	}

	if (f->behind)
	{
		assert(f->missile);
		attack = f->missile;
	}
	else
	{
		attack = max(f->missile, f->attack);
	}

	n = rnd(1, attack + defense);

	if (n > attack)
	{
		out(combat_pl, "    %s.%d failed to hit %s.%d",
				box_code_less(f->unit), f->kind,
				box_code_less(g->unit), g->kind);
		return;
	}

	out(combat_pl, "    %s.%d hit %s.%d",
				box_code_less(f->unit), f->kind,
				box_code_less(g->unit), g->kind);

	decrement_num(l, f, g);		/* f scores against g */
}


static struct fight *
find_attacker(struct fight **l, int man, struct fight **enemy)
{
	int i;

	for (i = 0; i < plist_len(l); i++)
	{
		man -= num_attackers(l[i], enemy);
		if (man <= 0)
			return l[i];
	}

	assert(FALSE);

	// To force VS to stop complaining
	return 0;
}


static struct fight *
find_defender(struct fight **l, int man, struct fight **enemy)
{
	int i;

	for (i = 0; i < plist_len(l); i++)
	{
		man -= num_valid_targets(l[i], enemy);
		if (man <= 0)
			return l[i];
	}

	assert(FALSE);

	// To force VS to stop complaining
	return 0;
}


static void
choose_attack(int attacker, struct fight **l_a, struct fight **l_b)
{
	int num_defend;
	struct fight *f;
	struct fight *g;
	int man = -1;

	f = find_attacker(l_a, attacker, l_b);

	if (is_siege_engine(f->kind) && siege_engine_useful(l_b))
	{
		g = l_b[0];		/* set defender to structure */

		assert(g->kind == FK_fort);

/*
 *  The siege engine shouldn't have been selected for an attack
 *  if the target fort is already destroyed
 */

		assert(g->num > 0);
	}
	else
	{
		num_defend = total_valid_targets(l_b, l_a);
		assert(num_defend > 0);

		man = rnd(1, num_defend);
		g = find_defender(l_b, man, l_a);
	}

	resolve_hit(l_b, f, g, man);		/* f tries to hit g */
}


#if 1

static int side_to_go;
static int num_attack_a;
static int num_attack_b;

static void
combat_round(struct fight **l_a, struct fight **l_b, int force_win)
{
	int total_attack_a = total_attackers(l_a, l_b);
	int total_attack_b = total_attackers(l_b, l_a);

	num_attack_a = min(num_attack_a, total_attack_a);
	num_attack_b = min(num_attack_b, total_attack_b);

	if (num_attack_a == 0 && num_attack_b == 0)
	{
		num_attack_a = total_attackers(l_a, l_b);
		num_attack_b = total_attackers(l_b, l_a);

		if (force_win)
			num_attack_b = 0;

		assert(num_attack_a + num_attack_b > 0);

		side_to_go = 0;		/* attacker goes first */
	}

	if (num_attack_a > 0 && num_attack_b > 0)
	{
		if (side_to_go == 0)	/* attacker goes */
		{
			choose_attack(rnd(1, total_attack_a), l_a, l_b);
			num_attack_a--;
		}
		else			/* defender goes */
		{
			choose_attack(rnd(1, total_attack_b), l_b, l_a);
			num_attack_b--;
		}

		side_to_go = !side_to_go;
	}
	else
	if (num_attack_a > 0)
	{
		choose_attack(rnd(1, total_attack_a), l_a, l_b);
		num_attack_a--;
	}
	else
	if (num_attack_b > 0)
	{
		choose_attack(rnd(1, total_attack_b), l_b, l_a);
		num_attack_b--;
	}
}

#else

static void
combat_round(struct fight **l_a, struct fight **l_b, int force_win)
{
	int num_attack_a;
	int num_attack_b;
	int man;

	num_attack_a = total_attackers(l_a, l_b);
	num_attack_b = total_attackers(l_b, l_a);

	if (force_win)
		num_attack_b = 0;

	assert(num_attack_a + num_attack_b > 0);

	man = rnd(1, num_attack_a + num_attack_b);

	if (man <= num_attack_a)
	{
		choose_attack(man, l_a, l_b);
	}
	else
	{
		choose_attack(man - num_attack_a, l_b, l_a);
	}
}
#endif


static int
side_has_skill(struct fight **l, int sk)
{
	int i;

	for (i = 0; i < plist_len(l); i++)
		if (l[i]->kind == FK_noble && has_skill(l[i]->unit, sk))
			return TRUE;

	return FALSE;
}

/*
 *  Deduct men-as-inventory who have been killed.
 *  Then kill or wound the noble.
 *
 *  The inherit parameter determines who gets the booty from dead nobles.
 */

static void
deduct_dead(struct fight **l_a, struct fight **l_b, int inherit)
{
	int i;
	int unit, item;
	int amount;
	int who;

	if (cannot_take_booty(lead_char(l_b)))
		inherit = 0;

/*
 *  First deduct all of the dead men
 */

	for (i = 0; i < plist_len(l_a); i++)
	{
		unit = l_a[i]->unit;
		item = l_a[i]->kind;

		if (item > 0)
		{
		    int num_to_kill;

		    num_to_kill = l_a[i]->sav_num - l_a[i]->num;

/*  I don't understand the MATES_SILENT part of the if... */

		    if (inherit != MATES_SILENT &&
		        beast_capturable(l_a[i]->unit) &&
			side_has_skill(l_b, sk_capture_beasts))
		    {
#if 1
			if (num_to_kill > 0 && rnd(1,2) == 1)
				num_to_kill--;
/* 
 *  This will leave randomly 1-2 beasts in the inventory
 *  The noble_item item will get added to the unit container
 *  later, so if num_to_kill kills them all, there will still
 *  be one turned over as booty.  Thus we kill off all or all-1
 *  here
 */
#else
			num_to_kill = rnd(0, num_to_kill/2);
#endif
		    }

/*
 *  The following assert has failed in the past when a unit has
 *  someone become a member of both the attacking and defending
 *  party.  What happens is that this routine is called twice
 *  for the same unit.  The second time, the unit has lost thus,
 *  and the following consistency check fails.
 */

		    assert(has_item(unit, item) == l_a[i]->sav_num);
		    consume_item(unit, item, num_to_kill);
		}
	}

#if 1
/*
 *  If a garrison units loses all of its men, terminate it
 *
 *  It's not clear that this case can ever happen.
 *  Since the garrison unit itself dies after one hit, and
 *  generates no hits itself, if all the men die, it should
 *  die too.
 */

	for (i = 0; i < plist_len(l_a); i++)
	{
		if (l_a[i]->kind == FK_noble &&
		    subkind(l_a[i]->unit) == sub_garrison &&
		    count_man_items(l_a[i]->unit) == 0)
		{
			if (l_a[i]->new_health || l_a[i]->num)
				log_write(LOG_CODE, "%s lost all men, zeroed out",
						box_name(l_a[i]->unit));

			l_a[i]->new_health = 0;
			l_a[i]->num = 0;
		}
	}
#endif

/*
 *  Now apply any wounds the nobles received, possibly killing them.
 */

	for (i = 0; i < plist_len(l_a); i++)
	{
		if (l_a[i]->kind != FK_noble || l_a[i]->num)	/* not hit */
			continue;

		who = l_a[i]->unit;

		if (l_a[i]->new_health == 0)
		{
			kill_char(who, inherit);
		}
		else if (l_a[i]->new_health < char_health(who))
		{
			assert(l_a[i]->new_health > 0);

			amount = char_health(who) - l_a[i]->new_health;
			add_char_damage(who, amount, inherit);
		}
/*
 *  else:
 *	either new_health == current_health, meaning the noble wasn't hit
 *	or if it's greater, then they survived a fatal wound
 */

	}
}


/*
 *  Hit wounded units with 1-100 health loss
 *  Store new health for unit (0 = dead) in ->new_health.
 */

static void
determine_noble_wounds(struct fight **l)
{
	int i;

	for (i = 0; i < plist_len(l); i++)
	{
		if (l[i]->kind != FK_noble)
			continue;

		assert(l[i]->num == 0 || l[i]->num == 1);

		if (l[i]->num)		/* not hit */
		{
			l[i]->new_health = l[i]->sav_num;
			continue;
		}

/*
 *  Already dead or undead when we started.  One hit kills an undead.
 */

		if (l[i]->sav_num <= 0)
			l[i]->new_health = 0;
		else {
			l[i]->new_health = max(l[i]->sav_num - rnd(1,100), 0);
			if (l[i]->new_health < 0)
				l[i]->new_health = 0;
		}
	}
}


/*
 *  See if any would-be dead nobles have Survive fatal wound [9101].
 */

static void
check_fatal_survive(struct fight **l)
{
	int i;

	for (i = 0; i < plist_len(l); i++)
	{
		if (l[i]->kind != FK_noble)
			continue;

		if (l[i]->new_health == 0 && survive_fatal(l[i]->unit))
		{
			l[i]->survived_fatal = TRUE;
			l[i]->new_health = 100;
		}
	}
}


/*
 *  Deduct any structure damage.  If the structure is completely
 *  destroyed, it may vanish, expelling the units.  If it is a ship
 *  it may sink, killing anyone left on board.
 */

static void
structure_damage(struct fight **l, int can_destroy)
{
	int unit;


	if (plist_len(l) > 0 && l[0]->kind == FK_fort)
	{
		int damage = l[0]->sav_num - l[0]->num;

		unit = l[0]->unit;

		assert(damage >= 0);

		if (damage)
			add_structure_damage(unit, damage, can_destroy);

		p_subloc(unit)->defense = l[0]->defense;
	}
}


/*
 *	1:1	25%
 *	2:1	50%
 *	3:1	75%
 */


static void
determine_prisoners(struct fight **l_a, struct fight **l_b)
{
	int num_a, num_b;
	int chance;		/* chance that a unit is taken prisoner */
	int i;
	int no_take;
	int capture_beasts;
	int lead_a = lead_char(l_a);
	int lead_b = lead_char(l_b);

#if 0
	no_take = cannot_take_prisoners(lead_a) || char_break(lead_b) == 0;
#else
	no_take = cannot_take_prisoners(lead_a);
#endif

	capture_beasts = side_has_skill(l_a, sk_capture_beasts);

	num_a = total_non_damage(l_a);
	num_b = total_non_damage(l_b);

/*
 *  If we're on a ship on the ocean, there is nowhere for the
 *  losers to flee to, so capture them all.
 */

	if (l_b[0]->kind == FK_fort &&
	    is_ship(l_b[0]->unit) &&
	    subkind(subloc(l_b[0]->unit)) == sub_ocean &&
	    l_a[lead_char_pos(l_a)]->seize_slot)
	{
		chance = 100;
	}
	else
	{
		if (num_b <= 0)
			chance = 100;
		else
			chance = 25 * num_a / num_b;

		if (chance < 25) chance = 25;
		if (chance > 75) chance = 75;
	}

/*
 *  Set prisoner flag based on chance for all nobles left alive.
 *
 *  If a unit can't be taken prisoner, or the winner cannot
 *  take prisoners, then kill the would-be prisoner.
 */

	for (i = 0; i < plist_len(l_b); i++)
	{
		if (l_b[i]->kind != FK_noble)
			continue;

/*
 *  if the attackers has the skill Capture beasts in battle [9506],
 *  capturable beasts (ni chars whose items have the capturable flag)
 *  shouldn't be killed, they should become the property of the victor.
 *  take_prisoners will later remove the unit wrapper and add the ni
 *  items to the winning character.
 */

		if (beast_capturable(l_b[i]->unit) && l_b[i]->new_health == 0)
		{
#if 0
/*
 *  All capturable beasts should have their break point set at 0,
 *  so they will fight to the death.  Otherwise, non-beastmasters
 *  could take them prisoner.  The assert below will fail if the
 *  beast's break point was not zero.
 *
 *  We could simply force new_health to be zero here to kill the
 *  beast, but then it wouldn't have fought to its fullest.
 */

/*
 *  Ah, this is wrong in the following case: 
 *  A human player is leading a stack containing a capturable beast.
 */

			if (l_b[i]->new_health != 0)
				fprintf(stderr, "%s\n", box_name_kind(l_b[i]->unit));
			assert(char_break(l_b[i]->unit) == 0);
			assert(l_b[i]->new_health == 0);
#endif

			if (capture_beasts)
			{
				l_b[i]->prisoner = TRUE;
				l_b[i]->new_health = l_b[i]->sav_num;
				continue;
			}
		}

		if (l_b[i]->new_health == 0)
			continue;

		if (rnd(1,100) > chance)
			continue;

		if (no_take)
			l_b[i]->new_health = 0;
		else
			l_b[i]->prisoner = TRUE;
	}
}


static void
take_prisoners(int winner, struct fight **l)
{
	int i;

	for (i = 0; i < plist_len(l); i++)
		if (l[i]->prisoner)
		{
			take_prisoner(winner, l[i]->unit);
		}
}


static void
seize_position(int winner, int loser_where, int loser_pos)
{

/*
 *  If loser is in a better spot than the winner, take it.
 *	Loser is in a structure we want
 *	Loser is before us in the location list
 */

	if (loser_where != subloc(winner))
	{
		ilist tmp;

		wout(subloc(winner), "%s moved into %s as a result of combat.",
			box_code(winner), box_code(loser_where));

		if (viewloc(loser_where) != viewloc(subloc(winner)))
			wout(loser_where, "%s moved into %s as a result of combat.",
				box_code(winner), box_code(loser_where));

		tmp = save_output_vector();

		vector_stack(winner, TRUE);
		wout(VECT, "We have taken %s.", box_name(loser_where));

		move_stack(winner, loser_where);
		promote(winner, 0);

		restore_output_vector(tmp);
	}
	else if (loser_pos < here_pos(winner))
	{
		promote(winner, loser_pos);
	}
}


static void
stack_flee(int who, int winner)
{
	int where;
	int to_where;

	assert(kind(who) == T_char);

	where = subloc(who);

/*
 *  If we're on a ship on an island, flee to the island
 *  Otherwise, flee to the province.
 */

	if (is_ship(where) && subkind(subloc(where)) == sub_island)
		to_where = subloc(where);
	else
		to_where = province(who);

/*
 *  Since there's nowhere to flee on a ship on the ocean,
 *  if someone is taking the ship (seize_slot) the prisoner
 *  percentage should have been set at 100%.  No one should
 *  be left to flee.
 */

#if 1
	if (subkind(to_where) == sub_ocean)
		return;
#else
	assert(subkind(to_where) != sub_ocean);
#endif

/*
 *  If we're already in the province, just move us to the end
 *  of the list.
 */

	if (to_where == where)
	{
		set_where(who, where);
		return;
	}

/*
 *  Combat has already set up the out_vector, so squirrel it
 *  away for this stack output
 */

	{
		ilist tmp;

		tmp = save_output_vector();

		vector_stack(who, TRUE);
		wout(VECT, "We flee to %s.", box_name(to_where));

		restore_output_vector(tmp);
	}

	wout(winner, "%s flees to %s.", box_name(who), box_name(to_where));

	leave_stack(who);
	move_stack(who, to_where);
}


static void
demote_units(int winner, struct fight **l)
{
	int i;

	for (i = 0; i < plist_len(l); i++)
	{
		if (l[i]->kind != FK_noble ||
		   l[i]->prisoner ||
		   !alive(l[i]->unit) ||
		   !stack_leader(l[i]->unit) == l[i]->unit)
		{
			continue;
		}

		stack_flee(l[i]->unit, winner);
	}
}


static char *
combat_display_with(struct fight **l)
{
	int i;

	for (i = 1; i < plist_len(l); i++)
		if (l[i]->kind == FK_noble)
		{
			if (l[0]->kind == FK_fort)
				return ", owner:";
			return ", accompanied~by:";
		}

	return "";
}


int show_combat_flag = FALSE;

static void
show_side_units(struct fight **l)
{
	int i;
	extern char *combat_ally;

	show_combat_flag = TRUE;
	combat_ally = "";

	out(VECT, "");
	wout(VECT, "%s%s", liner_desc(l[0]->unit), combat_display_with(l));

	indent += 3;

	for (i = 1; i < plist_len(l); i++)
		if (l[i]->kind == FK_noble)
		{
			if (l[i]->ally)
				combat_ally = ", ally";
			else
				combat_ally = "";

			wiout(VECT, 3, "%s", liner_desc(l[i]->unit));
		}

	show_combat_flag = FALSE;

	indent -= 3;
}


static void
out_side(struct fight **l, char *s)
{
	int i;

	for (i = 0; i < plist_len(l); i++)
		if (l[i]->kind == FK_noble)
			wout(l[i]->unit, "%s", s);
}


/*
 *  We attack Foo!
 *  Bar leads us in an attack aganist Foo!
 *  Bar attacks us!
 *  Bar attacks Castle!
 */

static void
combat_banner(struct fight **l_a, struct fight **l_b)
{
	int i;

	assert(plist_len(l_a) > 0);
	assert(plist_len(l_b) > 0);

	wout(VECT, "%s attacks %s!",
				box_name(lead_char(l_a)),
				box_name(l_b[0]->unit));

	indent += 3;
	show_side_units(l_a);
	show_side_units(l_b);
	indent -= 3;

	wout(lead_char(l_a), "Attack %s.", box_name(l_b[0]->unit));

	for (i = lead_char_pos(l_a)+1; i < plist_len(l_a); i++)
		if (l_a[i]->kind == FK_noble)
			wout(l_a[i]->unit,
				"%s leads us in an attack against %s.",
				box_name(lead_char(l_a)),
				box_name(l_b[0]->unit));

	for (i = 0; i < plist_len(l_b); i++)
		if (l_b[i]->kind == FK_noble)
		{
			if (l_b[i]->ally)
				wout(l_b[i]->unit, "%s attacks %s!  We rush "
						"to the defense.",
					box_name(lead_char(l_a)),
					box_name(l_b[0]->unit));
			else
				wout(l_b[i]->unit, "%s attacks us!",
						box_name(lead_char(l_a)));
		}
}


/*
 *  (Foo is fine.)
 *
 *  Foo was killed.
 *  Foo was wiped out.
 *  Foo was taken prisoner.
 *
 *  Foo lost n men.
 *  N soldiers, m archers, ... were killed.
 *
 *  Leader lost n soldiers, m archers, ...
 */

static char *
tally_side_losses(struct fight **l)
{
	int i;
	char *s = NULL;

	clear_temps(T_item);

	for (i = 0; i < plist_len(l); i++)
		if (l[i]->kind > 0)
			bx[l[i]->kind]->temp += l[i]->sav_num - l[i]->num;

	loop_item(i)
	{
		if (bx[i]->temp > 0)
			s = comma_append(s, just_name_qty(i, bx[i]->temp));
	}
	next_item;

	return s;
}


static char *
tally_personal_losses(struct fight **l, int pos)
{
	int i;
	char *s = NULL;

	assert(plist_len(l) > pos);
	assert(l[pos]->kind == FK_noble);

	for (i = pos+1; i < plist_len(l) && l[i]->unit == l[pos]->unit; i++)
		if (l[i]->kind > 0 && l[i]->num < l[i]->sav_num)
			s = comma_append(s, just_name_qty(l[i]->kind,
						l[i]->sav_num - l[i]->num));

	return s;
}


static char *
what_happened_to_noble(struct fight **l, int pos)
{
	char *s = NULL;

	if (l[pos]->prisoner)
	{
		if (subkind(l[pos]->unit) == sub_ni &&
		    beast_capturable(l[pos]->unit))
			s = "was captured";
		else
			s = "was taken prisoner.";
	}
	else if (l[pos]->num == 0 && l[pos]->new_health == 0)
	{
#if 0
		if (ilist_len(l) > pos+1 && l[pos+1]->unit == l[pos]->unit)
		{
			s = "was wiped out.";	/* noble + others */
		}
		else
#endif
		if (l[pos]->sav_num > 0)
		{
			s = "was killed.";	/* noble alone */
		}
		else
		{
			s = "was destroyed.";	/* "killed" for undead */
		}
	}
	else if (l[pos]->survived_fatal)
	{
		s = "survived a fatal wound!";
	}
	else if (l[pos]->new_health == -1)
	{
		s = NULL;	/* Undead who was not injured in battle */
	}
	else if (l[pos]->num == 0)
	{
		s = "was wounded.";
	}

	return s;
}


static void
show_side_results(struct fight **l)
{
	int lead;
	char *tally;
	int i;
	char *s;
	int first = TRUE;

	lead = lead_char(l);
	tally = tally_side_losses(l);

	if (tally)
	{
		indent += 3;
		wout(VECT, "%s lost %s.", just_name(lead), tally);
		indent -= 3;
		first = FALSE;
	}

	for (i = 0; i < plist_len(l); i++)
	{
		if (l[i]->kind != FK_noble)
			continue;

		if (s = tally_personal_losses(l, i))
			wout(l[i]->unit, "%s lost %s.",
					box_name(l[i]->unit), s);

		s = what_happened_to_noble(l, i);

		if (s)
		{
			indent += 3;
			wout(VECT, "%s %s", box_name(l[i]->unit), s);
			indent -= 3;
			first = FALSE;
		}
	}

	/* NOTYET:  show structure damage here */

	if (!first)
		out(VECT, "");
}


static int
best_here_pos(struct fight **l, int where)
{
	int i;
	int best = 99999;
	int n;

	for (i = 0; i < plist_len(l); i++)
	{
		if (l[i]->kind != FK_noble)
			continue;

		if (subloc(l[i]->unit) != where)
			continue;

		assert(subloc(l[i]->unit) == where);

		n = here_pos(l[i]->unit);
		if (n < best)
			best = n;
	}

	if (best == 99999)
		log_write(LOG_CODE, "best_here_pos: best == 99999, day=%d, l[0]=%s",
				sysclock.day, box_code_less(lead_char(l)));

	return best;
}


static void
combat_stop_movement(int who, struct fight **l)
{
	ilist tmp;
	int ship;
	int i;

	ship = subloc(who);
	if (is_ship(ship) && ship_moving(ship))
	{
		interrupt_order(who);
		p_subloc(ship)->moving = 0;

		tmp = save_output_vector();
		vector_char_here(ship);
		wout(VECT, "Loss in battle cancels movement.");
		restore_output_vector(tmp);

		log_write(LOG_CODE,
			"battle interrupts sailing, who=%d, where=%d",
			ship, subloc(who));
		return;
	}

	for (i = 0; i < plist_len(l); i++)
		if (l[i]->kind == FK_noble && char_moving(l[i]->unit))
		{
			interrupt_order(l[i]->unit);

			tmp = save_output_vector();
			vector_stack(l[i]->unit, TRUE);
			wout(VECT, "Loss in battle cancels movement.");
			restore_output_vector(tmp);

#if 0
			log_write(LOG_CODE,
				"battle interrupts movement, who=%d, where=%d",
				l[i]->unit, subloc(l[i]->unit));
#endif
		}
}


static void
reconcile(int winner, struct fight **l_a, struct fight **l_b)
{
	int loser;
	int loser_where, loser_pos;

	determine_noble_wounds(l_a);
	determine_noble_wounds(l_b);

	if (winner)
		determine_prisoners(l_a, l_b);

	check_fatal_survive(l_a);
	check_fatal_survive(l_b);

	out(VECT, "");
	if (winner)
	{
		wout(VECT, "%s is victorious!", box_name(lead_char(l_a)));
		out_side(l_a, "We won!");
		out_side(l_b, "We lost!");
	}
	else
	{
		wout(VECT, "No victor emerges from the fight.");
		out_side(l_a, "Neither side prevailed.");
		out_side(l_b, "Neither side prevailed.");
	}
	out(VECT, "");

	show_side_results(l_a);
	show_side_results(l_b);

	if (winner)
	{
		winner = lead_char(l_a);
		loser = lead_char(l_b);

		loser_where = subloc(loser);
		loser_pos = best_here_pos(l_b, loser_where);

		combat_stop_movement(loser, l_b);
		clear_guard_flag(loser);

		deduct_dead(l_a, l_b, MATES_SILENT);
		deduct_dead(l_b, l_a, winner);

		take_prisoners(winner, l_b);

		if (l_a[lead_char_pos(l_a)]->seize_slot)
			seize_position(winner, loser_where, loser_pos);

		demote_units(winner, l_b);

		if (combat_sea)
		    log_write(LOG_CODE, "sea combat unchecked NOTYET case, who=%s",
						box_name(winner));

/*
 *  NOTYET:
 *
 *	If this is a sea battle:
 *		If the loser's ship is sinking, and the victor is
 *		on it, the victor goes back to his own ship.
 *		(or doesn't go in the first place).
 *
 *		If the victor's ship is sinking, and the loser's
 *		isn't, then the victors go over to the loser's ship.
 *
 *		What about other, non-combat stacks on the victor's
 *		or the loser's ship?  Perhaps everyone on the ship
 *		should be involved in the battle?
 *
 *		What do other stacks on the loser ship do when the
 *		victor takes it over?
 */

	}
	else
	{
		deduct_dead(l_a, l_b, MATES_SILENT);
		deduct_dead(l_b, l_a, MATES_SILENT);
	}

/*
 *  Sink ships, destroy castles, etc.
 */

	structure_damage(l_a, !winner);
	structure_damage(l_b, TRUE);
}


#if 0
static void
increase_attack(struct fight **l)
{
	int i;

	for (i = 0; i < ilist_len(l); i++)
		if (l[i]->kind == FK_noble)
			p_char(l[i]->unit)->attack++;
}


static void
increase_defense(struct fight **l)
{
	int i;

	for (i = 0; i < ilist_len(l); i++)
		if (l[i]->kind == FK_noble)
			p_char(l[i]->unit)->defense++;
}
#endif


static int
run_combat(struct fight **l_a, struct fight **l_b, int force_win)
{
	int num_a, num_b;
	int thresh_a, thresh_b;
	int lead_a = lead_char(l_a);
	int lead_b = lead_char(l_b);

	out(combat_pl, "");
	out(combat_pl, "Combat between %s and %s on day %d",
				box_name(l_a[0]->unit),
				box_name(l_b[0]->unit),
				sysclock.day);
	out(combat_pl, "");

	dump_fighters(l_a);
	dump_fighters(l_b);

	thresh_a = total_combat_sum(l_a) * char_break(lead_a) / 100;
	thresh_b = total_combat_sum(l_b) * char_break(lead_b) / 100;

	/*
	 *  Initialize internal variables for combat_round
	 */

	side_to_go = 0;
	num_attack_a = 0;
	num_attack_b = 0;

	do
	{
		combat_round(l_a, l_b, force_win);

		num_a = total_combat_sum(l_a);
		num_b = total_combat_sum(l_b);
	}
	while (num_a > thresh_a && num_b > thresh_b);

/*
 *  Who won?
 *
 *	The loser must be below their break threshold,
 *	the winner must still be above theirs,
 *	and the lead noble of the winner must not have been wounded.
 *
 *	(The lead noble will be the last figure to take a wound,
 *	so if the lead noble is wounded, it means there's no one
 *	left unhurt on the winner's side)
 *
 *	If both sides are below their break threshold, it's a draw.
 */

	if (num_a <= thresh_a && num_b > thresh_b)
	{
		assert(l_b[lead_char_pos(l_b)]->num > 0);
		return B_WON;
	}

	if (num_b <= thresh_b && num_a > thresh_a)
	{
		assert(l_a[lead_char_pos(l_a)]->num > 0);
		return A_WON;
	}

	return TIE;
}


static int
combat_top(struct fight **l_a, struct fight **l_b, int force_win)
{
	int result;

	print_dot('*');

	{
#if 0
		int where = subloc(l_b[0]->unit);
#else
		int where = viewloc(subloc(l_a[0]->unit));
		int where2 = viewloc(subloc(l_b[0]->unit));
#endif

		vector_clear();
		vector_add(where);
		if (where2 != where)
			vector_add(where2);

		show_to_garrison = TRUE;
	}

	assert(plist_len(l_a) > 0);
	assert(plist_len(l_b) > 0);

	{
		int where = subloc(lead_char(l_a));
		int where2 = subloc(lead_char(l_b));

		if (weather_here(where, sub_rain))
			combat_rain = TRUE;
		else
			combat_rain = FALSE;

		if (weather_here(where, sub_wind))
			combat_wind = TRUE;
		else
			combat_wind = FALSE;

		if (weather_here(where, sub_fog))
			combat_fog = TRUE;
		else
			combat_fog = FALSE;

		if (where != where2 && subkind(province(where)) == sub_ocean)
			combat_sea = TRUE;
		else
			combat_sea = FALSE;

		if (subkind(province(where)) == sub_swamp)
			combat_swampy = TRUE;
		else
			combat_swampy = FALSE;
	}

	combat_banner(l_a, l_b);

	result = run_combat(l_a, l_b, force_win);

	switch (result)
	{
	case A_WON:
		reconcile(TRUE, l_a, l_b);
#if 0
		increase_attack(l_a);
#endif
		break;

	case B_WON:
		reconcile(TRUE, l_b, l_a);
#if 0
		increase_defense(l_b);
#endif
		break;

	case TIE:
		reconcile(FALSE, l_a, l_b);
		break;

	default:
		assert(FALSE);
	}

	show_to_garrison = FALSE;

	return result == A_WON;
}


#define		DEFEAT_DEFAULT		0
#define		DEFEAT_FORCE		1
#define		DEFEAT_UNREADY		2

static int
fail_defeat_check(int a, struct fight **l_b)
{
	int lead_b = lead_char(l_b);
	int n;

	n = only_defeatable(lead_b);

	if (n == 0)
		return DEFEAT_DEFAULT;

	if (has_item(a, n))
		return DEFEAT_FORCE;

	out(a, "Only one who possesses %s can defeat %s.",
			box_name(n), box_name(lead_b));

	return DEFEAT_UNREADY;
}


static ilist second_wait_list = NULL;

void
clear_second_waits()
{
	int i;

	for (i = 0; i < ilist_len(second_wait_list); i++)
		rp_command(second_wait_list[i])->second_wait = FALSE;

	ilist_clear(&second_wait_list);
}


static void
set_second_waits(struct fight **l, int already_waiting)
{
	int i;
	struct command *c;

/*
 *  already_waiting is the id of a character which issued
 *  the ATTACK order.  It is already paying the day cost for
 *  the attack.  Its stackmates and the defenders are not, so 
 *  we set second_wait for them.
 */

	for (i = 0; i < plist_len(l); i++)
		if (l[i]->kind == FK_noble && l[i]->unit != already_waiting)
		{
			c = p_command(l[i]->unit);

			if (c->second_wait == FALSE)
			{
				c->second_wait = TRUE;
				ilist_append(&second_wait_list, l[i]->unit);
			}
		}
}


static int
regular_combat(int a, int b, int seize_slot, int already_waiting)
{
	struct fight **l_a;
	struct fight **l_b;
	int result;
	int defeat_flag;
	int lead_a;

	assert(a != b);

	l_a = construct_fight_list(a, b, FALSE, FALSE);
	l_b = construct_fight_list(b, a, TRUE, TRUE);

	ready_fight_list(l_a);
	ready_fight_list(l_b);

	set_second_waits(l_a, already_waiting);

	/*
	 *  We don't force side b to wait as well.
 	 *  This is on purpose (a playability choice)
	 *  set_second_waits(l_b, already_waiting);
	 */

	lead_a = lead_char(l_a);

	if (is_loc_or_ship(b) && plist_len(l_b) < 1)
	{
		out(lead_a, "%s is unoccupied.", box_name(b));

		if (seize_slot)
			seize_position(lead_a, b, 0);

		result = A_WON;
		goto done;
	}

	defeat_flag = fail_defeat_check(a, l_b);
	if (defeat_flag == DEFEAT_UNREADY)
	{
		result = TIE;
		goto done;
	}

	assert(plist_len(l_a) > 0);
	assert(plist_len(l_b) > 0);

	if (seize_slot)
		l_a[lead_char_pos(l_a)]->seize_slot = TRUE;

/*
 *  Note that seize_slot is always false for the defender.
 */

	result = combat_top(l_a, l_b, defeat_flag);

done:
	reclaim_fight_list(&l_a);
	reclaim_fight_list(&l_b);

	return result;
}


/*
 *  Who can we attack?
 *
 *	another character here
 *	a building or subloc of distance <= 1 that we can get to
 *	    this should cover castles, ships, and sublocs
 *	    while the distance requirement prevents inter-province attacks.
 *
 *  Thus, we can't attack outside characters from inside of a building.
 */

static int
select_target(struct command *c)
{
	int target = c->a;
	int where = subloc(c->who);
	struct exit_view *v;

	if (kind(target) == T_deadchar)
	{
		wout(c->who, "%s is not here.", c->parse[1]);
		return 0;
	}

	if (kind(target) == T_char)
	{
#if 0
		if (subloc(c->who) != subloc(target))
		{
			wout(c->who, "%s is not here.", c->parse[1]);
			return 0;
		}
#else
		if (!check_char_here(c->who, target))
			return 0;
#endif

		if (is_prisoner(target))
		{
			wout(c->who, "Cannot attack prisoners.");
			return 0;
		}

		if (c->who == target)
		{
			wout(c->who, "Can't attack oneself.");
			return 0;
		}

		if (stack_leader(c->who) == stack_leader(target))
		{
			wout(c->who, "Can't attack a member of the same stack.");
			return 0;
		}

		return stack_leader(target);
	}

	v = parse_exit_dir(c, subloc(c->who), "attack");

	if (v == NULL)
		return 0;

	if (v->direction != DIR_IN) 
	{
		wout(c->who, "%s cannot be attacked from here.",
				box_name(v->destination));
		return 0;
	}

	if (v->distance > 1)
	{
		wout(c->who, "%s is too far away to attack from here.",
				box_name(v->destination));
		return 0;
	}

	if (v->impassable)
	{
		wout(c->who, "%s is impassable.", box_name(v->destination));
		return 0;
	}

	return v->destination;
}


static int
select_attacker(int who, int target)
{
	int where = subloc(who);

	if (loc_depth(where) == LOC_build &&
	    building_owner(where) == who &&
	    !somewhere_inside(where, target))
	{
/*
 *  This code is apparently for naval combat.
 *
 *  One would attack another ship by specifying the direction link
 *  to the other ship.
 */

		who = where;
	}

	return who;
}


int
v_attack(struct command *c)
{
	int attacker;
	int target;
	int targ_who;
	int flag = c->b;
	int seize_slot;

	if (in_safe_now(c->who))
	{
		wout(c->who, "Combat is not permitted in safe havens.");
		return FALSE;
	}

	if (stack_leader(c->who) != c->who)
	{
		wout(c->who, "Only the stack leader may initiate combat.");
		return FALSE;
	}

	target = select_target(c);
	if (target <= 0)
		return FALSE;

	attacker = select_attacker(c->who, target);
	if (attacker <= 0)
		return FALSE;

	if (is_loc_or_ship(target))
	{
		if (loc_depth(target) == LOC_build)
			targ_who = building_owner(target);
		else
			targ_who = first_character(target);
	}
	else
		targ_who = target;

	if (player(c->who) == player(targ_who))
	{
		wout(c->who, "Units in the same faction may not engage "
				"in combat.");
		return FALSE;
	}

	if (in_safe_now(target))
	{
		wout(c->who, "Combat is not permitted in safe havens.");
		return FALSE;
	}

	if (flag)
		seize_slot = FALSE;
	else
		seize_slot = TRUE;

	regular_combat(attacker, target, seize_slot, c->who);

	return TRUE;
}


static int
loc_guarded(int where, int except)
{
	int i;
	int ret = FALSE;

	loop_here(where, i)
	{
		if (kind(i) == T_char && char_guard(i) && player(i) != except)
		{
			ret = i;
			break;
		}
	}
	next_here;

	return ret;
}


static int
attack_guard_units(int a, int b)
{
	struct fight **l_a;
	struct fight **l_b;
	int result;

	l_a = construct_fight_list(a, b, FALSE, FALSE);
	l_b = construct_guard_fight_list(b, a, l_a);

	ready_fight_list(l_a);
	ready_fight_list(l_b);

	if (plist_len(l_b) <= 0)
		return TRUE;		/* no guards */

	assert(plist_len(l_a) > 0);

	result = combat_top(l_a, l_b, FALSE);

	reclaim_fight_list(&l_a);
	reclaim_fight_list(&l_b);

	return result;
}


int
v_pillage(struct command *c)
{
	int where = subloc(c->who);
	int has = has_item(where, item_tax_cookie);
	int men = count_stack_fighters(c->who);
	int flag = c->a;
	int guard;

	if (in_safe_now(c->who))
	{
		wout(c->who, "Pillaging is not permitted in safe havens.");
		return FALSE;
	}

	if (has < 30)
	{
		wout(c->who, "There is nothing to loot and pillage here.");
		return FALSE;
	}

	if (men < 10)
	{
		wout(c->who, "At least 10 fighters are needed to pillage.");
		return FALSE;
	}

	if (stack_leader(c->who) != c->who)
	{
		wout(c->who, "Only the stack leader may pillage.");
		return FALSE;
	}

	if (guard = loc_guarded(where, player(c->who)))
	{
		wout(c->who, "%s is protected by guards.", box_name(where));

		if (!flag || in_safe_now(c->who))
			return FALSE;

		if (!attack_guard_units(c->who, guard))
		{
			return FALSE;
		}
	}

	return TRUE;
}


int
d_pillage(struct command *c)
{
	int where = subloc(c->who);
	int has = has_item(where, item_tax_cookie);
	int men = count_stack_fighters(c->who);
	int amount;
	int mob = 0;
	extern int gold_pillage;

	if (men < 10)
	{
		wout(c->who, "No longer have 10 fighters.");
		return FALSE;
	}

	if (has < 30)
	{
		wout(c->who, "There is nothing to loot and pillage here.");
		return FALSE;
	}

	amount = has;
	consume_item(where, item_tax_cookie, amount);
	gen_item(c->who, item_gold, amount);
	gold_pillage += amount;

	p_subloc(where)->loot += 4;
	p_subloc(where)->recent_loot = TRUE;

	wout(c->who, "Pillaging yielded %s.", gold_s(amount));

	/* Pillaging will also scare off any mage's customers */
	amount = has_item(where, item_mage_menial);
	consume_item(where, item_mage_menial, amount);

	if (subkind(where) == sub_city)
		wout(where, "%s loots the city.", box_name(c->who));
	else
		wout(where, "%s loots the countryside.", box_name(c->who));

	if (rnd(1,3) == 1)
		mob = create_peasant_mob(where);

	if (mob)
	{
		wout(c->who, "%s has formed to resist pillaging.",	
							liner_desc(mob));
		wout(where, "%s has formed to resist pillaging.",
							liner_desc(mob));

		if (rnd(1,3) == 1)
			queue(mob, "attack %s", box_code_less(c->who));
	}

	return TRUE;
}


int
v_guard(struct command *c)
{
	int flag = c->a;
	int where = subloc(c->who);

	if (flag)
	{
		p_char(c->who)->guard = TRUE;
		wout(c->who, "Will guard %s.", box_name(where));
		return TRUE;
	}

	p_char(c->who)->guard = FALSE;
	wout(c->who, "Will not guard %s.", box_name(where));

	return TRUE;
}


static void
auto_attack(int who, int target)
{

	out(who, "> [auto-attack %s]", box_code_less(target));
	regular_combat(who, target, FALSE, 0);
}


static void
check_auto_attack_sup(int who)
{
	int i;
	int where = subloc(who);
	int okay = TRUE;
	int target;
	int targ_who;
	int n;

	if (in_safe_now(who))	/* safe haven, no combat permitted */
		return;

#if 0
	if (loc_depth(where) == LOC_province && weather_here(where, sub_fog))
		return;
#endif

	if (char_gone(who))
		return;

	loop_here(where, i)
	{
		if (!is_hostile(who, i))
			continue;

		if (kind(i) == T_char)
		{
			if (is_prisoner(i))
				continue;

#if 1
			if (!char_here(who, i))
				continue;
#else
			if (char_really_hidden(i))
				continue;
#endif

			target = stack_leader(i);

			if (stack_leader(who) == target)
				continue;

			n = only_defeatable(target);
			if (n && !has_item(who, n))
				continue;
		}
		else if (is_loc_or_ship(i))
		{
			if (kind(i) == T_loc &&
			    loc_hidden(i) &&
			    !test_known(who, i))
				continue;

			target = i;

			if (loc_depth(i) == LOC_build)
				targ_who = building_owner(target);
			else
				targ_who = first_character(target);

			if (targ_who == 0 ||
			    player(who) == player(targ_who))
				continue;
		}
		else
			continue;

		auto_attack(who, target);
		okay = FALSE;
		break;			/* only get to attack first target */
	}
	next_here;

	if (okay && is_ship(where))
	{
		int outer = subloc(where);

		loop_here(outer, i)
		{
			if (i == where || !is_ship(i))
				continue;

			if (!is_hostile(who, i))
				continue;

#if 1
			if (building_owner(i) == 0)
				continue;
#endif

			auto_attack(who, i);
			okay = FALSE;
			break;		/* only get to attack first target */
		}
		next_here;
	}
}


void
check_all_auto_attacks()
{
	int i;
	struct command *c;

	loop_char(i)
	{
		if (stack_parent(i))	/* must be stack leader to initiate */
			continue;	/* auto-attack */

		if (char_health(i) != 100 && char_health(i) != -1)
			continue;

		c = rp_command(i);

		check_auto_attack_sup(i);
	}
	next_char;
}

