
typedef		unsigned char	uchar;
typedef		signed char	schar;


#define		MAX_BOXES	150000
#define		MONTH_DAYS	30
#define		NUM_MONTHS	8

#define		T_deleted	0	/* forget on save */
#define		T_player	1
#define		T_char		2
#define		T_loc		3
#define		T_item		4
#define		T_skill		5
#define		T_gate		6
#define		T_road		7
#define		T_deadchar	8
#define		T_ship		9
#define		T_post		10
#define		T_storm		11
#define		T_unform	12	/* unformed noble */
#define		T_lore		13
#define		T_MAX		14	/* one past highest T_xxx define */

#define		sub_ocean		1
#define		sub_forest		2
#define		sub_plain		3
#define		sub_mountain		4
#define		sub_desert		5
#define		sub_swamp		6
#define		sub_under		7	/* underground */
#define		sub_faery_hill		8	/* gateway to Faery */
#define		sub_island		9	/* island subloc */
#define		sub_stone_cir		10	/* ring of stones */
#define		sub_mallorn_grove	11
#define		sub_bog			12
#define		sub_cave		13
#define		sub_city		14
#define		sub_lair		15	/* dragon lair */
#define		sub_graveyard		16
#define		sub_ruins		17
#define		sub_battlefield		18
#define		sub_ench_forest		19	/* enchanted forest */
#define		sub_rocky_hill		20
#define		sub_tree_circle		21
#define		sub_pits		22
#define		sub_pasture		23
#define		sub_oasis		24
#define		sub_yew_grove		25
#define		sub_sand_pit		26
#define		sub_sacred_grove	27
#define		sub_poppy_field		28
#define		sub_temple		29
#define		sub_galley		30
#define		sub_roundship		31
#define		sub_castle		32
#define		sub_galley_notdone	33
#define		sub_roundship_notdone	34
#define		sub_ghost_ship		35
#define		sub_temple_notdone	36
#define		sub_inn			37
#define		sub_inn_notdone		38
#define		sub_castle_notdone	39
#define		sub_mine		40
#define		sub_mine_notdone	41
#define		sub_scroll		42	/* item is a scroll */
#define		sub_magic		43	/* this skill is magical */
#define		sub_palantir		44
#define		sub_auraculum		45
#define		sub_tower		46
#define		sub_tower_notdone	47
#define		sub_pl_system		48	/* system player */
#define		sub_pl_regular		49	/* regular player */
#define		sub_region		50	/* region wrapper loc */
#define		sub_pl_savage		51	/* Savage King */
#define		sub_pl_npc		52
#define		sub_mine_collapsed	53
#define		sub_ni			54	/* ni=noble_item */
#define		sub_undead		55	/* undead lord */
#define		sub_dead_body		56	/* dead noble's body */
#define		sub_fog			57
#define		sub_wind		58
#define		sub_rain		59
#define		sub_hades_pit		60
#define		sub_artifact		61
#define		sub_pl_silent		62
#define		sub_npc_token		63	/* npc group control art */
#define		sub_garrison		64	/* npc group control art */
#define		sub_cloud		65	/* cloud terrain type */
#define		sub_raft		66	/* raft made out of flotsam */
#define		sub_raft_notdone	67
#define		sub_suffuse_ring	68
#define		sub_relic		69	/* 400 series artifacts */
#define		sub_tunnel		70
#define		sub_sewer		71
#define		sub_chamber		72
#define		sub_tradegood		73

#define		SUB_MAX			74	/* one past highest sub_ */

#define		item_gold		1

#define		item_peasant		10
#define		item_worker		11
#define		item_soldier		12
#define		item_archer		13
#define		item_knight		14
#define		item_elite_guard	15
#define		item_pikeman		16
#define		item_blessed_soldier	17
#define		item_ghost_warrior	18
#define		item_sailor		19
#define		item_swordsman		20
#define		item_crossbowman	21
#define		item_elite_arch		22
#define		item_angry_peasant	23
#define		item_pirate		24
#define		item_elf		25
#define		item_spirit		26

#define		item_corpse		31
#define		item_savage		32
#define		item_skeleton		33
#define		item_barbarian		34

#define		item_wild_horse		51
#define		item_riding_horse	52
#define		item_warmount		53
#define		item_pegasus		54
#define		item_nazgul		55

#define		item_flotsam		59
#define		item_battering_ram	60
#define		item_catapult		61
#define		item_siege_tower	62
#define		item_ratspider_venom	63
#define		item_lana_bark		64
#define		item_avinia_leaf	65
#define		item_spiny_root		66
#define		item_farrenstone	67
#define		item_yew		68
#define		item_elfstone		69
#define		item_mallorn_wood	70
#define		item_pretus_bones	71
#define		item_longbow		72
#define		item_plate		73
#define		item_longsword		74
#define		item_pike		75
#define		item_ox			76
#define		item_lumber		77
#define		item_stone		78
#define		item_iron		79
#define		item_leather		80
#define		item_ratspider		81
#define		item_mithril		82
#define		item_gate_crystal	83
#define		item_blank_scroll	84
#define		item_crossbow		85
#define		item_fish		87
#define		item_opium		93
#define		item_basket		94	/* woven basket */
#define		item_pot		95	/* clay pot */
#define		item_tax_cookie		96
#define		item_drum		98
#define		item_hide		99
#define		item_mob_cookie		101
#define		item_lead		102

#define		item_glue		261

#define		item_centaur		271
#define		item_minotaur		272
#define		item_undead_cookie	273
#define		item_fog_cookie		274
#define		item_wind_cookie	275
#define		item_rain_cookie	276
#define		item_mage_menial	277	/* mage menial labor cookie */
#define		item_spider		278	/* giant spider */
#define		item_rat		279	/* horde of rats */
#define		item_lion		280
#define		item_bird		281	/* giant bird */
#define		item_lizard		282
#define		item_bandit		283
#define		item_chimera		284
#define		item_harpie		285
#define		item_dragon		286
#define		item_orc		287
#define		item_gorgon		288
#define		item_wolf		289
#define		item_orb		290
#define		item_cyclops		291
#define		item_giant		292
#define		item_faery		293
#define		item_petty_thief	294
#define		item_hound		295


