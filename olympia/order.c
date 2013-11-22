
#include	<stdio.h>
#include	<sys/types.h>
#ifdef _WIN32
#include	<libc/sys/stat.h>
#include	<libc/dirent.h>
#include	<libc/unistd.h>
#else
#include	<sys/stat.h>
#include	<dirent.h>
#include	<unistd.h>
#endif
#include	"z.h"
#include	"oly.h"



/*  order.c -- manage list of unit orders for each faction */


static struct order_list *
p_order_head(int pl, int who)
{
	struct entity_player *p;
	struct order_list *new;
	int i;

	p = p_player(pl);

	for (i = 0; i < plist_len(p->orders); i++)
		if (p->orders[i]->unit == who)
			return p->orders[i];

	new = my_malloc(sizeof(*new));
	new->unit = who;

	plist_append((plist *) &p->orders, new);

	return new;
}


static struct order_list *
rp_order_head(int pl, int who)
{
	struct entity_player *p;
	int i;

	p = rp_player(pl);

	if (p == NULL)
		return NULL;

	for (i = 0; i < plist_len(p->orders); i++)
		if (p->orders[i]->unit == who)
			return p->orders[i];

	return NULL;
}


char *
top_order(int pl, int who)
{
	struct order_list *p;

	p = rp_order_head(pl, who);

	if (p && plist_len(p->l) > 0)
		return p->l[0];
	
	return NULL;
}


static int
is_stop_order(char *s)
{
	char buf[LEN];
	char *p;

	assert(s != NULL);

	while (iswhite(*s))
		s++;

	for (p = buf; *s && !iswhite(*s) && p < &buf[LEN]; p++, s++)
		*p = *s;
	*p = '\0';

	if (i_strcmp(buf, "stop") == 0)
		return TRUE;

	if (fuzzy_strcmp(s, "stop"))
		return TRUE;

	return FALSE;
}


/*
 *  Return TRUE if a stop order is queue for the given unit.
 *  STOP orders must be the first command in the order queue.
 */

int
stop_order(int pl, int who)
{
	char *s;

	s = top_order(pl, who);

	if (s == NULL)
		return FALSE;

	if (is_stop_order(s))
		return TRUE;

	return FALSE;
}


void
pop_order(int pl, int who)
{
	struct order_list *p;

	p = rp_order_head(pl, who);

	assert(p != NULL);
	assert(plist_len(p->l) > 0);

#if 1
	my_free(p->l[0]);
#endif
	plist_delete((plist *) &p->l, 0);
}


void
flush_unit_orders(int pl, int who)
{
	struct command *c;

	while (top_order(pl, who))
		pop_order(pl, who);

	if (player(who) == pl)
	{
		c = rp_command(who);
		if (c && c->state == STATE_LOAD)
			command_done(c);
	}
}


void
queue_order(int pl, int who, char *s)
{
	struct order_list *p;

	p = p_order_head(pl, who);

	if (plist_len(p->l) >= 250)
		return;

	plist_append((plist *) &p->l, str_save(s));
}


void
prepend_order(int pl, int who, char *s)
{
	struct order_list *p;

	p = p_order_head(pl, who);
	plist_prepend((plist *) &p->l, str_save(s));
}


void
queue_stop(int pl, int who)
{

	if (stop_order(pl, who))
		return;

	prepend_order(pl, who, "stop");
}


/*
 *  Loose, convenient interface for queue_order()
 */

queue(who, s, a1, a2, a3, a4, a5, a6, a7, a8, a9)
int who;
char *s;
long a1, a2, a3, a4, a5, a6, a7, a8, a9;
{
	char buf[LEN];

	sprintf(buf, s, a1, a2, a3, a4, a5, a6, a7, a8, a9);
	queue_order(player(who), who, buf);
}


void
save_player_orders(int pl)
{
	struct entity_player *p;
	FILE *fp = NULL;
	int i, j;
	char *fnam;

	assert(valid_box(pl));

	p = rp_player(pl);

	if (p == NULL)
		return;

	for (i = 0; i < plist_len(p->orders); i++)
	{
		if (!valid_box(p->orders[i]->unit) ||
		    kind(p->orders[i]->unit) == T_deadchar)
			continue;

		for (j = 0; j < plist_len(p->orders[i]->l); j++)
		{
			if (fp == NULL)
			{
				fnam = sout("%s/orders/%d", libdir, pl);
				fp = fopen(fnam, "w");

				if (fp == NULL)
				{
					fprintf(stderr, "can't write %s: ",
									fnam);
					perror("");
					return;
				}
			}

			fprintf(fp, "%d:%s\n",
					p->orders[i]->unit,
					p->orders[i]->l[j]);
		}
	}

	if (fp)
		fclose(fp);
}


static void
load_player_orders(int pl)
{
	char *fnam;
	char *line;

	assert(valid_box(pl));
	assert(rp_player(pl)->orders == NULL);

	fnam = sout("%s/orders/%d", libdir, pl);

	if (!readfile(fnam))
		return;

	while (line = readlin())
	{
		int unit;
		char *p;

		unit = atoi(line);

		for (p = line; *p && *p != ':'; p++)
			;
		if (*p == ':')
			p++;

		queue_order(pl, unit, p);
	}

	closefile(fnam);
}


