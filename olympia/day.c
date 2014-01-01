
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"z.h"
#include	"oly.h"



/*
 *  0	not on the ocean
 *  1	near a coast
 *  2	near a dangerous coast
 *  3	not near a coast
 */

static int
near_rocky_coast(int where)
{
	struct exit_view **l;
	int i;
	int ret = 3;

	if (subkind(where) != sub_ocean)
		return 0;

	l = exits_from_loc_nsew(0, where);

	for (i = 0; i < ilist_len(l); i++)
		if (subkind(l[i]->destination) != sub_ocean)
		{
			if (subkind(l[i]->destination == sub_mountain))
				return 2;
			ret = 1;
		}

	return ret;
}


static void
ship_coastal_damage()
{
	int ship;
	int n;

	loop_ship(ship)
	{
		if (!is_ship(ship))		/* not completed */
			continue;

		n = near_rocky_coast(subloc(ship));

		switch (n)
		{
		case 0:
			break;

		case 1:
			if (rnd(1,75) == 1)
			{
				wout(ship, "%s struck a coastal reef.  "
					"There is minor damage to the ship.",
							box_name(ship));
				add_structure_damage(ship, rnd(3,5));
			}
			break;

		case 2:
			if (rnd(1,50) == 1)
			{
				wout(ship, "%s struck some submerged rocks.  "
					"There is minor damage to the ship.",
							box_name(ship));
				add_structure_damage(ship, rnd(6,9));
			}
			break;

		case 3:
			switch (rnd(1,200))
			{
			case 1:
			case 2:
			    wout(ship, "Hungry looking birds circle overhead.");
			    break;

			case 3:
			    wout(ship, "Sharks circle in the water a short "
						"distance from the ship.");
			    break;
			}
			break;

		default:
			assert(FALSE);
		}
	}
	next_ship;
}


static void
mine_calamity(int mine)
{
	int what;
	int i;
	int to_kill;
	int m, n;

	what = rnd(1,4);

	vector_char_here(mine);
	vector_add(mine);

	switch (what)
	{
	case 1:
		wout(VECT, "A tunnel in %s has caved in!", box_name(mine));
		break;

	case 2:
		wout(VECT, "A roof has collapsed in a shaft in %s!",
						box_name(mine));
		break;

	case 3:
		wout(VECT, "A new fissure in a shaft wall has allowed "
					"poisonous gas to seep into %s.",
						box_name(mine));
		break;

	case 4:
		wout(VECT, "Rock dust has caused an explosion in %s!",
						box_name(mine));
		break;

	default:
		assert(FALSE);
	}

	to_kill = min(rnd(1,5), count_loc_char_item(mine, item_worker));

	if (to_kill > 0)
	{
		wout(VECT, "%s miner%s %s killed in the accident.",
					cap(nice_num(to_kill)),
					add_s(to_kill),
					to_kill == 1 ? "was" : "were");
		loop_char_here(mine, i)
		{
			if (m = has_item(i, item_worker))
			{
				n = min(m, to_kill);

				wout(i, "%s lost %s worker%s.",
						box_name(i),
						nice_num(n), add_s(n));
				consume_item(i, item_worker, n);

				to_kill -= n;

				if (to_kill <= 0)
					break;
			}
		}
		next_char_here;
	}

	add_structure_damage(mine, rnd(1, 15));
}



static void
inn_calamity(int where)
{
	int own;
	int dam;
	char buf[LEN];

	own = building_owner(where);
	dam = rnd(5, 15);

	switch (rnd(1,6))
	{
	case 1:	strcpy(buf, "Some customers ");				break;
	case 2:	strcpy(buf, "Some patrons ");				break;
	case 3:	strcpy(buf, "An irate customer ");			break;
	case 4:	strcpy(buf, "Two large, angry men ");			break;
	case 5:	strcpy(buf, "A surly local ");				break;
	case 6:	strcpy(buf, "A party of traveling entertainers ");	break;
	}

	switch (rnd(1,5))
	{
	case 1:	strcat(buf, "got drunk, ");			break;
	case 2:	strcat(buf, "started a fight, ");		break;
	case 3: strcat(buf, "got drunk and started a fight, ");	break;
	case 4:	strcat(buf, "insulted the chef, ");		break;
	case 5:	strcat(buf, "refused to pay, ");		break;
	}

	switch (rnd(1,7))
	{
	case 1: strcat(buf, "and broke some furniture");	break;
	case 2: strcat(buf, "and damaged a wall");		break;
	case 3: strcat(buf, "and kicked in the door");		break;
	case 4: strcat(buf, "and knocked over a keg of beer");	break;
	case 5: strcat(buf, "and set a fire in the closet");	break;
	case 6: strcat(buf, "and knocked over the smokehouse");	break;
	case 7: strcat(buf, "and broke some chairs");		break;
	}

	if (own)
	{
		wout(own, "%s:  %s, causing %d points of damage.",
				box_name(where), buf, dam);
	}

	add_structure_damage(where, dam);
}


