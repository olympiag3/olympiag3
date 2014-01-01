#include	<stdlib.h>
#include	<stdio.h>
#ifdef _WIN32
#include	<libc/sys/stat.h>
#include	<libc/unistd.h>
#else
#include	<sys/stat.h>
#include	<unistd.h>
#endif
#include	"z.h"
#include	"oly.h"


/*
 *  pretty_data_files:  include parenthesisted names in the data files,
 *  to make them easier to read.
 */

void setup_html_all(void);

int pretty_data_files = FALSE;

int immediate = FALSE;
int immed_after = FALSE;
int immed_see_all = FALSE;
int flush_always = FALSE;
int time_self = FALSE;		/* print timing info */
int save_flag = FALSE;
int win_flag = FALSE;

call_init_routines()
{

	init_lower();
	glob_init();		/* initialize global tables */
	initialize_buffer();	/* used by sout() */
	init_spaces();
	/* init_random();	/* seed random number generator */
}


void write_totimes(void)
{
	FILE *fp;
	char *fnam;
	int pl;

	fnam = sout("%s/totimes", libdir);

	fp = fopen(fnam, "w");

	if (fp == NULL)
	{
		fprintf(stderr, "can't write %s", fnam);
		perror("");
		return;
	}

	loop_player(pl)
	{
		if (rp_player(pl) &&
		    rp_player(pl)->email &&
		    !player_compuserve(pl))
		{
			fprintf(fp, "%s\n", rp_player(pl)->email);
		}
	}
	next_player;

	fclose(fp);
}


void write_email(void)
{
	FILE *fp;
	char *fnam;
	int pl;

	fnam = sout("%s/email", libdir);

	fp = fopen(fnam, "w");

	if (fp == NULL)
	{
		fprintf(stderr, "can't write %s", fnam);
		perror("");
		return;
	}

	loop_player(pl)
	{
		if (rp_player(pl) && rp_player(pl)->email)
			fprintf(fp, "%s\n", rp_player(pl)->email);
	}
	next_player;

	fclose(fp);
}

static void
list_a_player(FILE *fp, int pl, int *flag)
{
	struct entity_player *p;
	char *s;
	char c;
	char *email;

	p = p_player(pl);

	if (p->email || p->vis_email)
	{
		if (p->vis_email)
			email = p->vis_email;
		else
			email = p->email;

		if (p->full_name)
			s = sout("%s <%s>", p->full_name, email);
		else
			s = sout("<%s>", email);
	}
	else if (p->full_name)
		s = p->full_name;
	else
		s = "";

	if (ilist_lookup(new_players, pl) >= 0)
	{
		c = '*';
		*flag = TRUE;
	}
	else
		c = ' ';

	fprintf(fp, "%4s %c  %s\n",
			box_code_less(pl), c, just_name(pl));
	if (*s)
		fprintf(fp, "        %s\n", s);
	fprintf(fp, "\n");
}


void write_player_list(void)
{
	FILE *fp;
	char *fnam;
	int pl;
	int flag = FALSE;

	stage("write_player_list()");

	fnam = sout("%s/players", libdir);

	fp = fopen(fnam, "w");

	if (fp == NULL)
	{
		fprintf(stderr, "can't write %s", fnam);
		perror("");
		return;
	}

	fprintf(fp, "%4s   %s\n", "num", "faction");
	fprintf(fp, "%4s   %s\n", "---", "-------");

	loop_player(pl)
	{
		if (rp_player(pl) &&
		    rp_player(pl)->email &&
		    subkind(pl) == sub_pl_regular)
			list_a_player(fp, pl, &flag);
	}
	next_player;

	if (flag) {
		fprintf(fp, "\n                * -- New player this turn\n");
	}

	fclose(fp);
}


write_forward_sup(int who_for, int target, FILE *fp)
{
	int pl;
	char *s;

	pl = player(who_for);
	s = player_email(pl);

	if (s && *s)
	{
		fprintf(fp, "%s|%s\n", box_code_less(target), s);
	}
}


void write_forwards(void)
{
	FILE *fp;
	char *fnam;
	int i, j;
	ilist l;

	fnam = sout("%s/forward", libdir);

	fp = fopen(fnam, "w");

	if (fp == NULL)
	{
		fprintf(stderr, "can't write %s", fnam);
		perror("");
		return;
	}

	loop_player(i)
	{
		write_forward_sup(i, i, fp);
	}
	next_player;

	loop_char(i)
	{
		write_forward_sup(i, i, fp);
	}
	next_char;

	loop_garrison(i)
	{
		l = players_who_rule_here(i);

		for (j = 0; j < ilist_len(l); j++)
			if (l[j])
				write_forward_sup(l[j], i, fp);
	}
	next_garrison;

	fclose(fp);
}


