
#define		RANK_lord	10
#define		RANK_knight	20
#define		RANK_baron	30
#define		RANK_count	40
#define		RANK_earl	50
#define		RANK_marquess	60
#define		RANK_duke	70
#define		RANK_king	80

extern int top_ruler(int n);
extern void garrison_gold();
extern char *rank_s(int who);
extern void touch_garrison_locs();
extern void determine_noble_ranks();
extern int may_rule_here(int who, int where);
extern ilist players_who_rule_here(int where);
extern int garrison_notices(int garr, int target);
