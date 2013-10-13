
#include	<stdio.h>
#include	<string.h>
#include	"z.h"
#include	"oly.h"


static ilist ocean_chars = NULL;


void
departure_message(int who, struct exit_view *v)
{
	char *to = "";
	char *with;
	char *desc;
	char *comma = "";

	assert(valid_box(who));

	if (char_really_hidden(who))
		return;

	if (loc_depth(v->orig) == LOC_province &&
	    weather_here(v->orig, sub_fog))
		return;

	if (v->dest_hidden)
		return;

	desc = liner_desc(who);		/* consumes mucho souts */

	if (subloc(v->destination) == v->orig)
	{
		to = sout(" entered %s", box_name(v->destination));
	}
	else if (subloc(v->orig) == v->destination)
	{
		to = sout(" exited %s", box_name(v->orig));
	}
	else if (viewloc(v->orig) != viewloc(v->destination))
	{
		if (v->direction >= 1 && v->direction <= 4)
			to = sout(" went %s", full_dir_s[v->direction]);
		else
			to = sout(" left for %s", box_name(v->destination));
	}
	else
	{
		return;
	}

	with = display_with(who);

	if (strchr(desc, ','))
		comma = ",";

	if (!*with)
		with = ".";

	if (viewloc(v->orig) != viewloc(v->destination))
	{
		int garr = garrison_here(v->orig);

		if (garr && garrison_notices(garr, who))
			show_to_garrison = TRUE;
	}

	wout(v->orig, "%s%s%s%s", desc, comma, to, with);
	show_chars_below(v->orig, who);

	show_to_garrison = FALSE;
}


static void
arrival_message(int who, struct exit_view *v)
{
	char *from = "";
	char *with;
	char *desc;
	char *comma = "";

	if (char_really_hidden(who))
		return;

	if (loc_depth(v->destination) == LOC_province &&
	    weather_here(v->destination, sub_fog))
		return;

	desc = liner_desc(who);		/* consumes mucho souts */

	if (v->orig_hidden == FALSE)
	{
		if (v->direction >= 1 && v->direction <= 4)
			from = sout(" from the %s",
				full_dir_s[exit_opposite[v->direction]]);
		else
			from = sout(" from %s", box_name(v->orig));
	}

	with = display_with(who);

	if (strchr(desc, ','))
		comma = ",";

	if (!*with)
		with = ".";

	if (viewloc(v->orig) != viewloc(v->destination))
	{
		int garr = garrison_here(v->destination);

		if (garr)
		{
			if (garrison_notices(garr, who))
				show_to_garrison = TRUE;

			if (garrison_spot_check(garr, who))
			{
				indent += 3;
				wout(garr, "%s%s", desc, with);
				show_chars_below(garr, who);
				indent -= 3;
			}
		}
	}

	wout(v->destination, "%s%s arrived%s%s", desc, comma, from, with);
	show_chars_below(v->destination, who);

	show_to_garrison = FALSE;
}


/*
 *  Mark that we know both ends of a hidden road we're about to go through
 */

static void
discover_road(int who, int where, struct exit_view *v)
{
	struct exit_view **l;
	int i;
	int j;

	l = exits_from_loc(who, v->destination);

	for (i = 0; i < ilist_len(l); i++)
		if (l[i]->road && l[i]->destination == where)
		{
			set_known(who, l[i]->road);
			set_known(who, v->road);

			loop_char_here(who, j)
			{
				set_known(j, l[i]->road);
				set_known(j, v->road);
			}
			next_char_here;
		}
}