#define		lore_skeleton_npc_token		931
#define		lore_orc_npc_token		932
#define		lore_undead_npc_token		933
#define		lore_savage_npc_token		934
#define		lore_barbarian_npc_token	935
#define		lore_orb			936
#define		lore_faery_stone		937
#define		lore_barbarian_kill		938
#define		lore_savage_kill		939
#define		lore_undead_kill		940
#define		lore_orc_kill			941
#define		lore_skeleton_kill		942



#define		sk_shipcraft		600
#define		sk_pilot_ship		601
#define		sk_shipbuilding		602
#define		sk_fishing		603

#define		sk_combat		610
#define		sk_survive_fatal	611
#define		sk_fight_to_death	612
#define		sk_make_catapult	613
#define		sk_defense		614
#define		sk_archery		615
#define		sk_swordplay		616
#define		sk_weaponsmith		617

#define		sk_stealth		630
#define		sk_petty_thief		631
#define		sk_spy_inv		632	/* determine char inventory */
#define		sk_spy_skills		633	/* determine char skill */
#define		sk_spy_lord		634	/* determine char's lord */
#define		sk_hide_lord		635
#define		sk_find_rich		636
#define		sk_torture		637
#define		sk_sneak_build		639
#define		sk_hide_self		641

#define		sk_beast		650
#define		sk_bird_spy		651
#define		sk_capture_beasts	652
#define		sk_use_beasts		653	/* use beasts in battle */
#define		sk_breed_beasts		654
#define		sk_catch_horse		655
#define		sk_train_wild		656
#define		sk_train_warmount	657
#define		sk_summon_savage	658
#define		sk_keep_savage		659
#define		sk_breed_hound		661

#define		sk_persuasion		670
#define		sk_bribe_noble		671
#define		sk_persuade_oath	672
#define		sk_raise_mob		673
#define		sk_rally_mob		674
#define		sk_incite_mob		675
#define		sk_train_angry		676

#define		sk_construction		680
#define		sk_make_siege		681
#define		sk_quarry_stone		682

#define		sk_alchemy		690
#define		sk_extract_venom	691	/* from ratspider */
#define		sk_brew_slave		692	/* potion of slavery */
#define		sk_brew_heal		693
#define		sk_brew_death		694
#define		sk_record_skill		695
#define		sk_collect_elem		696
#define		sk_lead_to_gold		697

#define		sk_forestry		700
#define		sk_make_ram		701	/* make battering ram */
#define		sk_harvest_lumber	702
#define		sk_harvest_yew		703
#define		sk_collect_foliage	704
#define		sk_harvest_mallorn	705
#define		sk_harvest_opium	706
#define		sk_improve_opium	707

#define		sk_mining		720
#define		sk_mine_iron		721
#define		sk_mine_gold		722
#define		sk_mine_mithril		723

#define		sk_trade		730
#define		sk_cloak_trade		731
#define		sk_find_sell		732
#define		sk_find_buy		733

#define		sk_religion		750
#define		sk_reveal_vision	751
#define		sk_resurrect		752
#define		sk_pray			753
#define		sk_last_rites		754
#define		sk_remove_bless		755
#define		sk_vision_protect	756

#define		sk_basic		800
#define		sk_meditate		801
#define		sk_mage_menial		802	/* menial labor for mages */
#define		sk_appear_common	803
#define		sk_view_aura		804
#define		sk_reveal_mage		805	/* reveal abilities of mage */
#define		sk_tap_health		806
#define		sk_heal			807
#define		sk_write_basic		808
#define		sk_shroud_abil		809	/* ability shroud */
#define		sk_detect_abil		811	/* detect ability scry */
#define		sk_dispel_abil		812	/* dispel ability shroud */
#define		sk_adv_med		813	/* advanced meditation */
#define		sk_hinder_med		814	/* hinder meditation */

#define		sk_weather		820
#define		sk_fierce_wind		821
#define		sk_bind_storm		822
#define		sk_write_weather	823
#define		sk_summon_rain		824
#define		sk_summon_wind		825
#define		sk_summon_fog		826
#define		sk_direct_storm		827
#define		sk_dissipate		828
#define		sk_renew_storm		829
#define		sk_lightning		831
#define		sk_seize_storm		832
#define		sk_death_fog		833

#define		sk_scry			840
#define		sk_scry_region		841
#define		sk_shroud_region	842
#define		sk_dispel_region	843	/* dispel region shroud */
#define		sk_write_scry		844
#define		sk_bar_loc		845	/* create location barrier */
#define		sk_unbar_loc		846
#define		sk_locate_char		847
#define		sk_detect_scry		848	/* detect region scry */
#define		sk_proj_cast		849	/* project next cast */
#define		sk_save_proj		851	/* save projected cast */
#define		sk_banish_corpses	852

#define		sk_gate			860
#define		sk_teleport		861
#define		sk_detect_gates		862
#define		sk_jump_gate		863
#define		sk_seal_gate		864
#define		sk_unseal_gate		865
#define		sk_notify_unseal	866
#define		sk_rem_seal		867	/* forcefully unseal gate */
#define		sk_reveal_key		868
#define		sk_notify_jump		869
#define		sk_write_gate		871
#define		sk_rev_jump		872

#define		sk_artifact		880
#define		sk_forge_aura		881	/* forge auraculum */
#define		sk_show_art_creat	882	/* learn who created art */
#define		sk_show_art_reg		883	/* learn where art created */
#define		sk_cloak_creat		884
#define		sk_cloak_reg		885
#define		sk_curse_noncreat	886	/* curse noncreator loyalty */
#define		sk_forge_weapon		887
#define		sk_forge_armor		888
#define		sk_forge_bow		889
#define		sk_rem_art_cloak	891	/* dispel artifact cloaks */
#define		sk_write_art		892
#define		sk_destroy_art		893
#define		sk_forge_palantir	894

#define		sk_necromancy		900
#define		sk_transcend_death	901
#define		sk_write_necro		902
#define		sk_summon_ghost		903	/* summon ghost warriors */
#define		sk_raise_corpses	904	/* summon undead corpses */
#define		sk_undead_lord		905	/* summon undead unit */
#define		sk_renew_undead		906
#define		sk_banish_undead	907
#define		sk_eat_dead		908
#define		sk_aura_blast		909
#define		sk_absorb_blast		911

