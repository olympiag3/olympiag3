
#include	<stdio.h>
#include	"z.h"
#include	"oly.h"


char *from_host = "g3@olympia.v-labs.be";
char *reply_host = "g3@olympia.v-labs.be";
char *gm_address = "c@dvdc.be";
char *game_title = "Olympia G3";
char *game_url = "http://olympia.v-labs.be/g3";
char *rules_url = "http://oly.v-labs.be/g3/node/11";
char *times_url = "http://oly.v-labs.be/g3/node/409";
char *htpasswd_loc = "/u/oly/g2/lib/ht-passwords";
int garrison_pay = 4;
int army_slow_factor = 10;
int auto_quit_turns = 0;

struct box **bx;		/* all possible entities */
int box_head[T_MAX];		/* heads of x_next_kind chain */
int sub_head[SUB_MAX];		/* heads of x_next_sub chain */

char *libdir = "lib";
olytime sysclock;
int indep_player = 100;		/* independent player */
int gm_player = 200;		/* The Fates */
int skill_player = 202;		/* skill listing */
int eat_pl = 203;		/* Order scanner */
int npc_pl = 206;		/* Subloc monster player */
int garr_pl = 207;		/* Garrison unit owner */
int combat_pl = 0;		/* Combat log */
int show_day = FALSE;
int post_has_been_run = FALSE;
int garrison_magic = 999;
int mount_olympus = 0;


int v_look(struct command *c), v_stack(struct command *c),
v_unstack(struct command *c), v_promote(struct command *c),
v_die(struct command *c);
int v_explore(struct command *c), d_explore(struct command *c),
v_name(struct command *c), v_banner(struct command *c),
v_notab(struct command *c);
int v_move(struct command *c), d_move(struct command *c),
v_sail(struct command *c), d_sail(struct command *c),
i_sail(struct command *c);
int v_give(struct command *c), v_pay(struct command *c),
v_repair(struct command *c), d_repair(struct command *c),
i_repair(struct command *c), v_claim(struct command *c);
int v_swear(struct command *c), v_form(struct command *c),
d_form(struct command *c), v_use(struct command *c), d_use(struct command *c),
i_use(struct command *c);
int v_study(struct command *c), d_study(struct command *c),
v_research(struct command *c), d_research(struct command *c),
v_format(struct command *c);
int v_wait(struct command *c), d_wait(struct command *c),
i_wait(struct command *c), v_flag(struct command *c),
v_discard(struct command *c), v_guard(struct command *c);
int v_recruit(struct command *c), v_make(struct command *c),
d_make(struct command *c), i_make(struct command *c),
v_pillage(struct command *c), d_pillage(struct command *c);
int v_attack(struct command *c), v_behind(struct command *c),
v_bribe(struct command *c), d_bribe(struct command *c);
int v_buy(struct command *c), v_sell(struct command *c),
v_execute(struct command *c), v_surrender(struct command *c);
int v_honor(struct command *c), v_oath(struct command *c),
v_terrorize(struct command *c), d_terrorize(struct command *c),
v_quit(struct command *c);
int v_build(struct command *c), d_build(struct command *c),
v_quarry(struct command *c), v_fish(struct command *c),
v_emote(struct command *c);
int v_post(struct command *c), v_message(struct command *c),
v_rumor(struct command *c), v_press(struct command *c),
v_public(struct command *c);
int v_collect(struct command *c), d_collect(struct command *c),
i_collect(struct command *c), v_raze(struct command *c),
d_raze(struct command *c);
int v_wood(struct command *c), v_yew(struct command *c),
v_catch(struct command *c), v_mallorn(struct command *c),
v_stop(struct command *c);
int v_raise(struct command *c), d_raise(struct command *c),
v_rally(struct command *c), d_rally(struct command *c),
v_reclaim(struct command *c);
int v_incite(struct command *c), v_forget(struct command *c),
v_garrison(struct command *c), v_pledge(struct command *c);
int v_fly(struct command *c), d_fly(struct command *c),
v_sneak(struct command *c), d_sneak(struct command *c);
int v_admit(struct command *c), v_hostile(struct command *c),
v_defend(struct command *c), v_neutral(struct command *c),
v_att_clear(struct command *c);
int v_hide(struct command *c), d_hide(struct command *c),
v_contact(struct command *c), v_seek(struct command *c),
d_seek(struct command *c);
int v_opium(struct command *c), v_get(struct command *c),
v_breed(struct command *c), d_breed(struct command *c),
v_decree(struct command *c), v_ungarrison(struct command *c);
int v_torture(struct command *c), d_torture(struct command *c),
v_trance(struct command *c), d_trance(struct command *c);
int v_fee(struct command *c), v_board(struct command *c),
v_ferry(struct command *c), v_unload(struct command *c),
v_split(struct command *c);
int v_bind_storm(struct command *c), d_bind_storm(struct command *c),
v_credit(struct command *c), v_xyzzy(struct command *c),
v_plugh(struct command *c);
int v_fullname(struct command *c), v_times(struct command *c),
v_improve(struct command *c), d_improve(struct command *c);
int v_quest(struct command *c), d_quest(struct command *c),
v_accept(struct command *c);

