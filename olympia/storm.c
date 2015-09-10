
#include	<stdio.h>
#include	<string.h>
#include	"z.h"
#include	"oly.h"


int
v_bind_storm(struct command *c)
{
	int storm = c->a;
	int ship = subloc(c->who);

	if (kind(storm) != T_storm || npc_summoner(storm) != c->who)
	{
		wout(c->who, "%s doesn't control any storm %s.",
						box_name(c->who),
						box_code(storm));
		return FALSE;
	}

	if (!is_ship(ship))
	{
		wout(c->who, "%s must be on a ship to bind the storm to.",
					box_name(c->who));
		return FALSE;
	}

	if (province(storm) != province(ship))
	{
		wout(c->who, "Storm must be in the same province as the ship "
					"it is to be bound to.");
		return FALSE;
	}

	if (!check_aura(c->who, 3))
		return FALSE;

	return TRUE;
}


int
d_bind_storm(struct command *c)
{
	int storm = c->a;
	int ship = subloc(c->who);
	int old;
	struct entity_subloc *p;

	if (kind(storm) != T_storm || npc_summoner(storm) != c->who)
	{
		wout(c->who, "%s doesn't control storm %s anymore.",
						box_name(c->who),
						box_code(storm));
		return FALSE;
	}

	if (!is_ship(ship))
	{
		wout(c->who, "%s is no longer on a ship.", box_name(c->who));
		return FALSE;
	}

	if (province(storm) != province(ship))
	{
		wout(c->who, "Storm is no longer in the same province as the ship "
					"it is to be bound to.");
		return FALSE;
	}

	if (!charge_aura(c->who, 3))
		return FALSE;

	old = storm_bind(storm);
	if (old)
	{
		p = rp_subloc(old);
		if (p)
			ilist_rem_value(&p->bound_storms, storm);
	}

	p_misc(storm)->bind_storm = storm;
	p = p_subloc(ship);
	ilist_append(&p->bound_storms, storm);

	wout(c->who, "Bound %s to %s.", box_name(storm), box_name(ship));
	return TRUE;
}


static void
move_storm(int storm, int dest)
{
	int orig = subloc(storm);
	int sk = subkind(storm);
	int before;
	int owner;

	before = weather_here(dest, sk);

	set_where(storm, dest);

	owner = npc_summoner(storm);

	if (valid_box(owner) && valid_box(player(owner)))
		touch_loc_pl(player(owner), dest);

	show_to_garrison = TRUE;

	if (weather_here(orig, sk) == 0)
	{
		switch (sk)
		{
		case sub_rain:
			wout(orig, "It has stopped raining.");
			break;

		case sub_wind:
			wout(orig, "It is no longer windy.");
			break;

		case sub_fog:
			wout(orig, "The fog has cleared.");
			break;

		default:
			assert(FALSE);
		}
	}

	if (!before)
	{
		switch (sk)
		{
		case sub_rain:
			wout(dest, "It has begun to rain.");
			break;

		case sub_wind:
			wout(dest, "It has become quite windy.");
			break;

		case sub_fog:
			wout(dest, "It has become quite foggy.");
			break;
		}
	}

	show_to_garrison = FALSE;
}


void
move_bound_storms(int ship, int where)
{
	struct entity_subloc *p;
	int i;
	int storm;

	p = rp_subloc(ship);
	if (p == NULL)
		return;

	for (i = 0; i < ilist_len(p->bound_storms); i++)
	{
		storm = p->bound_storms[i];
		if (kind(storm) != T_storm)
		{
			ilist_delete(&p->bound_storms, storm);
			i--;
			continue;
		}

		move_storm(storm, province(where));
	}
}


