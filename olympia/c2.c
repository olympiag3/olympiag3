
#include	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include	<time.h>
#include	"z.h"
#include	"oly.h"



int
v_discard(struct command *c)
{
	int item = c->a;
	int qty = c->b;
	int have_left = c->c;
	int ret;

	if (kind(item) != T_item)
	{
		wout(c->who, "%s is not an item.", box_code(item));
		return FALSE;
	}

	if (has_item(c->who, item) < 1)
	{
		wout(c->who, "%s does not have any %s.", box_name(c->who),
						box_code(item));
		return FALSE;
	}

	qty = how_many(c->who, c->who, item, qty, have_left);
	
	if (qty <= 0)
		return FALSE;

	ret = drop_item(c->who, item, qty);
	assert(ret);

	wout(c->who, "Dropped.");

	return TRUE;
}


void
drop_player(int pl)
{
	int who;
	char *s = "";
	char *email = "";
	extern int save_flag;
	char cmd[LEN];
	int owner;
	int i;

	assert(kind(pl) == T_player);

	loop_units(pl, who)
	{
		if (is_prisoner(who))
		{
			unit_deserts(who, indep_player, TRUE, LOY_UNCHANGED, 0);
		}
		else
		{
			wout(subloc(who), "%s melts into the ground and vanishes.", box_name(who));
			char_reclaim(who);
		}
	}
	next_unit;

/*
 *  Immediately rot any dead bodies belonging to the dropped player.
 */

	loop_dead_body(i)
	{
		owner = item_unique(i);
		assert(owner);

		if (rp_misc(i) == NULL || rp_misc(i)->old_lord != pl)
			continue;

#if 1
		p_misc(i)->old_lord = indep_player;
#else
		if (kind(owner) == T_char)
			wout(owner, "%s decomposed.", box_name(i));

		destroy_unique_item(owner, i);
#endif
	}
	next_dead_body;

	if (rp_player(pl))
	{
		if (rp_player(pl)->email && *rp_player(pl)->email)
			email = rp_player(pl)->email;

		if (rp_player(pl)->full_name && *rp_player(pl)->full_name)
			s = rp_player(pl)->full_name;
	}

	log_write(LOG_DROP, "Dropped player %s", box_name(pl));
	log_write(LOG_DROP, "    %s <%s>", s, email);

	if (save_flag)
	{
		sprintf(cmd, "/u/oly/bin/acct -p %s -g g2 -d", box_code_less(pl));
		system(cmd);
	}

	delete_box(pl);
}


int
v_quit(struct command *c)
{
	int target = c->a;

	if (target == 0)
		target = player(c->who);

	if (target != c->who && c->who != gm_player)
	{
		wout(c->who, "Not allowed to drop another player.");
		return FALSE;
	}

	if (kind(target) != T_player)
	{
		wout(c->who, "%s is not a player.", box_name(target));
		return FALSE;
	}

	drop_player(target);

/*
 *  Don't call finish_command, we don't exist anymore
 */

	return FALSE;
}


void
text_list_free(char **l)
{
	int i;

	for (i = 0; i < plist_len(l); i++)
		my_free(l[i]);
}


static int
line_length_check(char **l)
{
	int i;
	int len = 0;

	for (i = 0; i < plist_len(l); i++)
		len = max(len, strlen(l[i]));

	return len;
}


static char **
parse_text_list(struct command *c)
{
	int lines = c->a;
	char **l = NULL;
	char *order;
	int pl = player(c->who);
	int done = FALSE;
	char *t;

	if (lines == 0)
	{
		while (!done)
		{
			order = top_order(pl, c->who);

			if (order == NULL)
			{
				wout(c->who, "Ran out of posting text.");
				text_list_free(l);
				return NULL;
			}

			t = eat_leading_trailing_whitespace(order);

			if (i_strcmp(t, "end") == 0)
				done = TRUE;
			else
				plist_append((plist *) &l, str_save(order));
			pop_order(pl, c->who);
		}
	}
	else
	{
		while (lines > 0)
		{
			order = top_order(pl, c->who);

			if (order == NULL)
			{
				wout(c->who, "Ran out of posting text.");
				text_list_free(l);
				return NULL;
			}

			plist_append((plist *) &l, str_save(order));
			pop_order(pl, c->who);

			lines--;
		}
	}

	return l;
}


