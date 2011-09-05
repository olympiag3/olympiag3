
#include	<stdio.h>
#include	<sys/types.h>
#include	<dirent.h>
#include	"z.h"
#include	"oly.h"

#define		MAX_ERR		50


static int cmd_begin = -1;
static int cmd_end = -1;
static int cmd_unit = -1;
static int cmd_email = -1;
static int cmd_vis_email = -1;
static int cmd_lore = -1;
static int cmd_post = -1;
static int cmd_message = -1;
static int cmd_rumor = -1;
static int cmd_press = -1;
static int cmd_format = -1;
static int cmd_notab = -1;
static int cmd_resend = -1;
static int cmd_passwd = -1;
static int cmd_password = -1;
static int cmd_stop = -1;
static int cmd_players = -1;
static int cmd_split = -1;
static int cmd_wait = -1;


static char *cc_addr = NULL;
static char *reply_addr = NULL;
static int pl = 0;
static int unit = 0;
static int n_queued = 0;
static int n_fail = 0;
static int already_seen = FALSE;

static char who_to[LEN];
static char save_line[LEN];
static int line_count = 0;


static void
find_meta_commands()
{
	extern int fuzzy_find;

	cmd_begin = find_command("begin");
	assert(cmd_begin > 0);
	assert(!fuzzy_find);

	cmd_end = find_command("end");
	assert(cmd_end > 0);
	assert(!fuzzy_find);

	cmd_unit = find_command("unit");
	assert(cmd_unit > 0);
	assert(!fuzzy_find);

	cmd_email = find_command("email");
	assert(cmd_email > 0);
	assert(!fuzzy_find);

	cmd_vis_email = find_command("vis_email");
	assert(cmd_vis_email > 0);
	assert(!fuzzy_find);

	cmd_lore = find_command("lore");
	assert(cmd_lore > 0);
	assert(!fuzzy_find);

	cmd_post = find_command("post");
	assert(cmd_post > 0);
	assert(!fuzzy_find);

	cmd_rumor= find_command("rumor");
	assert(cmd_rumor > 0);
	assert(!fuzzy_find);

	cmd_press = find_command("press");
	assert(cmd_press > 0);
	assert(!fuzzy_find);

	cmd_format = find_command("format");
	assert(cmd_format > 0);
	assert(!fuzzy_find);

	cmd_notab = find_command("notab");
	assert(cmd_notab > 0);
	assert(!fuzzy_find);

	cmd_message = find_command("message");
	assert(cmd_message > 0);
	assert(!fuzzy_find);

	cmd_resend = find_command("resend");
	assert(cmd_resend > 0);
	assert(!fuzzy_find);

	cmd_passwd = find_command("passwd");
	assert(cmd_passwd > 0);
	assert(!fuzzy_find);

	cmd_password = find_command("password");
	assert(cmd_password > 0);
	assert(!fuzzy_find);

	cmd_stop = find_command("stop");
	assert(cmd_stop > 0);
	assert(!fuzzy_find);

	cmd_players = find_command("players");
	assert(cmd_players > 0);
	assert(!fuzzy_find);

	cmd_split = find_command("split");
	assert(cmd_split > 0);
	assert(!fuzzy_find);

	cmd_wait = find_command("wait");
	assert(cmd_wait > 0);
	assert(!fuzzy_find);
}


static void
init_eat_vars()
{

	if (cmd_begin < 0)
		find_meta_commands();

	if (cc_addr)
	{
		my_free(cc_addr);
		cc_addr = NULL;
	}

	already_seen = FALSE;
	pl = 0;
	unit = 0;
	n_queued = 0;
	n_fail = 0;
	line_count = 0;
	*save_line = '\0';
}


/*
 *	rmatch("?*<(?*)>", s, &pat))
 *	rmatch("(?*)[ \t]+\\(?**\\)", s, &pat))
 */

static char *
crack_address_sup(char *s)
{
	char *t;
	extern char *strchr();

	if (t = strchr(s, '<'))
	{
		char *u;

		t++;
		for (u = t; *u && *u != '>'; u++)
			;
		*u = '\0';
		return t;
	}

	while (*s && iswhite(*s))
		s++;

	for (t = s; *t && !iswhite(*t); t++)
		;
	*t = '\0';

	return s;
}


#if 0
/*
 *  01234567890123456
 *  a%b@princeton.edu	==>  a@b
 */

