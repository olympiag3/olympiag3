
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>
#include	"z.h"
#include	"oly.h"


static int
output_order_comp(a, b)
int *a;
int *b;
{

	if (bx[*a]->output_order != bx[*b]->output_order)
		return bx[*a]->output_order - bx[*b]->output_order;

	return *a - *b;
}


void
sort_for_output(ilist l)
{
	qsort(l, ilist_len(l), sizeof(int), output_order_comp);
}


void
determine_output_order()
{
	int count = 0;
	int reg;
	int i;

	loop_loc(reg)
	{
		if (loc_depth(reg) != LOC_region)
			continue;

		bx[reg]->output_order = count++;

		loop_all_here(reg, i)
		{
			bx[i]->output_order = count++;
		}
		next_all_here;
	}
	next_loc;

/*
 *  Sort all player unit lists
 */

	{
		int pl;
		struct entity_player *p;

		loop_player(pl)
		{
			p = rp_player(pl);
			if (p == NULL)
				continue;

			sort_for_output(p->units);
		}
		next_player;
	}
}


void
show_carry_capacity(int who, int num)
{
	struct weights w;
	char *walk_percent = "";
	char buf[LEN];

	out(who, "");

	determine_unit_weights(num, &w);

	if (w.land_cap > 0)
		walk_percent = sout(" (%d%%)",
				w.land_weight * 100 / w.land_cap);
	sprintf(buf, "%s/%s land%s", 
		comma_num(w.land_weight), comma_num(w.land_cap), walk_percent);

	if (w.ride_cap > 0)
	{
		strcat(buf, sout(", %s/%s ride (%d%%)",
			comma_num(w.ride_weight), comma_num(w.ride_cap),
				w.ride_weight * 100 / w.ride_cap));
	}

	if (w.fly_cap > 0)
	{
		strcat(buf, sout(", %s/%s fly (%d%%)",
			comma_num(w.fly_weight), comma_num(w.fly_cap),
				w.fly_weight * 100 / w.fly_cap));
	}

	wiout(who, strlen("Capacity:  "), "Capacity:  %s", buf);
}


static void
show_item_skills_sup(int who, int item, struct item_magic *p)
{
	int i;
	char *req_s;
	int sk;
	int parent;
	int first;
#if 0
	int see_magic;

	see_magic = is_magician(who);
#endif

	first = TRUE;
	for (i = 0; i < ilist_len(p->may_study); i++)
	{
		sk = p->may_study[i];
		assert(valid_box(sk));
		parent = skill_school(sk);

#if 0
		if (magic_skill(sk) && !see_magic)
			continue;
#endif

		if (first)
		{
			out(who, "");
			wout(who, "%s permits study of the following skills:",
						box_name(item));
			indent += 3;
			first = FALSE;
		}

		if (sk == parent)
			req_s = "";
		else
			req_s = sout(" (requires %s)",
						just_name(parent));

		if (sk != parent && has_skill(who, parent) < 1)
			wiout(who, 3, "???%s", req_s);
		else
			wiout(who, 3, "%s%s", box_name(sk), req_s);
	}

	if (!first)
		indent -= 3;

	first = TRUE;
	for (i = 0; i < ilist_len(p->may_use); i++)
	{
		sk = p->may_use[i];
		assert(valid_box(sk));
		parent = skill_school(sk);

#if 0
		if (magic_skill(sk) && !see_magic)
			continue;
#endif

		if (first)
		{
			out(who, "");
			wout(who, "%s permits use of the following skills:",
						box_name(item));
			indent += 3;
			first = FALSE;
		}

		if (sk == parent)
			req_s = "";
		else
			req_s = sout(" (requires %s)",
						just_name(parent));

		if (sk != parent && has_skill(who, parent) < 1)
			wiout(who, 3, "???%s", req_s);
		else
			wiout(who, 3, "%s%s", box_name(sk), req_s);
	}

	if (!first)
		indent -= 3;
}


void
show_item_skills(int who, int num)
{
	struct item_ent *e;
	struct item_magic *p;

	loop_inv(num, e)
	{
		p = rp_item_magic(e->item);

		if (p)
			show_item_skills_sup(who, e->item, p);
	}
	next_inv;
}


static int
inv_item_comp(a, b)
struct item_ent **a;
struct item_ent **b;
{

	return (*a)->item - (*b)->item;
}