struct exit_view *
parse_exit_dir(struct command *c, int where, char *zero_arg)
{
	struct exit_view **l;
	int i;
	int dir;

	l = exits_from_loc(c->who, where);

	if (valid_box(c->a))
	{
		if (where == c->a)
		{
			if (zero_arg)
			    wout(c->who, "Already in %s.", box_name(where));
			return FALSE;
		}

/*
 *  Give priority to passable routes.  A secret passable route may
 *  parallel a visible impassable route.
 */

		{
			struct exit_view *ret = NULL;
			struct exit_view *impass_ret = NULL;

			for (i = 0; i < ilist_len(l); i++)
				if (l[i]->destination == c->a &&
				   (l[i]->hidden == FALSE || see_all(c->who)))
				{
					if (l[i]->impassable)
						impass_ret = l[i];
					else
						ret = l[i];
				}

			if (ret)
				return ret;
			if (impass_ret)
				return impass_ret;
		}

		if (zero_arg)
			wout(c->who, "No visible route from %s to %s.",
					box_name(where),
					c->parse[1]);

		return NULL;
	}

	dir = lookup(full_dir_s, c->parse[1]);

	if (dir < 0)
		dir = lookup(short_dir_s, c->parse[1]);

	if (dir < 0)
	{
		if (zero_arg)
			wout(c->who, "Unknown direction or destination '%s'.",
						c->parse[1]);
		return NULL;
	}

	for (i = 0; i < ilist_len(l); i++)
		if (l[i]->direction == dir &&
		   (l[i]->hidden == FALSE || see_all(c->who)))
		{
			if (dir == DIR_IN && zero_arg)
			{
				wout(c->who, "(assuming '%s %s')",
					zero_arg,
					box_code_less(l[i]->destination));
			}

			return l[i];
		}

	if (zero_arg)
		wout(c->who, "No visible %s route from %s.",
			full_dir_s[dir], box_name(where));
	return NULL;
}


static int
move_exit_land(struct command *c, struct exit_view *v, int show)
{
	struct weights w;
	int delay = v->distance;
	int terr;			/* destination terrain */
	int swamp = FALSE;		/* traveling in a swamp? */

	if (delay == 0)
		return 0;

	terr = subkind(v->destination);

	if (terr == sub_swamp || terr == sub_bog || terr == sub_pits)
		swamp = TRUE;

	determine_stack_weights(c->who, &w);

	if (delay > 1 && w.ride_cap >= w.ride_weight && swamp == FALSE)
	{
		delay -= delay / 2;
//		if (sysclock.turn <= 64)	/* XXX */
//			return delay;
	}
	else
	{
		if (w.land_weight > w.land_cap * 2)
		{
			if (show)
				wout(c->who, "%s is too overloaded to travel.",
						box_name(c->who));
			return -1;
		}

		if (swamp && w.animals)
		{
			if (show)
				wout(c->who, "Difficult terrain slows the animals.  "
					"Travel will take an extra day.");

			delay += 1;
		}

		if (w.land_weight > w.land_cap)
		{
			int ratio;
			int additional;

			ratio = (w.land_weight - w.land_cap) * 100 / w.land_cap;
			additional = delay * ratio / 100;

			if (show)
			{
			    if (additional == 1)
			    {
				wout(c->who, "Excess inventory slows movement.  "
					"Travel will take an extra day.");
			    }
			    else if (additional > 1)
			    {
				wout(c->who, "Excess inventory slows movement.  "
					"Travel will take an extra %s days.",
							nice_num(additional));
			    }
			}

			delay += additional;
		}
	}

	{
		int nobles;
		int men;
		int extra;

//		if (sysclock.turn < 40)
//			nobles = count_stack_units(c->who);
//		else
		nobles = count_stack_move_nobles(c->who);

		men = count_stack_figures(c->who) - nobles;

		assert(nobles > 0);

		extra = men / (nobles * army_slow_factor);

		if (extra > v->distance * 2)
			extra = v->distance * 2;

		if (extra == 1)
			wout(c->who, "%s noble%s, %s %s: travel will take an extra day.",
					cap(nice_num(nobles)), add_s(nobles),
					nice_num(men),
					men == 1 ? "man" : "men");
		else if (extra > 1)
			wout(c->who, "%s noble%s, %s %s: travel will take an extra %d days.",
					cap(nice_num(nobles)), add_s(nobles),
					nice_num(men),
					men == 1 ? "man" : "men",
					extra);

		delay += extra;
	}

	return delay;
}


