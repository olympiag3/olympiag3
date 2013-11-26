
#include	<stdio.h>
#include	"z.h"
#include	"oly.h"


int subloc_player = 0;

int nowhere_region = 0;
int nowhere_loc = 0;


void
create_nowhere()
{

	nowhere_region = new_ent(T_loc, sub_region);
	set_name(nowhere_region, "Nowhere");

	nowhere_loc = new_ent(T_loc, sub_under);
	set_name(nowhere_loc, "Nowhere");
	set_where(nowhere_loc, nowhere_region);

	fprintf(stderr, "INIT: created %s, %s\n",
				box_name(nowhere_region),
				box_name(nowhere_loc));
}


static void
create_a_relic(int n, char *name, int use, int weight)
{

	if (bx[n] != NULL)
		return;

	create_unique_item_alloc(n, nowhere_loc, sub_relic); 

	set_name(n, name);
	p_item_magic(n)->use_key = use;
	p_item(n)->weight = weight;

	fprintf(stderr, "Created %s\n", box_name(n));
}


void
create_relics()
{
	create_a_relic(RELIC_THRONE, "Imperial Throne", 0, 500);
	create_a_relic(RELIC_CROWN, "Crown of Prosperity", 0, 10);
	create_a_relic(RELIC_BTA_SKULL, "Skull of Bastrestric",
				use_bta_skull, 10);

#if 0
	create_a_relic(401, "Dagger of Fire", use_dagger_of_fire);
	create_a_relic(402, "Orb of Darkness", use_orb_of_darkness);
	create_a_relic(403, "Horn of Fear", use_horn_of_fear);
	create_a_relic(405, "Key stone", use_key_stone);
#endif
}


int
random_unassigned_relic()
{
	struct item_ent *e;
	int sum = 0;
	int choice;
	int item = 0;

	loop_inv(nowhere_loc, e)
	{
		if (!item_unique(e->item) || subkind(e->item) != sub_relic)
			continue;

		sum++;
	}
	next_inv;

	if (sum == 0)
		return 0;

	choice = rnd(1, sum);

	loop_inv(nowhere_loc, e)
	{
		if (!item_unique(e->item) || subkind(e->item) != sub_relic)
			continue;

		choice--;

		if (choice == 0)
		{
			item = e->item;
			break;
		}
	}
	next_inv;

	return item;
}


static char *art_att_s[] = {"sword", "dagger", "longsword"};
static char *art_def_s[] = {"helm", "shield", "armor"};
static char *art_mis_s[] = {"spear", "bow", "javelin", "dart"};
static char *art_mag_s[] = {"ring", "staff", "amulet"};

static char *pref[] = {"magic", "golden", "crystal", "enchanted", "elven"};

static char *of_names[] = {
	"Achilles", "Darkness", "Justice", "Truth", "Norbus", "Dirbrand",
	"Pyrellica", "Halhere", "Eadak", "Faelgrar", "Napich", "Renfast",
	"Ociera", "Shavnor", "Dezarne", "Roshun", "Areth Lorbin", "Anarth",
	"Vernolt", "Pentara", "Gravecarn", "Sardis", "Lethrys", "Habyn",
	"Obraed", "Beebas", "Bayarth", "Haim", "Balatea", "Bobbiek", "Moldarth",
	"Grindor", "Sallen", "Ferenth", "Rhonius", "Ragnar", "Pallia", "Kior",
	"Baraxes", "Coinbalth", "Raskold", "Lassan", "Haemfrith", "Earnberict",
	"Sorale", "Lorbin", "Osgea", "Fornil", "Kuneack", "Davchar", "Urvil",
	"Pantarastar", "Cyllenedos", "Echaliatic", "Iniera", "Norgar", "Broen",
	"Estbeorn", "Claunecar", "Salamus", "Rhovanth", "Illinod", "Pictar",
	"Elakain", "Antresk", "Kichea", "Raigor", "Pactra", "Aethelarn",
	"Descarq", "Plagcath", "Nuncarth", "Petelinus", "Cospera", "Sarindor",
	"Albrand", "Evinob", "Dafarik", "Haemin", "Resh", "Tarvik", "Odasgunn",
	"Areth Pirn", "Miranth", "Dorenth", "Arkaune", "Kircarth", "Perendor",
	"Syssale", "Aelbarik", "Drassa", "Pirn", "Maire", "Lebrus", "Birdan",
	"Fistrock", "Shotluth", "Aldain", "Nantasarn", "Carim", "Ollayos",
	"Hamish", "Sudabuk", "Belgarth", "Woodhead",
	NULL
};


/*
 *  Create a scroll which teaches a researcharable
 *  subskill that we don't know, in a category that we know
 *  (preferably).
 */

