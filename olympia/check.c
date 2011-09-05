
#include	<stdio.h>
#include	"z.h"
#include	"oly.h"


/*  check.c -- check database integrity and effect minor repairs */


/*
 *  1.  Go through every box.  If box claims it's in a location,
 *	but it doesn't show up in the here list of the location,
 *	then add it to the here list.
 *
 *  2.	Go through every location.  If the loc has a here list, see
 *	if each box in the list claims that its in that location.
 *	If not, remove it from the here list.
 *
 *  This scheme gives precendence to the location the unit claims
 *  to be in over the location's here-list of units.  If they
 *  disagree, we correct the database based on where the unit
 *  claims to be.
 */

static void
check_here()
{
    int i;
    int j;
    int where;

	loop_boxes(i)
	{
		where = loc(i);
		if (where > 0 && !in_here_list(where, i))
		{
		    fprintf(stderr, "\tcheck_here: adding [%d] "
					"to here list of [%d]\n", i, where);

		    add_to_here_list(where, i);
		}
	}
	next_box;

	loop_boxes(i)
	{
		loop_here(i, j)
		{
		    where = loc(j);
		    if (where != i)
		    {
			fprintf(stderr, "\tcheck_here: removing [%d] from "
				"here list of [%d]\n", j, i);

			remove_from_here_list(i, j);
		    }
		}
		next_here;
	}
	next_box;
}


/*
 *  1.	For every box, if that box is in a faction, then make sure
 *	the box appears in the faction's unit list. 
 *
 *  2.	For every faction, go through the unit list seeing if
 *	the units actually claim to be in the faction.  If not,
 *	remove them from the faction's unit list.
 *
 *  This scheme gives precedence to the Faction attribute
 *  over the player_ent's chars list.  If they disagree,
 *  the database will be corrected according to the what faction
 *  the unit claims to be in, over the faction's list of units.
 */

static void
check_swear()
{
    int i;
    int j;
    int over;
    extern int int_comp();

    loop_char(i)
    {
	    over = player(i);
	    if (over > 0 && !is_unit(over, i))
	    {
		fprintf(stderr, "\tcheck_swear: adding [%d] "
						"to player [%d]\n", i, over);
		ilist_append(&p_player(over)->units, i);

		qsort(p_player(over)->units,
				ilist_len(p_player(over)->units),
				sizeof(int),
				int_comp);
	    }
    }
    next_char;

    loop_player(i)
    {
	loop_units(i, j)
	{
		over = player(j);
		if (over != i)
		{
		    fprintf(stderr, "\tcheck_swear: removing [%d] from "
				"player list of [%d]\n", j, i);

		    ilist_rem_value(&p_player(i)->units, j);
		}
	}
	next_unit;
    }
    next_player;
}


static void
check_indep()
{
	int i;

	if (bx[indep_player] == NULL)
	{
		fprintf(stderr, "\tcheck_indep: creating independent "
					"player [%d]\n", indep_player);

		alloc_box(indep_player, T_player, sub_pl_npc);
	}

	assert(kind(indep_player) == T_player);

	if (*name(indep_player) == '\0')
		set_name(indep_player, "Independent player");

	loop_char(i)
	{
		if (player(i) == 0)
		{
			fprintf(stderr, "\tcheck_indep: swearing unit [%d] "
				"to %s\n", i, box_name(indep_player));

			set_lord(i, indep_player, LOY_unsworn, 0);
		}
	}
	next_char;
}


static void
check_gm()
{

	if (bx[gm_player] == NULL)
	{
		fprintf(stderr, "\tcheck_gm: creating gm "
					"player [%d]\n", gm_player);

		alloc_box(gm_player, T_player, sub_pl_system);
	}

	assert(kind(gm_player) == T_player);

	if (*name(gm_player) == '\0')
		set_name(gm_player, "Gamemaster");
}