static int
move_exit_fly(struct command *c, struct exit_view *v, int show)
{
	struct weights w;
	int delay = v->distance;

	if (subkind(v->destination) == sub_under ||
	    subkind(v->destination) == sub_tunnel)
	{
		if (show)
			wout(c->who, "Cannot fly underground.");
		return -1;
	}

	if (delay < 8)
	{
		if (delay > 3)
			delay = 3;
	} else
		delay = 4;

	determine_stack_weights(c->who, &w);

	if (w.fly_cap < w.fly_weight)
	{
		if (show)
			wout(c->who, "%s is too overloaded to fly.",
						box_name(c->who));
		return -1;
	}

	{
		int nobles;
		int men;
		int extra;

//		if (sysclock.turn < 40)
//			nobles = count_stack_units(c->who);
//		else
		nobles = count_stack_move_nobles(c->who);

		men = count_stack_figures(c->who) - nobles;

		assert(nobles > 0);

		extra = men / (nobles * army_slow_factor);

		if (extra > v->distance * 2)
			extra = v->distance * 2;

		if (extra == 1)
			wout(c->who, "%s noble%s, %s %s: travel will take an extra day.",
					cap(nice_num(nobles)), add_s(nobles),
					nice_num(men),
					men == 1 ? "man" : "men");
		else if (extra > 1)
			wout(c->who, "%s noble%s, %s %s: travel will take an extra %d days.",
					cap(nice_num(nobles)), add_s(nobles),
					nice_num(men),
					men == 1 ? "man" : "men",
					extra);

		delay += extra;
	}

	return delay;
}


static void
save_v_array(struct command *c, struct exit_view *v)
{

	c->b = v->direction;
	c->c = v->destination;
	c->d = v->road;
	c->e = v->dest_hidden;
	c->f = v->distance;
	c->g = v->orig;
	c->h = v->orig_hidden;
}


static void
restore_v_array(struct command *c, struct exit_view *v)
{

	memset(v, '\0', sizeof(*v));

	v->direction	= c->b;
	v->destination	= c->c;
	v->road		= c->d;
	v->dest_hidden	= c->e;
	v->distance	= c->f;
	v->orig		= c->g;
	v->orig_hidden	= c->h;
}


static void
suspend_stack_actions(int who)
{
	int i;

	loop_stack(who, i)
	{
		p_char(i)->moving = sysclock.days_since_epoch;
	}
	next_stack;
}


void
restore_stack_actions(int who)
{
	int i;

	loop_stack(who, i)
	{
		p_char(i)->moving = 0;
	}
	next_stack;
}


void
clear_guard_flag(int who)
{
	int pl;
	int i;

	if (kind(who) == T_char)
		p_char(who)->guard = FALSE;

	loop_char_here(who, i)
	{
		p_char(i)->guard = FALSE;
	}
	next_char_here;
}


static int
land_check(struct command *c, struct exit_view *v, int show)
{
	int owner;		/* owner of loc we're moving into, if any */

	if (v->water)
	{
		if (show)
			wout(c->who, "A sea-worthy ship is required "
				"for travel across water.");
		return FALSE;
	}

	if (v->impassable)
	{
		if (show)
			wout(c->who, "That route is impassable.");
		return FALSE;
	}

	if (v->in_transit)
	{
		if (show)
			wout(c->who, "%s is underway.  Boarding is not "
				"possible.", box_name(v->destination));
		return FALSE;
	}

	if (loc_depth(v->destination) == LOC_build &&
	    subkind(v->destination) != sub_sewer &&
	    (owner = building_owner(v->destination)) &&
	    !will_admit(owner, c->who, v->destination) &&
	    v->direction != DIR_OUT)
	{
		if (show)
		{
			wout(c->who, "%s refused to let us enter.",
						box_name(owner));
			wout(owner, "Refused to let %s enter.",
						box_name(c->who));
		}
		return FALSE;
	}

	return TRUE;
}


static int
can_move_here(int where, struct command *c)
{
	struct exit_view *v;

	v = parse_exit_dir(c, where, NULL);

	if (v &&
	    v->direction != DIR_IN &&
	    land_check(c, v, FALSE) &&
	    move_exit_land(c, v, FALSE) >= 0)
	{
		return TRUE;
	}

	return FALSE;
}


static int
can_move_at_outer_level(int where, struct command *c)
{
	int outer;

	outer = subloc(where);
	while (loc_depth(outer) > LOC_region)
	{
		if (can_move_here(outer, c))
			return loc_depth(outer) - loc_depth(where);
		outer = subloc(outer);
	}

	return 0;
}