static void
make_teach_book(int who, int questor)
{
	int i;
	int sk;
	ilist candidate = NULL;		/* categories we know */
	ilist candidate2 = NULL;	/* categories we don't know */
	struct entity_skill *q;
	int new;
	char *s;
	struct item_magic *p;

	loop_skill(sk)
	{
		if (skill_school(sk) != sk)
			continue;

		q = rp_skill(sk);

		if (!q)
			continue;

		if (has_skill(questor, sk))
			for (i = 0; i < ilist_len(q->research); i++)
			{
				if (has_skill(questor, q->research[i]))
					continue;

				ilist_append(&candidate, q->research[i]);
			}
		else if (sk != sk_adv_sorcery)
			ilist_append(&candidate2, sk);
	}
	next_skill;

	if (ilist_len(candidate) > 0)
	{
		ilist_scramble(candidate);
		sk = candidate[0];
	}
	else if (ilist_len(candidate2) > 0)
	{
		ilist_scramble(candidate2);
		sk = candidate2[0];
	}
	else
	{
		log_write(LOG_CODE, "?? %s knows all skills?", box_code(questor));
		fprintf(stderr, "?? %s knows all skills?", box_code(questor));
		return;
	}

	ilist_reclaim(&candidate);
	ilist_reclaim(&candidate2);

	new = create_unique_item(who, sub_scroll);
	set_name(new, "ancient scroll");
	p_item(new)->weight = 5;
	p = p_item_magic(new);
	ilist_append(&p->may_study, sk);
}


#if 0
/*
 *  Find an artifact in our region held by a subloc monster
 *  which is not only-defeatable by another artifact.
 */

static int
free_artifact(int where)
{
	int reg = region(where);
	int i;
	int owner;
	ilist l = NULL;
	int ret;

	loop_item(i)
	{
		if (subkind(i) != sub_artifact)
			continue;

		owner = item_unique(i);
		assert(owner);

		if (region(owner) != reg)
			continue;

		if (!is_npc(owner) ||
		    npc_program(owner) != PROG_subloc_monster)
			continue;

		if (only_defeatable(owner))
			continue;

		ilist_append(&l, i);
	}
	next_item;

	if (ilist_len(l) == 0)
		return 0;

	ret = l[rnd(0,ilist_len(l)-1)];

	ilist_reclaim(&l);

	return ret;
}
#endif


static int
new_artifact(int who)
{
	int new;
	char *s;

	new = create_unique_item(who, sub_artifact);

	switch (rnd(1,4))
	{
	case 1:
		s = art_att_s[rnd(0,2)];
		p_item_magic(new)->attack_bonus = rnd(1,10) * 5;
		break;

	case 2:
		s = art_def_s[rnd(0,2)];
		p_item_magic(new)->defense_bonus = rnd(1,10) * 5;
		break;

	case 3:
		s = art_mis_s[rnd(0,3)];
		p_item_magic(new)->missile_bonus = rnd(1,10) * 5;
		break;

	case 4:
		s = art_mag_s[rnd(0,2)];
		p_item_magic(new)->aura_bonus = rnd(1,3);
		break;

	default:
		assert(FALSE);
	}

	if (rnd(1,3) < 3)
	{
		s = sout("%s %s", pref[rnd(0,4)], s);
	}
	else
	{
		int i;

		for (i = 0; of_names[i]; i++)
			;
		i = rnd(0, i-1);

		s = sout("%s of %s", cap(s), of_names[i]);
	}

	p_item(new)->weight = 10;
	set_name(new, s);

	return new;
}