static int
new_storm(int new, int sk, int aura, int where)
{
	int before;

	assert(sk == sub_rain || sk == sub_wind || sk == sub_fog);
	assert(loc_depth(where) == LOC_province);

	before = weather_here(where, sk);

	if (new == 0)
	{
		new = new_ent(T_storm, sk);

		if (new <= 0)
			return -1;
	}

	p_misc(new)->storm_str = aura;
	set_where(new, where);

	show_to_garrison = TRUE;

	if (!before)
	{
		switch (sk)
		{
		case sub_rain:
			wout(where, "It has begun to rain.");
			break;

		case sub_wind:
			wout(where, "It has become quite windy.");
			break;

		case sub_fog:
			wout(where, "It has become quite foggy.");
			break;
		}
	}

	show_to_garrison = FALSE;

	return 0;
}


void
storm_report(pl)
int pl;
{
	int first = TRUE;
	int owner;
	int where;
	int i;

	loop_storm(i)
	{
		owner = npc_summoner(i);

		if (owner == 0 || player(owner) != pl)
			continue;

		where = province(i);

		if (first)
		{
			out(pl, "");
			out(pl, "%5s  %4s  %5s  %4s  %s",
				"storm", "kind", "owner", "loc", "strength");
			out(pl, "%5s  %4s  %5s  %4s  %s",
				"-----", "----", "-----", "----", "--------");

			first = FALSE;
		}

		out(pl, "%5s  %4s  %5s  %4s     %s",
				box_code_less(i),
				subkind_s[subkind(i)],
				box_code_less(owner),
				box_code_less(where),
				comma_num(storm_strength(i)));
	}
	next_storm;
}


void
dissipate_storm(int storm, int show)
{
	int owner;
	struct entity_misc *p;
	int where = subloc(storm);
	int ship;

	assert(kind(storm) == T_storm);

	owner = npc_summoner(storm);

	if (owner && kind(owner) == T_char)
		wout(owner, "%s has dissipated.", box_name(storm));

	if (show)
	{
		int sk = subkind(storm);

		if (weather_here(where, sk) == 0)
		{
			switch (sk)
			{
			case sub_rain:
				wout(where, "It has stopped raining.");
				break;

			case sub_wind:
				wout(where, "It is no longer windy.");
				break;

			case sub_fog:
				wout(where, "The fog has cleared.");
				break;

			default:
				assert(FALSE);
			}
		}
	}

	set_where(storm, 0);

	p = p_misc(storm);

	if (p->npc_home && p->npc_cookie)
		gen_item(p->npc_home, p->npc_cookie, 1);

	if (ship = storm_bind(storm))
	{
		struct entity_subloc *p;

		p = rp_subloc(ship);
		if (p)
			ilist_rem_value(&p->bound_storms, storm);

		rp_misc(storm)->bind_storm = 0;
	}

	delete_box(storm);
}


int
weather_here(int where, int sk)
{
	int i;
	int sum = 0;

	if (loc_depth(where) == LOC_build)
		return 0;

	where = province(where);

	loop_here(where, i)
	{
		if (kind(i) == T_storm && subkind(i) == sk)
			sum += storm_strength(i);
	}
	next_here;

	return sum;
}


int
v_summon_rain(struct command *c)
{
	int aura = c->a;
	int where;

	if (aura < 3)
		c->a = aura = 3;

	if (!check_aura(c->who, aura))
		return FALSE;

	where = province(reset_cast_where(c->who));
	c->d = where;

	if (!may_cookie_npc(c->who, where, item_rain_cookie))
		return FALSE;

	return TRUE;
}


int
d_summon_rain(struct command *c)
{
	int aura = c->a;
	int where = c->d;
	int new;
	char *name = c->parse[2];

	if (!may_cookie_npc(c->who, where, item_rain_cookie))
		return FALSE;

	if (!charge_aura(c->who, aura))
		return FALSE;

	new = do_cookie_npc(c->who, where, item_rain_cookie, where);

	if (new <= 0)
	{
		wout(c->who, "Failed to summon a storm.");
		return FALSE;
	}

	if (numargs(c) >= 2 && name && *name)
		set_name(new, name);

	new_storm(new, sub_rain, aura*2, where);

	wout(c->who, "Summoned %s.", box_name_kind(new));

	touch_loc_pl(player(c->who), where);

	return TRUE;
}