static char *
extra_item_info(int who, int item, int qty)
{
	char buf[LEN];
	int lc, rc, fc;
	int at, df, mi;
	int n;

	*buf = '\0';

	lc = item_land_cap(item);
	rc = item_ride_cap(item);
	fc = item_fly_cap(item);

	if (fc > 0)
		sprintf(buf, "fly %s", nice_num(fc * qty));
	else if (rc > 0)
		sprintf(buf, "ride %s", nice_num(rc * qty));
	else if (lc > 0)
		sprintf(buf, "cap %s", nice_num(lc * qty));

	at = item_attack(item);
	df = item_defense(item);
	mi = item_missile(item);

	if (is_fighter(item))
		strcat(buf, sout(" (%d,%d,%d)", at, df, mi));

	if (n = item_attack_bonus(item))
		strcat(buf, sout("+%d attack", n));

	if (n = item_defense_bonus(item))
		strcat(buf, sout("+%d defense", n));

	if (n = item_missile_bonus(item))
		strcat(buf, sout("+%d missile", n));

	if (n = item_aura_bonus(item))
	{
		if (who && is_magician(who))
			strcat(buf, sout("+%d aura", n));
	}

	return sout("%s", buf);
}


void
show_char_inventory(int who, int num)
{
	int first = TRUE;
	struct item_ent *e;
	int weight;
	int count = 0;
	int total_weight = 0;

	if (plist_len(bx[num]->items) > 0)
	{
		qsort(bx[num]->items, ilist_len(bx[num]->items),
					sizeof(*bx[num]->items), inv_item_comp);
	}

	loop_inv(num, e)
	{
		weight = item_weight(e->item) * e->qty;

		if (first)
		{
			out(who, "");
			out(who, "Inventory:");
			out(who, "{<b>}%9s  %-30s %9s{</b>}",
						   "qty", "name", "weight");
			out(who, "%9s  %-30s %9s", "---", "----", "------");
			first = FALSE;
		}

		out(who, "%9s  %-30s %9s  %s",
				comma_num(e->qty),
				plural_item_box(e->item, e->qty),
				comma_num(weight),
				extra_item_info(who, e->item, e->qty));
		count++;
		total_weight += weight;
	}
	next_inv;

	if (count > 0)
	{
		out(who, "%9s  %-30s %9s",
				"", "", "======");
		out(who, "%9s  %-30s %9s", "", "", comma_num(total_weight));
	}

	if (first)
	{
		out(who, "");
		out(who, "%s has no possessions.", box_name(num));
	}
}


/*
 *	1.  building		%s, in
 *
 *	2.  land subloc		%s, in province
 *	3.  ocean subloc	%s, in
 *
 *	4.  land province	%s, in region %s
 *	5.  ocean province	%s, in %s
 */

char *
char_rep_location(int who)
{
	int where = subloc(who);
	char *s = "";
	char *reg_name;

	if (where == 0)
		return "nowhere";

	while (loc_depth(where) > LOC_province)
	{
		if (*s)
			s = sout("%s, in %s", s, box_name(where));
		else
			s = box_name(where);
		where = loc(where);
	}

	if (subkind(province(where)) == sub_ocean)
	{
		if (*s)
			s = sout("%s, in %s", s, box_name(where));
		else
			s = box_name(where);

		reg_name = name(region(where));

		if (reg_name && *reg_name)
			s = sout("%s, in %s", s, reg_name);
	}
	else
	{
		if (*s)
			s = sout("%s, in province %s", s, box_name(where));
		else
			s = box_name(where);

		reg_name = name(region(where));

#if 0
		if (reg_name && *reg_name)
			s = sout("%s, in region %s", s, reg_name);
#else
		if (reg_name && *reg_name)
			s = sout("%s, in %s", s, reg_name);
#endif
	}

	return s;
}


static void
char_rep_stack_info(int who, int num)
{
	int n;
	int first = TRUE;

	if (n = stack_parent(num))
		wiout(who, 16,  "Stacked under:  %s", box_name(n));

	loop_here(num, n)
	{
	    if (kind(n) == T_char && !is_prisoner(n))
	    {
		if (first)
		{
			out(who, "Stacked over:   %s", box_name(n));
			first = FALSE;
		}
		else
			out(who, "                %s", box_name(n));
	    }
	}
	next_here;
}


static int pledge_backlinks = FALSE;

static void
collect_pledge_backlinks()
{
	int i;
	int n;
	struct char_magic *p;

	pledge_backlinks = TRUE;

	loop_char(i)
	{
		if (n = char_pledge(i))
		{
			p = p_magic(n);
			ilist_append(&p->pledged_to_us, i);
		}
	}
	next_char;
}


static void
show_pledged(int who, int num)
{
	int i, n;
	int first = TRUE;
	struct char_magic *p;

	if (!pledge_backlinks)
		collect_pledge_backlinks();

	if (n = char_pledge(num))
		wiout(who, 16,  "Pledged to:     %s", box_name(n));

	p = rp_magic(num);

	if (p)
	{
		for (i = 0; i < ilist_len(p->pledged_to_us); i++)
		{
			n = p->pledged_to_us[i];

			if (first)
			{
				out(who, "Pledged to us:  %s", box_name(n));
				first = FALSE;
			}
			else
				out(who, "                %s", box_name(n));
		}
	}
}