#define		sk_adv_sorcery		920
#define		sk_trance		921
#define		sk_teleport_item	922


/*  dead skills  */
#define		sk_quick_cast		999	/* speed next cast */
#define		sk_save_quick		998	/* save speeded cast */
#define		sk_add_ram		997	/* add ram to galley */


#define		PROG_bandit		1	/* wilderness spice */
#define		PROG_subloc_monster	2
#define		PROG_npc_token		3
#define		PROG_faery_bandit	4
#define		PROG_hades_bandit	5

#define		use_death_potion	1
#define		use_heal_potion		2
#define		use_slave_potion	3
#define		use_palantir		4
#define		use_proj_cast		5	/* stored projected cast */
#define		use_quick_cast		6	/* stored cast speedup */
#define		use_drum		7	/* beat savage's drum */
#define		use_faery_stone		8	/* Faery gate opener */
#define		use_orb			9	/* crystal orb */
#define		use_barbarian_kill	10
#define		use_savage_kill		11
#define		use_corpse_kill		12
#define		use_orc_kill		13
#define		use_skeleton_kill	14
#define		use_bta_skull		15



#define		DIR_N		1
#define		DIR_E		2
#define		DIR_S		3
#define		DIR_W		4
#define		DIR_UP		5
#define		DIR_DOWN	6
#define		DIR_IN		7
#define		DIR_OUT		8
#define		MAX_DIR		9	/* one past highest direction */

#define		LOC_region	1	/* top most continent/island group */
#define		LOC_province	2	/* main location area */
#define		LOC_subloc	3	/* inner sublocation */
#define		LOC_build	4	/* building, structure, etc. */

#define		LOY_UNCHANGED	(-1)
#define		LOY_unsworn	0
#define		LOY_contract	1
#define		LOY_oath	2
#define		LOY_fear	3
#define		LOY_npc		4
#define		LOY_summon	5

#define		exp_novice	1	/* apprentice */
#define		exp_journeyman	2
#define		exp_teacher	3
#define		exp_master	4
#define		exp_grand	5	/* grand master */



typedef ilist sparse;

typedef struct {
	short day;			/* day of month */
	short turn;			/* turn number */
	int days_since_epoch;		/* days since game begin */
} olytime;

#define	oly_month(a)	(((a).turn-1) % NUM_MONTHS)
#define oly_year(a)	(((a).turn-1) / NUM_MONTHS)


struct loc_info {
	int where;
	ilist here_list;
};

struct box {
	schar kind;
	schar skind;
	char *name;

	struct loc_info		x_loc_info;
	struct entity_player	*x_player;
	struct entity_char	*x_char;
	struct entity_loc	*x_loc;
	struct entity_subloc	*x_subloc;
	struct entity_item	*x_item;
	struct entity_skill	*x_skill;
	struct entity_gate	*x_gate;
	struct entity_misc	*x_misc;
	struct att_ent		*x_disp;

	struct command		*cmd;
	struct item_ent		**items;	/* ilist of items held */
	struct trade		**trades;	/* pending buys/sells */

	int temp;			/* scratch space */
	int output_order;		/* for report ordering -- not saved */

	int x_next_kind;		/* link to next entity of same type */
	int x_next_sub;			/* link to next of same subkind */
};

struct entity_player {
        char *full_name;
        char *email;
        char *vis_email;		/* address to put in player list */
        char *password;
	int first_turn;			/* which turn was their first? */
	int last_order_turn;		/* last turn orders were submitted */
	struct order_list **orders;	/* ilist of orders for units in */
					/* this faction */
	sparse known;			/* visited, lore seen, encountered */

	ilist units;			/* what units are in our faction? */
	struct admit **admits;		/* admit permissions list */
	ilist unformed;			/* nobles as yet unformed */

	int split_lines;		/* split mail at this many lines */
	int split_bytes;		/* split mail at this many bytes */
	short fast_study;		/* instant study days available */

	short noble_points;		/* how many NP's the player has */

	schar format;			/* turn report formatting control */
	schar notab;			/* player can't tolerate tabs */
	schar first_tower;		/* has player built first tower yet? */
	schar sent_orders;		/* sent in orders this turn? */
	schar dont_remind;		/* don't send a reminder */
	schar compuserve;		/* get Times from CIS */
	schar broken_mailer;		/* quote begin lines */

/* not saved: */

	char public_turn;		/* turn is public view */
	char times_paid;		/* only 1 Times credit per player */
	schar swear_this_turn;		/* have we used SWEAR this turn? */
	short cmd_count;		/* count of cmds started this turn */
	short np_gained;		/* np's added this turn -- not saved */
	short np_spent;			/* np's lost this turn -- not saved */
	ilist deliver_lore;		/* show these to player -- not saved */
	sparse weather_seen;		/* locs we've viewed the weather */
	sparse output;			/* units with output -- not saved */
	sparse locs;			/* locs we touched -- not saved */
};

struct order_list {
	int unit;			/* unit orders are for */
	char **l;			/* ilist of orders for unit */
};

struct accept_ent {
	int item;			/* 0 = any item */
	int from_who;			/* 0 = anyone, else char or player */
	int qty;			/* 0 = any qty */
};

#define		ATT_NONE	0	/* no attitude -- default */
#define		NEUTRAL		1	/* explicitly neutral */
#define		HOSTILE		2	/* attack on sight */
#define		DEFEND		3	/* defend if attacked */

struct att_ent {
	ilist neutral;
	ilist hostile;
	ilist defend;
};

struct entity_char {
	int unit_item;			/* unit is made of this kind of item */

        schar health;
	schar sick;			/* 1=character is getting worse */

	schar guard;			/* character is guarding the loc */
	schar loy_kind;			/* LOY_xxx */
	int loy_rate;			/* level with kind of loyalty */

	olytime death_time;		/* when was character killed */

	struct skill_ent **skills;	/* ilist of skills known by char */

