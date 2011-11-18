
#include	<stdio.h>
#include	<stdlib.h>
#include	"z.h"
#include	"oly.h"


struct use_tbl_ent {
	char *allow;		/* who may execute the command */
	int skill;

	int (*start)(struct command *);		/* initiator */
	int (*finish)(struct command *);	/* conclusion */
	int (*interrupt)(struct command *);	/* interrupted order */

	int time;		/* how long command takes */
	int poll;		/* call finish each day, not just at end */
};

int v_sail(struct command *c), d_sail(struct command *c), i_sail(struct command *c);
int v_brew(struct command *c), d_brew_heal(struct command *c), d_brew_death(struct command *c), d_brew_slave(struct command *c);
int v_add_ram(struct command *c), d_add_ram(struct command *c);

int v_mine_iron(struct command *c), d_mine_iron(struct command *c), v_mine_gold(struct command *c), d_mine_gold(struct command *c);
int v_mine_mithril(struct command *c), d_mine_mithril(struct command *c), v_quarry(struct command *c);
int v_wood(struct command *c), v_yew(struct command *c), v_catch(struct command *c);
int v_spy_inv(struct command *c), d_spy_inv(struct command *c), v_spy_lord(struct command *c), d_spy_lord(struct command *c);
int v_spy_skills(struct command *c), d_spy_skills(struct command *c);
int v_adv_med(struct command *c), d_adv_med(struct command *c), v_hinder_med(struct command *c), d_hinder_med(struct command *c);

int v_defense(struct command *c), d_defense(struct command *c);
int v_archery(struct command *c), d_archery(struct command *c);
int v_swordplay(struct command *c), d_swordplay(struct command *c);

int v_detect_gates(struct command *c), d_detect_gates(struct command *c), v_jump_gate(struct command *c), v_teleport(struct command *c);
int v_reverse_jump(struct command *c), v_reveal_key(struct command *c), d_reveal_key(struct command *c);
int v_seal_gate(struct command *c), d_seal_gate(struct command *c), v_unseal_gate(struct command *c), d_unseal_gate(struct command *c);
int v_notify_unseal(struct command *c), d_notify_unseal(struct command *c), v_rem_seal(struct command *c), d_rem_seal(struct command *c);
int v_notify_jump(struct command *c), d_notify_jump(struct command *c), v_meditate(struct command *c), d_meditate(struct command *c);
int v_heal(struct command *c), d_heal(struct command *c), v_reveal_mage(struct command *c), d_reveal_mage(struct command *c), v_appear_common(struct command *c);
int v_view_aura(struct command *c), d_view_aura(struct command *c), v_shroud_abil(struct command *c), d_shroud_abil(struct command *c);
int v_detect_abil(struct command *c), d_detect_abil(struct command *c), v_detect_scry(struct command *c), d_detect_scry(struct command *c);
int v_scry_region(struct command *c), d_scry_region(struct command *c), v_shroud_region(struct command *c), d_shroud_region(struct command *c);
int v_dispel_region(struct command *c), d_dispel_region(struct command *c), v_dispel_abil(struct command *c), d_dispel_abil(struct command *c);
int v_proj_cast(struct command *c), d_proj_cast(struct command *c), v_locate_char(struct command *c), d_locate_char(struct command *c);
int v_bar_loc(struct command *c), d_bar_loc(struct command *c), v_unbar_loc(struct command *c), d_unbar_loc(struct command *c);
int v_forge_palantir(struct command *c), d_forge_palantir(struct command *c), v_destroy_art(struct command *c), d_destroy_art(struct command *c);
int v_show_art_creat(struct command *c), d_show_art_creat(struct command *c), v_show_art_reg(struct command *c), d_show_art_reg(struct command *c);
int v_save_proj(struct command *c), d_save_proj(struct command *c), v_save_quick(struct command *c), d_save_quick(struct command *c);
int v_quick_cast(struct command *c), d_quick_cast(struct command *c), v_rem_art_cloak(struct command *c), d_rem_art_cloak(struct command *c);
int v_write_spell(struct command *c), d_write_spell(struct command *c), v_curse_noncreat(struct command *c), d_curse_noncreat(struct command *c);
int v_cloak_creat(struct command *c), d_cloak_creat(struct command *c), v_cloak_reg(struct command *c), d_cloak_reg(struct command *c);
int v_forge_aura(struct command *c), d_forge_aura(struct command *c), v_bribe(struct command *c), d_bribe(struct command *c);
int v_shipbuild(struct command *c), v_summon_savage(struct command *c), v_keep_savage(struct command *c), d_keep_savage(struct command *c);
int v_improve_opium(struct command *c), d_improve_opium(struct command *c), v_lead_to_gold(struct command *c), d_lead_to_gold(struct command *c);
int v_raise(struct command *c), d_raise(struct command *c), v_rally(struct command *c), d_rally(struct command *c), v_incite(struct command *c), d_incite(struct command *c);
int v_bird_spy(struct command *c), d_bird_spy(struct command *c), v_eat_dead(struct command *c), d_eat_dead(struct command *c);
int v_raise_corpses(struct command *c), v_undead_lord(struct command *c), d_undead_lord(struct command *c);
int v_banish_undead(struct command *c), d_banish_undead(struct command *c), v_keep_undead(struct command *c), d_keep_undead(struct command *c);
int v_aura_blast(struct command *c), d_aura_blast(struct command *c), v_aura_reflect(struct command *c), v_banish_corpses(struct command *c), d_banish_corpses(struct command *c);
int v_summon_rain(struct command *c), d_summon_rain(struct command *c), v_summon_wind(struct command *c), d_summon_wind(struct command *c);
int v_summon_fog(struct command *c), d_summon_fog(struct command *c), v_dissipate(struct command *c), d_dissipate(struct command *c);
int v_direct_storm(struct command *c), v_renew_storm(struct command *c), d_renew_storm(struct command *c);
int v_lightning(struct command *c), d_lightning(struct command *c);
int v_seize_storm(struct command *c), d_seize_storm(struct command *c), v_death_fog(struct command *c), d_death_fog(struct command *c);
int v_hide(struct command *c), d_hide(struct command *c), v_sneak(struct command *c), d_sneak(struct command *c), v_fierce_wind(struct command *c), d_fierce_wind(struct command *c);
int v_mage_menial(struct command *c), v_reveal_vision(struct command *c), d_reveal_vision(struct command *c);
int v_resurrect(struct command *c), d_resurrect(struct command *c), v_prep_ritual(struct command *c), d_prep_ritual(struct command *c);
int v_last_rites(struct command *c), d_last_rites(struct command *c), v_remove_bless(struct command *c), d_remove_bless(struct command *c);
int v_vision_protect(struct command *c), d_vision_protect(struct command *c);
int v_find_rich(struct command *c), d_find_rich(struct command *c), v_torture(struct command *c), d_torture(struct command *c);
int v_fight_to_death(struct command *c), v_breed(struct command *c), d_breed(struct command *c), v_fish(struct command *c);
int v_petty_thief(struct command *c), d_petty_thief(struct command *c), v_persuade_oath(struct command *c), d_persuade_oath(struct command *c);
int v_forge_art_x(struct command *c), d_forge_art_x(struct command *c), v_trance(struct command *c), d_trance(struct command *c);
int v_teleport_item(struct command *c), d_teleport_item(struct command *c), v_tap_health(struct command *c), d_tap_health(struct command *c);
int v_bind_storm(struct command *c), d_bind_storm(struct command *c);
int v_use_train_riding(struct command *c), v_use_train_war(struct command *c);
int v_breed_hound(struct command *c), d_breed_hound(struct command *c);
int v_find_buy(struct command *c), d_find_buy(struct command *c), v_find_sell(struct command *c), d_find_sell(struct command *c);

