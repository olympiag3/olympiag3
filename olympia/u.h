
/*
 *  defines, prototypes and externs for u.c
 */


struct weights {
	int animals;

	int total_weight;	/* total weight of unit or stack */

	int land_cap;		/* carryable weight on land */
	int land_weight;

	int ride_cap;		/* carryable weight on horseback */
	int ride_weight;

	int fly_cap;
	int fly_weight;
};

extern void determine_unit_weights(int who, struct weights *w);
extern void determine_stack_weights(int who, struct weights *w);
extern int ship_weight(int ship);

extern void olytime_increment(olytime *p);
extern void olytime_turn_change(olytime *p);

extern int lookup(char *table[], char *s);
extern char *cap(char *s);
extern char *nice_num(int n);
extern char *ordinal(int n);
extern char *comma_num(int n);
extern char *knum(int n, int nozero);
extern char *gold_s(int n);
extern char *weeks(int n);
extern char *more_weeks(int n);

extern int has_item(int who, int item);
extern void gen_item(int who, int item, int qty);
extern int consume_item(int who, int item, int qty);
extern int move_item(int from, int to, int item, int qty);
extern int create_unique_item(int who, int sk);
extern void destroy_unique_item(int who, int item);
extern int drop_item(int who, int item, int qty);

extern int charge(int who, int amount);
extern int can_pay(int who, int amount);

extern void set_known(int who, int i);
extern int test_known(int who, int i);
extern void clear_know_rec(sparse *kr);

extern void clear_temps(int kind);

extern int count_loc_char_item(int where, int item);
extern int check_char_here(int who, int target);
extern int check_char_gone(int who, int target);
extern int check_still_here(int who, int target);
extern int add_structure_damage(int fort, int damage, int can_destroy);
extern void delta_loyalty(int who, int amount, int silent);
extern void add_np(int pl, int num);
extern int deduct_np(int pl, int num);
extern int deduct_aura(int who, int amount);
extern int charge_aura(int who, int amount);
extern int count_man_items(int who);
extern int count_stack_units(int who);
extern int count_stack_figures(int who);
extern int check_aura(int who, int amount);
extern void get_rid_of_building(int fort);

#define		MATES		(-1)
#define		MATES_SILENT	(-2)
#define		TAKE_ALL	1
#define		TAKE_SOME	2
#define		TAKE_NI		3	/* noble item: wrapper adds one */

extern void add_char_damage(int who, int amount, int inherit);
extern void take_unit_items(int from, int inherit, int how_many);
extern int stackmate_inheritor(int who);
extern void kill_char(int who, int inherit);
extern int first_char_here(int where);
extern char *loyal_s(int who);
extern int find_nearest_land(int where);
extern int stack_has_item(int who, int item);
extern int new_char(int, int, int, int, int, int, int, char *);
extern void kill_stack_ocean(int who);
extern void hack_unique_item(int item, int owner);
extern int loc_hidden(int n);
extern char *rest_name(struct command *c, int a);
extern int nprovinces();
extern int has_use_key(int who, int key);
extern int stack_has_use_key(int who, int key);
extern int my_prisoner(int who, int pris);
extern int beast_capturable(int who);

extern int greater_region(int who);
extern int diff_region(int a, int b);

extern void set_bit(sparse * kr, int i);