	int moving;			/* daystamp of beginning of movement */
	int unit_lord;			/* who is our owner? */
	int prev_lord;			/* who was our previous owner? */

	ilist contact;			/* who have we contacted, also, who */
					/* has found us */

	struct char_magic *x_char_magic;

	schar prisoner;			/* is this character a prisoner? */
	schar behind;			/* are we behind in combat? */
	schar time_flying;		/* time airborne over ocean */
	schar break_point;		/* break point when fighting */
	schar rank;			/* noble peerage status */
	schar npc_prog;			/* npc program */

	short attack;			/* fighter attack rating */
	short defense;			/* fighter defense rating */
	short missile;			/* capable of missile attacks? */

/*
 *  The following are not saved by io.c:
 */

	schar melt_me;			/* in process of melting away */
	schar fresh_hire;		/* don't erode loyalty */
	schar new_lord;			/* got a new lord this turn */
	schar studied;			/* num days we studied */
	struct accept_ent **accept;	/* what we can be given */
};

struct char_magic {
	int max_aura;			/* maximum aura level for magician */
	int cur_aura;			/* current aura level for magician */
	int auraculum;			/* char created an auraculum */

	sparse visions;			/* visions revealed */
	int pledge;			/* lands are pledged to another */
	int token;			/* we are controlled by this art */
	int fee;			/* gold/100 wt. to board this ship */

	int project_cast;		/* project next cast */
	short quick_cast;		/* speed next cast */
	short ability_shroud;

	schar hide_mage;		/* hide magician status */
	schar hinder_meditation;
	schar magician;			/* is a magician */
	schar pray;			/* have prayed */
	schar aura_reflect;		/* reflect aura blast */
	schar hide_self;		/* character is hidden */
	schar swear_on_release;		/* swear to one who frees us */
	schar knows_weather;		/* knows weather magic */
	schar vis_protect;		/* vision protection level */
	schar default_garr;		/* default initial garrison */

	schar mage_worked;		/* worked this month -- not saved */
	schar ferry_flag;		/* ferry has tooted its horn -- ns */
	ilist pledged_to_us;		/* temp -- not saved */
};


#define		SKILL_dont	0	/* don't know the skill */
#define		SKILL_learning	1	/* in the process of learning it */
#define		SKILL_know	2	/* know it */

struct skill_ent {
	int skill;
	int days_studied;		/* days studied * TOUGH_NUM */
	short experience;		/* experience level with skill */
	char know;			/* SKILL_xxx */

/*
 *  Not saved:
 */

	char exp_this_month;		/* flag for add_skill_experience() */
};

struct item_ent {
	int item;
	int qty;
};

struct entity_loc {
	ilist prov_dest;
	short shroud;			/* magical scry shroud */
	short barrier;			/* magical barrier */
	schar civ;			/* civilization level (0 = wild) */
	schar hidden;			/* is location hidden? */
	schar dist_from_gate;
	schar sea_lane;			/* fast ocean travel here */
					/* also "tracks" for npc ferries */
};

struct entity_subloc {
	ilist teaches;			/* skills location offers */
	int opium_econ;			/* addiction level of city */
	int defense;			/* defense rating of structure */

	schar loot;			/* loot & pillage level */
	schar recent_loot;		/* pillaged this month -- not saved */
	uchar damage;			/* 0=none, 100=fully destroyed */
	schar galley_ram;		/* galley is fitted with a ram */
	short shaft_depth;		/* depth of mine shaft */
	schar castle_lev;		/* level of castle improvement */

	schar build_materials;		/* fifths of materials we've used */
	int effort_required;		/* not finished if nonzero */
	int effort_given;

	int moving;			/* daystamp of beginning of movement */
	int capacity;			/* capacity of ship */

	ilist near_cities;		/* cities rumored to be nearby */
	schar safe;			/* safe haven */
	schar major;			/* major city */
	schar prominence;		/* prominence of city */
	schar uldim_flag;		/* Uldim pass */
	schar summer_flag;		/* Summerbridge */
	schar quest_late;		/* quest decay counter */
	schar tunnel_level;		/* depth of tunnel */

	schar link_when;		/* month link is open, -1 = never */
	schar link_open;		/* link is open now */
	ilist link_to;			/* where we are linked to */
	ilist link_from;		/* where we are linked from */
	ilist bound_storms;		/* storms bound to this ship */
};

struct entity_item {
        short weight;
        short land_cap;
        short ride_cap;
        short fly_cap;
	short attack;		/* fighter attack rating */
	short defense;		/* fighter defense rating */
	short missile;		/* capable of missile attacks? */

	schar is_man_item;	/* unit is a character like thing */
	schar animal;		/* unit is or contains a horse or an ox */
	schar prominent;	/* big things that everyone sees */
	schar capturable;	/* ni-char contents are capturable */

	char *plural_name;
	int base_price;		/* base price of item for market seeding */
	int who_has;		/* who has this unique item */

	struct item_magic	*x_item_magic;
};

struct item_magic {
	int creator;
	int region_created;
	int lore;			/* deliver this lore for the item */

	schar curse_loyalty;		/* curse noncreator loyalty */
	schar cloak_region;
	schar cloak_creator;
	schar use_key;			/* special use action */

	ilist may_use;			/* list of usable skills via this */
	ilist may_study;		/* list of skills studying from this */

	int project_cast;		/* stored projected cast */
	int token_ni;			/* ni for controlled npc units */
	short quick_cast;		/* stored quick cast */

	short aura_bonus;
	short aura;			/* auraculum aura */
	short relic_decay;		/* countdown timer */

	schar attack_bonus;
	schar defense_bonus;
	schar missile_bonus;

	schar token_num;		/* how many token controlled units */
	schar orb_use_count;		/* how many uses left in the orb */

/*
 *  Not saved:
 */

	schar one_turn_use;		/* flag for one use per turn */
};

struct entity_skill {
	int time_to_learn;	/* days of study req'd to learn skill */
	int required_skill;	/* skill required to learn this skill */
	int np_req;		/* noble points required to learn */
	ilist offered;		/* skills learnable after this one */
	ilist research;		/* skills researable with this one */

	struct req_ent **req;	/* ilist of items required for use or cast */
	int produced;		/* simple production skill result */