int
v_move(struct command *c)
{
	struct exit_view *v;
	int delay;
	int where = subloc(c->who);
	int check_outer = TRUE;

	if (numargs(c) < 1)
	{
		wout(c->who, "Specify direction or destination to MOVE.");
		return FALSE;
	}

	while (numargs(c) > 0)
	{
		v = parse_exit_dir(c, where, "move");

		if (v)
		{
			check_outer = FALSE;

			if (land_check(c, v, TRUE))
				break;
			v = NULL;
		}

		cmd_shift(c);
	}

	if (v == NULL && check_outer && can_move_at_outer_level(where, c))
	{
		c->a = subloc(where);
		v = parse_exit_dir(c, where, NULL);
		assert(v);

		if (v)
		{
			assert(move_exit_land(c, v, FALSE) >= 0);
			wout(c->who, "(assuming 'move out' first)");
			prepend_order(player(c->who), c->who, c->line);
		}
	}

	if (v == NULL)
		return FALSE;

	delay = move_exit_land(c, v, TRUE);

	if (delay < 0)
		return FALSE;

	if (v->hades_cost)
	{
		int n = count_stack_figures(c->who);
		int cost;

		cost = v->hades_cost * n;

		log_write(LOG_SPECIAL, "%s (%s) tries to enter Hades",
					box_name(player(c->who)),
					box_name(c->who));

		if (!autocharge(c->who, cost))
		{
			wout(c->who, "Can't afford %s to enter Hades.",
						gold_s(cost));
			return FALSE;
		}

		wout(c->who, "The Gatekeeper Spirit of Hades took "
				"%s from us.", gold_s(cost));
	}

	v->distance = delay;
	c->wait = delay;

	save_v_array(c, v);
	leave_stack(c->who);

	if (delay > 0)
	{
		vector_stack(c->who, TRUE);
		wout(VECT, "Travel to %s will take %s day%s.",
				box_name(v->destination),
				nice_num(delay),
				delay == 1 ? "" : "s");
	}

	suspend_stack_actions(c->who);
	clear_guard_flag(c->who);

	if (delay > 1)
		prisoner_movement_escape_check(c->who);

	departure_message(c->who, v);

	return TRUE;
}


void
touch_loc_after_move(int who, int where)
{
	int pl;
	int i;

	if (kind(who) == T_char)
		touch_loc(who);

	loop_char_here(who, i)
	{
		if (!is_prisoner(i))
			touch_loc(i);
	}
	next_char_here;
}


void
move_stack(int who, int where)
{

	assert(kind(who) == T_char);

	if (!in_faery(subloc(who)) && in_faery(where))
	{
		log_write(LOG_SPECIAL, "%s enters Faery at %s.",
				box_name(who), box_name(where));
	}

	set_where(who, where);
	mark_loc_stack_known(who, where);
	touch_loc_after_move(who, where);
	update_weather_view_locs(who, where);
	clear_contacts(who);

	if (subkind(where) == sub_city)
	{
		int i;

		loop_stack(who, i)
		{
			match_trades(i);
		}
		next_stack;
	}

	if (subkind(where) == sub_ocean)
	{
		struct entity_char *p;

		p = p_char(who);

		if (p->time_flying == 0)
		{
			p->time_flying++;
			ilist_append(&ocean_chars, who);
		}
	}

	if (subkind(where) != sub_ocean)
	{
		struct entity_char *p;

		p = rp_char(who);
		if (p && p->time_flying)
		{
			p->time_flying = 0;
			ilist_rem_value(&ocean_chars, who);
		}
	}

	if (loc_depth(where) == LOC_province &&
	    subkind(where) != sub_ocean &&
	    in_faery(where))
		faery_attack_check(who, where);

	if (loc_depth(where) == LOC_province &&
	    subkind(where) != sub_ocean &&
	    in_hades(where))
		hades_attack_check(who, where);
}


int
d_move(struct command *c)
{
	struct exit_view vv;
	struct exit_view *v = &vv;

	restore_v_array(c, v);

	if (!valid_box(v->destination))
	{
		wout(c->who, "Your destination no longer exists!");
		return FALSE;
	}

	if (v->road)
		discover_road(c->who, subloc(c->who), v);

	vector_stack(c->who, TRUE);
	wout(VECT, "Arrival at %s.", box_name(v->destination));

	if (loc_depth(v->destination) == LOC_province &&
	    viewloc(subloc(c->who)) != viewloc(v->destination) &&
	    weather_here(v->destination, sub_fog))
	{
		wout(VECT, "The province is blanketed in fog.");
	}

	restore_stack_actions(c->who);

#if 0
/*
 *  Stackmates who are executing commands have gotten a free day.  
 *  Force a one evening delay into their command completion loop.
 */

	if (v->distance > 0)
	{
		int i;

		loop_char_here(c->who, i)
		{
			struct command *nc = rp_command(i);

			if (nc->wait != 0)
				nc->move_skip = TRUE;
		}
		next_char_here;
	}
#endif

	move_stack(c->who, v->destination);

	if (viewloc(v->orig) != viewloc(v->destination))
		arrival_message(c->who, v);

	return TRUE;
}


