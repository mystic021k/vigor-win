int vigor_init __P((GS *));
void vigor_comment __P((char *expr, GS *gp));
void vigor_confirm __P((char *expr, GS *gp));
void vigor_maybe_confirm __P((int prob, char *expr, GS *gp));
void vigor_maybe_wizard __P((int prob, char *wizname, char *task0, char *task1,
			     char *task2, int this_task, char *descr, GS *gp));
void vigor_gotchar __P((char ch));

