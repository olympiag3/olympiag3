
#include	<stdio.h>
#include	"z.h"
#include	"oly.h"


char *from_host = "skrenta@pbm.com (Rich skrenta)";
char *reply_host = "skrenta@pbm.com (Rich skrenta)";

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


int v_look(), v_stack(), v_unstack(), v_promote(), v_die();
int v_explore(), d_explore(), v_name(), v_banner(), v_notab();
int v_move(), d_move(), v_sail(), d_sail(), i_sail();
int v_give(), v_pay(), v_repair(), d_repair(), i_repair(), v_claim();
int v_swear(), v_form(), d_form(), v_use(), d_use(), i_use();
int v_study(), d_study(), v_research(), d_research(), v_format();
int v_wait(), d_wait(), i_wait(), v_flag(), v_discard(), v_guard();
int v_recruit(), v_make(), d_make(), i_make(), v_pillage(), d_pillage();
int v_attack(), v_behind(), v_bribe(), d_bribe();
int v_buy(), v_sell(), v_execute(), v_surrender();
int v_honor(), v_oath(), v_terrorize(), d_terrorize(), v_quit();
int v_build(), d_build(), v_quarry(), v_fish(), v_emote();
int v_post(), v_message(), v_rumor(), v_press(), v_public();
int v_collect(), d_collect(), i_collect(), v_raze(), d_raze();
int v_wood(), v_yew(), v_catch(), v_mallorn(), v_stop();
int v_raise(), d_raise(), v_rally(), d_rally(), v_reclaim();
int v_incite(), v_forget(), v_garrison(), v_pledge();
int v_fly(), d_fly(), v_sneak(), d_sneak();
int v_admit(), v_hostile(), v_defend(), v_neutral(), v_att_clear();
int v_hide(), d_hide(), v_contact(), v_seek(), d_seek();
int v_opium(), v_get(), v_breed(), d_breed(), v_decree(), v_ungarrison();
int v_torture(), d_torture(), v_trance(), d_trance();
int v_fee(), v_board(), v_ferry(), v_unload(), v_split();
int v_bind_storm(), d_bind_storm(), v_credit(), v_xyzzy(), v_plugh();
int v_fullname(), v_times(), v_improve(), d_improve();
int v_quest(), d_quest(), v_accept();

int v_enter(), v_exit(), v_north(), v_south(), v_east(), v_west();

int v_be(), v_listcmds(), v_poof(), v_see_all(), v_invent();
int v_add_item(), v_sub_item(), v_dump(), v_makeloc(), v_seed();
int v_lore(), v_know(), v_skills(), v_save(), v_postproc();
int v_los(), v_kill(), v_take_pris(), v_ct(), v_fix(), v_fix2();
int v_seedmarket(), v_relore(), v_remail();


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