	int no_exp;		/* this skill not rated for experience */

/* not saved */

	int use_count;		/* times skill used during turn */
	int last_use_who;	/* who last used the skill (this turn) */
};

#define	REQ_NO	0		/* don't consume item */
#define	REQ_YES	1		/* consume item */
#define	REQ_OR	2		/* or with next */

struct req_ent {
	int item;		/* item required to use */
	int qty;		/* quantity required */
	schar consume;		/* REQ_xx */
};

struct entity_gate {
	int to_loc;		/* destination of gate */
	int notify_jumps;	/* whom to notify */
	int notify_unseal;	/* whom to notify */
	short seal_key;		/* numeric gate password */
	schar road_hidden;	/* this is a hidden road or passage */
};

struct entity_misc {
	char *display;		/* entity display banner */
	int npc_created;	/* turn peasant mob created */
	int npc_home;		/* where npc was created */
	int npc_cookie;		/* allocation cookie item for us */
	int summoned_by;	/* who summoned us? */
	char *save_name;	/* orig name of noble for dead bodies */
	int old_lord;		/* who did this dead body used to belong to */
	sparse npc_memory;
	int only_vulnerable;	/* only defeatable with this rare artifact */
	int garr_castle;	/* castle which owns this garrison */
	int bind_storm;		/* storm bound to this ship */

	short storm_str;	/* storm strength */
	schar npc_dir;		/* last direction npc moved */
	schar mine_delay;	/* time until collapsed mine vanishes */
	char cmd_allow;		/* unit under restricted control */

	schar opium_double;	/* improved opium production -- not saved */
	char **post_txt;	/* text of posted sign -- not saved */
	int storm_move;		/* next loc storm will move to -- not saved */
	ilist garr_watch;	/* units garrison watches for -- not saved */
	ilist garr_host;	/* units garrison will attack -- not saved */
	int garr_tax;		/* garrison taxes collected -- not saved */
	int garr_forward;	/* garrison taxes forwarded -- not saved */
};


/*
 *  In-process command structure
 */

#define	STATE_DONE	0
#define	STATE_LOAD	1
#define	STATE_RUN	2
#define	STATE_ERROR	3

struct wait_arg {
	int tag;
	int a1, a2;
	char *flag;
};

struct command {
	int who;		/* entity this is under (redundant) */
	int wait;		/* time until completion */
	int cmd;		/* index into cmd_tbl */

	int use_skill;		/* skill we are using, if any */
	int use_ent;		/* index into use_tbl[] for skill usage */
	int use_exp;		/* experience level at using this skill */
	int days_executing;	/* how long has this command been running */

	int a,b,c,d,e,f,g,h;	/* command arguments */

	char *line;		/* original command line */
	char *parsed_line;	/* cut-up line, pointed to by parse */
	char **parse;		/* ilist of parsed arguments */

	schar state;		/* STATE_LOAD, STATE_RUN, STATE_ERROR, STATE_DONE */
	schar status;		/* success or failure */
	schar poll;		/* call finish routine each day? */
	schar pri;		/* command priority or precedence */
	schar conditional;	/* 0=none 1=last succeeded 2=last failed */
	schar inhibit_finish;	/* don't call d_xxx */

	schar fuzzy;		/* command matched fuzzy -- not saved */
	schar second_wait;	/* delay resulting from auto attacks -- ns */
	struct wait_arg **wait_parse;	/* not saved */
	schar debug;		/* debugging check -- not saved */
};

#define	numargs(c)	(ilist_len(c->parse) - 1)

/*
 *  How long a command has been running
 */

#define	command_days(c)		(c->days_executing)


struct cmd_tbl_ent {
	char *allow;		/* who may execute the command */
	char *name;		/* name of command */

	int (*start)(struct command *);		/* initiator */
	int (*finish)(struct command *);	/* conclusion */
	int (*interrupt)(struct command *);	/* interrupted order */

	int time;		/* how long command takes */
	int poll;		/* call finish each day, not just at end */
	int pri;		/* command priority or precedence */
};


#define	BUY		1
#define	SELL		2
#define	PRODUCE		3
#define	CONSUME		4

struct trade {
	int kind;		/* BUY or SELL */
	int item;
	int qty;
	int cost;
	int cloak;		/* don't reveal identity of trader */
	int have_left;
	int month_prod;		/* month city produces item */
	int expire;		/* countdown timer for tradegoods */
	int who;		/* redundant -- not saved */
	int sort;		/* temp key for sorting -- not saved */
};


struct admit {
	int targ;		/* char or loc admit is declared for */
	int sense;		/* 0=default no, 1=all but.. */
	ilist l;

	int flag;		/* first time set this turn -- not saved */
};

#define	if_malloc(p)		((p) ? (p) : ((p) = my_malloc(sizeof(*(p)))))

/*
 *  malloc-on-demand substructure references
 */

#define	p_loc_info(n)		(&bx[n]->x_loc_info)
#define	p_char(n)		if_malloc(bx[n]->x_char)
#define	p_loc(n)		if_malloc(bx[n]->x_loc)
#define	p_subloc(n)		if_malloc(bx[n]->x_subloc)
#define	p_item(n)		if_malloc(bx[n]->x_item)
#define	p_player(n)		if_malloc(bx[n]->x_player)
#define	p_skill(n)		if_malloc(bx[n]->x_skill)
#define	p_gate(n)		if_malloc(bx[n]->x_gate)
#define	p_misc(n)		if_malloc(bx[n]->x_misc)
#define	p_disp(n)		if_malloc(bx[n]->x_disp)
#define	p_command(n)		if_malloc(bx[n]->cmd)


/*
 *  "raw" pointers to substructures, may be NULL
 */

#define	rp_loc_info(n)		(&bx[n]->x_loc_info)
#define	rp_char(n)		(bx[n]->x_char)
#define	rp_loc(n)		(bx[n]->x_loc)
#define	rp_subloc(n)		(bx[n]->x_subloc)
#define	rp_item(n)		(bx[n]->x_item)
#define	rp_player(n)		(bx[n]->x_player)
#define	rp_skill(n)		(bx[n]->x_skill)
#define	rp_gate(n)		(bx[n]->x_gate)
#define	rp_misc(n)		(bx[n]->x_misc)
#define	rp_disp(n)		(bx[n]->x_disp)
#define	rp_command(n)		(bx[n]->cmd)