int
v_summon_wind(struct command *c)
{
	int aura = c->a;
	int where;

	if (aura < 3)
		c->a = aura = 3;

	if (!check_aura(c->who, aura))
		return FALSE;

	where = province(reset_cast_where(c->who));
	c->d = where;

	if (!may_cookie_npc(c->who, where, item_wind_cookie))
		return FALSE;

	return TRUE;
}


int
d_summon_wind(struct command *c)
{
	int aura = c->a;
	char *name = 0;
	int where = c->d;
	int new;

	if (numargs(c) >= 2 && c->parse[2] != NULL)
		name = c->parse[2];

	if (!may_cookie_npc(c->who, where, item_wind_cookie))
		return FALSE;

	if (!charge_aura(c->who, aura))
		return FALSE;

	new = do_cookie_npc(c->who, where, item_wind_cookie, where);

	if (new <= 0)
	{
		wout(c->who, "Failed to summon a storm.");
		return FALSE;
	}

	reset_cast_where(c->who);

	if (name && *name)
		set_name(new, name);

	new_storm(new, sub_wind, aura*2, where);

	wout(c->who, "Summoned %s.", box_name_kind(new));

	touch_loc_pl(player(c->who), where);

	return TRUE;
}


int
v_summon_fog(struct command *c)
{
	int aura = c->a;
	int where;

	if (aura < 3)
		c->a = aura = 3;

	if (!check_aura(c->who, aura))
		return FALSE;

	where = province(reset_cast_where(c->who));
	c->d = where;

	if (!may_cookie_npc(c->who, where, item_fog_cookie))
		return FALSE;

	return TRUE;
}


int
d_summon_fog(struct command *c)
{
	int aura = c->a;
	char *name = c->parse[2];
	int where = c->d;
	int new;

	if (!may_cookie_npc(c->who, where, item_fog_cookie))
		return FALSE;

	if (!charge_aura(c->who, aura))
		return FALSE;

	new = do_cookie_npc(c->who, where, item_fog_cookie, where);

	if (new <= 0)
	{
		wout(c->who, "Failed to summon a storm.");
		return FALSE;
	}

	reset_cast_where(c->who);

	if (name && *name)
		set_name(new, name);

	new_storm(new, sub_fog, aura*2, where);

	wout(c->who, "Summoned %s.", box_name_kind(new));

	touch_loc_pl(player(c->who), where);

	return TRUE;
}


static struct exit_view *
parse_storm_dir(struct command *c, int storm)
{
	int where = subloc(storm);
	struct exit_view **l;
	int i;
	int dir;

	l = exits_from_loc_nsew(c->who, where);

	if (valid_box(c->a))
	{
		if (where == c->a)
		{
			wout(c->who, "%s is already in %s.",
					box_name(storm), box_name(where));
			return FALSE;
		}

		{
			struct exit_view *ret = NULL;

			for (i = 0; i < ilist_len(l); i++)
				if (l[i]->destination == c->a)
				{
					ret = l[i];
				}

			if (ret)
				return ret;
		}

		wout(c->who, "No route from %s to %s.",
					box_name(where),
					c->parse[1]);

		return NULL;
	}

	dir = lookup(full_dir_s, c->parse[1]);

	if (dir < 0)
		dir = lookup(short_dir_s, c->parse[1]);

	if (dir < 0)
	{
		wout(c->who, "Unknown direction or destination '%s'.",
						c->parse[1]);
		return NULL;
	}

	if (!DIR_NSEW(dir))
	{
		wout(c->who, "Direction must be N, S, E or W.");
		return FALSE;
	}

	for (i = 0; i < ilist_len(l); i++)
		if (l[i]->direction == dir &&
				loc_depth(l[i]->destination) == LOC_province)
			return l[i];

	wout(c->who, "No %s route from %s.",
			full_dir_s[dir], box_name(where));
	return NULL;
}


