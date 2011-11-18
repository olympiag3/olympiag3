

extern struct skill_ent *p_skill_ent(int who, int skill);
extern struct skill_ent *rp_skill_ent(int who, int skill);
extern int has_skill(int who, int skill);
extern void set_skill(int who, int skill, int know);

extern int skill_school(int sk);
extern void list_skills(int who, int num);

extern void learn_skill(int who, int sk);
extern char *exp_s(int level);
extern int forget_skill(int who, int skill);
