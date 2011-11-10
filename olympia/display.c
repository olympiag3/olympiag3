
#include	<stdio.h>
#include	<string.h>
#include	"z.h"
#include	"oly.h"


char *
loc_inside_string(int where)
{
	char *reg_name;

	if (loc_depth(where) == LOC_build)
	{
		if (subkind(loc(where)) == sub_ocean)
			return sout(", in %s", box_name(province(where)));

		if (loc_depth(loc(where)) == LOC_province)
			return sout(", in province %s",
					box_name(province(where)));

		return sout(", in %s", box_name(province(where)));
	}
	else if (loc_depth(where) == LOC_subloc)
	{
		if (subkind(province(where)) == sub_ocean)
			return sout(", in %s", box_name(province(where)));
		else
			return sout(", in province %s",
					box_name(province(where)));
	}
	else if (subkind(where) == sub_ocean)
	{
		reg_name = name(region(where));

		if (reg_name && *reg_name)
			return sout(", in %s", reg_name);
		return "";
	}
	else
	{
		reg_name = name(region(where));

		if (reg_name && *reg_name)
			return sout(", in %s", reg_name);

		return "";
	}
}


static int
show_loc_barrier(int who, int where)
{

	if (loc_barrier(where))
	{
		wout(who, "A magical barrier surrounds %s.", box_name(where));
		return TRUE;
	}

	return FALSE;
}


static char *
safe_haven_s(int n)
{

	if (safe_haven(n))
		return ", safe haven";

	return "";
}


static char *
ship_cap_s(int n)
{
	int sc, sw;

	if (sc = ship_cap(n))
	{
		sw = ship_weight(n);
		return sout(", %d%% loaded", sw * 100 / sc);
	}

	return "";
}


static void
show_loc_stats(int who, int where)
{
	int sc, sw, n;
	int first = TRUE;

	if (n = loc_damage(where))
	{
		if (first)
		{
			out(who, "");
			first = FALSE;
		}

		out(who, "Damage: %d%%", n);
	}

	if (sc = ship_cap(where))
	{
		sw = ship_weight(where);

		if (first)
		{
			out(who, "");
			first = FALSE;
		}

		out(who, "Ship capacity: %s/%s (%d%%)",
			comma_num(sw), comma_num(sc), sw * 100 / sc);
	}
}


char *
loc_civ_s(int where)
{
	int n;

	if (loc_depth(where) != LOC_province ||
	    subkind(where) == sub_ocean ||
	    in_faery(where) || in_hades(where))
	{
		return "";
	}

	n = loc_civ(where);

	if (n == 0)
		return ", wilderness";

	return sout(", civ-%d", n);
}


char *
show_loc_header(int where)
{
	char buf[LEN];

	strcpy(buf, box_name_kind(where));
	strcat(buf, loc_inside_string(where));

	if (mine_depth(where))
		strcat(buf, sout(", depth~%d", mine_depth(where)));

	strcat(buf, safe_haven_s(where));

	if (loc_hidden(where))
		strcat(buf, ", hidden");

	if (subkind(where) != sub_tunnel)
		strcat(buf, loc_civ_s(where));

	return sout("%s", buf);
}


static char *
with_inventory_string(int who)
{
	char with[LEN];
	struct item_ent *e;
	int mk;

	mk = noble_item(who);
	with[0] = '\0';

	loop_inv(who, e)
	{
		if (mk == e->item || !item_prominent(e->item))
			continue;

		if (with[0])
			strcat(with, ", ");
		else
			strcpy(with, ", with ");

		strcat(with, just_name_qty(e->item, e->qty));
	}
	next_inv;

	return sout("%s", with);
}


char *
display_with(int who)
{

	if (rp_loc_info(who) && ilist_len(rp_loc_info(who)->here_list) > 0)
		return ", accompanied~by:";

	return "";
}


char *
display_owner(int who)
{

	if (first_char_here(who) <= 0)
		return "";

	return ", owner:";
}


static char *
incomplete_string(int n)
{
	struct entity_subloc *p;

	p = rp_subloc(n);
	if (p == NULL)
		return "";

	if (p->effort_required == 0)
		return "";

	return sout(", %d%% completed",
			p->effort_given * 100 / p->effort_required);
}