int v_implicit(struct command *c);

struct use_tbl_ent use_tbl[] = {
{NULL, 0,                0,                0,                0,        0, 0},

/*
allow  skill             start             finish            intr    time poll
 */

{"c", sk_meditate,       v_meditate,       d_meditate,       NULL,     7, 0},
{"c", sk_detect_gates,   v_detect_gates,   d_detect_gates,   NULL,     7, 0},
{"c", sk_jump_gate,      v_jump_gate,      NULL,             NULL,     1, 0},
{"c", sk_teleport,       v_teleport,       NULL,             NULL,     1, 0},
{"c", sk_seal_gate,      v_seal_gate,      d_seal_gate,      NULL,     7, 0},
{"c", sk_unseal_gate,    v_unseal_gate,    d_unseal_gate,    NULL,     7, 0},
{"c", sk_notify_unseal,  v_notify_unseal,  d_notify_unseal,  NULL,     7, 0},
{"c", sk_rem_seal,       v_rem_seal,       d_rem_seal,       NULL,     7, 0},
{"c", sk_reveal_key,     v_reveal_key,     d_reveal_key,     NULL,     7, 0},
{"c", sk_notify_jump,    v_notify_jump,    d_notify_jump,    NULL,     7, 0},
{"c", sk_heal,           v_heal,           d_heal,           NULL,     7, 0},
{"c", sk_rev_jump,       v_reverse_jump,   NULL,             NULL,     1, 0},
{"c", sk_reveal_mage,    v_reveal_mage,    d_reveal_mage,    NULL,     7, 0},
{"c", sk_view_aura,      v_view_aura,      d_view_aura,      NULL,     7, 0},
{"c", sk_shroud_abil,    v_shroud_abil,    d_shroud_abil,    NULL,     3, 0},
{"c", sk_detect_abil,    v_detect_abil,    d_detect_abil,    NULL,     7, 0},
{"c", sk_scry_region,    v_scry_region,    d_scry_region,    NULL,     7, 0},
{"c", sk_shroud_region,  v_shroud_region,  d_shroud_region,  NULL,     3, 0},
{"c", sk_detect_scry,    v_detect_scry,    d_detect_scry,    NULL,     7, 0},
{"c", sk_dispel_region,  v_dispel_region,  d_dispel_region,  NULL,     3, 0},
{"c", sk_dispel_abil,    v_dispel_abil,    d_dispel_abil,    NULL,     3, 0},
{"c", sk_adv_med,        v_adv_med,        d_adv_med,        NULL,     7, 0},
{"c", sk_hinder_med,     v_hinder_med,     d_hinder_med,     NULL,    10, 0},
{"c", sk_proj_cast,      v_proj_cast,      d_proj_cast,      NULL,     7, 0},
{"c", sk_locate_char,    v_locate_char,    d_locate_char,    NULL,    10, 0},
{"c", sk_bar_loc,        v_bar_loc,        d_bar_loc,        NULL,    10, 0},
{"c", sk_unbar_loc,      v_unbar_loc,      d_unbar_loc,      NULL,     7, 0},
{"c", sk_forge_palantir, v_forge_palantir, d_forge_palantir, NULL,    10, 0},
{"c", sk_destroy_art,    v_destroy_art,    d_destroy_art,    NULL,     7, 0},
{"c", sk_show_art_creat, v_show_art_creat, d_show_art_creat, NULL,     7, 0},
{"c", sk_show_art_reg,   v_show_art_reg,   d_show_art_reg,   NULL,     7, 0},
{"c", sk_save_proj,      v_save_proj,      d_save_proj,      NULL,     7, 0},
{"c", sk_save_quick,     v_save_quick,     d_save_quick,     NULL,     7, 0},
{"c", sk_quick_cast,     v_quick_cast,     d_quick_cast,     NULL,     4, 0},
{"c", sk_rem_art_cloak,  v_rem_art_cloak,  d_rem_art_cloak,  NULL,    10, 0},
{"c", sk_write_basic,    v_write_spell,    d_write_spell,    NULL,     7, 0},
{"c", sk_write_weather,  v_write_spell,    d_write_spell,    NULL,     7, 0},
{"c", sk_write_scry,     v_write_spell,    d_write_spell,    NULL,     7, 0},
{"c", sk_write_gate,     v_write_spell,    d_write_spell,    NULL,     7, 0},
{"c", sk_write_art,      v_write_spell,    d_write_spell,    NULL,     7, 0},
{"c", sk_write_necro,    v_write_spell,    d_write_spell,    NULL,     7, 0},
{"c", sk_cloak_creat,    v_cloak_creat,    d_cloak_creat,    NULL,     7, 0},
{"c", sk_cloak_reg,      v_cloak_reg,      d_cloak_reg,      NULL,     7, 0},
{"c", sk_curse_noncreat, v_curse_noncreat, d_curse_noncreat, NULL,    14, 0},
{"c", sk_forge_aura,     v_forge_aura,     d_forge_aura,     NULL,    14, 0},
{"c", sk_shipbuilding,   v_shipbuild,      NULL,             NULL,     0, 0},
{"c", sk_pilot_ship,     v_sail,           d_sail,           i_sail,  -1, 0},
{"c", sk_train_wild,     v_use_train_riding, NULL,           NULL,     7, 0},
{"c", sk_train_warmount, v_use_train_war,  NULL,             NULL,    14, 0},
{"c", sk_make_ram,       NULL,             NULL,             NULL,    14, 0},
{"c", sk_make_catapult,  NULL,             NULL,             NULL,    14, 0},
{"c", sk_make_siege,     NULL,             NULL,             NULL,    14, 0},
{"c", sk_brew_slave,     v_brew,           d_brew_slave,     NULL,     7, 0},
{"c", sk_brew_heal,      v_brew,           d_brew_heal,      NULL,     7, 0},
{"c", sk_brew_death,     v_brew,           d_brew_death,     NULL,    10, 0},
{"c", sk_mine_iron,      v_mine_iron,      d_mine_iron,      NULL,     7, 0},
{"c", sk_mine_gold,      v_mine_gold,      d_mine_gold,      NULL,     7, 0},
{"c", sk_mine_mithril,   v_mine_mithril,   d_mine_mithril,   NULL,     7, 0},
{"c", sk_quarry_stone,   v_quarry,         NULL,             NULL,    -1, 1},
{"c", sk_catch_horse,    v_catch,          NULL,             NULL,    -1, 1},
{"c", sk_extract_venom,  NULL,             NULL,             NULL,     7, 0},
{"c", sk_harvest_lumber, v_wood,           NULL,             NULL,    -1, 1},
{"c", sk_harvest_yew,    v_yew,            NULL,             NULL,    -1, 1},
{"c", sk_add_ram,        v_add_ram,        d_add_ram,        NULL,    10, 0},
{"c", sk_spy_inv,        v_spy_inv,        d_spy_inv,        NULL,     7, 0},
{"c", sk_spy_skills,     v_spy_skills,     d_spy_skills,     NULL,     7, 0},
{"c", sk_spy_lord,       v_spy_lord,       d_spy_lord,       NULL,     7, 0},
{"c", sk_record_skill,   v_write_spell,    d_write_spell,    NULL,     7, 0},
{"c", sk_bribe_noble,    v_bribe,          d_bribe,          NULL,     7, 0},
{"c", sk_summon_savage,  v_summon_savage,  NULL,             NULL,     1, 0},
{"c", sk_keep_savage,    v_keep_savage,    d_keep_savage,    NULL,     7, 0},
{"c", sk_improve_opium,  v_improve_opium,  d_improve_opium,  NULL,     7, 0},
{"c", sk_raise_mob,      v_raise,          d_raise,          NULL,     7, 0},
{"c", sk_rally_mob,      v_rally,          d_rally,          NULL,     7, 0},
{"c", sk_incite_mob,     v_incite,         d_incite,         NULL,     7, 0},
{"c", sk_bird_spy,       v_bird_spy,       d_bird_spy,       NULL,     3, 0},
{"c", sk_lead_to_gold,   v_lead_to_gold,   d_lead_to_gold,   NULL,     7, 0},
{"c", sk_raise_corpses,  v_raise_corpses,  NULL,             NULL,    -1, 1},
{"c", sk_undead_lord,    v_undead_lord,    d_undead_lord,    NULL,     7, 0},
{"c", sk_banish_undead,  v_banish_undead,  d_banish_undead,  NULL,     7, 0},
{"c", sk_renew_undead,   v_keep_undead,    d_keep_undead,    NULL,     7, 0},
{"c", sk_eat_dead,       v_eat_dead,       d_eat_dead,       NULL,    14, 0},
{"c", sk_aura_blast,     v_aura_blast,     d_aura_blast,     NULL,     1, 0},
{"c", sk_absorb_blast,   v_aura_reflect,   NULL,             NULL,     0, 0},
{"c", sk_summon_rain,    v_summon_rain,    d_summon_rain,    NULL,     7, 0},
{"c", sk_summon_wind,    v_summon_wind,    d_summon_wind,    NULL,     7, 0},
{"c", sk_summon_fog,     v_summon_fog,     d_summon_fog,     NULL,     7, 0},
{"c", sk_direct_storm,   v_direct_storm,   NULL,             NULL,     1, 0},
{"c", sk_renew_storm,    v_renew_storm,    d_renew_storm,    NULL,     3, 0},
{"c", sk_dissipate,      v_dissipate,      d_dissipate,      NULL,     7, 0},
{"c", sk_lightning,      v_lightning,      d_lightning,      NULL,     7, 0},
{"c", sk_fierce_wind,    v_fierce_wind,    d_fierce_wind,    NULL,     7, 0},
{"c", sk_seize_storm,    v_seize_storm,    d_seize_storm,    NULL,     7, 0},
{"c", sk_death_fog,      v_death_fog,      d_death_fog,      NULL,     7, 0},
{"c", sk_banish_corpses, v_banish_corpses, d_banish_corpses, NULL,     7, 0},
{"c", sk_hide_self,      v_hide,           d_hide,           NULL,     3, 0},
{"c", sk_sneak_build,    v_sneak,          d_sneak,          NULL,     3, 0},
{"c", sk_mage_menial,    v_mage_menial,    NULL,             NULL,    -1, 1},
{"c", sk_petty_thief,    v_petty_thief,    d_petty_thief,    NULL,     7, 0},
{"c", sk_appear_common,  v_appear_common,  NULL,             NULL,     1, 0},
{"c", sk_defense,        v_defense,        d_defense,        NULL,     7, 0},
{"c", sk_archery,        v_archery,        d_archery,        NULL,     7, 0},
{"c", sk_swordplay,      v_swordplay,      d_swordplay,      NULL,     7, 0},
{"c", sk_reveal_vision,  v_reveal_vision,  d_reveal_vision,  NULL,    10, 0},
{"c", sk_resurrect,      v_resurrect,      d_resurrect,      NULL,    10, 0},
{"c", sk_pray,           v_prep_ritual,    d_prep_ritual,    NULL,     3, 0},
{"c", sk_last_rites,     v_last_rites,     d_last_rites,     NULL,    10, 0},
{"c", sk_remove_bless,   v_remove_bless,   d_remove_bless,   NULL,    10, 0},
{"c", sk_vision_protect, v_vision_protect, d_vision_protect, NULL,    10, 0},
{"c", sk_find_rich,      v_find_rich,      d_find_rich,      NULL,     7, 0},
{"c", sk_harvest_opium,  v_implicit,       NULL,             NULL,     0, 0},
{"c", sk_train_angry,    v_implicit,       NULL,             NULL,     0, 0},
{"c", sk_weaponsmith,    v_implicit,       NULL,             NULL,     0, 0},
{"c", sk_hide_lord,      v_implicit,       NULL,             NULL,     0, 0},
{"c", sk_transcend_death,v_implicit,       NULL,             NULL,     0, 0},
{"c", sk_collect_foliage,v_implicit,       NULL,             NULL,     0, 0},
{"c", sk_fishing,        v_fish,           NULL,             NULL,     0, 0},
{"c", sk_summon_ghost,   v_implicit,       NULL,             NULL,     0, 0},
{"c", sk_capture_beasts, v_implicit,       NULL,             NULL,     0, 0},
{"c", sk_use_beasts,     v_implicit,       NULL,             NULL,     0, 0},
{"c", sk_collect_elem,   v_implicit,       NULL,             NULL,     0, 0},
{"c", sk_torture,        v_torture,        d_torture,        NULL,     7, 0},
{"c", sk_fight_to_death, v_fight_to_death, NULL,             NULL,     0, 0},
{"c", sk_breed_beasts,   v_breed,          d_breed,          NULL,     7, 0},
{"c", sk_breed_hound,    v_breed_hound,    d_breed_hound,    NULL,    28, 0},
{"c", sk_persuade_oath,  v_persuade_oath,  d_persuade_oath,  NULL,     7, 0},
{"c", sk_forge_weapon,   v_forge_art_x,    d_forge_art_x,    NULL,     7, 0},
{"c", sk_forge_armor,    v_forge_art_x,    d_forge_art_x,    NULL,     7, 0},
{"c", sk_forge_bow,      v_forge_art_x,    d_forge_art_x,    NULL,     7, 0},
{"c", sk_trance,         v_trance,         d_trance,         NULL,    28, 0},
{"c", sk_teleport_item,  v_teleport_item,  d_teleport_item,  NULL,     3, 0},
{"c", sk_tap_health,     v_tap_health,     d_tap_health,     NULL,     7, 0},
{"c", sk_bind_storm,     v_bind_storm,     d_bind_storm,     NULL,     7, 0},
{"c", sk_find_sell,      v_find_sell,      d_find_sell,      NULL,    21, 0},
{"c", sk_find_buy,       v_find_buy,       d_find_buy,       NULL,    14, 0},

{NULL, 0,                0,                0,                0,        0, 0}

};