void
init_ocean_chars()
{
	int i;
	int where;

	loop_char(i)
	{
		where = subloc(i);

		if (subkind(where) == sub_ocean)
			ilist_append(&ocean_chars, i);
	}
	next_char;
}


void
check_ocean_chars()
{
	int i;
	int where;
	struct entity_char *p;
	int who;
	static ilist l = NULL;

	ilist_clear(&l);
	l = ilist_copy(ocean_chars);

	for (i = 0; i < ilist_len(l); i++)
	{
		who = l[i];
		where = subloc(who);
		p = p_char(who);

		if (!alive(who) || subkind(where) != sub_ocean)
		{
			p->time_flying = 0;
			ilist_rem_value(&ocean_chars, who);
			continue;
		}

		p->time_flying++;

		if (p->time_flying <= 15)
			continue;

		if (stack_parent(who))
			continue;

/*
 *  Flying stack plunges into the sea.
 */

		vector_stack(who, TRUE);
		wout(VECT, "Flight can no longer be maintained.  %s "
				"plunges into the sea.", box_name(who));

		kill_stack_ocean(who);
	}
}


static int
fly_check(struct command *c, struct exit_view *v)
{
	int owner;

	if (v->in_transit) {
		wout(c->who, "%s is underway.	Boarding is not "
				 "possible.", box_name(v->destination));
		return FALSE;
	}

	if (loc_depth(v->destination) == LOC_build &&
			subkind(v->destination) != sub_sewer &&
			(owner = building_owner(v->destination)) &&
			!will_admit(owner, c->who, v->destination) && v->direction != DIR_OUT) {
		wout(c->who, "%s refused to let us enter.", box_name(owner));
		wout(owner, "Refused to let %s enter.", box_name(c->who));
		return FALSE;
	}

	return TRUE;
}


static int
can_fly_here(int where, struct command *c)
{
	struct exit_view *v;

	v = parse_exit_dir(c, where, NULL);

	if (v && v->direction != DIR_IN && move_exit_fly(c, v, FALSE) >= 0)
	{
		return TRUE;
	}

	return FALSE;
}


static int
can_fly_at_outer_level(int where, struct command *c)
{
	int outer;

	outer = subloc(where);
	while (loc_depth(outer) > LOC_region)
	{
		if (can_fly_here(outer, c))
			return loc_depth(outer) - loc_depth(where);
		outer = subloc(outer);
	}

	return 0;
}


int
v_fly(struct command *c)
{
	struct exit_view *v;
	int delay;
	int where = subloc(c->who);
	int check_outer = TRUE;

	if (numargs(c) < 1)
	{
		wout(c->who, "Specify direction or destination to FLY.");
		return FALSE;
	}

	while (numargs(c) > 0)
	{
		v = parse_exit_dir(c, where, "fly");

		if (v)
		{
			check_outer = FALSE;

			if (fly_check(c, v))
				break;
			v = NULL;
		}

		cmd_shift(c);
	}

	if (v == NULL && check_outer && can_fly_at_outer_level(where, c))
	{
		c->a = subloc(where);
		v = parse_exit_dir(c, where, NULL);
		assert(v);

		if (v)
		{
			assert(move_exit_fly(c, v, FALSE) >= 0);
			wout(c->who, "(assuming 'fly out' first)");
			prepend_order(player(c->who), c->who, c->line);
		}
	}

	if (v == NULL)
		return FALSE;

	delay = move_exit_fly(c, v, TRUE);

	if (delay < 0)
		return FALSE;

	v->distance = delay;
	c->wait = delay;

	save_v_array(c, v);
	leave_stack(c->who);

	if (delay > 0)
	{
		vector_stack(c->who, TRUE);
		wout(VECT, "Flying to %s will take %s day%s.",
				box_name(v->destination),
				nice_num(delay),
				delay == 1 ? "" : "s");
	}

	suspend_stack_actions(c->who);
	clear_guard_flag(c->who);

	departure_message(c->who, v);

	return TRUE;
}