int
v_post(struct command *c)
{
	char **l = NULL;
	int new;

	l = parse_text_list(c);

	if (l == NULL)
		return FALSE;

	if (line_length_check(l) > MAX_POST)
	{
		wout(c->who, "Line length of posted text exceeds %d "
				"characters.", MAX_POST);
		wout(c->who, "Post rejected.");
		text_list_free(l);
		return FALSE;
	}

	new = new_ent(T_post, 0);

	p_item_magic(new)->creator = c->who;
	set_where(new, subloc(c->who));
	p_misc(new)->post_txt = l;

	return TRUE;
}


int
v_message(struct command *c)
{
	int targ = c->b;
	char **l = NULL;
	int first;
	int i;

	l = parse_text_list(c);

	if (l == NULL)
		return FALSE;

	if (line_length_check(l) > MAX_POST)
	{
		wout(c->who, "Line length of message text exceeds %d "
				"characters.", MAX_POST);
		wout(c->who, "Message rejected.");
		text_list_free(l);
		return FALSE;
	}

	if (!valid_box(targ))
	{
		wout(c->who, "%s is not a valid entity.", box_code(targ));
		text_list_free(l);
		return FALSE;
	}

	if (kind(targ) != T_char)
	{
		wout(c->who, "May not send a message to %s.", box_code(targ));
		text_list_free(l);
		return FALSE;
	}

	wout(targ, "Received a message from %s:", box_name(c->who));

	indent += 3;
	first = TRUE;

	for (i = 0; i < plist_len(l); i++)
	{
		wout(targ, "%s%s%s",
				first ? "\"" : "",
				l[i],
				i+1 == plist_len(l) ? "\"" : "");

		if (first)
		{
			first = FALSE;
			indent += 1;
		}
	}

	if (!first)
		indent -= 1;

	indent -= 3;

	text_list_free(l);

	wout(c->who, "Message delivered.");
	return TRUE;
}


#define DASH_LINE "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n"


static FILE *press_fp = NULL;
static FILE *rumor_fp = NULL;

char *months[] = {
	"January", "Februrary", "March", "April", "May", "June", "July",
	"August", "September", "October", "November", "December",
};


void
open_times()
{
	char *fnam;

	if (press_fp == NULL)
	{
		fnam = sout("%s/times_press", libdir);
		press_fp = fopen(fnam, "w");
		if (press_fp == NULL)
		{
			fprintf(stderr, "can't write %s", fnam);
			exit(1);
		}
	}

	fprintf(press_fp, "\n");
	fprintf(press_fp, DASH_LINE);
	fprintf(press_fp, "\n                        Player-contributed press\n\n");
	fprintf(press_fp, DASH_LINE);
	fprintf(press_fp, "\n");
	fflush(press_fp);

	if (rumor_fp == NULL)
	{
		fnam = sout("%s/times_rumor", libdir);
		rumor_fp = fopen(fnam, "w");
		if (rumor_fp == NULL)
		{
			fprintf(stderr, "can't write %s", fnam);
			exit(1);
		}
	}

	fprintf(rumor_fp, "                                 Rumors\n\n");
	fprintf(rumor_fp, DASH_LINE);
	fprintf(rumor_fp, "\n");

	fflush(rumor_fp);
}


