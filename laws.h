#ifndef LAWS_H
#define LAWS_H

#include "logic.h"

int *non_path();

void print_path(int *path);

typedef int *(*LawSearch)(struct Expr *expr, int *path);
typedef struct Expr *(*LawApplication)(struct Expr *expr, int *path);

/*******************************************/
/* For Part 1.                             */
/*******************************************/

extern LawSearch law_searches[];
extern LawApplication law_applies[];
extern char* law_names[];
extern int n_laws();

/*******************************************/
/* For Part 2.                             */
/*******************************************/

extern LawSearch extra_law_searches[];
extern LawApplication extra_law_applies[];
extern char* extra_law_names[];
extern int n_extra_laws();

/*******************************************/
/* For Part 3.                             */
/*******************************************/

extern LawSearch cnf_law_searches[];
extern LawApplication cnf_law_applies[];
extern char* cnf_law_names[];
extern int n_cnf_laws();

#endif // LAWS_H