int
d_fly(struct command *c)
{
	struct exit_view vv;
	struct exit_view *v = &vv;

	restore_v_array(c, v);

	if (v->road)
		discover_road(c->who, subloc(c->who), v);

	vector_stack(c->who, TRUE);
	wout(VECT, "Arrival at %s.", box_name(v->destination));

	if (loc_depth(v->destination) == LOC_province &&
	    viewloc(subloc(c->who)) != viewloc(v->destination) &&
	    weather_here(v->destination, sub_fog))
	{
		wout(VECT, "The province is blanketed in fog.");
	}

	restore_stack_actions(c->who);

#if 0
/*
 *  Stackmates who are executing commands have gotten a free day.  
 *  Force a one evening delay into their command completion loop.
 */

	if (v->distance > 0)
	{
		int i;

		loop_char_here(c->who, i)
		{
			struct command *nc = rp_command(i);

			if (nc->wait != 0)
				nc->move_skip = TRUE;
		}
		next_char_here;
	}
#endif

	move_stack(c->who, v->destination);
	arrival_message(c->who, v);

	return TRUE;
}


/*
 *  Synonym for 'move out'
 */

int
v_exit(struct command *c)
{
	int ret;

	ret = oly_parse(c, "move out");
	assert(ret);

	return v_move(c);
}


int
v_enter(struct command *c)
{
	int ret;

	if (numargs(c) < 1)
	{
		ret = oly_parse(c, "move in");
		assert(ret);
		return v_move(c);
	}

	ret = oly_parse(c, sout("move %s", c->parse[1]));
	assert(ret);

	return v_move(c);
}


int
v_north(struct command *c)
{
	int ret;

	ret = oly_parse(c, "move north");
	assert(ret);

	return v_move(c);
}


int
v_south(struct command *c)
{
	int ret;

	ret = oly_parse(c, "move south");
	assert(ret);

	return v_move(c);
}


int
v_east(struct command *c)
{
	int ret;

	ret = oly_parse(c, "move east");
	assert(ret);

	return v_move(c);
}


int
v_west(struct command *c)
{
	int ret;

	ret = oly_parse(c, "move west");
	assert(ret);

	return v_move(c);
}


void
check_captain_loses_sailors(int qty, int target, int inform)
{
	static int cmd_sail = -1;
	int where = subloc(target);
	struct command *c;
	int hands_short = 0;
	int before, now;
	int should_have;
	int penalty;

	if (cmd_sail < 0)
	{
		cmd_sail = find_command("sail");
		assert(cmd_sail > 0);
	}

	if (!is_ship(subloc(target)))
		return;

	c = rp_command(target);

	if (c == NULL || c->state != STATE_RUN || c->cmd != cmd_sail)
		return;

	now = has_item(target, item_sailor) + has_item(target, item_pirate);
	before = now + qty;

	switch (subkind(where))
	{
	case sub_galley:
		should_have = 14;
		break;

	case sub_roundship:
		should_have = 8;
		break;

	default:
		fprintf(stderr, "kind is %d\n", subkind(where));
		assert(FALSE);
	}

	if (now >= should_have)
		return;		/* still have enough sailors */

	if (before > should_have)
		before = should_have;

	penalty = before - now;

	assert(penalty > 0);

	vector_clear();
	vector_add(target);
	if (inform && target != inform)
		vector_add(inform);

	if (penalty == 1)
		wout(VECT, "Loss of crew will cause travel to take an extra day.");
	else
		wout(VECT, "Loss of crew will cause travel to take %s extra days.",
					nice_num(penalty));

	assert(c->wait > 0);
	c->wait += penalty;

	log_write(LOG_SPECIAL, "Loss of sailors incurs penalty for %s.",
				box_code(player(target)));
}