int
v_direct_storm(struct command *c)
{
	int storm = c->a;
	struct exit_view *v;
	int dest;

	if (kind(storm) != T_storm || npc_summoner(storm) != c->who)
	{
		wout(c->who, "You don't control any storm %s.",
						box_code(storm));
		return FALSE;
	}

	cmd_shift(c);

	v = parse_storm_dir(c, storm);

	if (v == NULL)
		return FALSE;

	if (loc_depth(v->destination) != LOC_province)
	{
		wout(c->who, "Can't direct storm to %s.",
				box_code(v->destination));
		return FALSE;
	}

	dest = v->destination;
	p_misc(storm)->storm_move = dest;
	p_misc(storm)->npc_dir = v->direction;

	wout(c->who, "%s will move to %s at month end.",
					box_name(storm), box_name(dest));

	return TRUE;
}


int
v_dissipate(struct command *c)
{
	int storm = c->a;
	int where;
	char here_s[LEN];

	if (kind(storm) != T_storm || npc_summoner(storm) != c->who)
	{
		wout(c->who, "You don't control any storm %s.",
						box_code(storm));
		return FALSE;
	}

	where = province(reset_cast_where(c->who));

	if (where == province(subloc(c->who)))
		strcpy(here_s, "here");
	else
		sprintf(here_s, "in %s", box_name(where));

	c->d = where;

	if (subloc(storm) != where)
	{
		wout(c->who, "%s is not %s.", box_name(storm), here_s);
		return FALSE;
	}

	return TRUE;
}


int
d_dissipate(struct command *c)
{
	int storm = c->a;
	int where = c->d;
	char here_s[LEN];
	struct entity_misc *p;
	struct char_magic *pc;

	if (kind(storm) != T_storm || npc_summoner(storm) != c->who)
	{
		wout(c->who, "You don't control any storm %s.",
						box_code(storm));
		return FALSE;
	}

	if (where == province(subloc(c->who)))
		strcpy(here_s, "here");
	else
		sprintf(here_s, "in %s", box_name(where));

	if (subloc(storm) != where)
	{
		wout(c->who, "%s is not %s.", box_name(storm), here_s);
		return FALSE;
	}

	p = p_misc(storm);
	pc = p_magic(c->who);

	pc->cur_aura += p->storm_str / 4;
	limit_cur_aura(c->who);
	p->storm_str = 0;

	dissipate_storm(storm, TRUE);
	out(c->who, "Current aura is now %s.", comma_num(pc->cur_aura));

	return TRUE;
}


int
v_renew_storm(struct command *c)
{
	int storm = c->a;
	int aura = c->b;
	int where;
	char here_s[LEN];

	if (kind(storm) != T_storm)
	{
		wout(c->who, "%s is not a storm.", box_code(storm));
		return FALSE;
	}

	if (aura < 1)
		c->b = aura = 1;

	if (!check_aura(c->who, aura))
		return FALSE;

	where = province(reset_cast_where(c->who));

	if (where == province(subloc(c->who)))
		strcpy(here_s, "here");
	else
		sprintf(here_s, "in %s", box_name(where));

	c->d = where;

	if (subloc(storm) != where)
	{
		wout(c->who, "%s is not %s.", box_name(storm), here_s);
		return FALSE;
	}

	return TRUE;
}


int
d_renew_storm(struct command *c)
{
	int storm = c->a;
	int aura = c->b;
	int where = c->d;
	char here_s[LEN];
	struct entity_misc *p;

	if (kind(storm) != T_storm)
	{
		wout(c->who, "%s is not a storm.", box_code(storm));
		return FALSE;
	}

	if (where == province(subloc(c->who)))
		strcpy(here_s, "here");
	else
		sprintf(here_s, "in %s", box_name(where));

	if (subloc(storm) != where)
	{
		wout(c->who, "%s is not %s.", box_name(storm), here_s);
		return FALSE;
	}

	if (!charge_aura(c->who, aura))
		return FALSE;

	p = p_misc(storm);
	p->storm_str += aura * 2;

	out(c->who, "%s is now strength %s.",
			box_name(storm), comma_num(p->storm_str));

	return TRUE;
}