static void
random_loc_damage()
{
	int where;
	int depth;

	loop_loc(where)
	{
		switch (subkind(where))
		{
		case sub_mine:
			depth = mine_depth(where);

			if (rnd(1, 90) <= depth)
				mine_calamity(where);
			break;

		case sub_inn:
			if (rnd(1, 100) == 1)
				inn_calamity(where);
			break;
		}
	}
	next_loc;
}


static void
dogs_bark_at_hidden_chars()
{
	int where;
	int flag;
	int i;

	loop_loc(where)
	{
		flag = 0;

		loop_here(where, i)
		{
			if (kind(i) == T_char && char_really_hidden(i))
				flag = 1;
		}
		next_here;

		if (flag)
			bark_dogs(where);
	}
	next_loc;
}


static void
heal_char_sup(int who)
{
	int h;
	int inn = FALSE;
	int chance;
	int amount;

	h = char_health(who);

	if (h >= 100)
		return;

	if (subkind(subloc(who)) == sub_inn)
		inn = TRUE;

	chance = 5;
	if (inn)
		chance = 10;

	if (char_sick(who) && rnd(1,100) <= chance)
	{
		wout(who, "%s defeated illness and is now recovering.",
							box_name(who));

		p_char(who)->sick = FALSE;
	}

	amount = rnd(3, 15);

	if (char_sick(who))
	{
		if (h - amount < 0)
			amount = h;

		wout(who, "%s lost %s health.",
				box_name(who), nice_num(amount));

		p_char(who)->health -= amount;

		if (char_health(who) <= 0)
			kill_char(who, MATES);
	}
	else
	{
		if (h + amount > 100)
			amount = 100 - h;

		wout(who, "%s gained %s health.",
				box_name(who), nice_num(amount));

		p_char(who)->health += amount;

		if (h + amount >= 100)
			wout(who, "%s is fully healed.", box_name(who));
	}
}


static void
default_garrison_pay()
{
	int who;
	int parent;
	extern int gold_garrison;

	loop_char(who)
	{
		if (is_prisoner(who))
			continue;

		parent = stack_leader(who);

		if (parent == 0 || !default_garrison(parent))
			continue;

		if (default_garrison(who))
			continue;

		gen_item(who, item_gold, garrison_pay);
		if (!is_npc(who))
			gold_garrison += garrison_pay;

		wout(who, "Received %s from %s.",
			gold_s(garrison_pay),
			box_name(parent));
	}
	next_char;
}


static void
heal_characters()
{
	int who;
	int n;

	loop_char(who)
	{
		n = char_health(who);

		if (n >= 0 && n < 100)
			heal_char_sup(who);
	}
	next_char;
}


static void
add_claim_gold()
{
	int pl;

	loop_player(pl)
	{
		switch (subkind(pl))
		{
		case sub_pl_regular:
			gen_item(pl, item_gold, 25);
			break;
		}
	}
	next_player;
}

static void
add_noble_points()
{
	int pl;

	loop_player(pl)
	{
		switch (subkind(pl))
		{
		case sub_pl_regular:
			if (next_np_turn(pl) == 0)
				add_np(pl, 1);
			break;
		}
	}
	next_player;
}

void
add_unformed_sup(int pl)
{
	struct entity_player *p;
	int new;

	p = rp_player(pl);
	if (p == NULL)
		return;

	while (ilist_len(p->unformed) < 5)
	{
		new = new_ent(T_unform, 0);
		if (new <= 0)
			break;

		ilist_append(&p->unformed, new);
	}
}


static void
add_unformed()
{
	int pl;

	loop_player(pl)
	{
		add_unformed_sup(pl);

	}
	next_player;
}


static void
increment_current_aura()
{
	int who;
	struct char_magic *p;
	int ac;					/* auraculum */
	int ma;					/* max aura */

	loop_char(who)
	{
		if (!is_magician(who))
			continue;

		ac = has_auraculum(who);
		ma = max_eff_aura(who);

		p = p_magic(who);

		if (p->cur_aura < ma)
			p->cur_aura++;		/* two point natural rise */
		if (p->cur_aura < ma)
			p->cur_aura++;

		if (ac)			/* auraculum grants two more points */
		{
			if (p->cur_aura < ma)
				p->cur_aura++;
			if (p->cur_aura < ma)
				p->cur_aura++;
		}

		{
			struct item_ent *e;
			int n;

			loop_inv(who, e)
			{
				if (n = item_aura_bonus(e->item))
					if (p->cur_aura < ma)
						p->cur_aura++;
			}
			next_inv;
		}
	}
	next_char;
}