static void
check_skill_player()
{

	if (bx[skill_player] == NULL)
	{
		fprintf(stderr, "\tcheck_skill_player: creating skill "
					"player [%d]\n", skill_player);

		alloc_box(skill_player, T_player, sub_pl_system);
	}

	assert(kind(skill_player) == T_player);

	if (*name(skill_player) == '\0')
		set_name(skill_player, "Skill list");
}


static void
check_eat_player()
{

	if (bx[eat_pl] == NULL)
	{
		fprintf(stderr, "\tcheck_eat_player: creating eat "
					"player [%d]\n", eat_pl);

		alloc_box(eat_pl, T_player, sub_pl_system);
	}

	assert(kind(eat_pl) == T_player);

	if (*name(eat_pl) == '\0')
		set_name(eat_pl, "Order eater");
}


static void
check_npc_player()
{

	if (bx[npc_pl] == NULL)
	{
		fprintf(stderr, "\tcheck_npc_player: creating npc "
					"player [%d]\n", npc_pl);

		alloc_box(npc_pl, T_player, sub_pl_silent);
	}

	assert(kind(npc_pl) == T_player);

	if (*name(npc_pl) == '\0')
		set_name(npc_pl, "NPC control");
}


static void
check_garr_player()
{

	if (bx[garr_pl] == NULL)
	{
		fprintf(stderr, "\tcheck_garr_player: creating garrison "
					"player [%d]\n", garr_pl);

		alloc_box(garr_pl, T_player, sub_pl_silent);
	}

	assert(kind(garr_pl) == T_player);

	if (*name(garr_pl) == '\0')
		set_name(garr_pl, "Garrison units");
}


/*
 *  1.	Check that T_MAX and kind_s agree
 *  2.  Check that SUB_MAX and subkind_s agre
 */

static void
check_glob()
{
	int i;

	for (i = 1; kind_s[i] != NULL; i++)
		;
	assert(i == T_MAX);

	for (i = 1; subkind_s[i] != NULL; i++)
		;
	assert(i == SUB_MAX);
}


static void
check_nowhere()
{
	int i;

/*
 *  Not thorough enough?  What about other entity types?  sublocs, etc.
 */

	loop_char(i)
	{
		if (loc(i) == 0)
		{
			fprintf(stderr, "\twarning: unit %s is nowhere\n",
						box_code(i));
		}
	}
	next_char;

	loop_loc_or_ship(i)
	{
		if (loc_depth(i) > LOC_region && loc(i) == 0)
		{
			fprintf(stderr, "\twarning: loc %s is nowhere\n",
						box_code(i));
		}
	}
	next_loc_or_ship;
}



static void
check_skills()
{
	int sk;
	struct entity_skill *p;
	int i;

	loop_skill(sk)
	{
		if (sk >= 9000 && skill_school(sk) == sk)
			fprintf(stderr, "\twarning: orphaned subskill %s\n",
					box_code(sk));
		bx[sk]->temp = 0;
	}
	next_skill;

	loop_skill(sk)
	{
		p = rp_skill(sk);

		if (learn_time(sk) == 0)
			fprintf(stderr, "\twarning: learn time of %s is 0\n",
						box_name(sk));

		if (p == NULL)
			continue;

		for (i = 0; i < ilist_len(p->offered); i++)
		{
			if (bx[p->offered[i]]->temp)
			{
				fprintf(stderr, "\twarning: both %s and %s "
						"offer skill %d\n",
					box_name(sk),
					box_name(bx[p->offered[i]]->temp), 
					p->offered[i]);
			}
			else
				bx[p->offered[i]]->temp = sk;

			if (skill_school(p->offered[i]) != sk)
			{
				fprintf(stderr, "\twarning: %s offers %d, "
						"but %d is in school %d\n",
					box_name(sk), p->offered[i],
					p->offered[i],
					skill_school(p->offered[i]));
			}
		}

		for (i = 0; i < ilist_len(p->research); i++)
		{
			if (bx[p->research[i]]->temp)
			{
				fprintf(stderr, "\twarning: both %s and %s "
						"offer skill %d\n",
					box_name(sk),
					box_name(bx[p->research[i]]->temp), 
					p->research[i]);
			}
			else
				bx[p->research[i]]->temp = sk;

			if (skill_school(p->research[i]) != sk)
			{
				fprintf(stderr, "\twarning: %s offers %d, "
						"but %d is in school %d\n",
					box_name(sk), p->research[i],
					p->research[i],
					skill_school(p->research[i]));
			}
		}
	}
	next_skill;

	loop_skill(sk)
	{
		if (skill_school(sk) == sk)
			continue;

		if (bx[sk]->temp == 0)
			fprintf(stderr, "\twarning: non-offered skill %s\n",
						box_name(sk));
	}
	next_skill;
}