write_faction_sup(int who_for, int target, FILE *fp)
{
	int pl;
	char *s;

	pl = player(who_for);
	s = player_email(pl);

	if (s && *s)
	{
		fprintf(fp, "%s|%s\n", box_code_less(target), s);
	}
}


void write_factions(void)
{
	FILE *fp;
	char *fnam;
	int i;

	fnam = sout("%s/factions", libdir);

	fp = fopen(fnam, "w");

	if (fp == NULL)
	{
		fprintf(stderr, "can't write %s", fnam);
		perror("");
		return;
	}

	loop_player(i)
	{
		write_faction_sup(i, i, fp);
	}
	next_player;

	fclose(fp);
}


int
send_rep(int pl, int turn)
{
	struct entity_player *p;
	char report[LEN];
	FILE *fp;
	int ret;
	char *zfnam = NULL;
	char *fnam;
	char *cmd;
	int split_lines = player_split_lines(pl);
	int split_bytes = player_split_bytes(pl);

	p = rp_player(pl);

	if (p == NULL || p->email == NULL || *p->email == '\0')
		return FALSE;

	if (win_flag)
		sprintf(report, "tmp/sendrep%d.%s", getpid(), box_code_less(pl));
	else
		sprintf(report, "/tmp/sendrep%d.%s", getpid(), box_code_less(pl));

	fp = fopen(report, "w");
	if (fp == NULL)
	{
		fprintf(stderr, "send_rep: can't write %s:", report);
		perror("");
		return FALSE;
	}

	fprintf(fp, "From: %s\n", from_host);
	if (reply_host && *reply_host)
		fprintf(fp, "Reply-To: %s\n", reply_host);
	fprintf(fp, "To: %s (%s)\n", p->email,
			p->full_name ? p->full_name : "???");
	fprintf(fp, "Subject: %s - Turn %d report\n", game_title, turn);
	fprintf(fp, "\n");
	fclose(fp);

	fnam = sout("%s/save/%d/%d", libdir, turn, pl);

	if (access(fnam, R_OK) < 0)
	{
		zfnam = sout("%s/save/%d/%d.gz", libdir, turn, pl);

		if (access(zfnam, R_OK) < 0)
		{
			unlink(report);
			return FALSE;
		}

		if (win_flag)
		{
			fnam = sout("tmp/zrep.%d", pl);
			ret = system(sout("gzcat %s > %s", zfnam, fnam));
		}
		else
		{
			fnam = sout("/tmp/zrep.%d", pl);
			ret = system(sout("zcat %s > %s", zfnam, fnam));
		}

		if (ret)
		{
			fprintf(stderr, "couldn't unpack %s\n", zfnam);
			unlink(fnam);
			unlink(report);
			return FALSE;
		}
	}

	if (win_flag) {
		system(sout("g2rep %s >> rep\\%d_%d.txt", fnam, turn, pl));
		if (player_notab(pl)) {
			cmd = sout("g2rep %s >> %s", fnam, report);
		} else {
			system(sout("g2rep %s >> %s", fnam, report));
			cmd = sout("entab %s %s", report, report);
		}
	} else {
		if (player_notab(pl)) {
			cmd = sout("g2rep %s >> %s", fnam, report);
		} else {
			cmd = sout("g2rep %s | unexpand -t8 >> %s", fnam, report);
		}
	}

	ret = system(cmd);

	if (zfnam)
		unlink(fnam);

	if (ret)
	{
		fprintf(stderr, "send_rep: command failed: %s\n", cmd);
		unlink(report);
		return FALSE;
	}

	if (split_lines == 0 && split_bytes == 0)
	{
		if (win_flag) {
			cmd = sout("sendmail %s", report);
		} else {
			cmd = sout("sendmail -t -f %s < %s", from_host, report);
		}
	} else {
		cmd = sout("mailsplit -s %d -l %d -c 'sendmail -t -odq' < %s",
				split_bytes, split_lines, report);
	}

	fprintf(stderr, "   %s\n", cmd);
	ret = system(cmd);

	if (ret)
		fprintf(stderr, "send_rep: mail to %s failed: %s\n",
						p->email, cmd);

	unlink(report);

#if 0
	sleep(5);	/* don't need to sleep if we are just queuing */
#endif

	return (ret == 0);
}


mail_reports()
{
	int pl;

	stage("mail_reports()");

	loop_player(pl)
	{
		send_rep(pl, sysclock.turn);
	}
	next_player;
}

int
v_remail(c)
struct command *c;
{
	mail_reports();
	setup_html_all();
	return TRUE;
}