static int
move_exit_water(struct command *c, struct exit_view *v, int ship, int show)
{
	int delay = v->distance;
	int hands_short = 0;		/* how many hands we are short */
	int n;
	char *s;
	int wind_bonus = 0;
	int where = subloc(ship);

	switch (subkind(ship))
	{
	case sub_roundship:
		n = has_item(c->who, item_sailor) +
					has_item(c->who, item_pirate);
		if (n < 8)
		{
			hands_short = 8 - n;

			if (hands_short == 1)
				s = "day";
			else
				s = sout("%s days", nice_num(hands_short));

			if (show)
				wout(c->who, "The crew of a roundship is eight "
				"sailors, but you have %s.  Travel will "
				"take an extra %s.",
				n == 0 ? "none" : nice_num(n), s);
		}
		break;

	case sub_galley:
		n = has_item(c->who, item_sailor) +
					has_item(c->who, item_pirate);
		/* n += has_item(c->who, item_slave); */

		if (n < 14)
		{
			hands_short = 14 - n;

			if (hands_short == 1)
				s = "day";
			else
				s = sout("%s days", nice_num(hands_short));

			if (show)
				wout(c->who, "The crew of a galley is fourteen "
				"slaves or sailors, but you have %s.  "
				"Travel will take an extra %s.",
					n == 0 ? "none" : nice_num(n), s);
		}
		break;

	default:
		fprintf(stderr, "subkind = %d\n", subkind(ship));
		assert(FALSE);
	}

	if (subkind(ship) == sub_roundship &&
	    weather_here(where, sub_wind) &&
	    delay > 2)
	{
		wind_bonus = 1;
		if (show)
			wout(c->who, "Favorable winds speed our progress.");
	}

	delay = delay + hands_short - wind_bonus;

	return delay;
}


static void
sail_depart_message(int ship, struct exit_view *v)
{
	char *to = "";
	char *desc;
	char *comma = "";

	desc = liner_desc(ship);

	if (v->dest_hidden == FALSE)
		to = sout(" for %s.", box_name(v->destination));

	if (strchr(desc, ','))
		comma = ",";

	wout(v->orig, "%s%s departed%s", desc, comma, to);
}


static void
sail_arrive_message(int ship, struct exit_view *v)
{
	char *from = "";
	char *desc;
	char *comma = "";
	char *with;

	desc = liner_desc(ship);

	if (v->orig_hidden == FALSE)
		from = sout(" from %s", box_name(v->orig));

	if (strchr(desc, ','))
		comma = ",";

	with = display_owner(ship);

	if (!*with)
		with = ".";

	show_to_garrison = TRUE;

	wout(v->destination, "%s%s arrived%s%s", desc, comma, from, with);
	show_owner_stack(v->destination, ship);

	show_to_garrison = FALSE;
}


static int
sail_check(struct command *c, struct exit_view *v, int show)
{

	if (!v->water)
	{
		if (show)
			wout(c->who,
				"There is no water route in that direction.");
		return FALSE;
	}

	if (v->impassable)
	{
		if (show)
			wout(c->who, "That route is impassable.");
		return FALSE;
	}

	return TRUE;
}


static int
can_sail_here(int where, struct command *c, int ship)
{
	struct exit_view *v;

	v = parse_exit_dir(c, where, NULL);

	if (v && v->direction != DIR_IN &&
	    sail_check(c, v, FALSE) &&
	    move_exit_water(c, v, ship, FALSE) >= 0)
	{
		return TRUE;
	}

	return FALSE;
}


static int
can_sail_at_outer_level(int ship, int where, struct command *c)
{
	int outer;
extern int weight_display_flag;

	if (ship_cap(ship))
	{
		int loaded;

		loaded = ship_weight(ship) * 100 / ship_cap(ship);

		if (loaded > 100)
		{
			wout(c->who, "%s is too overloaded to sail.",
							box_name(ship));
			wout(c->who,
				"(ship capacity = %d, damage = %d, load = %d)",
					ship_cap_raw(ship),
					loc_damage(ship),
					ship_weight(ship));
weight_display_flag = c->who;
			(void) ship_weight(ship);
weight_display_flag = 0;

			return FALSE;
		}
	}

	outer = subloc(where);
	while (loc_depth(outer) > LOC_region)
	{
		if (can_sail_here(outer, c, ship))
			return loc_depth(outer) - loc_depth(where);
		outer = subloc(outer);
	}

	return 0;
}