static char *
prisoner_health(int who)
{
	int health = char_health(who);

	assert(health != 0);

	if (health < 0)
		return "";

	return sout(", health %d", health);
}


static void
char_rep_prisoners(int who, int num)
{
	int n;
	int first = TRUE;

	loop_here(num, n)
	{
	    if (kind(n) == T_char && is_prisoner(n))
	    {
		if (first)
		{
			out(who, "Prisoners:      %s%s",
					box_name(n),
					prisoner_health(n));
			first = FALSE;
		}
		else
			out(who, "                %s%s",
					box_name(n),
					prisoner_health(n));
	    }
	}
	next_here;
}


static void
char_rep_health(int who, int num)
{
	int n;
	char *s = "";

	n = char_health(num);

	if (n == -1)
	{
		out(who, "Health:         n/a");
		return;
	}

	if (n > 0 && n < 100)
	{
		if (char_sick(num))
			s = " (getting worse)";
		else
			s = " (getting better)";
	}

	out(who, "Health:         %d%%%s", n, s);
}

static void
char_rep_combat(int who, int num)
{
	int n;
	char *s;
	int mk, att, def, mis;

	mk = noble_item(num);
	if (mk == 0)
	{
		att = char_attack(num);
		def = char_defense(num);
		mis = char_missile(num);
	}
	else
	{
		att = item_attack(mk);
		def = item_defense(mk);
		mis = item_missile(mk);
	}

	out(who, "Combat:         attack %d, defense %d, missile %d",
					att, def, mis);

	n = char_behind(num);

	if (n == 0)
		s = " (front line in combat)";
	else
		s = " (stay behind in combat)";

	out(who, "                behind %d %s", n, s);

	if (char_break(num) == 0)
		s = " (fight to the death)";
	else if (char_break(num) == 100)
		s = " (break almost immediately)";
	else
		s = "";

	out(who, "Break point:    %d%%%s", char_break(num), s);
}


static void
char_rep_misc(int who, int num)
{
	char *s;
	struct char_magic *p;

	p = rp_magic(num);
	if (p && p->ability_shroud)
		out(who, "Ability shroud: %d aura", p->ability_shroud);

	if (has_skill(num, sk_hide_self))
	{
		if (char_hidden(num))
		{
			if (char_really_hidden(num))
				s = "concealing self";
			else
				s = "concealing self, but not alone";
		}
		else
			s = "not concealing self";

		out(who, "use %4d %d      (%s)",
			sk_hide_self, char_hidden(num), s);
	}

	if (vision_protect(num) > 0)
	{
		out(who, "Receive Vision: %d protection", vision_protect(num));
	}
}


static void
char_rep_magic(int who, int num)
{
	int ca, ma, mea;

	ca = char_cur_aura(num);
	ma = char_max_aura(num);
	mea = max_eff_aura(num);

	out(who, "");
	out(who, "Current aura:   %d", ca);

	if (ma < mea)
		out(who, "Maximum aura:   %d (%d+%d)",
				mea, ma, mea - ma);
	else
		out(who, "Maximum aura:   %d", ma);

	if (char_abil_shroud(num) > 0)
		out(who, "Ability shroud: %d aura",
					char_abil_shroud(num));

	if (is_loc_or_ship(char_proj_cast(num)))
		out(who, "Project cast:   %s",
				box_name(char_proj_cast(num)));

	if (char_quick_cast(num))
		out(who, "Quicken cast:   %d",
					char_quick_cast(num));
}


void
char_rep_sup(int who, int num)
{

	wiout(who, 16,	"Location:       %s", char_rep_location(num));
	out(who, "Loyalty:        %s", cap(loyal_s(num)));

	char_rep_stack_info(who, num);
	char_rep_health(who, num);
	char_rep_combat(who, num);
	char_rep_misc(who, num);

	if (banner(num))
		out(who, "Banner:         %s", banner(num));
	show_pledged(who, num);

	if (is_magician(num))
		char_rep_magic(who, num);
	char_rep_prisoners(who, num);

	print_att(who, num);
	list_skills(who, num);
	list_partial_skills(who, num);
	show_char_inventory(who, num);
	show_carry_capacity(who, num);
	show_item_skills(who, num);
	list_pending_trades(who, num);

	out(who, "");
}


