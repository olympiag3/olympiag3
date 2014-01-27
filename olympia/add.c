

#include	<stdio.h>
#include	<string.h>
// #include <libc/unistd.h>
#ifdef _WIN32
#include	<libc/dirent.h>
#else
#include	<dirent.h>
#endif
#include	<sys/types.h>
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

// by Cappinator:
// Added a get_city_id method that retrieves the
// internal id of a starting city from a file
// instead of using hard coded ones.
static int get_city_id(char *start_city) {
  FILE *fp;
  char *fnam;
  char *s;
  char idxs[1];
  int id = 0;
  char ids[5];
  char *cname;
  int i;
  int rnd_id_cnt;
  int rnd_line_cur;
  int rnd_id = 0;

  fnam = sout("%s/startloc", libdir);
  fp = fopen(fnam, "r");

  if (fp == NULL) {
    fprintf(stderr, "error: could not read startloc file.");
    return 0;
  }

  rnd_id_cnt = rnd(1, 6);
  rnd_line_cur = 1;

  while (s = getlin(fp)) {
	  if (strlen(s) > 9) {
		strncpy(idxs, &s[0], 1);
		strncpy(ids, &s[2], 5);
		id = atoi(ids);
		if (rnd_id_cnt == rnd_line_cur)
			rnd_id = id;
		cname = &s[8];
		if (strncmp(idxs, start_city, 1) == 0) {
			fclose(fp);
			return id;
		}
	  }
	  rnd_line_cur++;
  }

  fclose(fp);

  return rnd_id;
}

static int
pick_starting_city(char *start_city)
{
	int city, empty, garrison, here, prov;
	ilist garrisoned = NULL, ungarrisoned = NULL;

	if (!i_strcmp(start_city, "empty"))
	{
		loop_city(city)
		{
			if (safe_haven(city) || greater_region(city) != 0)
				continue;

			prov = province(city);
			if (garrison_here(prov))
				continue;

			empty = 1;
			garrison = 0;
			loop_all_here(city, here)
			{
				if (kind(here) == T_char)
				{
					if (default_garrison(here))
						garrison = 1;
					else
						empty = 0;
				}
			}
			next_all_here;

			loop_char_here(prov, here)
			{
				if (kind(here) == T_char && !is_npc(here))
					empty = 0;
			}
			next_char_here;

			if (empty)
			{
				if (garrison)
					ilist_append(&garrisoned, city);
				else
					ilist_append(&ungarrisoned, city);
			}
		}
		next_city;

		if (ilist_len(garrisoned) > 0)
		{
			ilist_scramble(garrisoned);
			return garrisoned[0];
		}
		else if (ilist_len(ungarrisoned) > 0)
		{
			ilist_scramble(ungarrisoned);
			return ungarrisoned[0];
		}

		fprintf(stderr, "No empty cities found for new player!\n");
	}

	return get_city_id(start_city);
}

static int
add_new_player(int pl, char *faction, char *character, char *start_city,
			char *full_name, char *email)
{
	int who, city, garrison;
  	int t = sysclock.turn;
	struct entity_char *cp;
	struct entity_player *pp;
	char password[10];
	int i;
	const char *symbols =	"abcdefghijklmnopqrstuvwxyz"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"1234567890";

	who = new_ent(T_char, 0);

	if (who < 0)
		return 0;

	set_name(pl, faction);
	set_name(who, character);

	pp = p_player(pl);
	cp = p_char(who);

	pp->full_name = full_name;
	pp->email = email;

	// by sbaillie:
	// give new players a randomly generated password by default
	for (i = 0; i < 8; i++)
		password[i] = symbols[rnd(1, strlen(symbols)) - 1];
	password[i] = '\0';
	pp->password = str_save(password);

  	// by Cappinator: 
    // Changed starting faction noble points
  	pp->noble_points = 18 + (t/8);

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

	city = pick_starting_city(start_city);
	set_where(who, city);
	garrison = garrison_here(city);
	promote(who, 0);
	// If there is a garrison in the city then they need to still
	// be at the top of the list, newcomer advantage notwithstanding.
	if (garrison)
		promote(garrison, 0);
	set_lord(who, pl, LOY_oath, 2);

	gen_item(who, item_peasant, 25);
	gen_item(who, item_gold, 200);

	// by Cappinator:
	// Changed starting faction claim gold
	gen_item(pl, item_gold, 5000);		/* CLAIM item */

	gen_item(pl, item_lumber, 50);		/* CLAIM item */
	gen_item(pl, item_stone, 100);		/* CLAIM item */

	// by Cappinator:
	// Added 5 riding horses to starting faction claim pool
	gen_item(pl, item_riding_horse, 5);   /* CLAIM item added by Cappy */
  
	// by Cappinator:
	// Changed starting faction fast study days
	if (t < 101)
		p_player(pl)->fast_study = 198 + (t * 2);       /* instant study days */
	else
		p_player(pl)->fast_study = 400;

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
	char *acct_dir = "act";
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

		fnam = sout("%s/%s/Join-g3", acct_dir, acct);

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
	char *acct_dir = "act";

	for (i = 0; i < ilist_len(new_players); i++)
	{
		pl = new_players[i];
		strcpy(acct, box_code_less(pl));

		old_name = sout("%s/%s/Join-g3", acct_dir, acct);
		new_name = sout("%s/%s/Join-g3-", acct_dir, acct);

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

		if (win_flag)
		{
			html(pl, "<img src=\"head.gif\" "
				"align=middle width=100 height=100 alt=\"\">");
		}
		else
		{
			html(pl, "<img src=\"../head.gif\" "
				"align=middle width=100 height=100 alt=\"\">");
		}

		html(pl, "<h1>");
		wout(pl, "%s turn %d", game_title, sysclock.turn);
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

		wout(pl, "Welcome to %s!", game_title);
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

		// by Cappinator:
		// Added this to create the folders on a Windows machine
		make_dir(sout("%s\\save", libdir));
		make_dir(sout("%s\\save\\%d", libdir, sysclock.turn));

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
		style(STYLE_TEXT);
		out(who, "       %s", s);
		style(0);

		style(STYLE_HTML);
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