#define	rp_magic(n)		(rp_char(n) ? rp_char(n)->x_char_magic : NULL)
#define	p_magic(n)		if_malloc(p_char(n)->x_char_magic)

#define	rp_item_magic(n)	(rp_item(n) ? rp_item(n)->x_item_magic : NULL)
#define	p_item_magic(n)		if_malloc(p_item(n)->x_item_magic)

extern struct box **bx;
extern int box_head[];			/* head of x_next_kind chain */
extern int sub_head[];			/* head of x_next_sub chain */


#define	kind(n)		(((n) > 0 && (n) < MAX_BOXES && bx[n]) ? \
						bx[n]->kind : T_deleted)

#define	subkind(n)	(bx[n] ? bx[n]->skind : 0)
#define	valid_box(n)	(kind(n) != T_deleted)

#define	kind_first(n)	(box_head[(n)])
#define	kind_next(n)	(bx[(n)]->x_next_kind)

#define	sub_first(n)	(sub_head[(n)])
#define	sub_next(n)	(bx[(n)]->x_next_sub)

#define	is_loc_or_ship(n)	(kind(n) == T_loc || kind(n) == T_ship)
#define	is_ship(n)    (subkind(n) == sub_galley || subkind(n) == sub_roundship || subkind(n) == sub_raft)
#define	is_ship_notdone(n)    (subkind(n) == sub_galley_notdone || subkind(n) == sub_roundship_notdone || subkind(n) == sub_raft_notdone)
#define	is_ship_either(n)	(is_ship(n) || is_ship_notdone(n))


/*
 *	_moving indicates that the unit has initiated movement
 *	_gone indicates that the unit has actually left the locations,
 *	and should not be interacted with anymore.
 *
 *	The distinction allows zero time commands to interact with
 *	the entity on the day movement is begun.
 */

#define	ship_moving(n)	(rp_subloc(n) ? rp_subloc(n)->moving : 0)
#define	ship_gone(n)	(ship_moving(n) ? sysclock.days_since_epoch - \
					ship_moving(n) + evening : 0)

#define	char_moving(n)	(rp_char(n) ? rp_char(n)->moving : 0)

#if 0
#define	char_gone(n)	(char_moving(n) ? sysclock.days_since_epoch - \
					char_moving(n) + evening : 0)
#else
#define	char_gone(n)	(char_moving(n) ? 1 : 0)
#endif