void
character_report()
{
	int who;

	stage("character_report()");

	indent += 3;

	loop_char(who)
	{
		if (subkind(player(who)) == sub_pl_silent)
			continue;

		if (is_prisoner(who))
		{
			p_char(who)->prisoner = FALSE;	/* turn output on */
			out(who, "%s is being held prisoner.", box_name(who));
			out(who, "");
			p_char(who)->prisoner = TRUE;	/* output off again */
		}
		else
		{
			out(who, "");
			char_rep_sup(who, who);
		}
	}
	next_char;

	indent -= 3;
}


void
show_unclaimed(int who, int num)
{
	int first = TRUE;
	struct item_ent *e;
	int weight;

	if (plist_len(bx[num]->items) > 0)
	{
		qsort(bx[num]->items, ilist_len(bx[num]->items),
					sizeof(*bx[num]->items), inv_item_comp);
	}

	loop_inv(num, e)
	{
		weight = item_weight(e->item) * e->qty;

		if (first)
		{
			out(who, "");
			out(who, "Unclaimed items:");
			out(who, "");
			out(who, "%9s  %-30s %9s", "qty", "name", "weight");
			out(who, "%9s  %-30s %9s", "---", "----", "------");
			first = FALSE;
		}

		out(who, "%9s  %-30s %9s  %s",
				comma_num(e->qty),
				plural_item_box(e->item, e->qty),
				comma_num(weight),
				extra_item_info(0, e->item, e->qty));
	}
	next_inv;
}


void
player_ent_info()
{
	int pl;

	loop_player(pl)
	{
		if (subkind(pl) == sub_pl_silent)
			continue;

		print_admit(pl);
		print_att(pl, pl);
		show_unclaimed(pl, pl);
	}
	next_player
}


static int
sum_fighters(int who)
{
	int sum = 0;
	struct item_ent *t;

	loop_inv(who, t)
	{
		if (man_item(t->item) && is_fighter(t->item))
		{
			int val;

			val = max(item_attack(t->item), max(item_defense(t->item), item_missile(t->item)));

			if (val > 1)
				sum += t->qty;
		}
	}
	next_inv;

	return sum;
}


#define	TRUNC_NAME	15


static char *
stupid_words[] = {
	"a",
	"the",
	"of",
	"de",
	"des",
	"la",
	"and",
	"du",
	"aux",
	"et",
	"ses",
	"avec",
	"un",
	"van",
	"von",
	"-",
	"--",
	NULL
};


static char *
strip_leading_stupid_word(char *s)
{
	char *t;
	int i;
	int len;

	for (i = 0; stupid_words[i]; i++)
	{
		len = strlen(stupid_words[i]);

		if (i_strncmp(s, stupid_words[i], len) == 0 &&
		    s[len] == ' ')
		{
			t = &s[len];
			while (*t == ' ')
				t++;
			if (*t)
				return t;
			else
				return s;
		}
	}

	return s;
}


int
stupid_word(char *s)
{

	return lookup(stupid_words, s) >= 0;
}


static char *
prev_word(char *s, char *t)
{

	while (t > s && *t != ' ')
		t--;

	if (t > s)
		return t;
	return NULL;
}


static char *
summary_trunc_name(int who)
{
	char *s, *t;

	s = sout("%s", just_name(who));

	if (strlen(s) <= TRUNC_NAME)
		return s;

	s = strip_leading_stupid_word(s);

	if (strlen(s) <= TRUNC_NAME)
		return s;

	t = prev_word(s, &s[TRUNC_NAME]);
	if (t)
		*t = '\0';

	while ((t = prev_word(s, t)) && *t == ' ' && stupid_word(t+1))
		*t = '\0';

	s[TRUNC_NAME] = '\0';		/* catches a case */

	return s;
}


static int sum_gold;
static int sum_peas;
static int sum_work;
static int sum_sail;
static int sum_fight;

static char *loyal_chars = "ucofns";


static void
unit_summary_sup(int pl, int who)
{
	char *nam;
	char *health_s;
	char *under_s;
	char *loy_s;
	char *cur_aura_s;
	int pr = is_prisoner(who);
	int gold, peas, work, sail, fight;
	char buf[LEN];
	int n;

	nam = summary_trunc_name(who);

	n = char_health(who);

	if (n == 100)
		health_s = "100 ";
	else if (n == -1)
		health_s = "n/a ";
	else if (char_sick(who))
		health_s = sout("%d-", char_health(who));
	else
		health_s = sout("%d+", char_health(who));

	if (pr)
		under_s = " ?? ";
	else if (stack_parent(who))
		under_s = box_code_less(stack_parent(who));
	else
		under_s = "";

	if (is_magician(who))
		cur_aura_s = sout("%d", char_cur_aura(who));
	else
		cur_aura_s = "";

	gold = has_item(who, item_gold);
	peas = has_item(who, item_peasant);
	work = has_item(who, item_worker);
	sail = has_item(who, item_sailor);
	fight = sum_fighters(who);

	loy_s = sout("%c%s", loyal_chars[loyal_kind(who)],
				knum(loyal_rate(who), FALSE));

	sprintf(buf, "%-*s %-*s %-5s %4s%2d%3s %4s %4s %4s %4s %4s %-*s %s",
		CHAR_FIELD, box_code_less(who),
		CHAR_FIELD, pr ? " ?? " : box_code_less(subloc(who)),
		loy_s,
		health_s,
		char_behind(who),
		cur_aura_s,
		knum(gold, TRUE),
		knum(peas, TRUE),
		knum(work, TRUE),
		knum(sail, TRUE),
		knum(fight, TRUE),
		CHAR_FIELD, under_s,
		nam);

	out(pl, "%s", buf);

	sum_gold += gold;
	sum_peas += peas;
	sum_work += work;
	sum_sail += sail;
	sum_fight += fight;
}