int
v_lightning(struct command *c)
{
	int storm = c->a;
	int target = c->b;
	int where;

	if (kind(storm) != T_storm || npc_summoner(storm) != c->who)
	{
		wout(c->who, "You don't control any storm %s.",
						box_code(storm));
		return FALSE;
	}

	if (subkind(storm) != sub_rain)
	{
		wout(c->who, "%s is not a rain storm.", box_name(storm));
		return FALSE;
	}

	where = subloc(storm);

	if (kind(target) != T_char && !is_loc_or_ship(target))
	{
		wout(c->who, "%s is not a valid target.", box_code(target));
		return FALSE;
	}

	if (is_loc_or_ship(target) && loc_depth(target) != LOC_build)
	{
		wout(c->who, "%s is not a valid target.", box_code(target));
		return FALSE;
	}

	if (subloc(target) != where)
	{
		wout(c->who, "Target %s isn't in the same place as the storm.",
				box_code(target));
		return FALSE;
	}

	if (in_safe_now(target))
	{
		wout(c->who, "Not allowed in a safe haven.");
		return FALSE;
	}

	return TRUE;
}


int
d_lightning(struct command *c)
{
	int storm = c->a;
	int target = c->b;
	int aura = c->c;
	int where;
	struct entity_misc *p;

	if (kind(storm) != T_storm || npc_summoner(storm) != c->who)
	{
		wout(c->who, "You don't control any storm %s.",
						box_code(storm));
		return FALSE;
	}

	if (subkind(storm) != sub_rain)
	{
		wout(c->who, "%s is not a rain storm.", box_name(storm));
		return FALSE;
	}

	where = subloc(storm);

	if (kind(target) != T_char && !is_loc_or_ship(target))
	{
		wout(c->who, "%s is not a valid target.", box_code(target));
		return FALSE;
	}

	if (is_loc_or_ship(target) && loc_depth(target) != LOC_build)
	{
		wout(c->who, "%s is not a valid target.", box_code(target));
		return FALSE;
	}

	if (subloc(target) != where)
	{
		wout(c->who, "Target %s isn't in the same place as the storm.",
				box_code(target));
		return FALSE;
	}

	if (in_safe_now(target))
	{
		wout(c->who, "Not allowed in a safe haven.");
		return FALSE;
	}

	p = p_misc(storm);

	if (aura == 0)
		aura = p->storm_str;

	if (aura > p->storm_str)
		aura = p->storm_str;

	p->storm_str -= aura;

	wout(c->who, "%s strikes %s with a lightning bolt!",
				box_name(storm), box_name(target));

	vector_clear();
	vector_add(where);
	vector_add(target);
	wout(VECT, "%s was struck by lightning!",
				box_name(target));

	if (is_loc_or_ship(target))
		add_structure_damage(target, aura, TRUE);
	else
		add_char_damage(target, aura, MATES);

	if (p->storm_str <= 0)
		dissipate_storm(storm, TRUE);

	return TRUE;
}


#if 0
int
v_list_storms(struct command *c)
{

	if (!check_aura(c->who, 1))
		return FALSE;

	return TRUE;
}


int
d_list_storms(struct command *c)
{
	int where;
	int i;
	int first = TRUE;
	char here_s[LEN];

	if (!charge_aura(c->who, 1))
		return FALSE;

	where = province(reset_cast_where(c->who));

	if (where == province(subloc(c->who)))
		strcpy(here_s, "here");
	else
		sprintf(here_s, "in %s", box_name(where));

	loop_here(where, i)
	{
		if (kind(i) != T_storm)
			continue;

		if (first)
		{
			wout(c->who, "Storms %s:", here_s);
			indent += 3;
			first = FALSE;
		}

		wout(c->who, "%s", liner_desc(i));
	}
	next_here;

	if (first)
		wout(c->who, "There are no storms %s.", here_s);
	else
		indent -= 3;

	return TRUE;
}
#endif