static char *
local_kludge(char *s)
{
	int l = strlen(s);
	char *t;
	extern char *strchr();

	if (l < 17)
		return s;

	if (i_strcmp("@Princeton.EDU", s + (l - 14)) == 0 &&
	   (t = strchr(s, '%')))
	{
		s[l-14] = '\0';
		*t = '@';
	}

	return s;
}
#endif


static char *
crack_address(char *s)
{

	s = crack_address_sup(s);

	if (s)
	{
#if 0
		s = local_kludge(s);
#endif
		return str_save(s);
	}

	return NULL;
}


static char *
parse_reply(FILE *fp)
{
	static char *from_space = NULL;
	static char *from_colon = NULL;
	static char *reply_to = NULL;
	char *s;
	char *t;

	if (from_space)
	{
		my_free(from_space);
		from_space = NULL;
	}

	if (from_colon)
	{
		my_free(from_colon);
		from_colon = NULL;
	}

	if (reply_to)
	{
		my_free(reply_to);
		reply_to = NULL;
	}

	s = getlin(fp);

	if (strncmp(s, "From ", 5) != 0)
		return NULL;

	s += 5;
	for (t = s; *t && !iswhite(*t); t++)
		;
	*t = '\0';

	if (!*s)		/* did we get the From_ address? */
		return NULL;

	from_space = str_save(s);

	while (s = getlin(fp))
	{
		if (!*s)
			break;

		if (i_strncmp(s, "From:", 5) == 0)
			from_colon = crack_address(&s[5]);
		else if (i_strncmp(s, "Reply-To:", 9) == 0)
			reply_to = crack_address(&s[9]);
		else if (i_strncmp(s, "X-Loop", 6) == 0)
			already_seen = TRUE;
	}

	if (reply_to)
		return reply_to;
	if (from_colon)
		return from_colon;
	return from_space;
}


static char *
eat_line_2(FILE *fp, int eat_white)
{
	char *line;

	if (eat_white)
		line = getlin_ew(fp);
	else
		line = getlin(fp);

	if (line == NULL)
		return NULL;

	remove_ctrl_chars(line);

	if (eat_white)
		while (iswhite(*line))
			line++;

	{
		strncpy(save_line, line, LEN-1);
		save_line[LEN-1] = '\0';

		line_count++;
	}

	return line;
}


static char *
eat_next_line_sup(FILE *fp)
{
	char *line;

	line = getlin_ew(fp);

	if (line == NULL)
		return NULL;

	remove_comment(line);
	remove_ctrl_chars(line);

	while (iswhite(*line))
		line++;

	{
		strncpy(save_line, line, LEN-1);
		save_line[LEN-1] = '\0';

		line_count++;
	}

	return line;
}


static char *
eat_next_line(FILE *fp)
{
	char *line;

	do
	{
		line = eat_next_line_sup(fp);
	}
	while (line && *line == '\0');

	return line;
}


static void
err(int k, char *s)
{

	out_alt_who = k;

	if (k == EAT_ERR)
		n_fail++;

	out(eat_pl, "line %d: %s: %s", line_count, s, save_line);
}


static void
next_cmd(FILE *fp, struct command *c)
{
	char *line;

	c->cmd = 0;

	while (1)
	{
		line = eat_next_line(fp);

		if (!line)
		{
			c->cmd = cmd_end;
			return;
		}

		if (!oly_parse(c, line))
		{
			err(EAT_ERR, "unrecognized command");

			if (n_fail > MAX_ERR)
			{
				err(EAT_ERR, "too many errors, aborting");
				c->cmd = cmd_end;
				return;
			}
			continue;
		}

		if (c->fuzzy)
		{
			err(EAT_WARN, sout("assuming you meant '%s'",
					cmd_tbl[c->cmd].name));
		}

		return;
	}
}


static int
do_begin(struct command *c)
{
	char *pl_pass;

	if (numargs(c) < 1)
	{
	    err(EAT_ERR, "No player specified on BEGIN line");
	    return FALSE;
	}

	if (kind(c->a) != T_player)
	{
	    err(EAT_ERR, "No such player");
	    return FALSE;
	}

	pl_pass = p_player(c->a)->password;

	if (numargs(c) > 1)
	{
		if (pl_pass == NULL || *pl_pass == '\0')
		{
			err(EAT_WARN, "No password is currently set");
		}
		else if (i_strcmp(pl_pass, c->parse[2]) != 0)
		{
			err(EAT_ERR, "Incorrect password");
			return FALSE;
		}
	}
	else if (pl_pass && *pl_pass)
	{
		err(EAT_ERR, "Incorrect password");
		err(EAT_ERR, "Must give password on BEGIN line.");
		return FALSE;
	}

	pl = c->a;

	p_player(pl)->sent_orders = 1;	/* okay, they sent something in */

#if 0
/*
*  We set unit here in case they forget the UNIT command for the
*  player entity.  If they do, they lose the auto-flush ability,
*  but at least their command will get queued, and echoed back
*  in the confirmation.
*/

	unit = pl;
#endif

	return TRUE;
}