void
times_masthead()
{
	char *fnam;
	time_t l;
	struct tm *tm;
	char date[60];
	char turn_s[60];
	char issue_s[60];
	int nplayers = 0;
	int i;
	FILE *fp;

	fnam = sout("%s/times_0", libdir);
	fp = fopen(fnam, "w");
	if (fp == NULL)
	{
		fprintf(stderr, "can't write %s", fnam);
		exit(1);
	}

	time(&l);
	tm = localtime(&l);
	sprintf(date, "%s %d, %d",
		months[tm->tm_mon], tm->tm_mday, tm->tm_year + 1900);

	loop_player(i)
	{
		if (subkind(i) == sub_pl_regular)
			nplayers++;
	}
	next_player;

	sprintf(turn_s, "turn %d %d players", sysclock.turn, nplayers);
	sprintf(issue_s, "issue g3-%d", sysclock.turn);

	fprintf(fp, "From: g3@olympia.v-labs.be(Olympia g3)\n");
	fprintf(fp, "Subject: Times g3-%d\n", sysclock.turn);
	fprintf(fp, "To: g3@olympia.v-labs.be\n");
//	fprintf(fp, "Bcc: totimes-g2\n\n");
	fprintf(fp, "   +----------------------------------------------------------------------+\n");
	fprintf(fp, "   | The Olympia Times                                  %17s |\n", issue_s);
	fprintf(fp, "   | %-68s |\n", date);
	fprintf(fp, "   |                                                                      |\n");
	fprintf(fp, "   | %s !game_email! |\n", turn_s);
	fprintf(fp, "   +----------------------------------------------------------------------+\n\n");
	fprintf(fp, "               Questions, comments, to play:  http://olympia.v-labs.be/g3\n\n");
	fprintf(fp, "                             Olympia PBEM\n\n");
	fprintf(fp, "                                *  *  *\n\n");
	fprintf(fp, "                                *  *  *\n\n");

	fclose(fp);
}


void
close_times()
{

	if (press_fp != NULL)
	{
		fclose(press_fp);
		press_fp = NULL;
	}

	if (rumor_fp != NULL)
	{
		fclose(rumor_fp);
		rumor_fp = NULL;
	}
}


int
v_rumor(struct command *c)
{
	char **l = NULL;
	int i;

	l = parse_text_list(c);

	if (l == NULL)
		return FALSE;

	if (line_length_check(l) > 78)
	{
		wout(c->who, "Line length of message text exceeds %d "
				"characters.", 78);
		wout(c->who, "Post rejected.");
		text_list_free(l);
		return FALSE;
	}

	for (i = 0; i < plist_len(l); i++)
	{
		if (strncmp(l[i], "=-=-", 4) == 0)
			fprintf(rumor_fp, "> %s\n", l[i]);
		else
			fprintf(rumor_fp, "%s\n", l[i]);
	}

	fprintf(rumor_fp, "\n");
	fprintf(rumor_fp, DASH_LINE);
	fprintf(rumor_fp, "\n");
	fflush(rumor_fp);

	text_list_free(l);

	wout(c->who, "Rumor posted.");
	return TRUE;
}


int
v_press(struct command *c)
{
	char **l = NULL;
	int i;
	char attrib[100];
	char *t;

	l = parse_text_list(c);

	if (l == NULL)
		return FALSE;

	if (line_length_check(l) > 78)
	{
		wout(c->who, "Line length of message text exceeds %d "
				"characters.", 78);
		wout(c->who, "Post rejected.");
		text_list_free(l);
		return FALSE;
	}

	for (i = 0; i < plist_len(l); i++)
	{
		if (strncmp(l[i], "=-=-", 4) == 0)
			fprintf(press_fp, "> %s\n", l[i]);
		else
			fprintf(press_fp, "%s\n", l[i]);
	}


	sprintf(attrib, "-- %s", box_name(c->who));
	for (t = attrib; *t; t++)
		if (*t == '~')
			*t = ' ';
	fprintf(press_fp, "\n%55s\n\n", attrib);

	fprintf(press_fp, DASH_LINE);
	fprintf(press_fp, "\n");
	fflush(press_fp);

	text_list_free(l);

	wout(c->who, "Press posted.");
	return TRUE;
}


int
v_public(struct command *c)
{
	int pl = player(c->who);
	struct entity_player *p;

	if (player_public_turn(pl))
	{
		wout(c->who, "Already a public turn.");
		return FALSE;
	}

	p = p_player(pl);
	p->public_turn = 1;

	gen_item(pl, item_gold, 100);

	wout(c->who, "Received 100 CLAIM gold.");

	return TRUE;
}


