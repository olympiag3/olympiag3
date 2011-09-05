

#include	<stdio.h>
#include	<sys/types.h>
#include	<dirent.h>
#include	"z.h"
#include	"oly.h"


/*
 *  add.c  --  add new players to Olympia
 *
 *  oly -a will read data on new characters from stdin:
 *
 *	player number (provided by accounting system)
 *	faction name
 *	primary character name
 *	start city choice
 *	player's full name
 *	player's email address
 */


ilist new_players = NULL;
static ilist new_chars = NULL;


static char *
fetch_inp(FILE *fp)
{
	char *s;

	while ((s = getlin_ew(fp)) && *s == '\0')
		;

	if (s == NULL || *s == '\0')
		return NULL;

	return str_save(s);
}


static int
pick_starting_city(char *start_city)
{
	int n = atoi(start_city);

	if (n == 0)
		n = rnd(1,6);

	switch (n) {
	case 1:	return 57140;	/* Drassa */
	case 2: return 58736;	/* Rimmon */
	case 3: return 57081;	/* Greyfell */
	case 4: return 58423;	/* Port Aurnos */
	case 5: return 58112;	/* Yellowleaf */
	case 6: return 58335;	/* Harn */

	default:
		return 57140;	/* Drassa */
	}
}


static int
add_new_player(int pl, char *faction, char *character, char *start_city,
			char *full_name, char *email)
{
	int who;
	struct entity_char *cp;
	struct entity_player *pp;

	who = new_ent(T_char, 0);

	if (who < 0)
		return 0;

	set_name(pl, faction);
	set_name(who, character);

	pp = p_player(pl);
	cp = p_char(who);

	pp->full_name = full_name;
	pp->email = email;
	pp->noble_points = 12;
	pp->first_turn = sysclock.turn + 1;
	pp->last_order_turn = sysclock.turn;

#if 0
	if (i_strcmp(email+(strlen(email) - 15), "@compuserve.com") == 0)
		pp->compuserve = TRUE;
#endif

	cp->health = 100;
	cp->break_point = 50;
	cp->attack = 80;
	cp->defense = 80;

	set_where(who, pick_starting_city(start_city));
	promote(who, 0);
	set_lord(who, pl, LOY_oath, 2);

	gen_item(who, item_peasant, 25);
	gen_item(who, item_gold, 200);
	gen_item(pl, item_gold, 3000);		/* CLAIM item */
	gen_item(pl, item_lumber, 50);		/* CLAIM item */
	gen_item(pl, item_stone, 100);		/* CLAIM item */

	p_player(pl)->fast_study = 100;		/* instant study days */

	ilist_append(&new_players, pl);
	ilist_append(&new_chars, who);

	add_unformed_sup(pl);

	return pl;
}


static int
make_new_players_sup(char *acct, FILE *fp)
{
	char *faction;
	char *character;
	char *start_city;
	char *full_name;
	char *email;
	int pl;

	faction	   = fetch_inp(fp);
	character  = fetch_inp(fp);
	start_city = fetch_inp(fp);
	full_name  = fetch_inp(fp);
	email	   = fetch_inp(fp);

	if (email == NULL)
	{
		fprintf(stderr, "error: partial read for '%s'\n", acct);
		return FALSE;
	}

	pl = scode(acct);
	assert(pl > 0 && pl < MAX_BOXES);

	alloc_box(pl, T_player, sub_pl_regular);

	add_new_player(pl, faction, character, start_city, full_name, email);
	fprintf(stderr, "\tadded player %s\n", box_name(pl));

	return TRUE;
}


static void
make_new_players()
{
	DIR *d;
	struct dirent *e;
	char *acct_dir = "/u/oly/act";
	char *fnam;
	char *acct;
	FILE *fp;

	d = opendir(acct_dir);

	if (d == NULL)
	{
		fprintf(stderr, "make_new_players: can't open %s: ", acct_dir);
		perror("");
		return;
	}

	while ((e = readdir(d)) != NULL)
	{
		if (*(e->d_name) == '.')
			continue;

		acct = e->d_name;

		fnam = sout("%s/%s/Join-g2", acct_dir, acct);

		fp = fopen(fnam, "r");
		if (fp == NULL)
			continue;

		if (!make_new_players_sup(acct, fp))
		{
			fclose(fp);
			continue;
		}

		fclose(fp);
	}

	closedir(d);
}


void
rename_act_join_files()
{
	int i;
	int pl;
	char acct[LEN];
	char *old_name;
	char *new_name;
	char *acct_dir = "/u/oly/act";

	for (i = 0; i < ilist_len(new_players); i++)
	{
		pl = new_players[i];
		sprintf(acct, box_code_less(pl));

		old_name = sout("%s/%s/Join-g2", acct_dir, acct);
		new_name = sout("%s/%s/Join-g2-", acct_dir, acct);

		if (rename(old_name, new_name) < 0)
		{
			fprintf(stderr, "rename(%s, %s) failed:",
					old_name, new_name);
			perror("");
		}
	}
}