int
v_seize_storm(struct command *c)
{
	int storm = c->a;
	int where;
	char here_s[LEN];

	if (kind(storm) != T_storm)
	{
		wout(c->who, "%s isn't a storm.", box_code(storm));
		return FALSE;
	}

	if (npc_summoner(storm) == c->who)
	{
		wout(c->who, "You already control %s.", box_name(storm));
		return FALSE;
	}

	if (!check_aura(c->who, 5))
		return FALSE;

	where = province(reset_cast_where(c->who));

	if (where == province(subloc(c->who)))
		strcpy(here_s, "here");
	else
		sprintf(here_s, "in %s", box_name(where));

	c->d = where;

	if (subloc(storm) != where)
	{
		wout(c->who, "%s is not %s.", box_name(storm), here_s);
		return FALSE;
	}

	return TRUE;
}


int
d_seize_storm(struct command *c)
{
	int storm = c->a;
	int where = c->d;
	char here_s[LEN];
	int owner;

	if (kind(storm) != T_storm)
	{
		wout(c->who, "%s isn't a storm.", box_code(storm));
		return FALSE;
	}

	owner = npc_summoner(storm);

	if (owner && owner == c->who)
	{
		wout(c->who, "You already control %s.", box_name(storm));
		return FALSE;
	}

	if (where == province(subloc(c->who)))
		strcpy(here_s, "here");
	else
		sprintf(here_s, "in %s", box_name(where));

	if (subloc(storm) != where)
	{
		wout(c->who, "%s is not %s.", box_name(storm), here_s);
		return FALSE;
	}

	if (!charge_aura(c->who, 5))
		return FALSE;

	vector_clear();
	vector_add(c->who);
	if (owner)
		vector_add(owner);

	wout(VECT, "%s seized control of %s!",
					box_name(c->who), box_name(storm));

	p_misc(storm)->summoned_by = c->who;

	touch_loc_pl(player(c->who), where);

	return TRUE;
}


int
v_death_fog(struct command *c)
{
	int storm = c->a;
	int target = c->b;
	int where;

	if (kind(storm) != T_storm || npc_summoner(storm) != c->who)
	{
		wout(c->who, "You don't control any storm %s.",
						box_code(storm));
		return FALSE;
	}

	if (subkind(storm) != sub_fog)
	{
		wout(c->who, "%s is not a fog.", box_name(storm));
		return FALSE;
	}

	where = subloc(storm);

	if (kind(target) != T_char)
	{
		wout(c->who, "%s is not a valid target.", box_code(target));
		return FALSE;
	}

	if (in_safe_now(target))
	{
		wout(c->who, "Not allowed in a safe haven.");
		return FALSE;
	}

	if (subloc(target) != where)
	{
		wout(c->who, "Target %s isn't in the same place as the fog.",
				box_code(target));
		return FALSE;
	}

	return TRUE;
}


static char *
fog_excuse()
{

	switch (rnd(1,3))
	{
	case 1:	return "wandered off in the fog and were lost.";
	case 2:	return "choked to death in the poisonous fog.";
	case 3:	return "disappeared in the fog.";
	default:
		assert(FALSE);
	}

	return 0;
}