static int
valid_char_or_player(int who)
{

	if (kind(who) == T_char || kind(who) == T_player)
		return TRUE;

	if (kind(who) == T_item && subkind(who) == sub_dead_body)
		return TRUE;

	return FALSE;
}


static int
do_unit(struct command *c)
{

	unit = -1;	/* ignore following unit commands */

	if (pl == 0)
	{
	    err(EAT_ERR, "BEGIN must appear before UNIT");
	    out(eat_pl, "      rest of commands for unit ignored");
	    return TRUE;
	}

	if (kind(c->a) == T_unform)
	{
		if (ilist_lookup(p_player(pl)->unformed, c->a) < 0)
		{
		    err(EAT_WARN, "Not an unformed unit of yours");
		}
	}
	else if (!valid_char_or_player(c->a))
	{
	    err(EAT_ERR, "Not a character or unformed unit");
	    return TRUE;
	}
	else if (player(c->a) != pl)
	{
	    err(EAT_WARN, "Not one of your controlled characters");
	}

	unit = c->a;
	flush_unit_orders(pl, unit);
	return TRUE;
}


static int
do_email(struct command *c)
{

	if (cc_addr)
	{
	    err(EAT_ERR, "no more than one EMAIL order per message");
	    out(eat_pl, "      new email address not set");
	    return TRUE;
	}

	if (pl == 0)
	{
	    err(EAT_ERR, "BEGIN must come before EMAIL");
	    out(eat_pl, "      new email address not set");
	    return TRUE;
	}

	if (numargs(c) < 1 || c->parse[1] == NULL || !*(c->parse[1]))
	{
	    err(EAT_ERR, "no new email address given");
	    out(eat_pl, "      new email address not set");
	    return TRUE;
	}

	cc_addr = rp_player(pl)->email;
	p_player(pl)->email = str_save(c->parse[1]);

	return TRUE;
}


static int
do_vis_email(struct command *c)
{

	if (pl == 0)
	{
	    err(EAT_ERR, "BEGIN must come before VIS_EMAIL");
	    out(eat_pl, "      new address not set");
	    return TRUE;
	}

	if (numargs(c) < 1 || c->parse[1] == NULL || !*(c->parse[1]))
	{
	    p_player(pl)->vis_email = NULL;
	    return TRUE;
	}

	p_player(pl)->vis_email = str_save(c->parse[1]);

	return TRUE;
}


static int
do_lore(struct command *c)
{
	int sheet = c->a;

	if (pl == 0)
	{
	    err(EAT_ERR, "BEGIN must appear before LORE");
	    return TRUE;
	}

	if (kind(sheet) == T_item)
		sheet = item_lore(sheet);

	if (!valid_box(sheet))
	{
		err(EAT_ERR, "no such lore sheet");
		return TRUE;
	}

	if (!test_known(pl, sheet))
	{
		err(EAT_ERR, "you haven't seen that lore sheet before");
		return TRUE;
	}

	out_alt_who = OUT_LORE;
	deliver_lore(eat_pl, c->a);

	return TRUE;
}


static int
do_players(struct command *c)
{
	FILE *fp;
	char *fnam;
	char *s;

	fnam = sout("%s/save/%d/players", libdir, sysclock.turn);
	fp = fopen(fnam, "r");

	if (fp == NULL)
	{
		err(EAT_ERR, sout("Sorry, couldn't find the player list."));
		return TRUE;
	}

	out_alt_who = EAT_PLAYERS;

	while (s = getlin(fp))
	{
		out(eat_pl, "%s", s);
	}

	fclose(fp);

	return TRUE;
}


static int
do_resend(struct command *c)
{
	int turn;

	if (pl == 0)
	{
	    err(EAT_ERR, "BEGIN must appear before RESEND");
	    return TRUE;
	}

	turn = c->a;
	if (turn == 0)
		turn = sysclock.turn;

	if (send_rep(pl, turn))
	{
	    out_alt_who = EAT_OKAY;
	    wout(eat_pl, "Turn %d report has been mailed to you "
				"in a separate message.", turn);
	}
	else
	{
	    err(EAT_ERR, sout("Sorry, couldn't find your turn %d "
			"report", turn));
	}

	return TRUE;
}