void
unit_summary(int pl)
{
	int i;
	int count = 0;

	clear_temps(T_char);

	sum_gold = 0;
	sum_peas = 0;
	sum_work = 0;
	sum_sail = 0;
	sum_fight = 0;

	out(pl, "");

	count = ilist_len(p_player(pl)->units);

	if (count <= 0)
		return;

	out(pl, "{<b>}%-*s %-*s loyal heal B CA gold peas work sail figh %-*s name{</b>}",
				CHAR_FIELD, "unit",
				CHAR_FIELD, "where",
				CHAR_FIELD, "under");
	out(pl, "%-*s %-*s ----- ---- - -- ---- ---- ---- ---- ---- %-*s ----",
				CHAR_FIELD, "----",
				CHAR_FIELD, "-----",
				CHAR_FIELD, "-----");

	loop_units(pl, i)
	{
		unit_summary_sup(pl, i);
	}
	next_unit;

	if (count > 1)
	{
	    out(pl, "%*s %-*s                 ==== ==== ==== ==== ====",
			CHAR_FIELD, "",
			CHAR_FIELD, "");
	    out(pl, "%*s %-*s                 %4s %4s %4s %4s %4s",
			CHAR_FIELD, "",
			CHAR_FIELD, "",
			knum(sum_gold, TRUE),
			knum(sum_peas, TRUE),
			knum(sum_work, TRUE),
			knum(sum_sail, TRUE),
			knum(sum_fight, TRUE));
	}
}


static char *
loc_ind_s(int where)
{
	int ld;

	ld = loc_depth(where) - 1;

	if (ld <= 0)
		return just_name(where);

	return sout("%s%s", &spaces[spaces_len - (ld * 2)], box_name(where));
}


static void
loc_stack_catchup(int pl, int where)
{

	if (where == 0 || bx[where]->temp)
		return;

	loc_stack_catchup(pl, loc(where));
	out(pl, "%s", loc_ind_s(where));
	bx[where]->temp = -1;
}


static int loc_stack_explain;


static void
loc_stack_rep_sup(int pl, int where, int who)
{
	char *where_s = "";
	char *star = "";
	char *ind = "";

	if (where)
	{
		loc_stack_catchup(pl, loc(where));
		where_s = loc_ind_s(where);
	}

	if (player(who) != pl)
	{
		star = " *";
		loc_stack_explain = TRUE;
	}

	if (kind(loc(who)) == T_char)
		ind = "  ";

	out(pl, "%-34s %s%s%s", where_s, ind, box_name(who), star);
}


void
loc_stack_report(int pl)
{
	int i, j;
	static ilist l = NULL;
	static ilist locs = NULL;

	ilist_clear(&l);
	ilist_clear(&locs);

	clear_temps(T_loc);
	clear_temps(T_ship);
	clear_temps(T_char);

	loc_stack_explain = FALSE;

	loop_units(pl, i)
	{
		if (is_prisoner(i))
			continue;

		ilist_append(&l, i);
	}
	next_unit;

	if (ilist_len(l) < 1)
		return;

	out(pl, "");
	out(pl, "{<b>}%-34s %s{</b>}", "Location", "Stack");
	out(pl, "%-34s %s", "--------", "-----");

	sort_for_output(l);

	for (i = ilist_len(l) - 1; i >= 0; i--)
	{
		int where = subloc(l[i]);

		if (bx[where]->temp == 0)
			ilist_append(&locs, where);

		bx[l[i]]->temp = bx[where]->temp;
		bx[where]->temp = l[i];
	}

	sort_for_output(locs);

	for (i = 0; i < ilist_len(locs); i++)
	{
		j = bx[locs[i]]->temp;
		assert(valid_box(j));

		loc_stack_rep_sup(pl, locs[i], j);

		while (j = bx[j]->temp)
		{
			assert(valid_box(j));
			loc_stack_rep_sup(pl, 0, j);
		}
	}

	if (loc_stack_explain)
	{
		out(pl, "");
		out(pl, "%-34s    %s", "",
			"* -- unit belongs to another faction");
	}
}