int
v_improve_opium(struct command *c)
{
	int where = subloc(c->who);

	if (subkind(where) != sub_poppy_field)
	{
		wout(c->who, "Opium is produced only in poppy fields.");
		return FALSE;
	}

	return TRUE;
}


int
d_improve_opium(struct command *c)
{
	int where = subloc(c->who);

	if (subkind(where) != sub_poppy_field)
	{
		wout(c->who, "Not in a poppy field anymore.");
		return FALSE;
	}

	p_misc(where)->opium_double = TRUE;

	return TRUE;
}


int
v_die(struct command *c)
{

	kill_char(c->who, MATES);
	return TRUE;
}


int
v_format(struct command *c)
{
	int pl;

	pl = player(c->who);

	p_player(pl)->format = c->a;

	wout(c->who, "Formatting for %s set to %d.", box_name(pl), c->a);

	return TRUE;
}


int
v_notab(struct command *c)
{
	int pl;

	pl = player(c->who);

	p_player(pl)->notab = c->a;

	if (c->a)
		wout(c->who, "No TAB characters will appear in turn reports.");
	else
		wout(c->who, "TAB characters may appear in turn reports.");

	return TRUE;
}


int
v_stop(struct command *c)
{

	return TRUE;
}


int
v_archery(struct command *c)
{

	return TRUE;
}


int
d_archery(struct command *c)
{
	struct entity_char *p;
	int amount;

	p = p_char(c->who);

	if (rnd(1,100) <= 5)
		amount = 10;
	// by Cappinator:
	// Fixed bug where defense was checked instead of
	// missile rating when determining the random factor
	else if (p->missile < 100)
		amount = rnd(3,5);
	else
		amount = rnd(1,3);

	p->missile += amount;

	wout(c->who, "Missile rating raised %d to %d.", amount, p->missile);
	return TRUE;
}


int
v_defense(struct command *c)
{

	return TRUE;
}


int
d_defense(struct command *c)
{
	struct entity_char *p;
	int amount;

	p = p_char(c->who);

	if (rnd(1,100) <= 5)
		amount = 10;
	else if (p->defense < 100)
		amount = rnd(3,5);
	else
		amount = rnd(1,3);

	p->defense += amount;

	wout(c->who, "Defense rating raised %d to %d.", amount, p->defense);
	return TRUE;
}


int
v_swordplay(struct command *c)
{

	return TRUE;
}


int
d_swordplay(struct command *c)
{
	struct entity_char *p;
	int amount;

	p = p_char(c->who);

	if (rnd(1,100) <= 5)
		amount = 10;
	else if (p->attack < 100)
		amount = rnd(3,5);
	else
		amount = rnd(1,3);

	p->attack += amount;

	wout(c->who, "Attack rating raised %d to %d.", amount, p->attack);
	return TRUE;
}


int
v_claim(struct command *c)
{
	int item = c->a;
	int qty = c->b;
	int have_left = c->c;
	int pl = player(c->who);
	int ret;

	if (region(c->who) == cloud_region ||
	    region(c->who) == hades_region ||
	    region(c->who) == faery_region)
	{
		wout(c->who, "CLAIM may not be used in the Cloudlands, "
				"Hades or Faery.");
		return FALSE;
	}

/*
 *  Common mistake checker!
 *
 *  If they said CLAIM 500, assume that they meant CLAIM 1 500.
 */

	if (numargs(c) < 2 &&
	   (kind(item) != T_item || has_item(pl, item) < 1) &&
	    qty == 0)
	{
		log_write(LOG_CODE, "correcting CLAIM for %s:  %s",
				box_code_less(player(c->who)), c->line);

		wout(c->who, "(assuming you meant CLAIM %d %d)", item_gold, item);
		qty = item;
		item = item_gold;
	}

	if (kind(item) != T_item)
	{
		wout(c->who, "%s is not an item.", box_code(item));
		return FALSE;
	}

	if (has_item(pl, item) < 1)
	{
		wout(c->who, "No %s for you to claim.", box_code(item));
		return FALSE;
	}

	qty = how_many(c->who, pl, item, qty, have_left);

	if (qty <= 0)
		return FALSE;

	ret = move_item(pl, c->who, item, qty);
	assert(ret);

	wout(c->who, "Claimed %s.", just_name_qty(item, qty));
	return TRUE;
}