static int
do_format(struct command *c)
{

	if (pl == 0)
	{
	    err(EAT_ERR, "BEGIN must appear before FORMAT");
	    return TRUE;
	}

	p_player(pl)->format = c->a;
	err(EAT_WARN, sout("Report formatting set to %d", c->a));

	return TRUE;
}


static int
do_split(struct command *c)
{
	int lines = c->a;
	int bytes = c->b;

	if (pl == 0)
	{
	    err(EAT_ERR, "BEGIN must appear before SPLIT");
	    return TRUE;
	}

	out_alt_who = EAT_OKAY;

	if (lines > 0 && lines < 500)
	{
		lines = 500;
		out(eat_pl, "Minimum lines to split at is 500");
	}

	if (bytes > 0 && bytes < 10000)
	{
		bytes = 10000;
		out(eat_pl, "Minimum bytes to split at is 10,000");
	}

	p_player(pl)->split_lines = lines;
	p_player(pl)->split_bytes = bytes;

	if (lines == 0 && bytes == 0)
		out(eat_pl, "Reports will not be split when mailed.");
	else if (lines && bytes)
		out(eat_pl, "Reports will be split at %d lines or "
			"%d bytes, whichever limit is hit first.",
				lines, bytes);
	else if (lines)
		out(eat_pl, "Reports will be split at %d lines.", lines);
	else
		out(eat_pl, "Reports will be split at %d bytes.", bytes);

	return TRUE;
}



static int
do_notab(struct command *c)
{

	if (pl == 0)
	{
	    err(EAT_ERR, "BEGIN must appear before NOTAB");
	    return TRUE;
	}

	p_player(pl)->notab = c->a;

	out_alt_who = EAT_OKAY;
	if (c->a)
		wout(eat_pl, "No TAB characters will appear in turn "
						"reports.");
	else
		wout(eat_pl, "TAB characters may appear in turn "
						"reports.");

	return TRUE;
}


static int
do_password(struct command *c)
{

	if (pl == 0)
	{
	    err(EAT_ERR, "BEGIN must appear before PASSWORD");
	    return TRUE;
	}

	if (numargs(c) < 1 || *(c->parse[1]) == '\0')
	{
		p_player(pl)->password = NULL;

		out_alt_who = EAT_OKAY;
		wout(eat_pl, "Password cleared.");
		return TRUE;
	}

	p_player(pl)->password = str_save(c->parse[1]);

	out_alt_who = EAT_OKAY;
	wout(eat_pl, "Password set to \"%s\".", c->parse[1]);

	set_html_pass(pl);

	return TRUE;
}


#define DASH_LINE "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="

static void
show_post(char **l, int cmd)
{
	int i;
	char attrib[100];
	char *t;
	int sav = out_alt_who;

	out_alt_who = OUT_SHOW_POSTS;

	for (i = 0; i < ilist_len(l); i++)
	{
		if (strncmp(l[i], "=-=-", 4) == 0)
			out(eat_pl, "> %s", l[i]);
		else
			out(eat_pl, "%s", l[i]);
	}

	sprintf(attrib, "-- %s", box_name(unit));
	for (t = attrib; *t; t++)
		if (*t == '~')
			*t = ' ';

	out(eat_pl, "");

	if (cmd == cmd_press)
	{
		out(eat_pl, "%55s", attrib);
		out(eat_pl, "");
	}

	out(eat_pl, DASH_LINE);

	out_alt_who = sav;
}