void
load_orders()
{
	DIR *d;
	struct dirent *e;
	char *fnam;
	int fact;

	fnam = sout("%s/orders", libdir);
	d = opendir(fnam);

	if (d == NULL)
	{
		fprintf(stderr, "can't open %s: ", fnam);
		perror("");
		return;
	}

	while ((e = readdir(d)) != NULL)
	{
		if (isdigit(*(e->d_name)))
		{
			fact = atoi(e->d_name);

			if (!valid_box(fact))
			{
				fprintf(stderr,
					"ERROR: orders/%d but no box [%d]\n",
								fact, fact);
				continue;
			}

			load_player_orders(fact);
		}
	}

	closedir(d);
}


void
save_orders()
{
	int i;

	if (win_flag) {
		system(sout("del /s /Q /F %s\\orders\\*", libdir));
	} else {
		system(sout("rm -rf %s/orders", libdir));
		mkdir(sout("%s/orders", libdir), 0755);
	}

	loop_player(i)
	{
		save_player_orders(i);
	}
	next_player;
}


static void
orders_template_sup(int who, int num, int pl)
{
	struct order_list *l;
	struct entity_player *p;
	int i, turns;
	int some = FALSE;
	char *time_left;
	char *nam = "";

	if (pl == player(num))
	{
		if (is_prisoner(num) || kind(num) != T_char)
			nam = sout("  # %s", just_name(num));
		else
			nam = sout("  # %s in %s",
				just_name(num), box_name(subloc(num)));
	}

	out(who, "unit %s%s", box_code_less(num), nam);
	indent += 3;

	if (pl == player(num))
	{
		struct command *c = rp_command(num);

		if (kind(num) == T_player && auto_quit_turns > 0)
		{
			p = p_player(num);
			turns = sysclock.turn - p->last_order_turn;
			if (turns >= (auto_quit_turns / 2))
			{
				out(who, "#");
				out(who, "# You have not sent in orders for %d turn%s.",
						turns, add_s(turns));
				out(who, "# If you do not submit orders for %d turn%s,",
						auto_quit_turns, add_s(auto_quit_turns));
				out(who, "# your faction will be removed from the game.");
				out(who, "#");
			}
		}

		if ((loyal_kind(num) == LOY_contract && loyal_rate(num) < 50) ||
			(loyal_kind(num) == LOY_fear && loyal_rate(num) < 2))
		{
			out(who, "#");
			out(who, "# %s has loyalty %s and will renounce",
					box_code_less(num), loyal_s(num));
			out(who, "# loyalty at the end of this turn.");
			out(who, "#");
		}

		if (c && (c->state == STATE_RUN || c->state == STATE_LOAD))
		{
			if (c->state == STATE_RUN)
			{
				if (c->wait < 0)
					time_left = " (still~executing)";
				else
					time_left = sout(" (executing for %s more day%s)",
						nice_num(c->wait),	
						add_s(c->wait));

				out(who, "# > %s%s", c->line, time_left);
			}
			else	/* command has loaded, but not started yet */
			{
				out(who, "%s", c->line);
			}
		}
	}

	l = rp_order_head(pl, num);

	if (l != NULL && plist_len(l->l) > 0)
	{
		for (i = 0; i < plist_len(l->l); i++)
		{
			out(who, "%s", l->l[i]);
		}
	}

	indent -= 3;
	out(who, "");
}


static void
orders_other(int who, int pl)
{
	struct entity_player *p;
	int first = TRUE;
	int i;

	assert(kind(pl) == T_player);

	p = rp_player(pl);

	if (p == NULL)
		return;

	for (i = 0; i < plist_len(p->orders); i++)
	{
		if (pl == p->orders[i]->unit ||
		    !valid_box(p->orders[i]->unit) ||
		    kind(p->orders[i]->unit) == T_deadchar)
			continue;

		if (ilist_lookup(p->units, p->orders[i]->unit) >= 0)
			continue;

/*
 *  Don't output an empty template for a unit we swore away this turn
 */
		{
			struct command *c = rp_command(p->orders[i]->unit);
			struct order_list *l =
				rp_order_head(pl, p->orders[i]->unit);

			if ((c == NULL || c->state == STATE_DONE) &&
			    (l == NULL || ilist_len(l->l) == 0))
				continue;
		}

		if (first)
		{
			out(who, "#");
			out(who, "# Orders for units you do not "
					"control as of now");
			out(who, "#");
			first = FALSE;
		}

		orders_template_sup(who, p->orders[i]->unit, pl);
	}
}


void
orders_template(int who, int pl)
{
	int i;
	char *pass = "";
	struct entity_player *p;

	if (p = rp_player(pl))
	{
		if (p->password && *(p->password))
			pass = sout(" \"%s\"", p->password);
	}

	out(who, "");
	if (player_broken_mailer(pl))
		out(who, " begin %s%s  # %s", box_code_less(pl), pass, just_name(pl));
	else
		out(who, "begin %s%s  # %s", box_code_less(pl), pass, just_name(pl));

	if (player(who) == pl && kind(who) == T_char &&
	    loyal_kind(who) == LOY_contract && loyal_rate(who) < 50)
	{
		out(who, "");
		out(who, "# WARNING -- loyalty is %s", loyal_s(who));
	}

	out(who, "");

	orders_template_sup(who, pl, pl);

	loop_units(pl, i)
	{
		orders_template_sup(who, i, pl);
	}
	next_unit;

	orders_other(who, pl);

	out(who, "end");
}


void
list_order_templates()
{
	int pl;

	out_path = MASTER;
	out_alt_who = OUT_TEMPLATE;

	loop_player(pl)
	{
		if (subkind(pl) == sub_pl_system ||
		    subkind(pl) == sub_pl_silent)
			continue;

		orders_template(pl, pl);
	}
	next_player;

	out_path = 0;
	out_alt_who = 0;
}