int v_enter(struct command *c), v_exit(struct command *c),
v_north(struct command *c), v_south(struct command *c),
v_east(struct command *c), v_west(struct command *c);

int v_be(struct command *c), v_listcmds(struct command *c),
v_poof(struct command *c), v_see_all(struct command *c),
v_invent(struct command *c);
int v_add_item(struct command *c), v_sub_item(struct command *c),
v_dump(struct command *c), v_makeloc(struct command *c),
v_seed(struct command *c);
int v_lore(struct command *c), v_know(struct command *c),
v_skills(struct command *c), v_save(struct command *c),
v_postproc(struct command *c);
int v_los(struct command *c), v_kill(struct command *c),
v_take_pris(struct command *c), v_ct(struct command *c),
v_fix(struct command *c), v_fix2(struct command *c);
int v_seedmarket(struct command *c), v_relore(struct command *c),
v_remail(struct command *c);


/*
 *  Allow field:
 *
 *	c	character
 *	p	player entity
 *	i	immediate mode only (debugging/maintenance)
 *	r	restricted -- for npc units under control
 *	g	garrison
 *	m	Gamemaster only
 */

struct cmd_tbl_ent cmd_tbl[] = {
/*
allow  name         start         finish      intr      time poll pri
 */

{"",   "",          NULL,         NULL,       NULL,       0,  0,  3},
{"cpr","accept",    v_accept,     NULL,       NULL,       0,  0,  0},
{"cpr","admit",     v_admit,      NULL,       NULL,       0,  0,  0},
{"cr", "attack",    v_attack,     NULL,       NULL,       1,  0,  3},
{"cr", "banner",    v_banner,     NULL,       NULL,       0,  0,  1},
{"cr", "behind",    v_behind,     NULL,       NULL,       0,  0,  1},
{"c",  "bind",      v_bind_storm, d_bind_storm, NULL,     7,  0,  3},
{"c",  "board",     v_board,      NULL,       NULL,       0,  0,  2},
{"c",  "breed",     v_breed,      d_breed,    NULL,       7,  0,  3},
{"c",  "bribe",     v_bribe,      d_bribe,    NULL,       7,  0,  3},
{"c",  "build",     v_build,      d_build,    NULL,      -1,  1,  3},
{"c",  "buy",       v_buy,        NULL,       NULL,       0,  0,  1},
{"c",  "catch",     v_catch,      NULL,       NULL,      -1,  1,  3},
{"c",  "claim",     v_claim,      NULL,       NULL,       0,  0,  1},
{"c",  "collect",   v_collect,    d_collect,  i_collect, -1,  1,  3},
{"cr", "contact",   v_contact,    NULL,       NULL,       0,  0,  0},
{"m",  "credit",    v_credit,     NULL,       NULL,       0,  0,  0},
{"c",  "decree",    v_decree,     NULL,       NULL,       0,  0,  0},
{"cpr","default",   v_att_clear,  NULL,       NULL,       0,  0,  0},
{"cpr","defend",    v_defend,     NULL,       NULL,       0,  0,  0},
{"c",  "die",       v_die,        NULL,       NULL,       0,  0,  1},
{"c",  "discard",   v_discard,    NULL,       NULL,       0,  0,  1},
{"cr", "drop",      v_discard,    NULL,       NULL,       0,  0,  1},
{"m",  "emote",     v_emote,      NULL,       NULL,       0,  0,  1},
{"cr", "execute",   v_execute,    NULL,       NULL,       0,  0,  1},
{"c",  "explore",   v_explore,    d_explore,  NULL,       7,  0,  3},
{"c",  "fee",       v_fee,        NULL,       NULL,       0,  0,  1},
{"c",  "ferry",     v_ferry,      NULL,       NULL,       0,  0,  1},
{"c",  "fish",      v_fish,       NULL,       NULL,      -1,  1,  3},
{"cr", "flag",      v_flag,       NULL,       NULL,       0,  0,  1},
{"c",  "fly",       v_fly,        d_fly,      NULL,      -1,  0,  2},
{"c",  "forget",    v_forget,     NULL,       NULL,       0,  0,  1},
{"c",  "form",      v_form,       d_form,     NULL,       7,  0,  3},
{"cp", "format",    v_format,     NULL,       NULL,       0,  0,  1},
{"c",  "garrison",  v_garrison,   NULL,       NULL,       1,  0,  3},
{"cr", "get",       v_get,        NULL,       NULL,       0,  0,  1},
{"cr", "give",      v_give,       NULL,       NULL,       0,  0,  1},
{"cr", "go",        v_move,       d_move,     NULL,      -1,  0,  2},
{"c",  "guard",     v_guard,      NULL,       NULL,       0,  0,  1},
{"c",  "hide",      v_hide,       d_hide,     NULL,       3,  0,  3},
{"c",  "honor",     v_honor,      NULL,       NULL,       1,  0,  3},
{"c",  "honour",    v_honor,      NULL,       NULL,       1,  0,  3},
{"cpr","hostile",   v_hostile,    NULL,       NULL,       0,  0,  0},
{"c",  "improve",   v_improve,    d_improve,  NULL,      -1,  1,  3},
{"c",  "incite",    v_incite,     NULL,       NULL,       7,  0,  3},
{"c",  "make",      v_make,       d_make,     i_make,    -1,  1,  3},
{"c",  "mallorn",   v_mallorn,    NULL,       NULL,      -1,  1,  3},
{"cp", "message",   v_message,    NULL,       NULL,       1,  0,  3},
{"cr", "move",      v_move,       d_move,     NULL,      -1,  0,  2},
{"cpr","name",      v_name,       NULL,       NULL,       0,  0,  1},
{"cpr","neutral",   v_neutral,    NULL,       NULL,       0,  0,  0},
{"cp", "notab",     v_notab,      NULL,       NULL,       0,  0,  1},
{"c",  "oath",      v_oath,       NULL,       NULL,       1,  0,  3},
{"c",  "opium",     v_opium,      NULL,       NULL,      -1,  1,  3},
{"cr", "pay",      v_pay,        NULL,       NULL,       0,  0,  1},
{"cr", "pillage",   v_pillage,    d_pillage,  NULL,       7,  0,  3},
{"c",  "pledge",    v_pledge,     NULL,       NULL,       0,  0,  1},
{"cr", "plugh",     v_plugh,      NULL,       NULL,       0,  0,  3},
{"c",  "post",      v_post,       NULL,       NULL,       1,  0,  3},
{"cp", "press",     v_press,      NULL,       NULL,       0,  0,  1},
{"cr", "promote",   v_promote,    NULL,       NULL,       0,  0,  1},
{"cp", "public",    v_public,     NULL,       NULL,       0,  0,  1},
{"c",  "quarry",    v_quarry,     NULL,       NULL,      -1,  1,  3},
{"c",  "quest",     v_quest,      d_quest,    NULL,       7,  0,  3},
{"p",  "quit",      v_quit,       NULL,       NULL,       0,  0,  1},
{"c",  "raise",     v_raise,      d_raise,    NULL,       7,  0,  3},
{"c",  "rally",     v_rally,      d_rally,    NULL,       7,  0,  3},
{"cr", "raze",      v_raze,       d_raze,     NULL,      -1,  1,  3},
{"cpr","realname",  v_fullname,   NULL,       NULL,       0,  0,  1},
{"c",  "reclaim",   v_reclaim,    NULL,       NULL,       0,  0,  1},
{"c",  "recruit",   v_recruit,    NULL,       NULL,      -1,  1,  3},
{"c",  "repair",    v_repair,     d_repair,   i_repair,  -1,  1,  3},
{"c",  "research",  v_research,   d_research, NULL,       7,  0,  3},
{"cp", "rumor",     v_rumor,      NULL,       NULL,       0,  0,  1},
{"c",  "sail",      v_sail,       d_sail,     i_sail,    -1,  0,  4},
{"c",  "sell",      v_sell,       NULL,       NULL,       0,  0,  1},
{"cr", "seek",      v_seek,       d_seek,     NULL,       7,  1,  3},
{"c",  "sneak",     v_sneak,      d_sneak,    NULL,       3,  0,  3},
{"cp", "split",     v_split,      NULL,       NULL,       0,  0,  1},
{"cr", "stack",     v_stack,      NULL,       NULL,       0,  0,  1},
{"c",  "stone",     v_quarry,     NULL,       NULL,      -1,  1,  3},
{"c",  "study",     v_study,      d_study,    NULL,       7,  1,  3},
{"c",  "surrender", v_surrender,  NULL,       NULL,       1,  0,  1},
{"c",  "swear",     v_swear,      NULL,       NULL,       0,  0,  1},
{"cr", "take",      v_get,        NULL,       NULL,       0,  0,  1},
{"cp", "times",     v_times,      NULL,       NULL,       0,  0,  1},
{"c",  "train",     v_make,       d_make,     i_make,    -1,  1,  3},
{"c",  "trance",    v_trance,     d_trance,   NULL,      28,  0,  3},
{"cr", "terrorize", v_terrorize,  d_terrorize,NULL,       7,  0,  3},
{"c",  "torture",   v_torture,    d_torture,  NULL,       7,  0,  3},
{"c",  "unload",    v_unload,     NULL,       NULL,       0,  0,  3},
{"c",  "ungarrison",v_ungarrison, NULL,       NULL,       1,  0,  3},
{"cr", "unstack",   v_unstack,    NULL,       NULL,       0,  0,  1},
{"c",  "use",       v_use,        d_use,      i_use,     -1,  1,  3},
{"crm","wait",      v_wait,       d_wait,     i_wait,    -1,  1,  1},
{"c",  "wood",      v_wood,       NULL,       NULL,      -1,  1,  3},
{"cr", "xyzzy",     v_xyzzy,      NULL,       NULL,       0,  0,  3},
{"c",  "yew",       v_yew,        NULL,       NULL,      -1,  1,  3},

{"cr", "north",     v_north,      NULL,       NULL,      -1,  0,  2},
{"cr", "n",         v_north,      NULL,       NULL,      -1,  0,  2},
{"cr", "s",         v_south,      NULL,       NULL,      -1,  0,  2},
{"cr", "south",     v_south,      NULL,       NULL,      -1,  0,  2},
{"cr", "east",      v_east,       NULL,       NULL,      -1,  0,  2},
{"cr", "e",         v_east,       NULL,       NULL,      -1,  0,  2},
{"cr", "west",      v_west,       NULL,       NULL,      -1,  0,  2},
{"cr", "w",         v_west,       NULL,       NULL,      -1,  0,  2},
{"cr", "enter",     v_enter,      NULL,       NULL,      -1,  0,  2},
{"cr", "exit",      v_exit,       NULL,       NULL,      -1,  0,  2},
{"cr", "in",        v_enter,      NULL,       NULL,      -1,  0,  2},
{"cr", "out",       v_exit,       NULL,       NULL,      -1,  0,  2},

{"",   "begin",     NULL,         NULL,       NULL,       0,  0,  0},
{"",   "unit",      NULL,         NULL,       NULL,       0,  0,  0},
{"",   "email",     NULL,         NULL,       NULL,       0,  0,  0},
{"",   "vis_email", NULL,         NULL,       NULL,       0,  0,  0},
{"",   "end",       NULL,         NULL,       NULL,       0,  0,  0},
{"",   "flush",     NULL,         NULL,       NULL,       0,  0,  0},
{"",   "lore",      NULL,         NULL,       NULL,       0,  0,  0},
{"",   "passwd",    NULL,         NULL,       NULL,       0,  0,  0},
{"",   "password",  NULL,         NULL,       NULL,       0,  0,  0},
{"",   "players",   NULL,         NULL,       NULL,       0,  0,  0},
{"",   "resend",    NULL,         NULL,       NULL,       0,  0,  0},
{"cpr","stop",      v_stop,       NULL,       NULL,       0,  0,  0},

{"i",  "look",      v_look,       NULL,       NULL,       0,  0,  1},
{"i",  "l",         v_look,       NULL,       NULL,       0,  0,  1},
{"i",  "ct",        v_ct,         NULL,       NULL,       0,  0,  1},
{"i",  "be",        v_be,         NULL,       NULL,       0,  0,  1},
{"i",  "additem",   v_add_item,   NULL,       NULL,       0,  0,  1},
{"i",  "subitem",   v_sub_item,   NULL,       NULL,       0,  0,  1},
{"i",  "h",         v_listcmds,   NULL,       NULL,       0,  0,  1},
{"i",  "dump",      v_dump,       NULL,       NULL,       0,  0,  1},
{"i",  "i",         v_invent,     NULL,       NULL,       0,  0,  1},
{"i",  "fix",       v_fix,        NULL,       NULL,       0,  0,  1},
{"i",  "fix2",      v_fix2,       NULL,       NULL,       0,  0,  1},
{"i",  "kill",      v_kill,       NULL,       NULL,       0,  0,  1},
{"i",  "los",       v_los,        NULL,       NULL,       0,  0,  1},
{"m",  "relore",    v_relore,     NULL,       NULL,       0,  0,  1},
{"i",  "sk",        v_skills,     NULL,       NULL,       0,  0,  1},
{"i",  "know",      v_know,       NULL,       NULL,       0,  0,  1},
{"i",  "seed",      v_seed,       NULL,       NULL,       0,  0,  1},
{"i",  "seedmarket",v_seedmarket, NULL,       NULL,       0,  0,  1},
{"i",  "sheet",     v_lore,       NULL,       NULL,       0,  0,  1},
{"i",  "poof",      v_poof,       NULL,       NULL,       0,  0,  1},
{"i",  "postproc",  v_postproc,   NULL,       NULL,       0,  0,  1},
{"i",  "save",      v_save,       NULL,       NULL,       0,  0,  1},
{"i",  "seeall",    v_see_all,    NULL,       NULL,       0,  0,  1},
{"i",  "tp",        v_take_pris,  NULL,       NULL,       0,  0,  1},
{"i",  "makeloc",   v_makeloc,    NULL,       NULL,       0,  0,  1},
{"i",  "remail",    v_remail,     NULL,       NULL,       0,  0,  1},
{NULL,  NULL,       NULL,         NULL,       NULL,       0,  0,  1}

};