#define player_split_lines(n)	(rp_player(n) ? rp_player(n)->split_lines : 0)
#define player_split_bytes(n)	(rp_player(n) ? rp_player(n)->split_bytes : 0)
#define	player_email(n)		(rp_player(n) ? rp_player(n)->email : NULL)
#define	times_paid(n)		(rp_player(n) ? rp_player(n)->times_paid : 0)
#define	player_public_turn(n)	(rp_player(n) ? rp_player(n)->public_turn : 0)
#define	player_format(n)	(rp_player(n) ? rp_player(n)->format : 0)
#define	player_notab(n)		(rp_player(n) ? rp_player(n)->notab : 0)
#define	player_compuserve(n)	(rp_player(n) ? rp_player(n)->compuserve : 0)
#define	player_broken_mailer(n)	(rp_player(n) ? rp_player(n)->broken_mailer : 0)
#define banner(n)		(rp_misc(n) ? rp_misc(n)->display : NULL)
#define storm_bind(n)		(rp_misc(n) ? rp_misc(n)->bind_storm : 0)
#define npc_program(n)		(rp_char(n) ? rp_char(n)->npc_prog : 0)
#define char_studied(n)		(rp_char(n) ? rp_char(n)->studied : 0)
#define char_guard(n)		(rp_char(n) ? rp_char(n)->guard : 0)
#define char_health(n)		(rp_char(n) ? rp_char(n)->health : 0)
#define char_sick(n)		(rp_char(n) ? rp_char(n)->sick : 0)
#define loyal_kind(n)		(rp_char(n) ? rp_char(n)->loy_kind : 0)
#define loyal_rate(n)		(rp_char(n) ? rp_char(n)->loy_rate : 0)
#define noble_item(n)		(rp_char(n) ? rp_char(n)->unit_item : 0)
#define char_new_lord(n)	(rp_char(n) ? rp_char(n)->new_lord : 0)
#define char_melt_me(n)		(rp_char(n) ? rp_char(n)->melt_me : 0)
#define char_behind(n)		(rp_char(n) ? rp_char(n)->behind : 0)
#define char_pray(n)		(rp_magic(n) ? rp_magic(n)->pray : 0)
#define char_hidden(n)		(rp_magic(n) ? rp_magic(n)->hide_self : 0)
#define vision_protect(n)	(rp_magic(n) ? rp_magic(n)->vis_protect : 0)
#define default_garrison(n)	(rp_magic(n) ? rp_magic(n)->default_garr : 0)
#define char_hide_mage(n)	(rp_magic(n) ? rp_magic(n)->hide_mage : 0)
#define char_cur_aura(n)	(rp_magic(n) ? rp_magic(n)->cur_aura : 0)
#define char_max_aura(n)	(rp_magic(n) ? rp_magic(n)->max_aura : 0)
#define reflect_blast(n)	(rp_magic(n) ? rp_magic(n)->aura_reflect : 0)
#define char_pledge(n)		(rp_magic(n) ? rp_magic(n)->pledge : 0)
#define char_auraculum(n)	(rp_magic(n) ? rp_magic(n)->auraculum : 0)
#define char_abil_shroud(n)	(rp_magic(n) ? rp_magic(n)->ability_shroud : 0)
#define board_fee(n)		(rp_magic(n) ? rp_magic(n)->fee : 0)
#define ferry_horn(n)		(rp_magic(n) ? rp_magic(n)->ferry_flag : 0)
#define loc_prominence(n)	(rp_subloc(n) ? rp_subloc(n)->prominence : 0)
#define loc_opium(n)		(rp_subloc(n) ? rp_subloc(n)->opium_econ : 0)
#define loc_barrier(n)		(rp_loc(n) ? rp_loc(n)->barrier : 0)
#define loc_shroud(n)		(rp_loc(n) ? rp_loc(n)->shroud : 0)
#define loc_civ(n)		(rp_loc(n) ? rp_loc(n)->civ : 0)
#define loc_sea_lane(n)		(rp_loc(n) ? rp_loc(n)->sea_lane : 0)
#define ship_cap_raw(n)		(rp_subloc(n) ? rp_subloc(n)->capacity : 0)
#define body_old_lord(n)	(rp_misc(n) ? rp_misc(n)->old_lord : 0)
#define gate_dist(n)		(rp_loc(n) ? rp_loc(n)->dist_from_gate : 0)
#define	road_dest(n)		(rp_gate(n) ? rp_gate(n)->to_loc : 0)
#define	road_hidden(n)		(rp_gate(n) ? rp_gate(n)->road_hidden : 0)
#define	item_animal(n)		(rp_item(n) ? rp_item(n)->animal : 0)
#define	item_prominent(n)	(rp_item(n) ? rp_item(n)->prominent : 0)
#define	item_weight(n)		(rp_item(n) ? rp_item(n)->weight : 0)
#define	item_price(n)		(rp_item(n) ? rp_item(n)->base_price : 0)
#define	item_unique(n)		(rp_item(n) ? rp_item(n)->who_has : 0)
#define	item_land_cap(n)	(rp_item(n) ? rp_item(n)->land_cap : 0)
#define	item_ride_cap(n)	(rp_item(n) ? rp_item(n)->ride_cap : 0)
#define	item_fly_cap(n)		(rp_item(n) ? rp_item(n)->fly_cap : 0)
#define	req_skill(n)		(rp_skill(n) ? rp_skill(n)->required_skill : 0)
#define char_persuaded(n)	(rp_char(n) ? rp_char(n)->persuaded : 0)
#define char_rank(n)		(rp_char(n) ? rp_char(n)->rank : 0)
#define	skill_produce(n)	(rp_skill(n) ? rp_skill(n)->produced : 0)
#define	skill_no_exp(n)		(rp_skill(n) ? rp_skill(n)->no_exp : 0)
#define	skill_np_req(n)		(rp_skill(n) ? rp_skill(n)->np_req : 0)
#define	skill_aura_req(n)	(rp_skill(n) ? rp_skill(n)->aura_req : 0)
#define	ship_has_ram(n)		(rp_subloc(n) ? rp_subloc(n)->galley_ram : 0)
#define	loc_link_open(n)	(rp_subloc(n) ? rp_subloc(n)->link_open : 0)
#define	loc_damage(n)		(rp_subloc(n) ? rp_subloc(n)->damage : 0)
#define	loc_defense(n)		(rp_subloc(n) ? rp_subloc(n)->defense : 0)
#define	loc_pillage(n)		(rp_subloc(n) ? rp_subloc(n)->loot : 0)
#define	recent_pillage(n)	(rp_subloc(n) ? rp_subloc(n)->recent_loot : 0)
#define	safe_haven(n)		(rp_subloc(n) ? rp_subloc(n)->safe: 0)
#define	major_city(n)		(rp_subloc(n) ? rp_subloc(n)->major : 0)
#define	uldim(n)		(rp_subloc(n) ? rp_subloc(n)->uldim_flag : 0)
#define	summerbridge(n)		(rp_subloc(n) ? rp_subloc(n)->summer_flag : 0)
#define	subloc_quest(n)		(rp_subloc(n) ? rp_subloc(n)->quest_late : 0)
#define	tunnel_depth(n)		(rp_subloc(n) ? rp_subloc(n)->tunnel_level : 0)
#define	learn_time(n)		(rp_skill(n) ? rp_skill(n)->time_to_learn : 0)
#define	player_np(n)		(rp_player(n) ? rp_player(n)->noble_points : 0)
#define	player_fast_study(n)	(rp_player(n) ? rp_player(n)->fast_study : 0)
#define	gate_seal(n)		(rp_gate(n) ? rp_gate(n)->seal_key : 0)
#define	gate_dest(n)		(rp_gate(n) ? rp_gate(n)->to_loc : 0)
#define	char_proj_cast(n)	(rp_magic(n) ? rp_magic(n)->project_cast : 0)
#define	char_quick_cast(n)	(rp_magic(n) ? rp_magic(n)->quick_cast : 0)
#define	is_magician(n)		(rp_magic(n) ? rp_magic(n)->magician : 0)
#define	weather_mage(n)		(rp_magic(n) ? rp_magic(n)->knows_weather : 0)
#define	garrison_castle(n)	(rp_misc(n) ? rp_misc(n)->garr_castle : 0)
#define	npc_last_dir(n)		(rp_misc(n) ? rp_misc(n)->npc_dir : 0)
#define	restricted_control(n)	(rp_misc(n) ? rp_misc(n)->cmd_allow : 0)
#define	item_capturable(n)	(rp_item(n) ? rp_item(n)->capturable : 0)
#define	storm_strength(n)	(rp_misc(n) ? rp_misc(n)->storm_str : 0)
#define	npc_summoner(n)		(rp_misc(n) ? rp_misc(n)->summoned_by : 0)
#define	char_break(n)		(rp_char(n) ? rp_char(n)->break_point : 0)
#define	only_defeatable(n)	(rp_misc(n) ? rp_misc(n)->only_vulnerable : 0)

#define	mine_depth(n)	  (rp_subloc(n) ? rp_subloc(n)->shaft_depth / 3 : 0)
#define	release_swear(n)  (rp_magic(n) ? rp_magic(n)->swear_on_release : 0)
#define	our_token(n)	  (rp_magic(n) ? rp_magic(n)->token : 0)
#define	castle_level(n)	  (rp_subloc(n) ? rp_subloc(n)->castle_lev : 0)

#define	item_token_num(n)  (rp_item_magic(n) ? rp_item_magic(n)->token_num : 0)
#define	item_token_ni(n)   (rp_item_magic(n) ? rp_item_magic(n)->token_ni : 0)