int
v_implicit(struct command *c)
{

	wout(c->who, "Use of this skill is automatic when appropriate.");
	wout(c->who, "No direct USE function exists.");
	return FALSE;
}


int
v_shipbuild(struct command *c)
{

	wout(c->who, "Use the BUILD order to build ships.");
	return FALSE;
}


static int
find_use_entry(int skill)
{
	int i;

	for (i = 1; use_tbl[i].skill; i++)
		if (use_tbl[i].skill == skill)
			return i;

	return -1;
}


/*
 *  If we know the skill, return the skill number
 *
 *  If an artifact grants us the ability to use the skill,
 *  return the item number of the artifact.
 *
 *  If a one-shot scroll lets us use the skill, return the
 *  item number of the scroll.
 */

static int
may_use_skill(int who, int sk)
{
	struct item_ent *e;
	struct item_magic *p;
	int ret = 0;
	int scroll = 0;

	if (has_skill(who, sk) > 0)
		return sk;

/*
 *  Items other than scrolls should take precedence, to preserve
 *  the one-shot scrolls.
 */

	loop_inv(who, e)
	{
		p = rp_item_magic(e->item);
		if (p && ilist_lookup(p->may_use, sk) >= 0)
		{
			if (subkind(e->item) == sub_scroll)
				scroll = e->item;
			else
				ret = e->item;
		}
	}
	next_inv;

	if (ret)
		return ret;

	if (scroll)
		return scroll;

	return 0;
}