void
player_report_sup(int pl)
{
	struct entity_player *p;

	if (subkind(pl) == sub_pl_system)
		return;

	p = p_player(pl);

	out(pl, "Noble points:  %d     (%d gained, %d spent)",
				p->noble_points,
				p->np_gained,
				p->np_spent);

	print_hiring_status(pl);
	print_unformed(pl);

	if (p->fast_study > 0)
	{
		out(pl, "%d fast study day%s are left.",
			p->fast_study, add_s(p->fast_study));
		out(pl, "");
	}
}


void
stack_capacity_report(int pl)
{
	struct weights w;
	int who;
	char *walk_s = "";
	char *ride_s = "";
	char *fly_s = "";
	char *s;
	int first = TRUE;
	int n;

	loop_units(pl, who)
	{
		if (first)
		{
			out(pl, "");
			out(pl, "{<b>}%*s  %10s %15s %15s %15s{</b>}",
					CHAR_FIELD, "stack",
					"total wt",
					"   walk   ",
					"   ride   ",
					"   fly    ");
			out(pl, "%*s  %10s %15s %15s %15s",
					CHAR_FIELD, "-----",
					"--------",
					"-----------",
					"-----------",
					"-----------");
			first = FALSE;
		}

		determine_stack_weights(who, &w);

		if (w.land_cap > 0)
		{
			n = w.land_weight * 100 / w.land_cap;

			if (n > 999)
				s = " -- ";
			else
				s = sout("%3d%%", n);

			if (w.land_weight > w.land_cap)
				walk_s = sout("(%s) %s",
					comma_num(w.land_weight - w.land_cap),
					s);
			else
				walk_s = sout("%s %s",
					comma_num(w.land_cap - w.land_weight),
					s);
		}
		else
			walk_s = "";

		if (w.ride_cap > 0)
		{
			n = w.ride_weight * 100 / w.ride_cap;

			if (n > 999)
				s = " -- ";
			else
				s = sout("%3d%%", n);

			if (w.ride_weight > w.ride_cap)
				ride_s = sout("(%s) %s",
					comma_num(w.ride_weight - w.ride_cap),
					s);
			else
				ride_s = sout("%s %s",
					comma_num(w.ride_cap - w.ride_weight),
					s);
		}
		else
			ride_s = "";

		if (w.fly_cap > 0)
		{
			n = w.fly_weight * 100 / w.fly_cap;

			if (n > 999)
				s = " -- ";
			else
				s = sout("%3d%%", n);

			if (w.fly_weight > w.fly_cap)
				fly_s = sout("(%s) %s",
					comma_num(w.fly_weight - w.fly_cap),
					s);
			else
				fly_s = sout("%s %s",
					comma_num(w.fly_cap - w.fly_weight),
					s);
		}
		else
			fly_s = "";

		out(pl, "%*s  %10s %15s %15s %15s",
			CHAR_FIELD, box_code_less(who),
			comma_num(w.total_weight),
			walk_s,
			ride_s,
			fly_s);
	}
	next_unit;
}


void
player_report()
{
	int pl;

	stage("player_report()");

	out_path = MASTER;
	out_alt_who = OUT_BANNER;

	loop_player(pl)
	{
		if (subkind(pl) == sub_pl_silent)
			continue;

		player_report_sup(pl);
		unit_summary(pl);
		loc_stack_report(pl);
		stack_capacity_report(pl);
		storm_report(pl);
		garrison_summary(pl);
		out(pl, "");
	}
	next_player;

	out_path = 0;
	out_alt_who = 0;
}


static void
rep_player(int pl)
{
	char *s;

	s = box_name(pl);

	lines(pl, s);
	out(pl, "#include %d", pl);
	out(pl, "");
}


static void
rep_char(int pl, ilist l)
{
	int i;
	char *s;

	sort_for_output(l);

	for (i = 0; i < ilist_len(l); i++)
	{
		if (subkind(l[i]) == sub_dead_body)
		{
			s = sout("%s~%s",
				p_misc(l[i])->save_name, box_code(l[i]));
		}
		else
			s = box_name(l[i]);

		out(pl, "");
		lines(pl, s);
		out(pl, "#include %d", l[i]);
	}
}


static void
rep_loc(int pl, ilist l)
{
	int i;
	char *s;

	sort_for_output(l);

	for (i = 0; i < ilist_len(l); i++)
	{
#if 1
		s = show_loc_header(l[i]);
#else
		s = char_rep_location(l[i]);
#endif
		lines(pl, s);
		out(pl, "#include %d", l[i]);
		out(pl, "");
	}
}