static char *
liner_desc_ship(int n)
{
	char buf[LEN];
	int fee;

	sprintf(buf, "%s%s",
			box_name_kind(n),
			incomplete_string(n));

	strcat(buf, ship_cap_s(n));

	if (loc_defense(n))
		strcat(buf, sout(", defense~%d", loc_defense(n)));

	if (loc_damage(n))
		strcat(buf, sout(", %d%%~damaged", loc_damage(n)));

	if (show_display_string)
	{
		char *s = banner(n);

		if (s && *s)
			strcat(buf, sout(", \"{<i>}%s{</i>}\"", s));
	}

	if (loc_hidden(n))
		strcat(buf, ", hidden");

	if (ship_has_ram(n))
		strcat(buf, ", with ram");

	if (fee = board_fee(n))
		strcat(buf, sout(", %s per 100 wt. to board", gold_s(fee)));

	return sout("%s", buf);
}


/*
 *	Name, mountain province, in region foo
 *	Name, port city, in province Name [, in region foo]
 *	Name, ocean, in Sea
 *	Name, island, in Ocean [, in Sea]
 *
 *	Mountain [aa01], mountain province, in region Tollus
 *	Island [aa01], island, in Ocean [bb01]
 *	City [aa01], port city, in province Mountain [aa01]
 *	Ocean [bb02], ocean, in South Sea
 */


static char *
liner_desc_loc(int n)
{
	char buf[LEN];

	sprintf(buf, "%s%s%s",
				box_name_kind(n),
				safe_haven_s(n),
				incomplete_string(n));

	if (loc_depth(n) == LOC_build)
	{
		if (loc_defense(n))
			strcat(buf, sout(", defense~%d", loc_defense(n)));

		if (mine_depth(n))
			strcat(buf, sout(", depth~%d", mine_depth(n)));

		if (loc_damage(n))
			strcat(buf, sout(", %d%%~damaged", loc_damage(n)));

		if (castle_level(n))
			strcat(buf, sout(", level~%d", castle_level(n)));
	}

	if (show_display_string)
	{
		char *s = banner(n);

		if (s && *s)
			strcat(buf, sout(", \"%s\"", s));
	}

	if (loc_hidden(n))
		strcat(buf, ", hidden");

	if (loc_depth(n) == LOC_subloc)
	{
		if (subkind(n) == sub_hades_pit)
			strcat(buf, ", 28 days");
		else
			strcat(buf, ", 1 day");
	}

	return sout("%s", buf);
}


static char *
mage_s(int n)
{
	int a;

	if (!is_magician(n) || char_hide_mage(n))
		return "";

	a = max_eff_aura(n);

	if (a <= 5)	return "";
	if (a <= 10)	return ", conjurer";
	if (a <= 15)	return ", mage";
	if (a <= 20)	return ", wizard";
	if (a <= 30)	return ", sorcerer";
	if (a <= 40)	return ", 6th black circle";
	if (a <= 50)	return ", 5th black circle";
	if (a <= 60)	return ", 4th black circle";
	if (a <= 70)	return ", 3rd black circle";
	if (a <= 80)	return ", 2nd black circle";

	return ", master of the black arts";
}


static char *
priest_s(int n)
{

	if (!is_priest(n))
		return "";

	return ", priest";
}


int
emperor()
{
	int who = 0;
	int where;

	if (valid_box(RELIC_THRONE))
		who = item_unique(RELIC_THRONE);

	if (!who)
		return 0;

	if (!player(who))
		return 0;

	if (subkind(player(who)) != sub_pl_regular)
		return 0;

	where = subloc(who);

	if (subkind(where) != sub_castle)
		return 0;

	if (province(where) != MAP_MT_OLY)	/* Mt. Olympus */
		return 0;

	return who;
}


int show_display_string = FALSE;
char *combat_ally = "";