static void
increment_stone_ring_aura()
{
	int who;
	struct char_magic *p;
	int ma;					/* max aura */

	loop_char(who)
	{
		if (!is_magician(who) || subkind(subloc(who)) != sub_stone_cir)
			continue;

		ma = max_eff_aura(who);

		p = p_magic(who);

		if (p->cur_aura < ma)
		{
			p->cur_aura++;		/* one point bonus */
			wout(who, "Current aura is now %d.", p->cur_aura);
		}
	}
	next_char;
}


static void
decrement_ability_shroud()
{
	int who;
	struct char_magic *p;

	loop_char(who)
	{
		p = rp_magic(who);

		if (p && p->ability_shroud > 0)
			p->ability_shroud--;
	}
	next_char;
}


static void
decrement_loc_barrier()
{
	int where;
	struct entity_loc *p;

	loop_loc(where)
	{
		p = rp_loc(where);

		if (p && p->barrier > 0)
		{
		    p->barrier--;
		    if (p->barrier == 0)
			wout(where, "The barrier over %s has dissipated.",
							box_name(where));
		}
	}
	next_loc;
}


static void
decrement_region_shroud()
{
	int where;
	struct entity_loc *p;

	loop_loc(where)
	{
		p = rp_loc(where);

		if (p && p->shroud > 0)
		{
			p->shroud--;
			notify_loc_shroud(where);
		}
	}
	next_loc;
}


static void
decrement_meditation_hinder()
{
	int who;
	struct char_magic *p;

	loop_char(who)
	{
		p = rp_magic(who);

		if (p && p->hinder_meditation > 0)
			p->hinder_meditation--;
	}
	next_char;
}


static void
noncreator_curse_erode()
{
	int who;
	struct item_ent *e;
	struct item_magic *im;
	int creator;
	int curse;

	loop_char(who)
	{
		loop_inv(who, e)
		{
			im = rp_item_magic(e->item);
			if (im == NULL)
				continue;

			if (im->curse_loyalty == 0)
				continue;

			if (im->creator == who)
				continue;

#if 0
			if (kind(im->creator) != T_char)	/* lazy cleanup */
			{
			    log_write(LOG_CODE, "noncreator_curse_erode: "
					"lazy cleanup (creat=%d,item=%d,"
					"curse=%d)",
					im->creator,
					e->item,
					im->curse_loyalty);

			    im->creator = 0;
			    im->curse_loyalty = 0;
			}
#endif

			if (loyal_kind(who) == LOY_oath)
				continue;
/*
 *  NOTYET:  must fix
 */


#if 1
			log_write(LOG_CODE, "noncreator_curse_erode: NOTYET!");
#else
			    delta_loyalty(who, -(im->curse_loyalty), TRUE);

			    log_write(LOG_SPECIAL, "%s loses %d loyalty from a "
					"curse on %s.",
					box_name(who), im->curse_loyalty,
					box_name(e->item));
#endif
		}
		next_inv;
	}
	next_char;
}


#if 0
/*
 *  Print the end-of-turn message at the end of each monthly events log.
 *
 *  It is important to generate some output for every character, so that
 *  even if nothing happened to a unit, it will still have a marker set
 *  saying that we have some output for it.  Presence of these markers is
 *  used to drive unit's inclusion into the master turn report.
 */

static void
announce_month_end(char *msg)
{
	int i;

	loop_loc_or_ship(i)
	{
		if (loc_depth(i) > LOC_region)
			out(i, msg);
	}
	next_loc_or_ship;

	loop_char(i)
	{
		out(i, msg);
	}
	next_char;

	loop_player(i)
	{
		out(i, msg);
	}
	next_player;
}
#endif


/*
 *  Decay unit loyalties at the end of the turn.
 */