int
v_sail(struct command *c)
{
	struct exit_view *v;
	int delay;
	int ship = subloc(c->who);
	int outer_loc;
	int check_outer = TRUE;
extern int weight_display_flag;

	if (!is_ship(ship))
	{
		if (subkind(ship) == sub_galley_notdone ||
		    subkind(ship) == sub_roundship_notdone)
		{
			wout(c->who, "%s is not yet completed.",
					box_name(ship));
		}
		else
			wout(c->who, "Must be on a sea-worthy ship to sail.");
		return FALSE;
	}

	if (building_owner(ship) != c->who)
	{
		wout(c->who, "Only the captain of a ship may sail.");
		return FALSE;
	}

	if (has_skill(c->who, sk_pilot_ship) <= 0)
	{
		wout(c->who, "Knowledge of %s is required to sail.",
					box_name(sk_pilot_ship));
		return FALSE;
	}

	if (numargs(c) < 1)
	{
		wout(c->who, "Specify direction or destination to sail.");
		return FALSE;
	}

	outer_loc = subloc(ship);

	while (numargs(c) > 0)
	{
		v = parse_exit_dir(c, outer_loc, "sail");

		if (v)
		{
			check_outer = FALSE;

			if (sail_check(c, v, TRUE))
				break;
			v = NULL;
		}

		cmd_shift(c);
	}

	if (v == NULL && check_outer &&
	    can_sail_at_outer_level(ship, outer_loc, c))
	{
		c->a = subloc(outer_loc);
		v = parse_exit_dir(c, outer_loc, NULL);
		assert(v);

		if (v)
		{
			assert(move_exit_water(c, v, ship, FALSE) >= 0);
			wout(c->who, "(assuming 'sail out' first)");
			prepend_order(player(c->who), c->who, c->line);
		}
	}

	if (v == NULL)
		return FALSE;

	if (ship_cap(ship))
	{
		int loaded;

		loaded = ship_weight(ship) * 100 / ship_cap(ship);

		if (loaded > 100)
		{
			wout(c->who, "%s is too overloaded to sail.",
							box_name(ship));

			wout(c->who,
				"(ship capacity = %d, damage = %d, load = %d)",
					ship_cap_raw(ship),
					loc_damage(ship),
					ship_weight(ship));
weight_display_flag = c->who;
			(void) ship_weight(ship);
weight_display_flag = 0;
			return FALSE;
		}
	}

	assert(!v->in_transit);

	delay = move_exit_water(c, v, ship, TRUE);

	if (delay < 0)
		return FALSE;

	c->wait = delay;
	v->distance = delay;

	save_v_array(c, v);

	if (delay > 0)
	{
		vector_char_here(c->who);
		vector_add(c->who);

		wout(VECT, "Sailing to %s will take %s day%s.",
				box_name(v->destination),
				nice_num(delay),
				delay == 1 ? "" : "s");
	}

	sail_depart_message(ship, v);

/*
 *  Mark the in_transit field with the daystamp of the beginning
 *  of the voyage
 */

	p_subloc(ship)->moving = sysclock.days_since_epoch;

	if (ferry_horn(ship))		/* clear ferry horn signal */
		p_magic(ship)->ferry_flag = 0;

	return TRUE;
}


int
d_sail(struct command *c)
{
	int ship = subloc(c->who);
	struct exit_view vv;
	struct exit_view *v = &vv;

	assert(is_ship(ship));			/* still a ship */
	assert(building_owner(ship) == c->who);	/* still captain */

	restore_v_array(c, v);

	if (v->road)
		discover_road(c->who, subloc(ship), v);

	vector_char_here(ship);
	wout(VECT, "Arrival at %s.", box_name(v->destination));

	if (loc_depth(v->destination) == LOC_province &&
	    viewloc(subloc(ship)) != viewloc(v->destination) &&
	    weather_here(v->destination, sub_fog))
	{
		wout(VECT, "The province is blanketed in fog.");
	}

	p_subloc(ship)->moving = 0;		/* no longer moving */
	set_where(ship, v->destination);
	mark_loc_stack_known(ship, v->destination);
	move_bound_storms(ship, v->destination);

	if (ferry_horn(ship))		/* clear ferry horn signal */
		p_magic(ship)->ferry_flag = 0;

	touch_loc_after_move(ship, v->destination);
	sail_arrive_message(ship, v);

	if (c->use_skill == 0)
		add_skill_experience(c->who, sk_pilot_ship);

	return TRUE;
}


/*
 *  If sailing is interrupted, we must zero subloc.moving
 *  to indicate that the ship is no longer in transit.
 */

i_sail(struct command *c)
{
	int ship = subloc(c->who);

	assert(is_ship(ship));

	p_subloc(ship)->moving = 0;

	if (ferry_horn(ship))		/* clear ferry horn signal */
		p_magic(ship)->ferry_flag = 0;

	return TRUE;
}

