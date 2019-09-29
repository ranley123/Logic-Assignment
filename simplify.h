#ifndef SIMPLIFY_H
#define SIMPLIFY_H

#include "laws.h"

void find_derivations_for_strings(int max_depth,
		LawSearch searches[], LawApplication applies[], char* names[], int n_laws);
int min_deri(int size, int *deri, int max_depth);
int apply(struct Expr *expr_tree, int cur_depth, int max_depth, LawSearch searches[],
          LawApplication applies[], int n_laws);

#endif // SIMPLIFY_H