static int
do_eat_command(struct command *c, FILE *fp)
{

	assert(c->cmd != cmd_end);

	if (c->cmd == cmd_begin)	return do_begin(c);
	if (c->cmd == cmd_unit)		return do_unit(c);
	if (c->cmd == cmd_email)	return do_email(c);
	if (c->cmd == cmd_vis_email)	return do_vis_email(c);
	if (c->cmd == cmd_lore)		return do_lore(c);
	if (c->cmd == cmd_resend)	return do_resend(c);
	if (c->cmd == cmd_format)	return do_format(c);
	if (c->cmd == cmd_notab)	return do_notab(c);
	if (c->cmd == cmd_split)	return do_split(c);
	if (c->cmd == cmd_players)	return do_players(c);
	if (c->cmd == cmd_passwd ||
	    c->cmd == cmd_password)	return do_password(c);

	if (unit == 0)
	{
	    err(EAT_ERR, "can't queue orders, missing UNIT command");
	    unit = -1;
	    return TRUE;
	}

	if (unit == -1)
	{
		n_fail++;
		return TRUE;
	}

	if (c->cmd == cmd_stop)
		queue_stop(pl, unit);
	else
		queue_order(pl, unit, c->line);
	n_queued++;

	if (c->cmd == cmd_wait)
	{
		extern char *parse_wait_args();
		extern char *clear_wait_parse();
		char *s;

		s = parse_wait_args(c);

		if (s)
			err(EAT_ERR, sout("Bad WAIT: %s", s));

		clear_wait_parse(c);
	}

	if (c->cmd == cmd_post || c->cmd == cmd_message ||
	    c->cmd == cmd_rumor || c->cmd == cmd_press)
	{
		int count = c->a;
		char *s;
		int len = 0;
		int reject_flag = FALSE;
		int max_len = MAX_POST;
		char **l = NULL;

		if (c->cmd == cmd_rumor || c->cmd == cmd_press)
			max_len = 78;

		while (1)
		{
			if (c->cmd == cmd_post || c->cmd == cmd_message)
				s = eat_line_2(fp, TRUE);
			else
				s = eat_line_2(fp, FALSE);
			if (!s)
			{
				err(EAT_ERR, "End of input reached before end of post!");
				break;
			}

			len = strlen(s);
			if (len > max_len)
			{
				err(EAT_ERR, sout("Line length exceeds %d characters", max_len));
				reject_flag = TRUE;
			}

			queue_order(pl, unit, s);

			if (count == 0)
			{
				char *t = eat_leading_trailing_whitespace(s);

				if (i_strcmp(t, "end") == 0)
					break;

				ilist_append((ilist *) &l, (int) str_save(s));
			}
			else
			{
				ilist_append((ilist *) &l, (int) str_save(s));

				if (--count <= 0)
					break;
			}
		}

		if (reject_flag)
			err(EAT_ERR, "Post will be rejected.");
		else if (c->cmd == cmd_press || c->cmd == cmd_rumor)
			show_post(l, c->cmd);

		text_list_free(l);
	}

	return TRUE;
}


static struct command dummy_cmd;


static void
parse_and_munch(FILE *fp)
{
	struct command *c;

	c = &dummy_cmd;

	next_cmd(fp, c);
	while (c->cmd != cmd_end)
	{
		if (!do_eat_command(c, fp))
			return;
		next_cmd(fp, c);
	}
}


static void
eat_banner()
{
	char *to;
	char *full_name = "";

	out_alt_who = OUT_BANNER;

	out(eat_pl, "From: %s", from_host);
	out(eat_pl, "Reply-To: %s", reply_host);

	if (pl && rp_player(pl)->email && *(rp_player(pl)->email))
		to = rp_player(pl)->email;
	else
		to = reply_addr;

	if (valid_box(pl))
	{
		struct entity_player *p = rp_player(pl);

		if (p && p->full_name && *(p->full_name))
			full_name = sout(" (%s)", p->full_name);
	}

	if (already_seen)
	{
		to = "nobody@pbm.com";
		full_name = " (Error Watcher)";

		if (cc_addr)
		{
			my_free(cc_addr);
			cc_addr = NULL;
		}
	}

	strcpy(who_to, to);

	out(eat_pl, "To: %s%s", to, full_name);

	if (cc_addr && *cc_addr)
	{
		out(eat_pl, "Cc: %s", cc_addr);
		strcat(who_to, " ");
		strcat(who_to, cc_addr);
	}

	out(eat_pl, "Subject: Acknowledge");
	out(eat_pl, "X-Loop: orders@g2.pbm.com");
	out(eat_pl, "Bcc: g2watch");
	out(eat_pl, "");
	out(eat_pl, "     - Olympia order scanner -");
	out(eat_pl, "");
	if (pl)
		out(eat_pl, "Hello, %s", box_name(pl));
	out(eat_pl, "");
	out(eat_pl, "%d queued, %d error%s.",
				n_queued, n_fail,
				n_fail == 1 ? "" : "s");
}


static void
include_orig(FILE *fp)
{
	char *s;

	out_alt_who = EAT_HEADERS;

	rewind(fp);
	while (s = getlin(fp))
		out(eat_pl, "%s", s);
}


static void
show_pending()
{

	out_alt_who = EAT_QUEUE;
	orders_template(eat_pl, pl);
}


int eat_queue_mode = 0;