int
d_death_fog(struct command *c)
{
	int storm = c->a;
	int target = c->b;
	int aura = c->c;
	int where;
	struct entity_misc *p;
	int save_aura;
	int aura_used;

	if (kind(storm) != T_storm || npc_summoner(storm) != c->who)
	{
		wout(c->who, "You don't control any storm %s.",
						box_code(storm));
		return FALSE;
	}

	if (subkind(storm) != sub_fog)
	{
		wout(c->who, "%s is not a fog.", box_name(storm));
		return FALSE;
	}

	where = subloc(storm);

	if (kind(target) != T_char)
	{
		wout(c->who, "%s is not a valid target.", box_code(target));
		return FALSE;
	}

	if (subloc(target) != where)
	{
		wout(c->who, "Target %s isn't in the same place as the fog.",
				box_code(target));
		return FALSE;
	}

	p = p_misc(storm);

	aura *= 2;
	p->storm_str *= 2;

	if (aura == 0)
		aura = p->storm_str;

	if (aura > p->storm_str)
		aura = p->storm_str;

	{
		int peas = has_item(target, item_peasant);
		int work = has_item(target, item_worker);
		int sold = has_item(target, item_soldier);
		int sail = has_item(target, item_sailor);
		int cross = has_item(target, item_crossbowman);

		save_aura = aura;

		if (peas > aura)
		{
			peas = aura;
			work = sold = sail = cross = 0;
			aura = 0;
		}
		else
			aura -= peas;

		if (work > aura)
		{
			work = aura;
			sold = sail = cross = 0;
			aura = 0;
		}
		else
			aura -= work;

		if (sold > aura)
		{
			sold = aura;
			sail = cross = 0;
			aura = 0;
		}
		else
			aura -= sold;

		if (sail > aura)
		{
			sail = aura;
			cross = 0;
			aura = 0;
		}
		else
			aura -= sail;

		if (cross > aura)
		{
			cross = aura;
			aura = 0;
		}
		else
			aura -= cross;

		consume_item(target, item_peasant, peas);
		consume_item(target, item_worker, work);
		consume_item(target, item_sailor, sail);
		consume_item(target, item_soldier, sold);
		consume_item(target, item_crossbowman, cross);

		aura_used = save_aura - aura;

		if (aura_used == 0)
		{
			wout(c->who, "%s has no vulnerable men.",
					box_name(target));
			p->storm_str /= 2;
			return FALSE;
		}

		wout(target, "%s %s %s.", cap(nice_num(aura_used)),
				aura_used == 1 ? "man" : "men",
				fog_excuse());

		wout(c->who, "Killed %s %s.",
				nice_num(aura_used),
				aura_used == 1 ? "man" : "men",
				box_name(target));
	}

	p->storm_str -= save_aura - aura;

	p->storm_str /= 2;

	if (p->storm_str <= 0)
		dissipate_storm(storm, TRUE);

	return TRUE;
}


int
v_banish_corpses(struct command *c)
{
	c->d = reset_cast_where(c->who);

	return TRUE;
}


int
d_banish_corpses(struct command *c)
{
	int where = c->d;
	int i;
	int sum = 0;
	int n;

	loop_char_here(where, i)
	{
		sum += has_item(i, item_corpse);
	}
	next_char_here;

	if (sum == 0)
	{
		wout(c->who, "There are no %s here.",
					plural_item_name(item_corpse, 2));
		return FALSE;
	}

	if (!charge_aura(c->who, sum))
		return FALSE;

	wout(c->who, "Banished %s %s.", comma_num(sum),
				plural_item_name(item_corpse, sum));
	wout(where, "%s banished %s %s!",
				box_name(c->who), comma_num(sum),
				plural_item_name(item_corpse, sum));

	loop_char_here(where, i)
	{
		n = has_item(i, item_corpse);
		if (n == 0)
			continue;

		consume_item(i, item_corpse, n);
		wout(i, "%s banished our %s!", box_name(c->who),
					plural_item_name(item_corpse, n));
	}
	next_char_here;

	return TRUE;
}


int
v_fierce_wind(struct command *c)
{
	int storm = c->a;
	int target = c->b;
	int where;

	if (kind(storm) != T_storm || npc_summoner(storm) != c->who)
	{
		wout(c->who, "You don't control any storm %s.",
						box_code(storm));
		return FALSE;
	}

	if (subkind(storm) != sub_wind)
	{
		wout(c->who, "%s is not a wind storm.", box_name(storm));
		return FALSE;
	}

	where = subloc(storm);

	if (!is_loc_or_ship(target) || loc_depth(target) != LOC_build)
	{
		wout(c->who, "%s is not a valid target.", box_code(target));
		return FALSE;
	}

	if (subloc(target) != where)
	{
		wout(c->who, "Target %s isn't in the same place as the storm.",
				box_code(target));
		return FALSE;
	}

	return TRUE;
}