char *kind_s[] = {
	"deleted",			/* T_deleted */
	"player",			/* T_player */
	"char",				/* T_char */
	"loc",				/* T_loc */
	"item",				/* T_item */
	"skill",			/* T_skill */
	"gate",				/* T_gate */
	"road",				/* T_road */
	"deadchar",			/* T_deadchar */
	"ship",				/* T_ship */
	"post",				/* T_post */
	"storm",			/* T_storm */
	"unform",			/* T_unform */
	"lore",				/* T_lore */
	NULL
};

char *subkind_s[] = {
	"<no subkind>",
	"ocean",			/* sub_ocean */
	"forest",			/* sub_forest */
	"plain",			/* sub_plain */
	"mountain",			/* sub_mountain */
	"desert",			/* sub_desert */
	"swamp",			/* sub_swamp */
	"underground",			/* sub_under */
	"faery hill",			/* sub_faery_hill */
	"island",			/* sub_island */
	"ring of stones",		/* sub_stone_cir */
	"mallorn grove",		/* sub_mallorn_grove */
	"bog",				/* sub_bog */
	"cave",				/* sub_cave */
	"city",				/* sub_city */
	"lair",				/* sub_lair */
	"graveyard",			/* sub_graveyard */
	"ruins",			/* sub_ruins */
	"battlefield",			/* sub_battlefield */
	"enchanted forest",		/* sub_ench_forest */
	"rocky hill",			/* sub_rocky_hill */
	"circle of trees",		/* sub_tree_cir */
	"pits",				/* sub_pits */
	"pasture",			/* sub_pasture */
	"oasis",			/* sub_oasis */
	"yew grove",			/* sub_yew_grove */
	"sand pit",			/* sub_sand_pit */
	"sacred grove",			/* sub_sacred_grove */
	"poppy field",			/* sub_poppy_field */
	"temple",			/* sub_temple */
	"galley",			/* sub_galley */
	"roundship",			/* sub_roundship */
	"castle",			/* sub_castle */
	"galley-in-progress",		/* sub_galley_notdone */
	"roundship-in-progress",	/* sub_roundship_notdone */
	"ghost ship",			/* sub_ghost_ship */
	"temple-in-progress",		/* sub_temple_notdone */
	"inn",				/* sub_inn */
	"inn-in-progress",		/* sub_inn_notdone */
	"castle-in-progress",		/* sub_castle_notdone */
	"mine",				/* sub_mine */
	"mine-in-progress",		/* sub_mine_notdone */
	"scroll",			/* sub_scroll */
	"magic",			/* sub_magic */
	"palantir",			/* sub_palantir */
	"auraculum",			/* sub_auraculum */
	"tower",			/* sub_tower */
	"tower-in-progress",		/* sub_tower_notdone */
	"pl_system",			/* sub_pl_system */
	"pl_regular",			/* sub_pl_regular */
	"region",			/* sub_region */
	"pl_savage",			/* sub_pl_savage */
	"pl_npc",			/* sub_pl_npc */
	"collapsed mine",		/* sub_mine_collapsed */
	"ni",				/* sub_ni */
	"demon lord",			/* sub_undead */
	"dead body",			/* sub_dead_body */
	"fog",				/* sub_fog */
	"wind",				/* sub_wind */
	"rain",				/* sub_rain */
	"pit",				/* sub_hades_pit */
	"artifact",			/* sub_artifact */
	"pl_silent",			/* sub_pl_silent */
	"npc_token",			/* sub_npc_token */
	"garrison",			/* sub_garrison */
	"cloud",			/* sub_cloud */
	"raft",				/* sub_raft */
	"raft-in-progress",		/* sub_raft_notdone */
	"suffuse_ring",			/* sub_suffuse_ring */
	"relic",			/* sub_relic */
	"tunnel",			/* sub_tunnel */
	"sewer",			/* sub_sewer */
	"chamber",			/* sub_chamber */
	"tradegood",			/* sub_tradegood */

	NULL
};

char *short_dir_s[] = {
	"<no dir>",
	"n",
	"e",
	"s",
	"w",
	"u",
	"d",
	"i",
	"o",
	NULL
};

char *full_dir_s[] = {
	"<no dir>",
	"north",
	"east",
	"south",
	"west",
	"up",
	"down",
	"in",
	"out",
	NULL
};

int exit_opposite[] = {
	0,
	DIR_S,
	DIR_W,
	DIR_N,
	DIR_E,
	DIR_OUT,
	DIR_IN,
	DIR_DOWN,
	DIR_UP,
	0
};

char *loc_depth_s[] = {
	"<no depth",
	"region",
	"province",
	"subloc",
	NULL 
};

char *month_names[] = {
	"Fierce winds",
	"Snowmelt",
	"Blossom bloom",
	"Sunsear",
	"Thunder and rain",
	"Harvest",
	"Waning days",
	"Dark night",
	NULL
};


void
glob_init()
{
	int i;

#if 0
	for (i = 0; i < MAX_BOXES; i++)
		bx[i] = NULL;
#endif

	for (i = 0; i < T_MAX; i++)
		box_head[i] = 0;

	bx = my_malloc(sizeof(*bx) * MAX_BOXES);
}