static void
magically_speed_casting(struct command *c, int sk)
{
	struct char_magic *p;
	int n;		/* amount speeded by */

	if (magic_skill(sk) &&
	    char_quick_cast(c->who) &&
	    sk != sk_save_quick &&
	    sk != sk_trance)
	{
		p = p_magic(c->who);

		if (c->wait == 0)
		{
			n = 0;			/* don't do anything */
		}
		else if (p->quick_cast < c->wait)
		{
			n = p->quick_cast;

			c->wait -= p->quick_cast;
			p->quick_cast = 0;
		}
		else
		{
			n = c->wait - 1;
			p->quick_cast = 0;
			c->wait = 1;
		}

		wout(c->who, "(speeded cast by %d day%s)", n, add_s(n));
	}
}


/*
 *  To use a spell through a scroll, the character should USE the
 *  spell number, and not USE the scroll.  If they use the scroll,
 *  guess which spell they meant to use from within the scroll,
 *  and use it.
 */

static int
correct_use_item(struct command *c)
{
	int item = c->a;
	struct item_magic *p;

	if (item_use_key(item))
		return item;

	p = rp_item_magic(item);

	if (p == NULL || ilist_len(p->may_use) < 1)
		return item;

	c->a = p->may_use[0];
	return c->a;
}


static int
meets_requirements(int who, int skill)
{
	int i;
	struct entity_skill *p;
	struct req_ent **l;

	p = rp_skill(skill);
	if (p == NULL)
		return TRUE;

	l = p->req;

	i = 0;
	while (i < ilist_len(l))
	{
		while (has_item(who, l[i]->item) < l[i]->qty &&
		       l[i]->consume == REQ_OR)
		{
			i++;

			if (i >= ilist_len(l))
			{
				fprintf(stderr, "skill = %d\n", skill);
				assert(FALSE);
			}
/*
 *  If this assertion fails then a req list ended
 *  with a REQ_OR instead of a REQ_YES or a REQ_NO
 */
		}

		if (has_item(who, l[i]->item) < l[i]->qty)
		{
			wout(who, "%s does not have %s.", just_name(who),
					box_name_qty(l[i]->item, l[i]->qty));
			return FALSE;
		}

		while (l[i]->consume == REQ_OR && i < ilist_len(l))
			i++;
		i++;
	}

	return TRUE;
}


static void
consume_requirements(int who, int skill)
{
	int i;
	struct entity_skill *p;
	struct req_ent **l;
	int ret;
	int item, qty;

	p = rp_skill(skill);
	if (p == NULL)
		return;

	l = p->req;

	i = 0;
	while (i < ilist_len(l))
	{
		while (has_item(who, l[i]->item) < l[i]->qty &&
		       l[i]->consume == REQ_OR)
		{
			i++;

			if (i >= ilist_len(l))
			{
				fprintf(stderr, "req list ends with REQ_OR, "
						"skill = %d\n", skill);
				assert(FALSE);
			}
/*
 *  If this assertion fails then a req list ended
 *  with a REQ_OR instead of a REQ_YES or a REQ_NO
 */
		}

		item = l[i]->item;
		qty = l[i]->qty;

		while (l[i]->consume == REQ_OR && i < ilist_len(l))
			i++;

		if (l[i]->consume == REQ_YES)
			ret = consume_item(who, item, qty);

		i++;
	}
}


static void
consume_scroll(int who, int basis)
{

/*
 *  Use or study of a one-shot scrolls causes the scroll
 *  to vanish.
 */

	if (subkind(basis) == sub_scroll)
	{
		wout(who, "%s vanishes.", box_name(basis));
		destroy_unique_item(who, basis);
	}
}


void
experience_use_speedup(struct command *c)
{
	int exp;

	exp = max(c->use_exp - 1, 0);
/*
 *  Shorten use of some skills based on experience
 */

	if (exp && c->wait >= 7)
	{
		if (c->wait >= 14)
			c->wait -= exp;
		else if (c->wait >= 10)
			c->wait -= exp / 2;
		else if (exp >= 2)
			c->wait--;
	}
}