static void
eat(char *fnam)
{
	FILE *fp;
	int ret;
	int okay_flag = 0;
	char buf[1025];

	init_eat_vars();
	eat_queue_mode = 1;

	fp = fopen(fnam, "r");

	if (fp == NULL)
	{
		fprintf(stderr, "can't open %s", fnam);
		perror("");
		return;
	}

	while (fgets(buf, 1024, fp) != NULL) {
		if (i_strncmp(buf, "begin", 5) == 0) {
			okay_flag = 1;
		}
	}

	fclose(fp);

	if (!okay_flag) {
		fprintf(stderr, "%s spam, ignoring\n");
		return;
	}

	fp = fopen(fnam, "r");

	reply_addr = parse_reply(fp);

	if (reply_addr)
	{
		if (i_strncmp(reply_addr, "postmaster", 10) == 0 ||
		    i_strncmp(reply_addr, "root@nyx.net", 12) == 0 ||
		    i_strncmp(reply_addr, "mailer-daemon", 13) == 0 ||
		    i_strncmp(reply_addr, "mail-daemon", 11) == 0)
		{
			already_seen = TRUE;
		}

		unlink(sout("%s/log/%d", libdir, eat_pl));
		open_logfile_nondestruct();
		clear_know_rec(&(p_player(eat_pl)->output));
		out_path = MASTER;

		parse_and_munch(fp);
		eat_banner();

		if (pl)
			show_pending();

		include_orig(fp);

		out_alt_who = OUT_INCLUDE;
		gen_include_sup(eat_pl);	/* must be last */

		out_path = 0;
		out_alt_who = 0;

		if (pl)
		{
			unlink(sout("%s/orders/%d", libdir, pl));
			save_player_orders(pl);
			unlink(sout("%s/fact/%d", libdir, pl));
			write_player(pl);
		}

		close_logfile();

#if 0
		ret = system(sout("g2rep %s/log/%d | %srmail %s g2watch",
				libdir, eat_pl, entab(eat_pl), who_to));
#else
		ret = system(sout("g2rep %s/log/%d | %ssendmail -t",
				libdir, eat_pl, entab(eat_pl)));
#endif

		if (ret)
		{
			fprintf(stderr, "error: couldn't mail ack to %s\n",
					who_to);
			fprintf(stderr, "command was: %s\n",
				sout("g2rep %s/log/%d | %ssendmail -t",
					libdir, eat_pl, entab(eat_pl)));
		}
	}

	fclose(fp);

	eat_queue_mode = 0;
}


static void
write_remind_list()
{
	FILE *fp;
	char *fnam;
	int pl;
	struct entity_player *p;

	fnam = sout("%s/remind", libdir);
	fp = fopen(fnam, "w");
	if (fp == NULL)
	{
		fprintf(stderr, "can't write %s:", fnam);
		perror("");
		return;
	}

	loop_player(pl)
	{
		if (subkind(pl) != sub_pl_regular)
			continue;

		p = rp_player(pl);
		if (p == NULL || p->sent_orders || p->dont_remind)
			continue;

		if (p->email == NULL)
		{
			fprintf(stderr, "player %s has no email address\n",
					box_code(pl));
			continue;
		}

		fprintf(fp, "%s\n", p->email);
	}
	next_player;

	fclose(fp);
}


static int
read_spool()
{
	DIR *d;
	struct dirent *e;
	char fnam[LEN];
	int ret = TRUE;
	int did_one = FALSE;

	sprintf(fnam, "%s/spool", libdir);
	d = opendir(fnam);

	if (d == NULL)
	{
		fprintf(stderr, "read_spool: can't open %s: ", fnam);
		perror("");
		return FALSE;
	}

	while ((e = readdir(d)) != NULL)
	{
		if (strncmp(e->d_name, "stop", 4) == 0)
		{
			ret = FALSE;
			unlink(sout("%s/spool/%s", libdir, e->d_name));
		}

		if (*(e->d_name) == 'm')
		{
			sprintf(fnam, "%s/spool/%s", libdir, e->d_name);
			eat(fnam);
			unlink(fnam);
			sleep(5);

			did_one = TRUE;
		}
	}

	closedir(d);

	if (did_one)
		write_remind_list();

	return ret;
}


void
eat_loop()
{

	setbuf(stdout, NULL);
	mkdir(sout("%s/orders", libdir), 0755);
	mkdir(sout("%s/spool", libdir), 0777);
	chmod(sout("%s/spool", libdir), 0777);

	write_remind_list();

	while (read_spool())
	{
		sleep(10);
	}
}