#define	item_lore(n)         (rp_item_magic(n) ? rp_item_magic(n)->lore : 0)
#define	item_use_key(n)      (rp_item_magic(n) ? rp_item_magic(n)->use_key : 0)
#define	item_creator(n)      (rp_item_magic(n) ? rp_item_magic(n)->creator : 0)
#define	item_aura(n)         (rp_item_magic(n) ? rp_item_magic(n)->aura : 0)
#define	item_creat_cloak(n)  (rp_item_magic(n) ? \
					rp_item_magic(n)->cloak_creator : 0)
#define	item_reg_cloak(n)    (rp_item_magic(n) ? \
					rp_item_magic(n)->cloak_region : 0)
#define	item_creat_loc(n)    (rp_item_magic(n) ? \
					rp_item_magic(n)->region_created : 0)
#define	item_curse_non(n)    (rp_item_magic(n) ? \
					rp_item_magic(n)->curse_loyalty : 0)

#define	item_attack(n)		(rp_item(n) ? rp_item(n)->attack : 0)
#define	item_defense(n)		(rp_item(n) ? rp_item(n)->defense : 0)
#define	item_missile(n)		(rp_item(n) ? rp_item(n)->missile : 0)

#define	char_attack(n)		(rp_char(n) ? rp_char(n)->attack : 0)
#define	char_defense(n)		(rp_char(n) ? rp_char(n)->defense : 0)
#define	char_missile(n)		(rp_char(n) ? rp_char(n)->missile : 0)

#define	item_attack_bonus(n)	\
		(rp_item_magic(n) ? rp_item_magic(n)->attack_bonus : 0)
#define	item_defense_bonus(n)	\
		(rp_item_magic(n) ? rp_item_magic(n)->defense_bonus : 0)
#define	item_missile_bonus(n)	\
		(rp_item_magic(n) ? rp_item_magic(n)->missile_bonus : 0)
#define	item_aura_bonus(n)	\
		(rp_item_magic(n) ? rp_item_magic(n)->aura_bonus : 0)

#define	item_relic_decay(n)	\
		(rp_item_magic(n) ? rp_item_magic(n)->relic_decay : 0)

#define	is_fighter(n)		(item_attack(n) || item_defense(n) || item_missile(n) || n == item_ghost_warrior)

#define man_item(n)	(rp_item(n) ? rp_item(n)->is_man_item : 0)
#define	is_priest(n)		has_skill((n), sk_religion)

/*
 *  Return exactly where a unit is
 *  May point to another character, a structure, or a region
 */

#define	loc(n)		(rp_loc_info(n)->where)


#include "loop.h"

/*
 *  Prototypes, defines and externs
 */

#include "code.h"
#include "dir.h"
#include "display.h"
#include "etc.h"
#include "garr.h"
#include "loc.h"
#include "order.h"
#include "sout.h"
#include "stack.h"
#include "swear.h"
#include "u.h"
#include "use.h"

extern char *libdir;

/*
 *  Saved in libdir/system:
 */

extern olytime sysclock;		/* current time in Olympia */
extern int indep_player;		/* independent unit player */
extern int gm_player;			/* The Fates */
extern int eat_pl;			/* Order scanner */
extern int skill_player;		/* Player for skill list report */
extern int post_has_been_run;
extern int tunnel_region;
extern int under_region;
extern int faery_region;
extern int faery_player;
extern int hades_region;
extern int hades_pit;			/* Pit of Hades */
extern int hades_player;
extern int nowhere_region;
extern int nowhere_loc;
extern int cloud_region;
extern int npc_pl;
extern int garr_pl;			/* garrison player */
extern int combat_pl;			/* combat log */
extern int garrison_magic;
extern int show_to_garrison;

#define	in_faery(n)		(region(n) == faery_region)
#define	in_hades(n)		(region(n) == hades_region)
#define	in_clouds(n)		(region(n) == cloud_region)

extern int immed_see_all;	/* override hidden-ness, for debugging */

#define	see_all(n)		immed_see_all


extern ilist trades_to_check;
extern char *kind_s[];
extern char *subkind_s[];
extern char *dir_s[];
extern char *loc_depth_s[];
extern char *short_dir_s[];
extern char *full_dir_s[];
extern char *month_names[];
extern char *entab(int);
extern int exit_opposite[];
extern int immediate;
extern int win_flag;
extern int indent;
extern int show_day;
extern struct cmd_tbl_ent cmd_tbl[];
extern int evening;			/* are we in the evening phase? */
extern char *from_host;
extern char *reply_host;
extern char *gm_address;
extern char *game_title;
extern char *game_url;
extern char *rules_url;
extern char *times_url;
extern char *htpasswd_loc;
extern int garrison_pay;
extern int auto_quit_turns;
extern int army_slow_factor;
extern ilist new_players;		/* new players added this turn */


#define		wait(n)		(rp_command(n) ? rp_command(n)->wait : 0)
#define		is_prisoner(n)	(rp_char(n) ? rp_char(n)->prisoner : FALSE)
#define		magic_skill(n)	(subkind(skill_school(n)) == sub_magic)

#define		add_s(n)	((n) == 1 ? "" : "s")
#define		add_ds(n)	n, ((n) == 1 ? "" : "s")

#define		alive(n)	(kind(n) == T_char)

#define		is_npc(n)	(subkind(n) || loyal_kind(n) == LOY_npc || loyal_kind(n) == LOY_summon)

#define	char_alone(n)	(stack_parent(n) == 0 && count_stack_any(n) == 1)
#define	char_really_hidden(n)	(char_hidden(n) && char_alone(n))

#define		CHAR_FIELD	6	/* field length for box_code_less */


#define	MAX_POST 60	/* max line length for posts and messages */

#define	MAX_CURRENT_AURA(n)	(max_eff_aura(n) * 5)


/*
 *  style() tags:
 */

/* default style is 0 (regular) */

#define	STYLE_TEXT	1
#define	STYLE_HTML	2
#define	STYLE_PREV	(-1)

extern void style(int n);

#define		RELIC_THRONE		401
#define		RELIC_CROWN		402
#define		RELIC_BTA_SKULL		403


#define		MAP_MT_OLY		15350