int
v_use(struct command *c)
{
	int sk = c->a;
	int ent;
	int n;
	int basis;		/* what our skill ability is based upon */
	int parent;

	c->use_skill = sk;

	if (!valid_box(sk))
	{
		wout(c->who, "%s is not a valid skill to use.", c->parse[1]);
		return FALSE;
	}

	if (kind(sk) == T_item)
		sk = correct_use_item(c);

	if (kind(sk) == T_item)
		return v_use_item(c);

	if (kind(sk) != T_skill)
	{
		wout(c->who, "%s is not a valid skill to use.", c->parse[1]);
		return FALSE;
	}

	parent = skill_school(sk);

	if (parent == sk)
	{
		wout(c->who, "Skill schools have no direct use.  "
			"Only subskills within a school may be used.");
		return FALSE;
	}

	basis = may_use_skill(c->who, sk);

	if (!basis)
	{
		wout(c->who, "%s does not know %s.",
					just_name(c->who), box_code(sk));
		return FALSE;
	}

/*
 *  If this was (has_skill(c->who, parent) < 1) then a category skill 
 *  couldn't be used from an item.
 */

	if (!may_use_skill(c->who, parent))
	{
		wout(c->who, "Knowledge of %s is first required before "
				"%s may be used.",
					box_name(parent),
					box_code(sk));
		return FALSE;
	}

	ent = find_use_entry(sk);

	if (ent <= 0)
	{
		fprintf(stderr, "v_use: no use table entry for %s\n",
					c->parse[1]);
		out(c->who, "Internal error.");
		return FALSE;
	}

	if (magic_skill(sk) && in_safe_now(c->who))
	{
		wout(c->who, "Magic may not be used in safe havens.");
		return FALSE;
	}

	cmd_shift(c);
	c->use_ent = ent;
	c->use_skill = sk;
	c->use_exp = has_skill(c->who, sk);
	c->poll = use_tbl[ent].poll;
	c->wait = use_tbl[ent].time;
	c->h = basis;

	experience_use_speedup(c);

	if (!meets_requirements(c->who, sk))
		return FALSE;

	if (use_tbl[ent].start != NULL)
	{
		int ret;

		ret = (*use_tbl[ent].start)(c);

		if (ret)
			magically_speed_casting(c, sk);

		return ret;
	}

/*
 *  Perhaps an assertion here that we are indeed a production
 *  skill use, to catch skills without implementations
 */

	if (n = skill_produce(sk))
	{
		wout(c->who, "Work to produce one %s.", just_name(n));
	}
	else
	{
		fprintf(stderr, "sk == %s\n", box_name(sk));
		assert(FALSE);
	}

	return TRUE;
}


/*
 *  Increment the experience counts for a skill and its parent
 */

void
add_skill_experience(int who, int sk)
{
	struct skill_ent *p;

	p = rp_skill_ent(who, sk);

/*
 *  Don't increase the experience if we don't actually know the
 *  skill.  For instance, use through a scroll or book should
 *  not add experience to the character, unless the character
 *  knows the skill himself.
 */

	if (p == NULL)
		return;

	if (p->exp_this_month == FALSE)
	{
		p->experience++;
		p->exp_this_month = TRUE;
	}
}


int
d_use(struct command *c)
{
	int sk = c->use_skill;
	int ent = c->use_ent;
	int n;
	int basis = c->h;

	if (kind(sk) == T_item)
		return d_use_item(c);

/*
 *  c->use_ent is not saved
 *  if it is zero here, look it up again
 *  This is so that it will be re-looked-up across turn boundaries
 */

	if (ent <= 0)
		ent = find_use_entry(sk);

	if (ent <= 0)
	{
		fprintf(stderr, "d_use: no use table entry for %s\n",
					c->parse[1]);
		out(c->who, "Internal error.");
		return FALSE;
	}

/*
 *  Don't call poll routine for ordinary delays
 */

	if (c->wait > 0 && c->poll == 0)
		return TRUE;

	if (c->poll == 0 && !meets_requirements(c->who, sk))
		return FALSE;

/*
 *  Maintain count of how many times each skill is used during
 *  a turn, for informational purposes only.
 */

	if (sk != sk_breed_beasts)		/* taken care of in d_breed */
		p_skill(sk)->use_count++;
	p_skill(sk)->last_use_who = c->who;

	if (use_tbl[ent].finish != NULL)
	{
		int ret;

		ret = (*use_tbl[ent].finish)(c);

		if (c->wait == 0 && ret)
			add_skill_experience(c->who, sk);

		if (ret)
		{
			consume_scroll(c->who, basis);
			consume_requirements(c->who, sk);
		}

		return ret;
	}

	add_skill_experience(c->who, sk);

	if (n = skill_produce(sk))
	{
		gen_item(c->who, n, 1);
		wout(c->who, "Produced one %s.", box_name(n));
	}

	consume_scroll(c->who, basis);
	consume_requirements(c->who, sk);

	return TRUE;
}


int
i_use(struct command *c)
{
	int sk = c->use_skill;
	int ent = c->use_ent;

	if (ent < 0)
	{
		out(c->who, "Internal error.");
		fprintf(stderr, "i_use: c->use_ent is %d\n", c->use_ent);
		return FALSE;
	}

	if (use_tbl[ent].interrupt != NULL)
		return (*use_tbl[ent].interrupt)(c);

	return FALSE;
}


int
v_use_item(struct command *c)
{
	int item = c->a;
	int n;
	int ret;

	c->poll = FALSE;
	c->wait = 0;

	if (has_item(c->who, item) < 1)
	{
		wout(c->who, "%s has no %s.", just_name(c->who),
						box_code(item));
		return FALSE;
	}

	n = item_use_key(item);

	if (n == 0)
	{
		wout(c->who, "Nothing special happens.");
		return FALSE;
	}

#if 0
	cmd_shift(c);
#endif

/*
 *  If they use a magical object, and we're in a safe
 *  haven, don't allow it.
 */

	switch (n)
	{
	case use_palantir:
	case use_proj_cast:
	case use_quick_cast:
	case use_orb:
	case use_barbarian_kill:
	case use_savage_kill:
	case use_corpse_kill:
	case use_orc_kill:
	case use_skeleton_kill:

		if (in_safe_now(c->who))
		{
			wout(c->who, "Magic may not be used in safe havens.");
			c->wait = 0;
			c->inhibit_finish = TRUE;
			return FALSE;
		}
	}

	switch (n)
	{
	case use_heal_potion:
		ret = v_use_heal(c);
		break;

	case use_slave_potion:
		ret = v_use_slave(c);
		break;

	case use_death_potion:
		ret = v_use_death(c);
		break;

	case use_palantir:
		ret = v_use_palantir(c);
		break;

	case use_proj_cast:
		ret = v_use_proj_cast(c);
		break;

	case use_quick_cast:
		ret = v_use_quick_cast(c);
		break;

	case use_drum:
		ret = v_use_drum(c);
		break;

	case use_faery_stone:
		ret = v_use_faery_stone(c);
		break;

	case use_orb:
		ret = v_use_orb(c);
		break;

	case use_barbarian_kill:
		ret = v_suffuse_ring(c, item_barbarian);
		break;

	case use_savage_kill:
		ret = v_suffuse_ring(c, item_savage);
		break;

	case use_corpse_kill:
		ret = v_suffuse_ring(c, item_corpse);
		break;

	case use_orc_kill:
		ret = v_suffuse_ring(c, item_orc);
		break;

	case use_skeleton_kill:
		ret = v_suffuse_ring(c, item_skeleton);
		break;

	case use_bta_skull:
		ret = v_use_bta_skull(c);
		break;

	default:
		fprintf(stderr, "bad use key: %d\n", n);
		assert(FALSE);
	}

	if (ret != TRUE || c->wait == 0)
	{
		c->wait = 0;
		c->inhibit_finish = TRUE;
	}

	return ret;
}