static void
loyalty_decay()
{
	int who;
	struct entity_char *p;

	loop_char(who)
	{
		p = rp_char(who);
		if (p == NULL || p->fresh_hire)
			continue;

		if (p->loy_kind == LOY_unsworn ||
		    p->loy_kind == LOY_oath ||
		    p->loy_kind == LOY_npc)
			continue;		/* no decay */

		if (p->loy_rate <= 0 && p->loy_kind == LOY_summon)
		{
			leave_stack(who);
			set_loyal(who, LOY_npc, 0);	/* redundant */
			continue;
		}

		if (p->loy_rate < 50 &&
		    p->loy_kind == LOY_contract &&
		    rnd(1,2) == 1)
		{
			log_write(LOG_DEATH, "%s deserts, %s", box_name(who), loyal_s(who));
			unit_deserts(who, indep_player, TRUE, LOY_unsworn, 0);
			continue;
		}

		switch (p->loy_kind)
		{
		case LOY_summon:
			p->loy_rate--;
			break;

		case LOY_fear:
			p->loy_rate--;
			if (p->loy_rate <= 0 && rnd(1,2) == 1)
			{
				log_write(LOG_DEATH, "%s deserts, %s", box_name(who), loyal_s(who));
				unit_deserts(who, indep_player, TRUE, LOY_unsworn, 0);
			}
			break;

		case LOY_contract:
			p->loy_rate -= max(50, p->loy_rate/10);
			break;

		default:
			assert(FALSE);
		}

		if (p->loy_rate < 0)
			p->loy_rate = 0;
	}
	next_char;
}


static void
relic_decay()
{
	int item;
	int owner;

	loop_item(item)
	{
		if (subkind(item) != sub_relic)
			continue;

		if (item_relic_decay(item) == 0)
			continue;

		p_item_magic(item)->relic_decay--;

		if (item_relic_decay(item) > 0)
			continue;

		owner = item_unique(item);
		if (kind(owner) == T_char)
			wout(owner, "%s vanishes.", box_name(item));

		move_item(owner, nowhere_loc, item, 1);
	}
	next_item;
}


static void
pillage_decay()
{
	int where;

	loop_loc(where)
	{
		if (loc_pillage(where))
		{
			if (!recent_pillage(where))
				p_subloc(where)->loot--;
		}
	}
	next_loc;
}


static void
auto_drop()
{
	int pl;
	struct entity_player *p;

	loop_pl_regular(pl)
	{
		p = p_player(pl);

		if (sysclock.turn - p->last_order_turn >= auto_quit_turns)
		{
			char *s = "";
			char *email = "";

			if (rp_player(pl))
			{
				if (rp_player(pl)->email && *rp_player(pl)->email)
					email = rp_player(pl)->email;

				if (rp_player(pl)->full_name &&
					*rp_player(pl)->full_name)
					s = rp_player(pl)->full_name;
			}

			queue(pl, "quit");
			log_write(LOG_SPECIAL, "Queued drop for %s", box_name(pl));
			log_write(LOG_SPECIAL, "    %s <%s>", s, email);
		}
	}
	next_pl_regular;
}


int
maint_cost(int item)
{

	switch (item)
	{
	case item_peasant:		return 1;

	case item_worker:
	case item_soldier:
	case item_sailor:
	case item_angry_peasant:
	case item_crossbowman:		return 2;

	case item_blessed_soldier:
	case item_pikeman:
	case item_swordsman:
	case item_pirate:
	case item_archer:		return 3;

	case item_knight:
	case item_elite_arch:		return 4;

	case item_elite_guard:		return 5;
	}

	return 0;
}


static void
men_starve(int who, int have)
{
	static ilist item = NULL;
	static ilist qty = NULL;
	static ilist cost = NULL;
	static ilist starve = NULL;
	int failcheck = 0;
	struct item_ent *e;
	int npaid = 0;
	int gold;
	int nstarve;
	int nmen = 0;
	int n;
	int i;
	char *s;
	int hit_one;

	ilist_clear(&item);
	ilist_clear(&qty);
	ilist_clear(&cost);
	ilist_clear(&starve);

	loop_inv(who, e)
	{
		if (n = maint_cost(e->item))
		{
			ilist_append(&item, e->item);
			ilist_append(&qty, e->qty);
			ilist_append(&cost, n);
			ilist_append(&starve, 0);

			nmen += e->qty;
		}
	}
	next_inv;

	gold = have;

	do {
		hit_one = FALSE;

		for (i = 0; i < ilist_len(item); i++)
		{
			if (qty[i] > 0 && have >= cost[i])
			{
				have -= cost[i];
				qty[i]--;
				npaid++;
				hit_one = TRUE;
			}
		}
	}
	while (hit_one && have > 0);

	gold -= have;
	nstarve = nmen - npaid;
	nstarve = (nstarve + 2) / 3;

	assert(nstarve > 0);

	i = 0;
	while (nstarve > 0)
	{
		assert(failcheck++ < 10000);

		if (qty[i])
		{
			nstarve--;
			starve[i]++;
			qty[i]--;
		}

		if (++i >= ilist_len(item))
			i = 0;
	}

	autocharge(who, gold);

	for (i = 0; i < ilist_len(item); i++)
	{
		if (starve[i])
		{
			if (item[i] == item_peasant)
				s = "starved";
			else
			{
				if (rnd(1,2) == 1)
					s = "left service";
				else
					s = "deserted";
			}

			wout(who, "%s %s.",
				cap(just_name_qty(item[i], starve[i])), s);
			consume_item(who, item[i], starve[i]);

			if (item[i] == item_sailor || item[i] == item_pirate)
				check_captain_loses_sailors(starve[i], who, 0);
		}
	}
}