static char *
liner_desc_char(int n)
{
	char buf[LEN];
	extern int show_combat_flag;
	char *s;
	int sk;

	strcpy(buf, box_name(n));

	sk = subkind(n);

	if (sk == sub_ni)
	{
		int mk = noble_item(n);
		int num = has_item(n, mk) + 1;

		if (num == 1)
			strcat(buf, sout(", %s", plural_item_name(mk, num)));
		else
			strcat(buf, sout(", %s, number:~%s",
					plural_item_name(mk, num),
					comma_num(num)));
	}
	else if (sk)
	{
		strcat(buf, sout(", %s", subkind_s[sk]));
	}

	strcat(buf, rank_s(n));
	strcat(buf, mage_s(n));
	strcat(buf, priest_s(n));
	if (emperor() == n)
		strcat(buf, ", Emperor");
	strcat(buf, wield_s(n));

	if (show_combat_flag)
	{
		if (char_behind(n))
			strcat(buf, sout(", behind~%d%s",
					char_behind(n), combat_ally));
		else
			strcat(buf, combat_ally);
	}
	else if (char_guard(n) && stack_leader(n) == n)
		strcat(buf, ", on guard");

#if 0
	if (subkind(n) == 0)	/* only show lord for regular players */
	{
		int sp = lord(n);

		if (sp != indep_player && !cloak_lord(n))
			strcat(buf, sout(", of~%s", box_code_less(sp)));
	}
#endif

	if (show_display_string)
	{
		s = banner(n);

		if (s && *s)
			strcat(buf, sout(", \"{<i>}%s{</i>}\"", s));
	}

	strcat(buf, with_inventory_string(n));

	if (is_prisoner(n))
		strcat(buf, ", prisoner");

	return sout("%s", buf);
}


static char *
liner_desc_road(int n)
{
	int dest;
	char *hid = "";
	int dist;

	dest = road_dest(n);

	if (road_hidden(n))
		hid = ", hidden";

	dist = exit_distance(loc(n), dest);

	return sout("%s, to %s%s, %d~day%s",
			box_name(n), box_name(dest), hid,
			add_ds(dist));
}


static char *
liner_desc_storm(int n)
{
	char buf[LEN];
	int owner;
	struct entity_misc *p;

	sprintf(buf, "%s", box_name_kind(n));

	p = rp_misc(n);

	owner = npc_summoner(n);
	if (owner)
		strcat(buf, sout(", owner~%s", box_code_less(owner)));

	strcat(buf, sout(", strength~%s", comma_num(storm_strength(n))));

	if (p && p->npc_dir)
		strcat(buf, sout(", heading %s", full_dir_s[p->npc_dir]));

	return sout("%s", buf);
}


/*
 *  Viewed from outside
 */

char *
liner_desc(int n)
{

	switch (kind(n))
	{
	case T_ship:	return liner_desc_ship(n);
	case T_loc:	return liner_desc_loc(n);
	case T_char:	return liner_desc_char(n);
	case T_road:	return liner_desc_road(n);
	case T_storm:	return liner_desc_storm(n);

	default:
		assert(FALSE);
	}
	return 0;
}


static char *
highlight_units(int who, int n, int depth)
{

	assert(depth >= 3);
	assert(indent == 0);

	if (kind(who) == T_player && player(n) == who)
		return sout(" *%s", &spaces[spaces_len - (depth - 2)]);

	return &spaces[spaces_len - depth];
}


void
show_chars_below(int who, int n)
{
	int i;

	assert(valid_box(who));

	indent += 3;
	loop_char_here(n, i)
	{
		assert(valid_box(who));
		wiout(who, 3, "%s", liner_desc(i));
	}
	next_char_here;
	indent -= 3;
}


static void
show_chars_below_highlight(int who, int n, int depth)
{
	int i;

	depth += 3;

	loop_char_here(n, i)
	{
		wiout(who, depth, "%s%s",
				highlight_units(who, i, depth),
				liner_desc(i));
	}
	next_char_here;
}


void
show_owner_stack(int who, int n)
{
	int i;
	int depth;
	int first = TRUE;

	depth = indent + 3;
	indent = 0;

	loop_here(n, i)
	{
		if (kind(i) == T_char)
		{
			if (!first && char_really_hidden(i))
				continue;

			wiout(who, depth, "%s%s%s",
						highlight_units(who, i, depth),
						liner_desc(i),	
						display_with(i));

			show_chars_below_highlight(who, i, depth);

			first = FALSE;
		}
	}
	next_here;

	indent = depth - 3;
}