int
v_fight_to_death(struct command *c)
{
	int flag = c->a;

	if (flag) {
		p_char(c->who)->break_point = 50;
		wout(c->who, "Troops led by %s will break at 50%%.",
					box_name(c->who));
	}
	else
	{
		p_char(c->who)->break_point = 0;
		wout(c->who, "Troops led by %s will fight to the death.",
					box_name(c->who));
	}

	return TRUE;
}


int
v_fee(struct command *c)
{
	int amount = c->a;

	p_magic(c->who)->fee = amount;

	wout(c->who, "Ship boarding fee set to %s per 100 weight.",
					gold_s(amount));

	return TRUE;
}


static void
board_message(int who, int ship)
{
	char *with;
	char *desc;
	char *comma = "";
	int where = subloc(ship);

	if (char_really_hidden(who))
		return;

	if (weather_here(where, sub_fog))
		return;

	with = display_with(who);
	desc = liner_desc(who);

	if (strchr(desc, ','))
		comma = ",";

	if (!*with)
		with = ".";

	wout(where, "%s%s%s boarded %s%s", desc, comma,
					box_name(ship), with);
	show_chars_below(where, who);
}


int
v_board(struct command *c)
{
	int ship = c->a;
	int max_fee = c->b;
	struct exit_view *v;
	int ship_fee;			/* fee the captain is charging */
	int owner;			/* captain of the ship */
	struct weights w;		/* how much we weigh */
	int amount;			/* how much we have to pay */
	int sw, sc;			/* ship weight, capacity */
	extern int gold_ferry;

	if (!is_ship(ship))
	{
		wout(c->who, "%s is not a ship.", box_code(ship));
		return FALSE;
	}

	log_write(LOG_SPECIAL, "BOARD for %s", box_name(player(c->who)));

	v = parse_exit_dir(c, subloc(c->who), "board");

	if (v == NULL)
		return FALSE;

	assert(v->destination == ship);

	if (v->in_transit)
	{
		wout(c->who, "%s is underway.  Boarding is not "
				"possible.", box_name(v->destination));
		return FALSE;
	}

	owner = building_owner(ship);

	if (!valid_box(owner) || (ship_fee = board_fee(owner)) == 0)
	{
		wout(c->who, "%s is not being operated as a ferry "
				"(no boarding FEE is set).",
					box_name(ship));
		return FALSE;
	}

	determine_stack_weights(c->who, &w);

/*
 *  Check that the ship isn't already overloaded, and that it won't
 *  be overloaded if we board.
 */

	sc = ship_cap(ship);
	if (sc)
	{
		sw = ship_weight(ship);

		if (sw > sc)
		{
			wout(c->who, "%s is already overloaded.  It can "
				"take no more passengers.", box_name(ship));
			wout(owner, "Refused to let %s board because we "
				"are overloaded.", box_name(c->who));
			return FALSE;
		}

		if (sw + w.total_weight > sc)
		{
			wout(c->who, "%s would be overloaded with us.  "
				"We can't board.", box_name(ship));
			wout(owner, "Refused to let %s board because then we "
				"would be overloaded.", box_name(c->who));
			return FALSE;
		}
	}

	amount = w.total_weight * ship_fee / 100;

	if (max_fee && amount > max_fee)
	{
		wout(c->who, "Refused to pay a boarding fee of %s.",
				gold_s(amount));
		wout(owner, "%s refused to pay a boarding fee of %s.",
				box_name(c->who),
				gold_s(amount));
		return FALSE;
	}

	if (!charge(c->who, amount))
	{
		wout(c->who, "Can't afford a boarding fee of %s.",
				gold_s(amount));
		wout(owner, "%s couldn't afford a boarding fee of %s.",
				box_name(c->who),
				gold_s(amount));

		return FALSE;
	}

	wout(c->who, "Paid %s to board %s.", gold_s(amount), box_name(ship));
	wout(owner, "%s paid %s to board.", box_name(c->who), gold_s(amount));
	board_message(c->who, ship);

	gen_item(owner, item_gold, amount);
	gold_ferry += amount;
	move_stack(c->who, ship);

	return TRUE;
}