int
unit_maint_cost(int who)
{
	struct item_ent *e;
	int cost = 0;

	loop_inv(who, e)
	{
		if (e->item != noble_item(who))   /* don't charge ni beasts */
			cost += maint_cost(e->item) * e->qty;
	}
	next_inv;

	return cost;
}


void
charge_maint_sup(int who)
{
	int cost;
	int have;

	cost = unit_maint_cost(who);

	if (cost < 1)
		return;

	if (autocharge(who, cost))
	{
		wout(who, "Paid maintenance of %s.", gold_s(cost));
		return;
	}

	have = stack_has_item(who, item_gold);

	wout(who, "Maintenance costs are %s, can afford %s.",
				gold_s(cost), gold_s(have));

	men_starve(who, have);
}


static void
charge_maint_costs()
{
	int who;

	loop_char(who)
	{
#if 0
		if (loyal_kind(who) == LOY_unsworn)
			continue;
#endif

		if (subkind(player(who)) != sub_pl_regular)
			continue;

		charge_maint_sup(who);
	}
	next_char;
}


static void
animal_deaths()
{
	int who;
	struct item_ent *e;
	int num_to_kill;

	stage("animal_deaths()");

	loop_char(who)
	{
		if (subkind(player(who)) != sub_pl_regular)
			continue;

		loop_inv(who, e)
		{
			if (e->qty > 0 && item_animal(e->item))
			{
				int dead = 0;
				int i;

				for (i = 1; i <= e->qty; i++)
					if (rnd(1, 1000) < 10)
						dead++;

				if (dead > 0)
				{
					consume_item(who, e->item, dead);
					wout(who, "%s %s died.",
					    cap(nice_num(dead)),
					    plural_item_name(e->item, dead));
				}
			}
		}
		next_inv;
	}
	next_char;
}


static void
inn_income()
{
	int i;			/* variable to iterate over inns */
	int owner;		/* owner of inn */
	int n_inns;		/* number of inns sharing this province */
	int where;		/* where is the inn */
	int base;		/* base of money inn will get */
	int pil;		/* location pillage severity */
	extern int gold_inn;

	loop_inn(i)
	{
		owner = building_owner(i);

		if (owner == 0)
			continue;

		where = subloc(i);
		n_inns = count_loc_structures(where, sub_inn, 0);

		base = rnd(50, 75);

		if (pil = loc_pillage(where))
			base /= (pil + 1);

		base /= n_inns;

		if (pil == 0 && rnd(1,8) == 1)
		{
			int bonus = rnd(5,13) * 10;

			wout(owner, "A rich traveller stayed in %s this "
					"month, spending %s.",
				box_name(i), gold_s(bonus));

			base += bonus;
		}

		gen_item(owner, item_gold, base);
		gold_inn += base;
		wout(owner, "%s yielded %s in income.",
				box_name(i), gold_s(base));

		if (pil)
		{
			switch (rnd(1,3))
			{
			case 1:
				wout(owner, "Patrons were scared away by "
					"recent looting in the province.");
				break;
			case 2:
				wout(owner, "Profits were hurt by pillaging "
							"in the area.");
				break;
			case 3:
				wout(owner, "Recent pillaging in the area "
							"lowered profits.");
				break;
			}
		}
	}
	next_inn;
}


static void
temple_income()
{
	int i;			/* variable to iterate over temples */
	int owner;		/* owner of inn */
	int where;
	int amount;
	int pil;
	extern int gold_temple;

	loop_temple(i)
	{
		owner = building_owner(i);

		if (owner == 0 || !is_priest(owner))
			continue;

		amount = 100;
		where = subloc(i);
		if (pil = loc_pillage(where))
			amount /= (pil + 1);
		gen_item(owner, item_gold, amount);
		gold_temple += amount;
		wout(owner, "%s collected offerings of %s.",
				box_name(i), gold_s(amount));
	}
	next_temple;
}