struct quest_monster {
	int terr;
	int item;
	int low, high;
	int level;			/* level for sub_chamber */
} quest_monster [] = {
	{sub_island,	item_pirate,	  5, 30,	0},
	{sub_island,	item_cyclops,	  1, 5,		0},

	{sub_cave,	item_rat,	  10, 50,	0},
	{sub_cave,	item_wolf,	  3, 10,	0},
	{sub_cave,	item_ratspider,	  5, 20,	0},
	{sub_cave,	item_gorgon,	  3, 5,		0},
	{sub_cave,	item_orc,	  5, 20,	0},

	{sub_ruins,	item_bandit,	  2, 10,	0},
	{sub_ruins,	item_cyclops,	  2, 5,		0},
	{sub_ruins,	item_minotaur,	  3, 10,	0},
	{sub_ruins,	item_centaur,	  3, 10,	0},
	{sub_ruins,	item_lizard,	  1, 3,		0},

	{sub_battlefield, item_skeleton,  10, 100,	0},
	{sub_battlefield, item_spirit,	  5, 50,	0},
	{sub_battlefield, item_giant,	  3, 10,	0},
	{sub_battlefield, item_nazgul,	  5, 20,	0},

	{sub_graveyard,	item_corpse,	  10, 100,	0},
	{sub_graveyard,	item_harpie,	  3, 10,	0},
	{sub_graveyard,	item_spider,	  3, 10,	0},
	{sub_graveyard,	item_bird,	  1, 3,		0},

	{sub_lair,	item_lion,	  3, 8,		0},
	{sub_lair,	item_chimera,	  2, 10,	0},
	{sub_lair,	item_dragon,	  1, 1,		0},

	{sub_ench_forest, item_faery,	  5, 20,	0},
	{sub_ench_forest, item_elf,	  5, 20,	0},

	{sub_faery_hill,  item_faery,	  5, 20,	0},
	{sub_faery_hill,  item_elf,	  5, 20,	0},

	{sub_pits,	item_rat,	  5, 25,	0},
	{sub_pits,	item_gorgon,	  3, 7,		0},
	{sub_pits,	item_cyclops,	  2, 3,		0},
	{sub_pits,	item_minotaur,	  1, 5,		0},

	{sub_bog,	item_rat,	  5, 25,	0},
	{sub_bog,	item_gorgon,	  3, 7,		0},
	{sub_bog,	item_cyclops,	  2, 3,		0},
	{sub_bog,	item_minotaur,	  1, 5,		0},

	{sub_sand_pit,	item_lizard,	  15, 30,	0},

	{sub_stone_cir,	item_skeleton,	  3, 15,	0},
	{sub_stone_cir,	item_gorgon,	  3, 15,	0},
	{sub_stone_cir,	item_orc,	  3, 15,	0},
	{sub_stone_cir,	item_cyclops,	  3, 15,	0},
	{sub_stone_cir,	item_minotaur,	  3, 15,	0},
	{sub_stone_cir,	item_centaur,	  3, 15,	0},
	{sub_stone_cir,	item_spirit,	  3, 15,	0},
	{sub_stone_cir,	item_nazgul,	  3, 15,	0},
	{sub_stone_cir,	item_harpie,	  3, 15,	0},
	{sub_stone_cir,	item_chimera,	  3, 15,	0},
	{sub_stone_cir,	item_nazgul,	  3, 15,	0},

	{sub_chamber,	item_rat,	  10, 50,	0},
	{sub_chamber,	item_ratspider,	  5, 20,	0},
	{sub_chamber,	item_gorgon,	  3, 6,		0},
	{sub_chamber,	item_orc,	  5, 20,	0},

	{sub_chamber,	item_cyclops,	  5, 10,	4},
	{sub_chamber,	item_minotaur,	  5, 15,	4},
	{sub_chamber,	item_spider,	  5, 15,	4},
	{sub_chamber,	item_lizard,	  5, 15,	4},

	{0,		0,		  0, 0,		0},
};


int
choose_quest_monster(int where)
{
	int terr = subkind(where);
	int level = tunnel_depth(where);
	int sum = 0, choice;
	int i;

	for (i = 0; quest_monster[i].terr; i++)
		if (quest_monster[i].terr == terr &&
		    level >= quest_monster[i].level)
			sum++;

	assert(sum > 0);

	choice = rnd(1, sum);

	for (i = 0; quest_monster[i].terr; i++)
		if (quest_monster[i].terr == terr &&
		    level >= quest_monster[i].level)
		{
			choice--;
			if (choice == 0)
				return i;
		}

	assert(FALSE);

	return 0;
}


static int
new_monster(int where)
{
	int i;
	int new;

	i = choose_quest_monster(where);

	if (i < 0)
		return 0;

	new = new_char(sub_ni, quest_monster[i].item, where,
			-1, npc_pl, LOY_npc, 0, NULL);

	gen_item(new,	
		quest_monster[i].item,
		rnd(quest_monster[i].low-1, quest_monster[i].high-1));

	p_char(new)->npc_prog = PROG_subloc_monster;

	return new;
}