static void
inc(int pl, int code, char *s)
{

	lines(pl, s);
	out(pl, "#include %d", code);
	out(pl, "");
}


void
gen_include_sup(int pl)
{
	static ilist char_l = NULL;
	static ilist loc_l = NULL;
	int n;

	int player_output = FALSE;
	int lore_flag = FALSE;
	int new_flag = FALSE;
	int loc_flag = FALSE;
	int code_flag = FALSE;
	int special_flag = FALSE;
	int death_flag = FALSE;
	int misc_flag = FALSE;
	int eat_queue = FALSE;
	int eat_warn = FALSE;
	int eat_error = FALSE;
	int eat_headers = FALSE;
	int eat_okay = FALSE;
	int eat_players = FALSE;
	int template_flag = FALSE;
	int garr_flag = FALSE;
	int drop_flag = FALSE;
	int show_post = FALSE;

	ilist_clear(&char_l);
	ilist_clear(&loc_l);

	loop_known(p_player(pl)->output, n)
	{
		switch (n)
		{
		case OUT_BANNER:
		case OUT_INCLUDE:
			continue;

		case OUT_LORE:		lore_flag = TRUE;	continue;
		case OUT_NEW:		new_flag = TRUE;	continue;
		case OUT_LOC:		loc_flag = TRUE;	continue;
		case OUT_TEMPLATE:	template_flag = TRUE;	continue;
		case OUT_GARR:		garr_flag = TRUE;	continue;
		case OUT_SHOW_POSTS:	show_post = TRUE;	continue;
		case LOG_CODE:		code_flag = TRUE;	continue;
		case LOG_SPECIAL:	special_flag = TRUE;	continue;
		case LOG_DROP:		drop_flag = TRUE;	continue;
		case LOG_DEATH:		death_flag = TRUE;	continue;
		case LOG_MISC:		misc_flag = TRUE;	continue;
		case EAT_ERR:		eat_error = TRUE;	continue;
		case EAT_WARN:		eat_warn = TRUE;	continue;
		case EAT_QUEUE:		eat_queue = TRUE;	continue;
		case EAT_HEADERS:	eat_headers = TRUE;	continue;
		case EAT_OKAY:		eat_okay = TRUE;	continue;
		case EAT_PLAYERS:	eat_players = TRUE;	continue;
		}

		if (!valid_box(n))	/* doesn't exist anymore */
			continue;

		switch (kind(n))
		{
		case T_char:
		case T_deadchar:
			ilist_append(&char_l, n);
			break;

		case T_loc:
		case T_ship:
			ilist_append(&loc_l, n);
			break;

		case T_player:
			assert(n == pl);
			player_output = TRUE;
			break;

		case T_item:
			if (subkind(n) == sub_dead_body)
				ilist_append(&char_l, n);
			else
				assert(FALSE);
			break;

		default:
			assert(FALSE);
		}
	}
	next_known;

	out(pl, "#include %d", OUT_BANNER);
	out(pl, "");

	if (eat_okay)
	{
		out(pl, "#include %d", EAT_OKAY);
		out(pl, "");
	}

	if (drop_flag)		inc(pl, LOG_DROP, "Player drops");
	if (code_flag)		inc(pl, LOG_CODE, "Code alerts");
	if (special_flag)	inc(pl, LOG_SPECIAL, "Special events");
	if (misc_flag)		inc(pl, LOG_MISC, "Miscellaneous");
	if (death_flag)		inc(pl, LOG_DEATH, "Character deaths");
	if (eat_error)		inc(pl, EAT_ERR, "Errors");
	if (eat_warn)		inc(pl, EAT_WARN, "Warnings");
	if (show_post)		inc(pl, OUT_SHOW_POSTS, "Press and rumors");
	if (eat_queue)		inc(pl, EAT_QUEUE, "Current order queues");

	if (pl != eat_pl && player_output)
		rep_player(pl);

	if (garr_flag)
		inc(pl, OUT_GARR, "Garrison log");

	if (loc_flag)
	{
		out(pl, "#include %d", OUT_LOC);
	}

	rep_char(pl, char_l);
	rep_loc(pl, loc_l);

#if 1
	if (lore_flag)
		inc(pl, OUT_LORE, "Lore sheets");
#else
	if (lore_flag)
	{
		out(pl, "#include %d", OUT_LORE);
	}
#endif

	if (new_flag)
		inc(pl, OUT_NEW, "New players");

	if (template_flag)
		inc(pl, OUT_TEMPLATE, "Order template");

	if (eat_players)
		inc(pl, EAT_PLAYERS, "Current player list");

	if (eat_headers)
		inc(pl, EAT_HEADERS, "Original message");
}