static void
new_player_banners()
{
	int pl;
	int i;
	struct entity_player *p;

	out_path = MASTER;
	out_alt_who = OUT_BANNER;

	for (i = 0; i < ilist_len(new_players); i++)
	{
		pl = new_players[i];
		p = p_player(pl);

#if 1
		html(pl, "<pre>");

		html(pl, "<center>");

		html(pl, "<img src=\"http://www.pbm.com//gif/head.gif\""
			"align=middle width=100 height=100 alt=\"\">");

		html(pl, "<h1>");
		wout(pl, "Olympia G2 turn %d", sysclock.turn);
		wout(pl, "Initial Position Report for %s.", box_name(pl));
		html(pl, "</h1>");

		{
			int month, year;

			month = oly_month(sysclock);
			year = oly_year(sysclock);

			wout(pl, "{<i>}Season \"%s\", month %d, in the year %d.{</i>}",
					month_names[month],
					month + 1,
					year + 1);
		}

		html(pl, "</center>");
		out(pl, "");
#endif

		wout(pl, "Welcome to Olympia G2!");
		wout(pl, "");
		wout(pl, "This is an initial position report for your new "
					"faction.");

		wout(pl, "You are player %s, \"%s\".", box_code_less(pl),
						just_name(pl));
		wout(pl, "");

		wout(pl, "The next turn will be turn %d.", sysclock.turn + 1);

#if 0
		{
			int month, year;

			month = (sysclock.turn) % NUM_MONTHS;
			year = (sysclock.turn + 1) / NUM_MONTHS;

			wout(pl, "It is season \"%s\", month %d, in the "
					"year %d.",
					month_names[month],
					month + 1,
					year + 1);
		}
#endif

		out(pl, "");

		report_account_sup(pl);
	}

	out_path = 0;
	out_alt_who = 0;
}


static void
show_new_char_locs()
{
	int i;
	int where;
	int who;
	extern int show_loc_no_header;	/* argument to show_loc() */

	out_path = MASTER;
	show_loc_no_header = TRUE;

	for (i = 0; i < ilist_len(new_chars); i++)
	{
		who = new_chars[i];
		where = subloc(who);

		out_alt_who = where;
		show_loc(player(who), where);

		where = loc(where);
		if (loc_depth(where) == LOC_province)
		{
			out_alt_who = where;
			show_loc(player(who), where);
		}
	}

	show_loc_no_header = FALSE;
	out_path = 0;
	out_alt_who = 0;
}


static void
new_player_report()
{
	int i;

	out_path = MASTER;
	out_alt_who = OUT_BANNER;

	for (i = 0; i < ilist_len(new_players); i++)
		player_report_sup(new_players[i]);

	out_path = 0;
	out_alt_who = 0;

	for (i = 0; i < ilist_len(new_players); i++)
		show_unclaimed(new_players[i], new_players[i]);
}


static void
new_char_report()
{
	int i;

	indent += 3;

	for (i = 0; i < ilist_len(new_chars); i++)
		char_rep_sup(new_chars[i], new_chars[i]);

	indent -= 3;
}


static void
mail_initial_reports()
{
	int i;
	char *s, *t;
	int pl;
	int ret;

	for (i = 0; i < ilist_len(new_players); i++)
	{
		pl = new_players[i];

		s = sout("%s/log/%d", libdir, pl);
		t = sout("%s/save/%d/%d", libdir, sysclock.turn, pl);

		ret = rename(s, t);

		if (ret < 0)
		{
			fprintf(stderr, "couldn't rename %s to %s:", s, t);
			perror("");
		}

		send_rep(pl, sysclock.turn);
	}
}


static void
new_order_templates()
{
	int pl, i;

	out_path = MASTER;
	out_alt_who = OUT_TEMPLATE;

	for (i = 0; i < ilist_len(new_players); i++)
	{
		pl = new_players[i];
		orders_template(pl, pl);
	}

	out_path = 0;
	out_alt_who = 0;
}


static void
new_player_list_sup(int who, int pl)
{
	struct entity_player *p;
	char *s;
	char *t;

	p = p_player(pl);

	if (p->email)
	{
		if (p->full_name)
		{
			s = sout("%s <%s>", p->full_name, p->email);
			t = sout("%s &lt;%s&gt;", p->full_name, p->email);
		}
		else
		{
			s = sout("<%s>", p->email);
			t = sout("&lt;%s&gt;", p->email);
		}
	}
	else if (p->full_name)
		s = p->full_name;
	else
		s = "";

	out(who, "%4s   %s", box_code_less(pl), just_name(pl));

	if (*s)
	{
		style(TEXT);
		out(who, "       %s", s);
		style(0);

		style(HTML);
		out(who, "       %s", t);
		style(0);
	}

	out(who, "");
}


void
new_player_list()
{
	int pl;
	int i;

	stage("new_player_list()");

	out_path = MASTER;
	out_alt_who = OUT_NEW;

	vector_players();

#if 0
	for (i = 0; i < ilist_len(new_players); i++)
	{
		pl = new_players[i];
		ilist_rem_value(&out_vector, pl);
	}
#endif

	for (i = 0; i < ilist_len(new_players); i++)
	{
		pl = new_players[i];
		new_player_list_sup(VECT, pl);
	}

	out_path = 0;
	out_alt_who = 0;
}


void
new_player_top(int mail)
{

	stage("new_player_top()");

	open_logfile();
	make_new_players();
	show_new_char_locs();
	new_char_report();
	new_player_banners();
	new_player_report();
	new_order_templates();
	gen_include_section();		/* must be last */
	close_logfile();

	if (mail)
		mail_initial_reports();
}


void
add_new_players()
{

	stage("add_new_players()");

	make_new_players();
	show_new_char_locs();
	new_char_report();
	new_player_banners();
	new_player_report();
	new_order_templates();
	new_player_list();	/* show new players to the old players */
}