int
d_use_item(struct command *c)
{
	int item = c->a;
	int n;

	if (has_item(c->who, item) < 1)
	{
		wout(c->who, "%s no longer has %s.",
				just_name(c->who), box_code(item));
		return FALSE;
	}

	n = item_use_key(item);

	if (n == 0)
	{
		wout(c->who, "Nothing special happens.");
		return FALSE;
	}

	switch (n)
	{
	case use_palantir:		return d_use_palantir(c);

	default:
		fprintf(stderr, "bad use key: %d\n", n);
		assert(FALSE);
	}
 
	return TRUE;
}


static int
exp_level(int exp)
{

	if (exp <= 4)
		return exp_novice;

	if (exp <= 11)
		return exp_journeyman;

	if (exp <= 20)
		return exp_teacher;

	if (exp <= 34)
		return exp_master;

	return exp_grand;
}


char *
exp_s(int level)
{

	switch (level)
	{
	case exp_novice:	return "apprentice";
	case exp_journeyman:	return "journeyman";
	case exp_teacher:	return "adept";
	case exp_master:	return "master";
	case exp_grand:		return "grand master";

	default:
		assert(FALSE);
	}

	return 0;
}


struct skill_ent *
rp_skill_ent(int who, int skill)
{
	struct entity_char *p;
	int i;

	p = rp_char(who);

	if (p == NULL)
		return NULL;

	for (i = 0; i < ilist_len(p->skills); i++)
		if (p->skills[i]->skill == skill)
			return p->skills[i];

	return NULL;
}


struct skill_ent *
p_skill_ent(int who, int skill)
{
	struct entity_char *p;
	struct skill_ent *new;
	int i;

	p = p_char(who);

	for (i = 0; i < ilist_len(p->skills); i++)
		if (p->skills[i]->skill == skill)
			return p->skills[i];

	new = my_malloc(sizeof(*new));
	new->skill = skill;

	ilist_append((ilist *) &p->skills, (int) new);

	return new;
}


int
forget_skill(int who, int skill)
{
	struct entity_char *p;
	struct skill_ent *t;
	struct char_magic *ch;

	p = rp_char(who);
	if (p == NULL)
		return FALSE;

	t = rp_skill_ent(who, skill);

	if (t == NULL)
		return FALSE;

	ilist_rem_value((ilist *) &p->skills, (int) t);

	if (magic_skill(skill))
	{
		ch = p_magic(who);
		ch->max_aura--;
		if (ch->max_aura < 0)
			ch->max_aura = 0;

	if (ch->cur_aura > ch->max_aura)
			ch->cur_aura--;

	}

	return TRUE;
}


int
v_forget(struct command *c)
{
	int skill = c->a;
	int i;
	int sum = 0;
	int pl;
	int teller;

	/* FORGET BUG FIX */
	// Not allowed to forget magic skills when the noble has an auraculum
	// Exception: Transcend Death
	if (has_auraculum(c->who) && magic_skill(skill) && skill != sk_transcend_death )
		return FALSE;
	/* FORGET BUG FIX */

	if (kind(skill) != T_skill)
	{
		wout(c->who, "%s is not a skill.", box_code(skill));
		return FALSE;
	}

	sum = skill_np_req(skill);

	if (!forget_skill(c->who, skill))
	{
		wout(c->who, "Don't know any %s.", box_code(skill));
		return FALSE;
	}

	wout(c->who, "Forgot all knowledge of %s.", box_code(skill));

	if (skill_school(skill) == skill)
	{
	    loop_skill(i)
	    {
		if (skill_school(i) == skill && p_skill_ent(c->who, i))
		{
			forget_skill(c->who, i);
			wout(c->who, "Forgot %s.", box_code(i));
		}
	    }
	    next_skill;
	}

	pl = player(c->who);
	add_np(pl,sum);
	wout(c->who, "Refunded %d noble point%s.",
		sum, add_s(sum));

	return TRUE;
}


int
has_skill(int who, int skill)
{
	struct skill_ent *p;

	p = rp_skill_ent(who, skill);

	if (p == NULL || p->know != SKILL_know)
		return 0;

	return exp_level(p->experience);
}


/*
 *  Use learn_skill() to grant a character a skill
 */

void
set_skill(int who, int skill, int know)
{
	struct skill_ent *p;

	p = p_skill_ent(who, skill);

	p->know = know;
}


int
skill_school(int sk)
{
	int n;
	int count = 0;

	while (1)
	{
		assert(count++ < 1000);

		n = req_skill(sk);
		if (n == 0)
			return sk;
		sk = n;
	}
}


/*
 *  Order skills for display
 *
 *  Subskills follow their parent
 *  Skills we don't know are pushed to the end
 */

static int
rep_skill_comp(a, b)
struct skill_ent **a;
struct skill_ent **b;
{
	int pa;		/* parent skill of a */
	int pb;		/* parent skill of b */

#if 1
	if ((*a)->know != SKILL_know && ((*b)->know == SKILL_know))
		return -1;

	if ((*b)->know != SKILL_know && ((*a)->know == SKILL_know))
		return 1;
#else
	if ((*a)->know == 0)
		return (*a)->level - (*b)->level;

	if ((*b)->level == 0)
		return (*b)->level - (*a)->level;
#endif

	pa = skill_school((*a)->skill);
	pb = skill_school((*b)->skill);

	if (pa != pb)
		return pa - pb;

	return (*a)->skill - (*b)->skill;
}


static int
flat_skill_comp(a, b)
struct skill_ent **a;
struct skill_ent **b;
{

	return (*a)->skill - (*b)->skill;
}


#if 0
void
list_skill_sup(int who, struct skill_ent *e)
{
	char *exp;

	if (skill_no_exp(e->skill) || skill_school(e->skill) == e->skill)
		wiout(who, CHAR_FIELD+2, "%*s  %s",
			CHAR_FIELD,
			box_code_less(e->skill),
			cap(just_name(e->skill)));
	else
		wiout(who, CHAR_FIELD+2, "%*s  %s, %s",
			CHAR_FIELD,
			box_code_less(e->skill),
			cap(just_name(e->skill)),
			exp_s(exp_level(e->experience)));
}

#else

void
list_skill_sup(int who, struct skill_ent *e)
{

	if (skill_no_exp(e->skill) || skill_school(e->skill) == e->skill)
		wout(who, "%s", box_name(e->skill));
	else
		wout(who, "%s, %s", box_name(e->skill),
					exp_s(exp_level(e->experience)));
}

#endif