void setup_html_dir(int pl)
{
	char fnam[LEN];
	char fnam2[LEN];
	FILE *fp;

	sprintf(fnam, "%s/html/%s", libdir, box_code_less(pl));
	if (win_flag) {
		system(sout("mkdir %s\\html\\%s", libdir, box_code_less(pl)));
	} else {
		mkdir(fnam, 0755);
	}

	sprintf(fnam2, "%s/.htaccess", fnam);

	fp = fopen(fnam2, "w");
	if (fp == NULL)
	{
		fprintf(stderr, "can't write %s: ", fnam2);
		perror("");
		return;
	}

	fprintf(fp, "AuthUserFile %s\n", htpasswd_loc);
	fprintf(fp, "AuthGroupFile /dev/null\n");
	fprintf(fp, "AuthName ByPassword\n");
	fprintf(fp, "AuthType Basic\n");
	fprintf(fp, "\n");
	fprintf(fp, "<Limit GET>\n");
	fprintf(fp, "require user %s admin\n", box_code_less(pl));
	fprintf(fp, "</Limit>\n");

	fclose(fp);
}

void
set_html_pass(pl)
int pl;
{
	char buf[LEN];
	struct entity_player *p;
	char *pw;
	int ret;

	p = rp_player(pl);
	if (p == NULL)
		return;

	pw = p->password;
	if (pw == NULL) {

	/* To override the default password below, create/edit the file "PWD" which contains:

fairy fairypassword
combat combatpassword
random randompassword

	   The string up to the first whitespace contains the keyword used to look up the password below
	   The string after the whitespace contains the password to use instead of the default one
	 */
		
		pw = read_pw("random");
		if (pw == NULL)
			pw = "slartibartfast";

	}

	if (win_flag) {
		sprintf(buf, "htpasswd -b lib\\.htpasswd %s \"%s\"",
		box_code_less(pl), pw);
		ret = system(buf);
		if (ret) {
			sprintf(buf, "htpasswd -c -b lib\\.htpasswd %s \"%s\"",
			box_code_less(pl), pw);
			system(buf);
		}
	} else {
		sprintf(buf, "htpasswd -b %s %s \"%s\"",
			htpasswd_loc,
			box_code_less(pl),
			pw);
		ret = system(buf);
		if (ret) {
			sprintf(buf, "htpasswd -c -b %s %s \"%s\"",
				htpasswd_loc,
				box_code_less(pl),
				pw);
			system(buf);
		}
	}

}

output_html_rep(pl)
int pl;
{
	char fnam[LEN];
	char fnam2[LEN];

	sprintf(fnam, "%s/html/%s/index.html", libdir, box_code_less(pl));
	sprintf(fnam2, "%s/html/%s/prev.html", libdir, box_code_less(pl));

	unlink(fnam2);
	rename(fnam, fnam2);

	sprintf(fnam2, "g2rep -h %s/save/%d/%d > %s", libdir,
					sysclock.turn, pl, fnam);
	system(fnam2);
}

copy_public_turns()
{
	char fnam[LEN];
	char fnam2[LEN];
	char cmd[LEN];
	int pl;

	loop_player(pl)
	{
		if (!player_public_turn(pl))
			continue;


		if (win_flag) {
			sprintf(fnam, "%s\\html\\%s\\index.html", libdir, box_code_less(pl));
			sprintf(fnam2, "public\\%s.html", box_code_less(pl));
			sprintf(cmd, "copy %s %s", fnam, fnam2);
			printf(cmd);
			system(cmd);
		} else {
			sprintf(fnam, "%s/html/%s/index.html", libdir, box_code_less(pl));
			sprintf(fnam2, "public/%d.%s", sysclock.turn, box_code_less(pl));
			sprintf(cmd, "sed -e '/<name=\"Order template\">/,$d' < %s > %s",
					fnam, fnam2);
			system(cmd);
		}
	}
	next_player;
}

void
setup_html_all(void)
{
	int pl;

	stage("setup_html()");

	loop_player(pl) {
		setup_html_dir(pl);
		set_html_pass(pl);
		output_html_rep(pl);
	}
	next_player;

	copy_public_turns();
}

void
extract_startlocs()
{
	int city;
	int sequence = 0;
	char *fnam;
	FILE *fp = NULL;

	fnam = sout("%s/startloc", libdir);
	fp = fopen(fnam, "w");
	if (fp)
	{
		loop_city(city)
		{
			if (safe_haven(city))
			{
				fprintf(fp, "%d %d %s\n",
					sequence++,
					city,
					display_name(city)
				);
			}
		}
		next_city;

		fclose(fp);
	}
	else
		fprintf(stderr, "Can't write %s!\n", fnam);
}