static void
collapsed_mine_decay()
{
	int i;
	struct entity_misc *p;

	loop_collapsed_mine(i)
	{
		p = p_misc(i);

		p->mine_delay--;
		if (p->mine_delay < 0)
			p->mine_delay = 0;

		if (p->mine_delay == 0)
			get_rid_of_collapsed_mine(i);
	}
	next_collapsed_mine;
}


/*
 *  1 ghost warrior evaporates at the end of each turn
 */

static void
ghost_warrior_decay()
{
	int i;
	int has;

	loop_char(i)
	{
		if (is_npc(i))
			continue;

		has = has_item(i, item_ghost_warrior);
		if (has <= 0)
			continue;

		has = 1;

		if (has)
		{
			wout(i, "%s evaporated.",
				cap(box_name_qty(item_ghost_warrior,has)));
			consume_item(i, item_ghost_warrior, has);
		}
	}
	next_char;
}


/*
 *  0-1-2 corpses decompose at the end of each month
 */

static void
corpse_decay()
{
	int i;
	int has;

	loop_char(i)
	{
		if (is_npc(i))
			continue;

		has = has_item(i, item_corpse);
		if (has <= 0)
			continue;

		has = min(has, rnd(0,2));

		if (has)
		{
			wout(i, "%s decomposed.",
				cap(box_name_qty(item_corpse, has)));
			consume_item(i, item_corpse, has);
		}
	}
	next_char;
}


/*
 *  Bodies of dead nobles rot after 12 turns
 */

static void
dead_body_rot()
{
	int i;
	int owner;
	int t;

	loop_dead_body(i)
	{
		owner = item_unique(i);
		assert(owner);

		t = sysclock.turn - p_char(i)->death_time.turn;

		if (t < 12)
			continue;

		if (kind(owner) == T_char)
			wout(owner, "%s decomposed.", box_name(i));

		destroy_unique_item(owner, i);
	}
	next_dead_body;
}


static void
storm_decay()
{
	int i;
	struct entity_misc *p;

	loop_storm(i)
	{
		p = p_misc(i);

		p->storm_str--;
		if (p->storm_str > 0)
			continue;

		p->storm_str = 0;

		dissipate_storm(i, TRUE);
	}
	next_storm;
}


static void
storm_owner_touch_loc()
{
	int i;
	int owner;
	int where;
	int pl;

	loop_storm(i)
	{
		owner = npc_summoner(i);

		if (owner)
		{
			where = subloc(i);
			pl = player(owner);
			if (pl)
				touch_loc_pl(pl, where);
		}
	}
	next_storm;
}


static void
storm_move()
{
	int i;
	struct entity_misc *p;
	int owner;

	loop_storm(i)
	{
		p = p_misc(i);

		if (!p->npc_dir)
			continue;

		assert(loc_depth(p->storm_move) == LOC_province);

		set_where(i, p->storm_move);

		owner = npc_summoner(i);
		if (owner && valid_box(owner))
			set_known(owner, p->storm_move);

		p->npc_dir = 0;
		p->storm_move = 0;
	}
	next_storm;
}


static void
quest_decay()
{
	int where;

	loop_loc(where)
	{
		if (subloc_quest(where) > 0)
			p_subloc(where)->quest_late--;
	}
	next_loc;
}


static void
collect_taxes()
{
	int fort;
	int prov;
	int amount;
	int owner;
	extern int gold_taxes;

	loop_castle(fort)
	{
		prov = province(fort);
		owner = building_owner(fort);

		if (owner == 0)
			continue;		/* no one to collect taxes */

		amount = has_item(prov, item_tax_cookie);

		consume_item(prov, item_tax_cookie, amount);
		amount /= 2;
		gold_taxes += amount;
		gen_item(owner, item_gold, amount);

		wout(owner, "Collected %s in taxes.", gold_s(amount));
#if 0
		fprintf(stderr, "  castle %s, %s gets %d\n", box_code_less(fort), box_code_less(owner), amount);
#endif
	}
	next_castle;
}