void
list_skills(int who, int num)
{
	int i;
	struct skill_ent **l;
	int flag = TRUE;

	assert(valid_box(num));

	out(who, "");
	out(who, "Skills known:");
	indent += 3;

	if (rp_char(num) == NULL)
		goto list_skills_end;

	if (ilist_len(rp_char(num)->skills) < 1)
		goto list_skills_end;

	l = (struct skill_ent **) ilist_copy((ilist) rp_char(num)->skills);
	qsort(l, ilist_len(l), sizeof(int), rep_skill_comp);

	for (i = 0; i < ilist_len(l); i++)
	{
		if (l[i]->know != SKILL_know)
			continue;

		flag = FALSE;
#if 0
		if (i > 0 &&
		    skill_school(l[i]->skill) != skill_school(l[i-1]->skill))
			out(who, "");
#endif

		if (req_skill(l[i]->skill))
		{
			indent += 6;
			list_skill_sup(who, l[i]);
			indent -= 6;
		}
		else
		{
#if 0
			if (i > 0 && req_skill(l[i-1]->skill))
				out(who, "");
#endif

			list_skill_sup(who, l[i]);
		}
	}

	ilist_reclaim((ilist *) &l);

list_skills_end:

	if (flag)
		out(who, "none");

	indent -= 3;
}



/*
 *  Archery, 0/7
 *  Archery, 1/7
 *  Archery, 0/7, 1 NP req'd
 */

static char *
fractional_skill_qualifier(struct skill_ent *p)
{
	extern char *np_req_s();

	assert(p->know != SKILL_know);

	if (p->know == SKILL_dont)
	{
		return sout("0/%d%s",
				learn_time(p->skill),
				np_req_s(p->skill));
	}

	assert(p->know == SKILL_learning);

	return sout("%d/%d", p->days_studied, learn_time(p->skill));
}


void
list_partial_skills(int who, int num)
{
	int i;
	struct skill_ent **l;
	int flag = TRUE;

	assert(valid_box(num));

	if (rp_char(num) == NULL)
		return;

	if (ilist_len(rp_char(num)->skills) < 1)
		return;

	l = (struct skill_ent **) ilist_copy((ilist) rp_char(num)->skills);
	qsort(l, ilist_len(l), sizeof(int), flat_skill_comp);

	for (i = 0; i < ilist_len(l); i++)
	{
		if (l[i]->know == SKILL_know)
			continue;

		if (flag)
		{
			out(who, "");
			out(who, "Partially known skills:");
			indent += 3;
			flag = FALSE;
		}

		wiout(who, 6, "%s, %s",
				box_name(l[i]->skill),
				fractional_skill_qualifier(l[i]));
	}

	if (!flag)
	{
		indent -= 3;
	}

	ilist_reclaim((ilist *) &l);
}


int
skill_cost(int sk)
{

	return 100;
}



/*
 *  If it's taught by the location
 *  If it's offered by a skill that we know
 *  If it's offered by an item that we have
 */

static int
may_study(int who, int sk)
{
	int where = subloc(who);

/*
 *  Does the location offer the skill?
 */

	{
		struct entity_subloc *p;

		p = rp_subloc(where);

		if (p && ilist_lookup(p->teaches, sk) >= 0)
			return where;
	}

/*
 *  Is the skill offered by a skill that we already know?
 */

	{
		struct entity_skill *q;
		struct skill_ent *e;
		int ret = 0;

		loop_char_skill_known(who, e)
		{
			q = rp_skill(e->skill);

			if (q && ilist_lookup(q->offered, sk) >= 0)
			{
				ret = e->skill;
				break;
			}
		}
		next_char_skill_known;

		if (ret)
			return ret;
	}

/*
 *  Is instruction offered by a scroll or a book? 
 *
 *  Items other than scrolls should take precedence, to preserve
 *  the one-shot scrolls.
 */

	{
		struct item_ent *e;
		struct item_magic *p;
		int ret = 0;
		int scroll = 0;

		loop_inv(who, e)
		{
			p = rp_item_magic(e->item);
			if (p && ilist_lookup(p->may_study, sk) >= 0)
			{
				if (subkind(e->item) == sub_scroll)
					scroll = e->item;
				else
					ret = e->item;
			}
		}
		next_inv;

		if (ret)
			return ret;

		if (scroll)
			return scroll;
	}

	return 0;
}


static int
begin_study(struct command *c, int sk)
{
	int cost;
	struct skill_ent *p;
	int np_req;

	cost = skill_cost(sk);
	np_req = skill_np_req(sk);

	if (np_req > player_np(player(c->who)))
	{
		wout(c->who, "%s noble point%s %s required to begin "
			"study of %s.",
				cap(nice_num(np_req)),
				add_s(np_req),
				np_req == 1 ? "is" : "are",
				box_code(sk));
		return FALSE;
	}

	if (cost > 0)
	{
		if (!charge(c->who, cost))
		{
			wout(c->who, "Cannot afford %s to begin study.",
						gold_s(cost));
			return FALSE;
		}

		wout(c->who, "Paid %s to begin study.", gold_s(cost));
	}

	if (np_req > 0)
	{
		wout(c->who, "Deducted %s noble point%s to begin study.",
				nice_num(np_req), add_s(np_req));
		deduct_np(player(c->who), np_req);
	}

	p = p_skill_ent(c->who, sk);
	p->know = SKILL_learning;

	return TRUE;
}

/*
 *  To study through a scroll, the character should STUDY the
 *  skill number, not study the scroll.  If they study the scroll,
 *  guess which skill they meant to learn from within the scroll.
 */

static int
correct_study_item(struct command *c)
{
	int item = c->a;
	struct item_magic *p;

	p = rp_item_magic(item);

	if (p == NULL || ilist_len(p->may_study) < 1)
		return item;

	c->a = p->may_study[0];
	return c->a;
}


/*
 *	if we know it already, then error
 *	if we have already started studying, then continue
 *	if we may not study it, then error
 *
 *	if we don't have enough money, then error
 *
 *	start studying skill:
 *		init skill entry
 *		deduct money
 */

int
v_study(struct command *c)
{
	struct skill_ent *p;
	int sk = c->a;
	int fast = c->b;
	int parent;
	int basis = 0;

	if (char_studied(c->who) >= 14 && fast <= 0)
	{
		wout(c->who, "Maximum 14 days studied this month.");
		return FALSE;
	}

	if (kind(sk) == T_item)
		sk = correct_study_item(c);

	if (numargs(c) < 1)
	{
		wout(c->who, "Must specify a skill to study.");
		return FALSE;
	}

	if (kind(sk) != T_skill)
	{
		wout(c->who, "%s is not a valid skill.", c->parse[1]);
		return FALSE;
	}

	parent = skill_school(sk);

	if (parent != sk && has_skill(c->who, parent) < 1)
	{
		wout(c->who, "%s must be learned before %s can be known.",
				cap(box_name(parent)),
				box_code(sk));
		return FALSE;
	}

	p = rp_skill_ent(c->who, sk);

	if (p && p->know == SKILL_know)
	{
		wout(c->who, "Already know %s.", box_name(sk));
		return FALSE;
	}

	if (p == NULL && (basis = may_study(c->who, sk)) == 0)
	{
		wout(c->who, "Instruction in %s is not available here.",
						box_code(sk));
		return FALSE;
	}

/*
 *  Skill has never been studied
 */

	if (p == NULL || p->know == SKILL_dont)
	{
		if (!begin_study(c, sk))
			return FALSE;
	}

	if (fast > 0) {
		struct skill_ent *p;
		int required;
		int pl = player(c->who);

		if (fast > player_fast_study(pl))
			fast = player_fast_study(pl);

		p = p_skill_ent(c->who, sk);

		required = learn_time(sk) - p->days_studied;

		if (fast > required)
			fast = required;

		wout(c->who, "Using %d fast study day%s to accelerate "
			"learning of %s.", fast, add_s(fast), just_name(sk));

		p->days_studied += fast;
		p_player(pl)->fast_study -= fast;

		if (p->days_studied >= learn_time(sk))
			learn_skill(c->who, sk);

		c->wait = 0;
		c->inhibit_finish = TRUE;	/* don't call d_wait */

		if (basis && rnd(1,4) == 4)
			consume_scroll(c->who, basis);

		return TRUE;
	}

	wout(c->who, "Study %s for %s day%s.",
					just_name(sk),
					nice_num(c->wait), add_s(c->wait));

	if (basis && rnd(1,4) == 4)
		consume_scroll(c->who, basis);

	return TRUE;
}