static void
show_chars_here(int who, int where)
{
	int first = TRUE;
	int i;
	int depth;
	char *flying = "";

	if (loc_depth(where) == LOC_province && weather_here(where, sub_fog))
	{
		out(who, "");
		out(who, "No one can be seen through the fog.");
		return;
	}

	depth = indent;
	indent = 0;

	if (subkind(where) == sub_ocean)
		flying = ", flying";

	loop_here(where, i)
	{
		if (kind(i) == T_char)
		{
			if (char_really_hidden(i))
				continue;

			if (first)
			{
				first = FALSE;
				out(who, "");
				out(who, "Seen here:");
				depth += 3;
			}

			wiout(who, depth, "%s%s%s%s",
						highlight_units(who, i, depth),
						liner_desc(i),	
						flying,
						display_with(i));

			show_chars_below_highlight(who, i, depth);
		}
	}
	next_here;

	if (!first)
	{
		depth -= 3;
	}

	indent = depth;
}


static void
show_inner_locs(int who, int where)
{
	int first = TRUE;
	int i;

	loop_here(where, i)
	{
		if (is_loc_or_ship(i))
		{
			if (loc_hidden(i) &&
			    !test_known(who, i) &&
			    !see_all(who))
				continue;

			if (first)
			{
				first = FALSE;
				indent += 3;
			}

			wout(who, "%s%s", liner_desc(i),
					display_owner(i));
			show_owner_stack(who, i);
			show_loc_barrier(who, i);
			show_inner_locs(who, i);
		}
	}
	next_here;

	if (!first)
		indent -= 3;
}


static void
show_sublocs_here(int who, int where)
{
	int first = TRUE;
	int i;
	struct entity_subloc *p;

	loop_here(where, i)
	{
		if (kind(i) == T_loc)
		{
			if (loc_hidden(i) &&
			    !test_known(who, i) &&
			    !see_all(who))
				continue;

#if 0
			if (subkind(where) == sub_tunnel &&
			    subkind(i) == sub_sewer)
				continue;
#endif

			if (first)
			{
				first = FALSE;
				out(who, "");
				out(who, "Inner locations:");
				indent += 3;
			}

			if (subkind(i) == sub_city)
			{
				wout(who, "%s", liner_desc(i));
				show_loc_barrier(who, i);
			}
			else
			{
				wout(who, "%s%s", liner_desc(i),
						display_owner(i));
				show_owner_stack(who, i);
				show_loc_barrier(who, i);
				show_inner_locs(who, i);
			}
		}
	}
	next_here;

	p = rp_subloc(where);

	if (p)
	{
	    for (i = 0; i < ilist_len(p->link_from); i++)
	    {
		if (loc_hidden(p->link_from[i]) &&
		    !test_known(who, p->link_from[i]) &&
		    !see_all(who))
			continue;

		if (loc_link_open(p->link_from[i]))
		{
			if (first)
			{
				first = FALSE;
				out(who, "");
				out(who, "Inner locations:");
				indent += 3;
			}

			wout(who, "%s", liner_desc(p->link_from[i]));
		}
	    }
	}

	if (!first)
	{
		indent -= 3;
	}
}


static void
show_ships_here(int who, int where)
{
	int first = TRUE;
	int i;

	loop_here(where, i)
	{
		if (kind(i) == T_ship)
		{
			if (loc_hidden(i) &&
			    !test_known(who, i) &&
			    !see_all(who))
				continue;

			if (first)
			{
				first = FALSE;

				out(who, "");
				if (subkind(where) == sub_ocean)
					out(who, "Ships sighted:");
				else
					out(who, "Ships docked at port:");

				indent += 3;
			}

			wiout(who, 3, "%s%s", liner_desc(i), display_owner(i));
			show_owner_stack(who, i);
			show_loc_barrier(who, i);
		}
	}
	next_here;

	if (!first)
	{
		indent -= 3;
	}
}


static void
show_nearby_cities(int who, int where)
{
	struct entity_subloc *p;
	int i;
	char *s;

	p = rp_subloc(where);

	if (p == NULL || ilist_len(p->near_cities) < 1)
		return;

	out(who, "");
	out(who, "Cities rumored to be nearby:");
	indent += 3;
	for (i = 0; i < ilist_len(p->near_cities); i++)
	{
		if (safe_haven(p->near_cities[i]))
			s = ", safe haven";
		else
			s = "";

		out(who, "%s, in %s%s",
				box_name(p->near_cities[i]),
				box_name(province(p->near_cities[i])), s);
	}
	indent -= 3;
}