void
compute_civ_levels()
{
	int where;
	int flag;
	int i;
	int dest_civ;
	int crown_loc = -1;
 
	int flag_castle;
	int flag_tower;
	int flag_temple;
	int flag_inn;
	int flag_mine;
	int flag_city;

	int prev_loc = -1;

	if (bx[RELIC_CROWN]) {
		int crown = item_unique(RELIC_CROWN);
		crown_loc = province(crown);
	}
	stage("compute_civ_levels()");

	clear_temps(T_loc);

	if (valid_box(crown_loc))
		bx[crown_loc]->temp += 8;	/* crown gives +2 civ */

	loop_province(where)
	{
	    if (where != prev_loc)
	    {
		prev_loc = where;

		flag_castle = TRUE;
		flag_tower = TRUE;
		flag_temple = TRUE;
		flag_inn = TRUE;
		flag_mine = TRUE;
		flag_city = TRUE;
	    }

	    loop_all_here(where, i)
	    {

		switch (subkind(i))
		{
		case sub_castle:
			if (flag_castle)
			{
				bx[province(i)]->temp += 6 + castle_level(i);
				flag_castle = FALSE;
			}
			break;

		case sub_tower:
			if (flag_tower)
			{
				bx[province(i)]->temp += 4;
				flag_tower = FALSE;
			}
			break;

		case sub_temple:
			if (flag_temple)
			{
				bx[province(i)]->temp += 4;
				flag_temple = FALSE;
			}
			break;

		case sub_inn:
			if (flag_inn)
			{
				bx[province(i)]->temp += 4;
				flag_inn = FALSE;
			}
			break;

		case sub_mine:
			if (flag_mine)
			{
				bx[province(i)]->temp += 4;
				flag_mine = FALSE;
			}
			break;

		case sub_city:
			if (flag_city)
			{
				if (major_city(i))
					bx[province(i)]->temp += 8;
				else
					bx[province(i)]->temp += 4;
				flag_city = FALSE;
			}
			break;
		}
	    }
	    next_all_here;
	}
	next_province;

	do
	{
		flag = FALSE;

		loop_province(where)
		{
			if (subkind(where) == sub_ocean)
				continue;

			loop_prov_dest(where, i)
			{
				if (i == 0)
					continue;

				dest_civ = bx[i]->temp / 2;

				if (dest_civ > bx[where]->temp)
				{
					bx[where]->temp = dest_civ;
					flag = TRUE;
				}
			}
			next_prov_dest;
		}
		next_province;
	}
	while (flag);

	loop_province(where)
	{
		p_loc(where)->civ = bx[where]->temp / 4;
	}
	next_province;
}


void
post_production()
{

	compute_civ_levels();
	location_trades();
	location_production();
	seed_taxes();

	post_has_been_run = TRUE;
}


static void
hide_mage_decay()
{
	int i;
	struct char_magic *p;

	loop_char(i)
	{
		p = rp_magic(i);
		if (p && p->hide_mage > 0)
			p->hide_mage--;
	}
	next_char;
}


static void
link_decay()
{
	int i;
	struct entity_subloc *p;

	loop_loc(i)
	{
		p = rp_subloc(i);

		if (p == NULL || ilist_len(p->link_to) < 1)
			continue;

		if (p->link_open > 0)
			p->link_open--;

		if (p->link_when == oly_month(sysclock))
		{
			if (p->link_open < 2 && p->link_open >= 0)
				p->link_open = 2;
		}
	}
	next_loc;
}


/*
 *  Move tax base from cities out to province level, so it
 *  can be collected
 */

static void
move_city_gold()
{
	int i;
	int prov;
	int has;

	loop_city(i)
	{
		prov = province(i);
		has = has_item(i, item_tax_cookie);

		move_item(i, prov, item_tax_cookie, has);
	}
	next_city;
}


/*
 *       Season  Month   Name
 *       ------  -----   ----
 *
 *       Spring    1     Fierce winds
 *       Spring    2     Snowmelt
 *       Summer    3     Blossom bloom
 *       Summer    4     Sunsear
 *       Fall      5     Thunder and rain
 *       Fall      6     Harvest
 *       Winter    7     Waning days
 *       Winter    8     Dark night
 *
 *
 *	Uldim pass and Summerbridge are open during months 3-4-5-6
 *	At the end of month 2, issue the "now open" message.
 *	At the end of month 6, issue the "now closed" message.
 */

static void
special_locs_open()
{
	int i;

	loop_province(i)
	{
		if (summerbridge(i) == 1)
		{
			log_write(LOG_CODE, "%s open to the north.", box_name(i));
			wout(i, "The swamps of Summerbridge have dried "
					"enough to permit passage north.");
		}
		else if (summerbridge(i) == 2)
		{
			log_write(LOG_CODE, "%s open to the south.", box_name(i));
			wout(i, "The swamps of Summerbridge have dried "
					"enough to permit passage south.");
		}
		else if (uldim(i) == 3)
		{
			log_write(LOG_CODE, "%s open to the south.", box_name(i));
			wout(i, "The snows blocking Uldim pass to the south "
				"have melted.");
		}
		else if (uldim(i) == 4)
		{
			log_write(LOG_CODE, "%s open to the north.", box_name(i));
			wout(i, "The snows blocking Uldim pass to the north "
				"have melted.");
		}
	}
	next_province;
}