int
make_subloc_monster(int where, int questor)
{
	int monster;
	int relic = 0;
	int low = 2;

	monster = new_monster(where);

	if (monster == 0)
		return 0;

/*
 *  Give monster a treasure
 */

	relic = random_unassigned_relic();

	if (relic)
		low = 0;

	gen_item(monster, item_gold, rnd(100, 500));

	switch (rnd(low, 18))
	{
	case 0:
	case 1:
		assert(relic);
		move_item(nowhere_loc, monster, relic, 1);

		switch (relic)
		{
		case RELIC_CROWN:
			p_item_magic(relic)->relic_decay = rnd(8,16)+1;
			break;

		case RELIC_BTA_SKULL:
			p_item_magic(relic)->relic_decay = rnd(10,20)+1;
			break;
		}
		break;

	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		gen_item(monster, item_gold, rnd(100, 3000));
		break;

	case 7:
	case 8:
	{
		int art;

		art = new_artifact(monster);
		break;
	}

	case 9:
	case 10:
	{
		gen_item(monster, item_elfstone, 1);
		break;
	}

	case 11:
	{
		int item;

		if (rnd(0,1) == 0)
			item = create_npc_token(monster);
		break;
	}

	case 12:
	case 13:
	{
		make_teach_book(monster, questor);
		break;
	}

	case 14:
	case 15:
	{
		int item;

		item = new_orb(monster);
		break;
	}

	case 16:
	case 17:
	{
		int item;

		item = item_pegasus;

		gen_item(monster, item, rnd(1, 6));
		break;
	}

	case 18:	/* no treasure */
		break;

	default:
		assert(FALSE);
	}

#if 0
	if (rnd(1,6) == 1)
	{
		int item;

/*
 *  Temporarily set only_vulnerable for ourselves so we don't
 *  have a circular problem.  free_artifact() will take care of
 *  skipping over other only_vulnerable's.
 */
		p_misc(monster)->only_vulnerable = 1;
		item = free_artifact(monster);

		if (item)
			rp_misc(monster)->only_vulnerable = item;
		else
			rp_misc(monster)->only_vulnerable = 0;
	}
#endif

	return monster;
}


int
v_quest(struct command *c)
{
	int where = subloc(c->who);
	int terr = subkind(where);
	int i;

	if (in_safe_now(c->who))
	{
		wout(c->who, "Questing is not permitted in safe havens.");
		return FALSE;
	}

	if (stack_leader(c->who) != c->who)
	{
		wout(c->who, "Only the stack leader may initiate quests.");
		return FALSE;
	}

	for (i = 0; quest_monster[i].terr; i++)
		if (terr == quest_monster[i].terr)
			break;

	if (quest_monster[i].terr == 0)
	{
		wout(c->who, "There are no quests to be found here.");
		return FALSE;
	}

	if (subloc_quest(where)) {
		wout(c->who, "This area has been quested in recently, "
			"nothing of interest is found.");
		return FALSE;
	}

	return TRUE;
}


int
d_quest(struct command *c)
{
	int where = subloc(c->who);
	int monster;
	int terr = subkind(where);
	int i;
	int ret;

	if (in_safe_now(c->who))
	{
		wout(c->who, "Questing is not permitted in safe havens.");
		return FALSE;
	}

	if (stack_leader(c->who) != c->who)
	{
		wout(c->who, "Only the stack leader may initiate quests.");
		return FALSE;
	}

	for (i = 0; quest_monster[i].terr; i++)
		if (terr == quest_monster[i].terr)
			break;

	if (quest_monster[i].terr == 0)
	{
		wout(c->who, "There are no quests to be found here.");
		return FALSE;
	}

	if (subloc_quest(where))
	{
		wout(c->who, "This area has been quested in recently, "
				"nothing of interest is found.");
		return TRUE;
	}

	if (rnd(1,100) <= 50)
	{
		wout(c->who, "Nothing of interest was found.");
		return TRUE;
	}

	p_subloc(where)->quest_late = rnd(5,13);   /* no quests following 4-12 turns */

	monster = make_subloc_monster(where, c->who);

	if (monster == 0)
	{
		wout(c->who, "Internal error.");
		return TRUE;
	}

	wout(c->who, "%s discovers %s", box_name(c->who),
			liner_desc(monster));

	ret = oly_parse(c, sout("attack %s", box_code_less(monster)));
	assert(ret);

	return v_attack(c);
}


int
v_use_bta_skull(struct command *c)
{
	int item = c->a;

	assert(item == RELIC_BTA_SKULL);

	p_item_magic(item)->relic_decay = 0;
	move_item(c->who, nowhere_loc, item, 1);

	if (!is_magician(c->who) || rnd(1,100) <= 25)
	{
		wout(c->who, "The skull erupts with an intense blast of aura,"
			" killing %s!", just_name(c->who));
		kill_char(c->who, MATES);
	}
	else
	{
		int aura = rnd(50,75);

		wout(c->who, "The skull radiates a burst of %s aura!",
				comma_num(aura));

		p_magic(c->who)->cur_aura += aura;
		limit_cur_aura(c->who);

		wout(c->who, "Current aura is now %d.",
					rp_magic(c->who)->cur_aura);
		wout(c->who, "The skull has vanished.");
	}

	return TRUE;
}