static void
show_loc_skills(int who, int where)
{
	int i;
	char *s = "";

	loop_loc_teach(where, i)
	{
		s = comma_append(s, box_name(i));
	}
	next_loc_teach;

	if (s && *s)
	{
		out(who, "");
		out(who, "Skills taught here:");
		indent += 3;
		wout(who, "%s", s);
		indent -= 3;
	}
}


void
show_loc_posts(int who, int where, int show_full_loc)
{
	int post;
	int i;
	int flag = TRUE;
	int first;
	char **l;

	loop_here(where, post)
	{
		if (kind(post) != T_post)
			continue;

		if (rp_misc(post) == NULL ||
		    ilist_len(rp_misc(post)->post_txt) < 1)
		{
			assert(FALSE);	/* what happened to the post? */
			continue;
		}

		l = rp_misc(post)->post_txt;

		if (flag)
		{
			out(who, "");
			wout(who, "Posted in %s:",
					show_full_loc ? box_name(where) :
						just_name(where));
			flag = FALSE;
			indent += 3;
		}
		else
			out(who, "");

		if (item_creator(post))
			wout(who, "Posted by %s:",
					box_name(item_creator(post)));
		else
			wout(who, "Posted:");

		indent += 3;

		first = TRUE;

		for (i = 0; i < ilist_len(l); i++)
		{
			wout(who, "%s%s%s",
					first ? "\"" : "",
					l[i],
					i+1 == ilist_len(l) ? "\"" : "");

			if (first)
			{
				first = FALSE;
				indent += 1;
			}
		}

		if (!first)
			indent -= 1;

		indent -= 3;
	}
	next_here;

	if (!flag)
		indent -= 3;
}


static void
show_weather(who, where)
{
	int rain, wind, fog;

	rain = weather_here(where, sub_rain);
	wind = weather_here(where, sub_wind);
	fog = weather_here(where, sub_fog);

	if (!rain && !wind && !fog)
		return;

	out(who, "");

	if (rain)
		out(who, "It is raining.");

	if (wind)
		out(who, "It is windy.");

	if (fog)
		out(who, "The province is blanketed in fog.");

	if (can_see_weather_here(who, where))
	{
		int i;
		int first = TRUE;

		loop_here(where, i)
		{
			if (kind(i) != T_storm)
				continue;

			if (first)
			{
				indent += 3;
				first = FALSE;
			}

			wout(who, "%s", liner_desc(i));
		}
		next_here;

		if (!first)
			indent -= 3;
	}
}


static void
show_loc_ruler(int who, int where)
{
	int garr;
	int castle;
	int ruler;

	if (loc_depth(where) != LOC_province || subkind(where) == sub_ocean)
		return;

	garr = garrison_here(where);

	if (garr == 0)
		return;

	castle = garrison_castle(garr);
	ruler = top_ruler(garr);

	if (castle)
	{
		int prov = province(castle);

		out(who, "Province controlled by %s, in %s",
				box_name_kind(castle), box_name(prov));

		if (ruler)
			out(who, "Ruled by %s%s",
					box_name(ruler), rank_s(ruler));

		out(who, "");
	}
}


/*
 *  Don't include the leading location name, kind, etc. on the location 
 *  report.  Handled with a global since we don't have default parameters.
 */

int show_loc_no_header = FALSE;

void
show_loc(int who, int where)
{
	int pil;

	assert(valid_box(where));

	show_display_string = TRUE;  /* previously only for show_chars_here */

	if (!show_loc_no_header)
	{
		wout(who, "%s", show_loc_header(where));
		out(who, "");
	}

	if (show_loc_barrier(who, where))
		out(who, "");

	if (pil = loc_pillage(where))
	{
		wout(who, "Recovery from%s pillaging will take %s month%s.",
				recent_pillage(where) ? " recent" : "",
				nice_num(pil),
				add_s(pil));
		out(who, "");
	}

	show_loc_ruler(who, where);

	list_sailable_routes(who, where);
	list_exits(who, where);
	show_loc_stats(who, where);
	show_nearby_cities(who, where);
	show_loc_skills(who, where);

	if (subkind(where) == sub_city)
		market_report(who, where);

	show_sublocs_here(who, where);
	show_loc_posts(who, where, FALSE);
	show_weather(who, where);
	show_chars_here(who, where);
	show_ships_here(who, where);

	show_display_string = FALSE;
}