static void
special_locs_close()
{
	int i;

	loop_province(i)
	{
		if (summerbridge(i) == 1)
		{
			log_write(LOG_CODE, "%s closed to the north.", box_name(i));
			wout(i, "Seasonal rains have made Summerbridge an "
				"impassable bog to the north.");
		}
		else if (summerbridge(i) == 2)
		{
			log_write(LOG_CODE, "%s closed to the south.", box_name(i));
			wout(i, "Seasonal rains have made Summerbridge an "
				"impassable bog to the south.");
		}
		else if (uldim(i) == 3)
		{
			log_write(LOG_CODE, "%s closed to the south.", box_name(i));
			wout(i, "Falling snow blocks Uldim pass to the south "
				"for the winter.");
		}
		else if (uldim(i) == 4)
		{
			log_write(LOG_CODE, "%s closed to the north.", box_name(i));
			wout(i, "Falling snow blocks Uldim pass to the north "
				"for the winter.");
		}
	}
	next_province;
}


static void
clear_orders_sent()
{
	int pl;
	struct entity_player *p;

	loop_player(pl)
	{
		p = rp_player(pl);

		if (p)
			p->sent_orders = 0;
	}
	next_player;
}


void
post_month()
{

	stage("post_month()");

	clear_orders_sent();

	if (oly_month(sysclock) == 2)
		special_locs_open();

	if (oly_month(sysclock) == 6)
		special_locs_close();

#if 0
	announce_month_end("** End of turn **");
	announce_month_end("@null");
#endif

	move_city_gold();
	garrison_gold();
	collect_taxes();
	add_claim_gold();
	add_noble_points();
	add_unformed();
	increment_current_aura();
	decrement_ability_shroud();
	decrement_region_shroud();
	decrement_meditation_hinder();
	decrement_loc_barrier();
	loyalty_decay();
	pillage_decay();
	relic_decay();
	hide_mage_decay();
	inn_income();
	temple_income();
	charge_maint_costs();
	animal_deaths();
	ghost_warrior_decay();
	corpse_decay();
	dead_body_rot();
	storm_decay();
	storm_move();
	collapsed_mine_decay();
	post_production();
	if (auto_quit_turns > 0)
		auto_drop();
	link_decay();
	quest_decay();
	check_token_units();
	determine_noble_ranks();

#if 0
	swap_region_locs(hades_region);
	swap_region_locs(faery_region);
#endif

	post_has_been_run = TRUE;
}


void
daily_events()
{
	static int curse_erode_day = 0;
	static ilist weather_days = NULL;
	static int wday_index = 0;
	static int faery_day = 0;
	static int dog_bark_day = 0;

	if (curse_erode_day == 0)
		curse_erode_day = rnd(1, MONTH_DAYS);

	if (faery_day == 0)
		faery_day = rnd(MONTH_DAYS/2, MONTH_DAYS);

	if (dog_bark_day == 0)
		dog_bark_day = rnd(1, MONTH_DAYS);

	if (ilist_len(weather_days) == 0)
	{
		int i;
		extern int int_comp();

		for (i = 1; i <= MONTH_DAYS; i++)
			ilist_append(&weather_days, i);

		assert(4 <= MONTH_DAYS);

		ilist_scramble(weather_days);
		qsort(weather_days, 4, sizeof(int), int_comp);
	}

	default_garrison_pay();
	ship_coastal_damage();
	random_loc_damage();
	check_ocean_chars();

	if (sysclock.day % 7 == 0)
	{
		heal_characters();
		weekly_prisoner_escape_check();
	}

	if (sysclock.day == curse_erode_day)
		noncreator_curse_erode();

	if (sysclock.day == 15)
		increment_stone_ring_aura();

	if (sysclock.day == dog_bark_day)
		dogs_bark_at_hidden_chars();

	if (sysclock.day == weather_days[wday_index])
	{
		wday_index++;

		natural_weather();
	}

	if (sysclock.day == faery_day)
		auto_faery();
}


void
touch_loc_pl(int pl, int where)
{
	struct entity_player *p;
	int inside;

	p = p_player(pl);
	where = viewloc(where);

	set_bit(&p->locs, where);

	if (loc_depth(where) > LOC_province)
	{
		inside = loc(where);
		set_bit(&p->locs, inside);
	}
}


void
touch_loc(int who)
{
	int where = subloc(who);

	touch_loc_pl(player(who), where);
}


void
init_locs_touched()
{
	int who;

	loop_char(who)
	{
		if (!is_prisoner(who))
			touch_loc(who);
	}
	next_char;

	storm_owner_touch_loc();
#if 0
	touch_garrison_locs();
#endif
}