int
main(int argc, char **argv)
{
	extern int optind, opterr;
	extern char *optarg;
	int errflag = 0;
	int c;
	int run_flag = FALSE;
	int add_flag = FALSE;
	int eat_fast = FALSE;
	int eat_flag = FALSE;
	int mail_now = FALSE;
	int acct_flag = FALSE;
	int html_flag = FALSE;
	int startloc_flag = FALSE;

	printf("\tsizeof(struct box) = %d\n", sizeof (struct box));
	setbuf(stderr, NULL);

	call_init_routines();

	while ((c = getopt(argc, argv, "waeEfirl:pR?sStMTAh")) != EOF) {
		switch (c) {
	case 'w':
			win_flag = TRUE;
		break;

		case 'a':
			add_flag = TRUE;
			immediate = FALSE;
			break;

		case 'A':
			acct_flag = TRUE;
			break;

		case 'f':
			flush_always = TRUE;
			setbuf(stdout, NULL);
			break;

		case 'e':
			eat_flag = TRUE;
			eat_fast = FALSE;
			immediate = FALSE;
			break;

		case 'E':
			eat_flag = TRUE;
			eat_fast = TRUE;
			immediate = FALSE;
			break;

		case 'i':
			immed_after = TRUE;
			break;

		case 'l':									/* set libdir */
			libdir = str_save(optarg);
			break;

		case 'p':
			pretty_data_files = !pretty_data_files;
			break;

		case 'r':									/* run a turn */
			immediate = FALSE;
			run_flag = TRUE;
			break;

		case 'R':									/* test random number generator */
			test_random();
			return 0;

		case 's':
			startloc_flag = TRUE;
			break;

		case 'S':									/* save database when done */
			save_flag = TRUE;
			break;

		case 't':
			ilist_test();
			return 0;

		case 'T':
			time_self = TRUE;
			break;

		case 'M':
			mail_now = TRUE;
			break;

		case 'h':
			html_flag = TRUE;
			break;

		default:
			errflag++;
		}
	}

	if (errflag) {
		fprintf(stderr, "usage: oly [options]\n");
		fprintf(stderr, "	-w		Windows mode\n");
		fprintf(stderr, "	-a		Add new players mode\n");
		fprintf(stderr, "	-e		Eat orders from libdir/spool\n");
		fprintf(stderr, "	-E		Eat orders from libdir/spool and terminate\n");
		fprintf(stderr, "	-f		Don't buffer files for debugging\n");
		fprintf(stderr, "	-i		Immediate mode\n");
		fprintf(stderr, "	-l dir		Specify libdir, default ./lib\n");
		fprintf(stderr, "	-p		Don't make data files pretty\n");
		fprintf(stderr, "	-r		Run a turn\n");
		fprintf(stderr, "	-R		Test the random number generator\n");
		fprintf(stderr, "	-s		Extract safe havens as start city list\n");
		fprintf(stderr, "	-S		Save the database at completion\n");
		fprintf(stderr, "	-t		Test ilist code\n");
		fprintf(stderr, "	-T		Print timing info\n");
		fprintf(stderr, "	-M		Mail reports\n");
		fprintf(stderr, "	-A		Charge player accounts\n");
		return 1;
	}

	load_db();

	if (eat_flag) {
		eat_loop(eat_fast);
		return 0;
	}

	if (startloc_flag) {
		extract_startlocs();
		return 0;
	}

	if (run_flag) {
		open_logfile();
		open_times();

		show_day = TRUE;
		process_orders();
		post_month();
		show_day = FALSE;

		determine_output_order();
		turn_end_loc_reports();
		list_order_templates();
		player_ent_info();
		character_report();

		player_banner();
		if (acct_flag)
			charge_account();
		report_account();
		summary_report();
		player_report();

		scan_char_skill_lore();
		show_lore_sheets();
		gm_report(gm_player);

		gm_show_all_skills(skill_player);

		add_new_players();
		gen_include_section();			/* must be last */
		close_logfile();

		write_player_list();
		write_email();
		write_totimes();
		write_forwards();
		write_factions();
	}

	if (add_flag) {
		new_player_top(mail_now);
		mail_now = FALSE;
	}

	if (immediate || immed_after) {
		immediate = TRUE;

		open_logfile();
		immediate_commands();
		close_logfile();
	}

	check_db();									 /* check database integrity */

	if (save_flag)
		save_db();

	if (save_flag && run_flag)
		save_logdir();

	if (mail_now)
		mail_reports();

	if (html_flag)
		setup_html_all();

	times_masthead();
	close_times();
	stage(NULL);

	{
		extern int malloc_size, realloc_size;

		printf("\tmalloc, realloc = %d, %d\n", malloc_size, realloc_size);
	}

	return 0;
}