static void
unboard_message(int who, int ship)
{
	char *with;
	char *desc;
	char *comma = "";
	int where = subloc(ship);

	if (char_really_hidden(who))
		return;

	if (weather_here(where, sub_fog))
		return;

	with = display_with(who);
	desc = liner_desc(who);

	if (strchr(desc, ','))
		comma = ",";

	if (!*with)
		with = ".";

	wout(where, "%s%s disembarked from %s%s",
					desc, comma,
					box_name(ship), with);
	show_chars_below(where, who);
}


/*
 *  Unload passengers from a ferry
 */

int
v_unload(struct command *c)
{
	int ship = subloc(c->who);
	int where;
	int i;
	int any = FALSE;

	if (!is_ship(ship) || building_owner(ship) != c->who)
	{
		wout(c->who, "%s is not the captain of a ship.",
				box_name(c->who));
		return FALSE;
	}

	where = subloc(ship);

	if (subkind(where) == sub_ocean)
	{
		wout(c->who, "Can't unload passengers at sea.  "
					"They won't go.");
		return FALSE;
	}

	loop_char_here(ship, i)
	{
		if (i == c->who)
			continue;

		wout(c->who, "%s disembarks.", box_name(i));
		wout(i, "%s disembarks.", box_name(i));
		unboard_message(i, ship);

		move_stack(i, where);
		any = TRUE;
	}
	next_char_here;

	if (any)
		wout(c->who, "All passengers unloaded.");
	else
		wout(c->who, "No passengers to unload.");

	return TRUE;
}


/*
 *  Toot our horn -- wakeup any WAIT FERRY's in the port
 */

int
v_ferry(struct command *c)
{
	int ship = subloc(c->who);
	int where;

	if (!is_ship(ship) || building_owner(ship) != c->who)
	{
		wout(c->who, "%s is not the captain of a ship.",
				box_name(c->who));
		return FALSE;
	}

	where = subloc(ship);

	wout(where, "%s sounds a blast on its horn.", box_name(ship));
	log_write(LOG_SPECIAL, "FERRY for %s", box_name(player(c->who)));

	p_magic(ship)->ferry_flag = TRUE;

	return TRUE;
}


#if 0
int
v_tell(struct command *c)
{
	int target = c->a;
	int what = c->b;

#if 1
	wout(c->who, "The TELL order has been removed as of turn 50.");
	return FALSE;
#endif

	if (numargs(c) < 2)
	{
		wout(c->who, "Usage: TELL <who> <what>");
		return FALSE;
	}

	if (kind(target) != T_char && kind(target) != T_player)
	{
		wout(c->who, "%s is not a character or player.",
				c->parse[1]);

		return FALSE;
	}

	if (i_strcmp(c->parse[2], "all") == 0)
		what = -1;
	else if (!valid_box(what) || !test_known(c->who, what))
	{
		wout(c->who, "%s doesn't know anything about %s.",
				box_name(c->who), c->parse[2]);
		return FALSE;
	}

	if (what > 0 && !is_loc_or_ship(what))
	{
		wout(c->who, "Can only TELL about hidden locations.");
		return FALSE;
	}

	if (what == -1)
	{
		wout(c->who, "Not implemented.");
		return FALSE;
	}

	set_known(target, what);

	wout(c->who, "Told %s about %s.", box_name(target), box_name(what));
	wout(target, "%s told us about %s.", box_name(c->who), box_name(what));

	return TRUE;
}
#endif