int
d_fierce_wind(struct command *c)
{
	int storm = c->a;
	int target = c->b;
	int aura = c->c;
	int where;
	struct entity_misc *p;

	if (kind(storm) != T_storm || npc_summoner(storm) != c->who)
	{
		wout(c->who, "You don't control any storm %s.",
						box_code(storm));
		return FALSE;
	}

	if (subkind(storm) != sub_wind)
	{
		wout(c->who, "%s is not a wind storm.", box_name(storm));
		return FALSE;
	}

	where = subloc(storm);

	if (!is_loc_or_ship(target) || loc_depth(target) != LOC_build)
	{
		wout(c->who, "%s is not a valid target.", box_code(target));
		return FALSE;
	}

	if (subloc(target) != where)
	{
		wout(c->who, "Target %s isn't in the same place as the storm.",
				box_code(target));
		return FALSE;
	}

	p = p_misc(storm);

	if (aura == 0)
		aura = p->storm_str;

	if (p->storm_str > aura)
		aura = p->storm_str;

	p->storm_str -= aura;

	vector_clear();
	vector_add(where);
	vector_add(target);
	vector_add(c->who);
	wout(VECT, "%s is buffeted by a fierce wind!", box_name(target));

	add_structure_damage(target, aura, TRUE);

	if (p->storm_str <= 0)
		dissipate_storm(storm, TRUE);

	return TRUE;
}


static void
create_some_storms(int num, int kind)
{
	static ilist l = NULL;
	int i;

	ilist_clear(&l);

	loop_province(i)
	{
		if (greater_region(i) != 0)
			continue;

		if (weather_here(i, kind))
			continue;

		ilist_append(&l, i);
	}
	next_province;

	ilist_scramble(l);

	for (i = 0; i < ilist_len(l) && i < num; i++)
		new_storm(0, kind, rnd(2,3), l[i]);
}


void
natural_weather()
{
	int nprov = nprovinces();
	int n;

/*
 *  One natural storm per 4 (formerly 16) provinces.
 *  Half of storms made each month.
 *  Called four times per month.
 */

	n = nprov / 4 / 2 / 4;

	switch (oly_month(sysclock))
	{
	case 0:	/* Fierce winds */
		create_some_storms(n, sub_fog);
		create_some_storms(n, sub_wind);
		break;

	case 1:	/* Snowmelt */
		create_some_storms(n, sub_fog);
		create_some_storms(n, sub_rain);
		break;

	case 2:	/* Blossom bloom */
		break;

	case 3:	/* Sunsear */
		create_some_storms(n, sub_rain);
		break;

	case 4:	/* Thunder and rain */
		create_some_storms(n*2, sub_rain);
		break;

	case 5:	/* Harvest */
		break;

	case 6:	/* Waning days */
#if 1
		create_some_storms(n, sub_rain);
		create_some_storms(n, sub_fog);
		create_some_storms(n, sub_rain);
#endif
		break;

	case 7:	/* Dark night */
		create_some_storms(n, sub_wind);
		break;

	default:
		assert(FALSE);
	}
}


static void
update_weather_view_loc_sup(int who, int where)
{
	int pl;

	pl = player(who);
	assert(valid_box(pl));

	set_bit(&(p_player(pl)->weather_seen), where);
}


void
update_weather_view_locs(int stack, int where)
{
	int i;

	where = province(where);

	if (kind(stack) == T_char && weather_mage(stack))
		update_weather_view_loc_sup(stack, where);

	loop_char_here(stack, i)
	{
		if (!is_prisoner(i) && weather_mage(i))
			update_weather_view_loc_sup(i, where);
	}
	next_char_here;
}


void
init_weather_views()
{
	int who;

	loop_char(who)
	{
		if (weather_mage(who))
			update_weather_view_loc_sup(who, province(who));
	}
	next_char;
}


int
can_see_weather_here(int who, int where)
{
	int pl = player(who);
	struct entity_player *p;

	assert(valid_box(pl));

	where = province(where);

	p = rp_player(pl);
	if (p == NULL)
		return FALSE;

	return test_bit(p->weather_seen, where);
}