void
gen_include_section()
{
	int pl;

	out_path = MASTER;
	out_alt_who = OUT_INCLUDE;

	loop_player(pl)
	{
		if (subkind(pl) != sub_pl_silent)
			gen_include_sup(pl);
	}
	next_player;

	out_path = 0;
	out_alt_who = 0;
}


void
turn_end_loc_reports()
{
	int pl;
	int i;
	struct entity_player *p;
	extern int show_loc_no_header;	/* argument to show_loc() */
	int separate;

	stage("turn_end_loc_reports()");

	out_path = MASTER;
	show_loc_no_header = TRUE;

	loop_player(pl)
	{
		if (subkind(pl) == sub_pl_silent)
			continue;

		if (player_format(pl))
			separate = TRUE;
		else
			separate = FALSE;

		p = p_player(pl);

		loop_known(p->locs, i)
		{
			if (!valid_box(i))	/* loc doesn't exit anymore */
				continue;	/* ex: mine has collapsed */

			if (separate)
			{
				out_alt_who = OUT_LOC;

				lines(pl, show_loc_header(i));
				show_loc(pl, i);
				out(pl, "");
			}
			else
			{
				out_alt_who = i;
				out(pl, "");
				show_loc(pl, i);
			}
		}
		next_known;
	}
	next_player;

	out_path = 0;
	out_alt_who = 0;
	show_loc_no_header = FALSE;
}


void
player_banner()
{
	int pl;
	struct entity_player *p;

	stage("player_banner()");

	out_path = MASTER;
	out_alt_who = OUT_BANNER;

	loop_player(pl)
	{
		if (subkind(pl) == sub_pl_silent)
			continue;

		html(pl, "<pre>");

		p = p_player(pl);

		if (player_compuserve(pl))
		{
			indent += 3;
			wout(pl, "Note:  Olympia Times mailing is turned off for you."
				"To begin receiving the Times via email again, issue "
				"the order TIMES 0");
			indent -= 3;
			out(pl, "");
		}

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
		wout(pl, "Report for %s.", box_name(pl));
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
	}
	next_player;

	out_path = 0;
	out_alt_who = 0;
}


void
report_account_sup(int pl)
{
	char fnam[LEN];
	char cmd[LEN];
	FILE *fp;
	char *line;

#if 0
	sprintf(fnam, "/tmp/oly-acct.tmp");

	sprintf(cmd, "/u/oly/bin/acct -g g2 -p %s -s 4 > %s",
					box_code_less(pl), fnam);
	system(cmd);

	style(TEXT);
	out(pl, "Account summary");
	out(pl, "---------------");
	style(HTML);
	out(pl, "<font size=+1><b>Account summary</b></font>");
	out(pl, "<hr>");
	style(0);

	indent += 3;

	fp = fopen(fnam, "r");

	if (fp == NULL)
	{
		out(pl, "<account summary not available>");
		fprintf(stderr, "can't open %s: ", fnam);
		perror("");
		out(pl, "");
		unlink(fnam);
		indent -= 3;
		return;
	}

	while (line = getlin(fp))
		out(pl, "%s", line);

	fclose(fp);
	unlink(fnam);

	indent -= 3;

	style(TEXT);
	out(pl, "---------------");
#endif


#if 0
	style(HTML);
	out(pl, "<hr>");
#endif
	style(0);

	html(pl, "<hr><center><font size=+1>");
	html(pl, "<a href=\"%s\">Rules</a>   "
		"<a href=\"%s\"><i>The Olympia Times</i></a>",
		rules_url,
		times_url);
	html(pl, "</font></center>");

	out(pl, "");

}


void
report_account()
{
	int pl;

	stage("report_account()");

	out_path = MASTER;
	out_alt_who = OUT_BANNER;

	loop_player(pl)
	{
		if (subkind(pl) != sub_pl_regular)
			continue;

		report_account_sup(pl);
	}
	next_player;

	out_path = 0;
	out_alt_who = 0;
}


void
charge_account()
{
	int pl;
	struct entity_player *p;
	char cmd[LEN];
	char *val_s;

	stage("charge_account()");

	out_path = MASTER;
	out_alt_who = OUT_BANNER;

	loop_player(pl)
	{
		if (subkind(pl) != sub_pl_regular)
			continue;

		p = rp_player(pl);
		if (p == NULL)
			continue;

		if (p->cmd_count < 2)
			val_s = "0.50";
		else
			val_s = "2.50";

		sprintf(cmd, "/u/oly/bin/acct -g g2 -p %s -t%s -y \"olympia g2 turn %d\"",
				box_code_less(pl),
				val_s,
				sysclock.turn);

		system(cmd);
	}
	next_player;

	out_path = 0;
	out_alt_who = 0;
}