static void
check_item_counts()
{
	int i;
	struct item_ent *e;

	clear_temps(T_item);

	loop_boxes(i)
	{
		loop_inv(i, e)
		{
			if (kind(e->item) != T_item)
			{
				fprintf(stderr, "\t%s has non-item %s\n",
							box_name(i),
							box_name(e->item));
				continue;
			}

			if (!item_unique(e->item))
				continue;

			if (item_unique(e->item) != i)
			{
				fprintf(stderr, "\tunique item %s: "
					"whohas=%s, actual=%s\n",
					box_name(e->item),
					box_name(item_unique(e->item)),
					box_name(i));
				p_item(e->item)->who_has = i;
			}

			if (e->qty != 1)
				fprintf(stderr, "\t%s has qty %d of "
					"unique item %s\n",
					box_name(i),
					e->qty,
					box_name(e->item));

			bx[e->item]->temp += e->qty;
		}
		next_inv;
	}
	next_box;

	loop_item(i)
	{
		if (item_unique(i))
		{
			if (bx[i]->temp != 1)
				fprintf(stderr, "\tunique item %s count %d\n",
						box_name(i),
						bx[i]->temp);
		}
	}
	next_item;
}


static void
check_loc_name_lengths()
{
	int i;
	int len;

	loop_loc(i)
	{
		len = strlen(just_name(i));
		if (len > 25)
			fprintf(stderr, "\twarning: %s name too long\n",
						box_name(i));
	}
	next_loc;
}


static void
check_moving()
{
	int i;
	struct command *c;
	int leader;

	loop_char(i)
	{
		if (stack_leader(i) != i || !char_moving(i))
			continue;

		c = rp_command(i);

		if (c == NULL || c->state != RUN)
		{
			fprintf(stderr, "\t%s moving but no command\n",
						box_name(i));

			restore_stack_actions(i);
		}
	}
	next_char;

	loop_char(i)
	{
		leader = stack_leader(i);

		if (leader == i || char_moving(i) == char_moving(leader))
			continue;

		fprintf(stderr, "\t%s moving disagrees with leader\n",
						box_name(i));
		p_char(i)->moving = char_moving(leader);
	}
	next_char;
}


static void
check_prisoner()
{
	int who;

	loop_char(who)
	{
		if (!is_prisoner(who))
			continue;

		if (stack_parent(who) == 0)
		{
			fprintf(stderr, "\t%s prisoner but unstacked\n", box_name(who));
			p_char(who)->prisoner = FALSE;
		}
	}
	next_char;
}


/*
 *  Check database integrity.  Fixes minor problem in backlinks and lists.
 *  Always notes a database correction with a message to strerr.
 */

check_db()
{

	stage("check_db()");

	check_glob();
	check_here();
	check_swear();
	check_indep();
	check_gm();
	check_skill_player();
	check_eat_player();
	check_npc_player();
	check_garr_player();
	check_nowhere();
	check_skills();
	check_item_counts();
	check_loc_name_lengths();
	check_moving();
	check_prisoner();

	if (bx[garrison_magic] != NULL)
	{
		fprintf(stderr, "\twarning: %s should not be allocated,\n",
					box_name(garrison_magic));
		fprintf(stderr, "\t\treserved for garrison_magic\n");
	}
}