/*
 *  Use learn_skill() to grant a character a skill
 */

void
learn_skill(int who, int sk)
{
	struct skill_ent *p;
	struct char_magic *ch;

	p = p_skill_ent(who, sk);

	wout(who, "Learned %s.", box_name(sk));
	p->know = SKILL_know;

	if (sk == sk_archery)
	{
		struct entity_char *pc;

		pc = p_char(who);

		if (pc->missile < 50)
			pc->missile += 50;
	}

	ch = p_magic(who);

	if (magic_skill(sk))
	{
		ch->max_aura++;
		ch->cur_aura++;

		wout(who, "Maximum aura now %d.", ch->max_aura);

		p_magic(who)->magician = TRUE;

		if (sk == sk_weather)
			p_magic(who)->knows_weather = 1;
	}
}


/*
 *  Note:  d_study is polled daily
 */

int
d_study(struct command *c)
{
	struct skill_ent *p;
	int sk = c->a;
	struct entity_char *ch;

	if (kind(sk) != T_skill)
	{
		log_write(LOG_CODE, "d_study: skill %d is gone, who=%d\n",
							sk, c->who);
		out(c->who, "Internal error: skill %s is gone", box_code(sk));
		return FALSE;
	}

	ch = p_char(c->who);
	ch->studied++;

	p = p_skill_ent(c->who, sk);
	p->days_studied++;

	if (p->days_studied >= learn_time(sk))
	{
		learn_skill(c->who, sk);
		c->wait = 0;
		return TRUE;
	}

	if (char_studied(c->who) >= 14 && c->wait > 0)
	{
		wout(c->who, "Maximum 14 days studied this month.");
		c->wait = 0;
		return TRUE;
	}

	return TRUE;
}


static int
research_notknown(int who, int sk)
{
	static ilist l = NULL;
	int i;
	struct entity_skill *p;
	int new;

	p = rp_skill(sk);

	if (p == NULL)
		return 0;

	ilist_clear(&l);

	for (i = 0; i < ilist_len(p->research); i++)
	{
		new = p->research[i];

		if (rp_skill_ent(who, new) == NULL &&
		    rp_skill_ent(who, req_skill(new)) != NULL)
		{
			ilist_append(&l, new);
		}
	}

	if (ilist_len(l) <= 0)
		return 0;

	i = rnd(0, ilist_len(l)-1);

	return l[i];
}


int
v_research(struct command *c)
{
	int sk = c->a;
	int where = subloc(c->who);

	if (numargs(c) < 1)
	{
		wout(c->who, "Must specify skill to research.");
		return FALSE;
	}

	if (kind(sk) != T_skill)
	{
		wout(c->who, "%s is not a valid skill.", c->parse[1]);
		return FALSE;
	}

	if (has_skill(c->who, sk) < 1)
	{
		wout(c->who, "%s does not know %s.",
				box_name(c->who), c->parse[1]);
		return FALSE;
	}

	if (sk == sk_religion)
	{
		if (subkind(where) != sub_temple)
		{
			wout(c->who, "%s may only be researched "
					"in a temple.", box_name(sk_religion));
			return FALSE;
		}

		if (building_owner(where) != c->who)
		{
			wout(c->who, "Must be the first character inside "
					"the temple to research.");
			return FALSE;
		}
	}
	else
	{
		if (subkind(where) != sub_tower)
		{
			wout(c->who, "Research must be performed in a tower.");

			return FALSE;
		}

		if (building_owner(where) != c->who)
		{
			wout(c->who, "Must be the first character inside "
					"the tower to research.");
			return FALSE;
		}
	}

	if (is_magician(c->who) &&
	    max_eff_aura(c->who) > 30 &&
	    loc_civ(province(c->who)) > 1) {
		wout(c->who, "Research by 6th black circle level mages "
		"and above must be done in provinces with a civilization "
		"level of 1 or less.");
	}

	if (research_notknown(c->who, sk) == 0)
	{
		wout(c->who, "No unknown researchable skills exist for %s.",
					just_name(sk));
		return FALSE;
	}

	if (!can_pay(c->who, 25))
	{
		wout(c->who, "Can't afford 25 gold to research.");
		return FALSE;
	}

	wout(c->who, "Research %s.", box_name(sk));
	return TRUE;
}


int
d_research(struct command *c)
{
	int sk = c->a;
	int new_skill;
	struct entity_char *ch;
	int chance = 25;

	if (kind(sk) != T_skill)
	{
		wout(c->who, "Internal error.");
		fprintf(stderr, "d_research: skill %d is gone, who=%d\n",
							sk, c->who);
		return FALSE;
	}

	if (!charge(c->who, 25))
	{
		wout(c->who, "Can't afford 25 gold to research.");
		return FALSE;
	}

	ch = p_char(c->who);

	if (rnd(1,100) > chance)
	{
		wout(c->who, "Research uncovers no new skills.");
		return FALSE;
	}

	new_skill = research_notknown(c->who, sk);

	if (new_skill == 0)
	{
		wout(c->who, "Research uncovers no new skills.");
		fprintf(stderr, "d_research: skill evaporated: "
				"who=%d, sk=%d\n", c->who, sk);
		return FALSE;
	}

/*
 *  Cause the new skill to be partially known
 */

	(void) p_skill_ent(c->who, new_skill);

	wout(c->who, "Research uncovers a new skill:  %s",
					box_name(new_skill));

	wout(c->who, "To begin learning this skill, order 'study %s'.",
					box_code_less(new_skill));

	return TRUE;
}


int
char_np_total(int who)
{
	int sum = 1;		/* chars cost 1 NP to start */
	struct skill_ent *e;

	if (is_npc(who) && (subkind(who) != sub_dead_body))
		return 0;

	if (loyal_kind(who) == LOY_oath)
		sum += loyal_rate(who);

	loop_char_skill(who, e)
	{
		sum += skill_np_req(e->skill);
	}
	next_char_skill;

	return sum;
}

