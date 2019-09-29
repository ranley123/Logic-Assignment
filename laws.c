#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "laws.h"
#include "logic.h"

/* A path is an array of numbers referring to a subexpression
 * of a given expression. Cf. the concept of Gorn address.
 * A '1' means follow the first child in the parse tree
 * and '2' means follow the second child in the parse tree
 * and '0' means 'take this node' (at the end of the path).
 * We use a path consisting of just '-1' for 'there is no path'.
 */
int *non_path() {
	int *path = malloc(sizeof(int));
	path[0] = -1;
	return path;
}

/* Non-path that doesn't need to be (should not be) freed.
 */
static int NON_PATH[] = {-1};

void print_path(int *path) {
	int i = 0;
	while (true) {
		printf("%d ", path[i]);
		if (path[i] < 1)
			break;
		else
			i++;
	}
	printf("\n");
}

/* Does this match left-hand, right-hand side of:
 * A|B = B|A
 * For uniformity, both are included, but they are of course the same.
 */
static bool is_comm_disj_lhs(struct Expr *expr) {
	return expr->tag == isDisj;
}
static bool is_comm_disj_rhs(struct Expr *expr) {
	return expr->tag == isDisj;
}

/* A&B = B&A
 */
static bool is_comm_conj_lhs(struct Expr *expr) {
	return expr->tag == isConj;
}
static bool is_comm_conj_rhs(struct Expr *expr) {
	return expr->tag == isConj;
}

/* (A|B)|C = A|(B|C)
 */
static bool is_assoc_disj_lhs(struct Expr *expr) {
	return expr->tag == isDisj && expr->expr1->tag == isDisj;
}
static bool is_assoc_disj_rhs(struct Expr *expr) {
	return expr->tag == isDisj && expr->expr2->tag == isDisj;
}

/* (A&B)&C = A&(B&C)
 */
static bool is_assoc_conj_lhs(struct Expr *expr) {
	return expr->tag == isConj && expr->expr1->tag == isConj;
}
static bool is_assoc_conj_rhs(struct Expr *expr) {
	return expr->tag == isConj && expr->expr2->tag == isConj;
}

/* A|(B&C) = (A|B)&(A|C)
 */
static bool is_distr_disj_lhs(struct Expr *expr) {
	return expr->tag == isDisj && expr->expr2->tag == isConj;
}
static bool is_distr_disj_rhs(struct Expr *expr) {
	return expr->tag == isConj &&
		expr->expr1->tag == isDisj &&
		expr->expr2->tag == isDisj &&
		equal_expr(expr->expr1->expr1, expr->expr2->expr1);
}

/* A&(B|C) = (A&B)|(A&C)
 */
static bool is_distr_conj_lhs(struct Expr *expr) {
	return expr->tag == isConj && expr->expr2->tag == isDisj;
}
static bool is_distr_conj_rhs(struct Expr *expr) {
	return expr->tag == isDisj &&
		expr->expr1->tag == isConj &&
		expr->expr2->tag == isConj &&
		equal_expr(expr->expr1->expr1, expr->expr2->expr1);
}

/* A|(A&B) = A
 */
static bool is_abs_disj_lhs(struct Expr *expr) {
	return expr->tag == isDisj && expr->expr2->tag == isConj &&
		equal_expr(expr->expr1, expr->expr2->expr1);
}
static bool is_abs_disj_rhs(struct Expr *expr) {
	(void) expr; // keep compiler happy
	return true;
}

/* A&(A|B) = A
 */
static bool is_abs_conj_lhs(struct Expr *expr) {
	return expr->tag == isConj && expr->expr2->tag == isDisj &&
		equal_expr(expr->expr1, expr->expr2->expr1);
}
static bool is_abs_conj_rhs(struct Expr *expr) {
	(void) expr; // keep compiler happy
	return true;
}

/* A|-A = T
 */
static bool is_compl_disj_lhs(struct Expr *expr) {
	return expr->tag == isDisj &&
		expr->expr2->tag == isNeg &&
		equal_expr(expr->expr1, expr->expr2->expr1);
}
static bool is_compl_disj_rhs(struct Expr *expr) {
	return expr->tag == isTrue;
}

/* A&-A = F
 */
static bool is_compl_conj_lhs(struct Expr *expr) {
	return expr->tag == isConj &&
		expr->expr2->tag == isNeg &&
		equal_expr(expr->expr1, expr->expr2->expr1);
}
static bool is_compl_conj_rhs(struct Expr *expr) {
	return expr->tag == isFalse;
}

/*******************************************/
/* Additional laws may be added here.      */
/*******************************************/

/* A&F = F
 */
static bool is_domi_conj_lhs(struct Expr *expr){
	return expr->tag == isConj && expr->expr2->tag == isFalse;
}
static bool is_domi_conj_rhs(struct Expr *expr){
	return expr->tag == isFalse;
}

/* A|T = T
 */
static bool is_domi_disj_lhs(struct Expr *expr){
	return expr->tag == isDisj && expr->expr2->tag == isTrue;
}
static bool is_domi_disj_rhs(struct Expr *expr){
	return expr->tag == isTrue;
}

/* --A = A
 */
static bool is_dou_neg_lhs(struct Expr *expr){
	return expr->tag == isNeg && expr->expr1->tag == isNeg;
}
static bool is_dou_neg_rhs(struct Expr *expr){
	(void) expr;
	return true;
}

/* -F = T
 */
static bool is_f_neg_lhs(struct Expr *expr){
	return expr->tag == isNeg && expr->expr1->tag == isFalse;
}
static bool is_f_neg_rhs(struct Expr *expr){
	return expr->tag == isTrue;
}

/* Part 3: De Morgan
 * -(A|B) = -A&-B
 */
static bool is_mor_disj_lhs(struct Expr *expr){
	return expr->tag == isNeg && expr->expr1->tag == isDisj;
}
/* -(A&B) = -A|-B
 */
static bool is_mor_conj_lhs(struct Expr *expr){
	return expr->tag == isNeg && expr->expr1->tag == isConj;
}

/* A&A = A
 */
static bool is_idemp_conj_lhs(struct Expr *expr){
	return expr->tag == isConj && equal_expr(expr->expr1, expr->expr2);
}

/*******************************************/
/* END ADDED                               */
/*******************************************/

/* Find subexpression satisfying predicate, at first position
 * lexicographically following the given path address.
 * Return NULL if none is found.
 */
static int *search_subexpression(struct Expr *expr, int *path, int depth,
		bool (*pred)(struct Expr *)) {
	// printf("depth %d\n", depth);
	// print_path(path);
	if (path[0] == -1) {
		if (pred(expr)) {
			int *found_path = malloc((depth+1) * sizeof(int));
			found_path[depth] = 0;
			return found_path;
		}
	}
	if (path[0] != 2) {
		switch (expr->tag) {
			case isDisj:
			case isConj:
			case isNeg:
			{
				int *recur_path = path[0] == 1 ? path+1 : NON_PATH;
				int *path1 = search_subexpression(expr->expr1, recur_path, depth+1, pred);
				if (path1 != NULL) {
					path1[depth] = 1;
					return path1;
				}
				break;
			}
			default:
				return NULL;
		}
	}
	switch (expr->tag) {
		case isDisj:
		case isConj:
		{
			int *recur_path = path[0] == 2 ? path+1 : NON_PATH;
			int *path2 = search_subexpression(expr->expr2, recur_path, depth+1, pred);
			if (path2 != NULL) {
				path2[depth] = 2;
				return path2;
			}
			break;
		}
		default:
			return NULL;
	}
	return NULL;
}

int *search_comm_disj_lhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_comm_disj_lhs);
}
int *search_comm_disj_rhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_comm_disj_rhs);
}
int *search_comm_conj_lhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_comm_conj_lhs);
}
int *search_comm_conj_rhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_comm_conj_rhs);
}

int *search_assoc_disj_lhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_assoc_disj_lhs);
}
int *search_assoc_disj_rhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_assoc_disj_rhs);
}
int *search_assoc_conj_lhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_assoc_conj_lhs);
}
int *search_assoc_conj_rhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_assoc_conj_rhs);
}

int *search_distr_disj_lhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_distr_disj_lhs);
}
int *search_distr_disj_rhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_distr_disj_rhs);
}
int *search_distr_conj_lhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_distr_conj_lhs);
}
int *search_distr_conj_rhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_distr_conj_rhs);
}

int *search_abs_disj_lhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_abs_disj_lhs);
}
int *search_abs_disj_rhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_abs_disj_rhs);
}
int *search_abs_conj_lhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_abs_conj_lhs);
}
int *search_abs_conj_rhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_abs_conj_rhs);
}

int *search_compl_disj_lhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_compl_disj_lhs);
}
int *search_compl_disj_rhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_compl_disj_rhs);
}
int *search_compl_conj_lhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_compl_conj_lhs);
}
int *search_compl_conj_rhs(struct Expr *expr, int *path) {
	return search_subexpression(expr, path, 0, is_compl_conj_rhs);
}

/*******************************************/
/* Additional laws may be added here.      */
/*******************************************/

int *search_domi_disj_lhs(struct Expr *expr, int *path){
	return search_subexpression(expr, path, 0, is_domi_disj_lhs);
}
int *search_domi_disj_rhs(struct Expr *expr, int *path){ // can be omitted as redundant
	return search_subexpression(expr, path, 0, is_domi_disj_rhs); 
}
int *search_domi_conj_lhs(struct Expr *expr, int *path){
	return search_subexpression(expr, path, 0, is_domi_conj_lhs);
}
int *search_domi_conj_rhs(struct Expr *expr, int *path){ // can be omitted as redundant
	return search_subexpression(expr, path, 0, is_domi_conj_rhs);
}

int *search_dou_neg_lhs(struct Expr *expr, int *path){
	return search_subexpression(expr, path, 0, is_dou_neg_lhs);
}
int *search_dou_neg_rhs(struct Expr *expr, int *path){
	return search_subexpression(expr, path, 0, is_dou_neg_rhs);
}

int *search_f_neg_lhs(struct Expr *expr, int *path){
	return search_subexpression(expr, path, 0, is_f_neg_lhs);
}
int *search_f_neg_rhs(struct Expr *expr, int *path){
	return search_subexpression(expr, path, 0, is_f_neg_rhs);
}
int *search_idemp_lhs(struct Expr *expr, int *path){
	return search_subexpression(expr, path, 0, is_idemp_conj_lhs);
}


/** Part 3
 */
int *search_mor_disj_lhs(struct Expr *expr, int *path){
	return search_subexpression(expr, path, 0, is_mor_disj_lhs);
}
int *search_mor_conj_lhs(struct Expr *expr, int *path){
	return search_subexpression(expr, path, 0, is_mor_conj_lhs);
}

/*******************************************/
/* END ADDED                               */
/*******************************************/

/* A|B => B|A or A|B <= B|A, which is the same.
 */
static struct Expr *apply_comm_disj_forward(struct Expr *expr) {
	return make_disj(copy_expr(expr->expr2), copy_expr(expr->expr1));
}
static struct Expr *apply_comm_disj_backward(struct Expr *expr) {
	return apply_comm_disj_forward(expr);
}

/* A&B => B&A or A&B <= B&A, which is the same.
 */
static struct Expr *apply_comm_conj_forward(struct Expr *expr) {
	return make_conj(copy_expr(expr->expr2), copy_expr(expr->expr1));
}
static struct Expr *apply_comm_conj_backward(struct Expr *expr) {
	return apply_comm_conj_forward(expr);
}

/* (A|B)|C => A|(B|C)
 */
static struct Expr *apply_assoc_disj_forward(struct Expr *expr) {
	return make_disj(copy_expr(expr->expr1->expr1),
			make_disj(copy_expr(expr->expr1->expr2), copy_expr(expr->expr2)));
}
/* (A|B)|C <= A|(B|C)
 */
static struct Expr *apply_assoc_disj_backward(struct Expr *expr) {
	return make_disj(
			make_disj(copy_expr(expr->expr1), copy_expr(expr->expr2->expr1)),
				copy_expr(expr->expr2->expr2));
}

/* (A&B)&C => A&(B&C)
 */
static struct Expr *apply_assoc_conj_forward(struct Expr *expr) {
	return make_conj(copy_expr(expr->expr1->expr1),
			make_conj(copy_expr(expr->expr1->expr2), copy_expr(expr->expr2)));
}
/* (A&B)&C <= A&(B&C)
 */
static struct Expr *apply_assoc_conj_backward(struct Expr *expr) {
	return make_conj(
			make_conj(copy_expr(expr->expr1), copy_expr(expr->expr2->expr1)),
				copy_expr(expr->expr2->expr2));
}

/* A|(B&C) => (A|B)&(A|C)
 */
static struct Expr *apply_distr_disj_forward(struct Expr *expr) {
	return make_conj(
			make_disj(copy_expr(expr->expr1), copy_expr(expr->expr2->expr1)),
			make_disj(copy_expr(expr->expr1), copy_expr(expr->expr2->expr2)));
}
/* A|(B&C) <= (A|B)&(A|C)
 */
static struct Expr *apply_distr_disj_backward(struct Expr *expr) {
	return make_disj(copy_expr(expr->expr1->expr1),
			make_conj(copy_expr(expr->expr1->expr2), copy_expr(expr->expr2->expr2)));
}

/* A&(B|C) => (A&B)|(A&C)
 */
static struct Expr *apply_distr_conj_forward(struct Expr *expr) {
	return make_disj(
			make_conj(copy_expr(expr->expr1), copy_expr(expr->expr2->expr1)),
			make_conj(copy_expr(expr->expr1), copy_expr(expr->expr2->expr2)));
}
/* A&(B|C) <= (A&B)|(A&C)
 */
static struct Expr *apply_distr_conj_backward(struct Expr *expr) {
	return make_conj(copy_expr(expr->expr1->expr1),
			make_disj(copy_expr(expr->expr1->expr2), copy_expr(expr->expr2->expr2)));
}

/* A|(A&B) => A, cannot easily be used backwards.
 */
static struct Expr *apply_abs_disj_forward(struct Expr *expr) {
	return copy_expr(expr->expr1);
}

/* A&(A|B) => A, cannot easily be used backwards.
 */
static struct Expr *apply_abs_conj_forward(struct Expr *expr) {
	return copy_expr(expr->expr1);
}

/* A|-A => T, cannot easily be used backwards.
 */
static struct Expr *apply_compl_disj_forward(struct Expr *expr) {
	(void) expr; // keep compiler happy
	return make_true();
}

/* A&-A => F, cannot easily be used backwards.
 */
static struct Expr *apply_compl_conj_forward(struct Expr *expr) {
	(void) expr; // keep compiler happy
	return make_false();
}

/*******************************************/
/* Additional laws may be added here.      */
/*******************************************/

/* A&F => F
 */
static struct Expr *apply_domi_conj_forward(struct Expr *expr) {
	(void) expr;
	return make_false();
}
/* A|T => T
 */
static struct Expr *apply_domi_disj_forward(struct Expr *expr){
	(void) expr;
	return make_true();
}

/* --A => A
 */
static struct Expr *apply_dou_neg_forward(struct Expr *expr){
	return copy_expr(expr->expr1->expr1);
}
/* A => --A
 */
static struct Expr *apply_dou_neg_backward(struct Expr *expr){
	return make_neg(make_neg(copy_expr(expr)));
}

/* -F => T
 */
static struct Expr *apply_f_neg_forward(struct Expr *expr){
	(void) expr;
	return make_true();
}
/* -T => F
 */
static struct Expr *apply_f_neg_backward(struct Expr *expr){
	(void) expr;
	return make_neg(make_false());
}

static struct Expr *apply_idemp_conj_forward(struct Expr *expr){
	return copy_expr(expr->expr1);
}
// Part 3 

/* -(A|B) => -A&-B
 */
static struct Expr *apply_mor_disj_forward(struct Expr *expr){
	return make_conj(make_neg(copy_expr(expr->expr1)), make_neg(copy_expr(expr->expr2)));
}
/* -(A&B) => -A|-B
 */
static struct Expr *apply_mor_conj_forward(struct Expr *expr){
	return make_disj(make_neg(copy_expr(expr->expr1)), make_neg(copy_expr(expr->expr2)));
}
/*******************************************/
/* END ADDED                               */
/*******************************************/

/* Transform subexpression at path.
 */
static struct Expr *apply_law(struct Expr *expr, int *path,
		struct Expr *(*transform)(struct Expr *)) {
	if (path[0] == 0) {
		return transform(expr);
	} else if (path[0] == 1) {
		switch (expr->tag) {
			case isDisj:
				return make_disj(apply_law(expr->expr1, path+1, transform),
									copy_expr(expr->expr2));
			case isConj:
				return make_conj(apply_law(expr->expr1, path+1, transform),
									copy_expr(expr->expr2));
			case isNeg:
				return make_neg(apply_law(expr->expr1, path+1, transform));
			default:
				return NULL;
		}
	} else {
		switch (expr->tag) {
			case isDisj:
				return make_disj(copy_expr(expr->expr1),
									apply_law(expr->expr2, path+1, transform));
			case isConj:
				return make_conj(copy_expr(expr->expr1),
									apply_law(expr->expr2, path+1, transform));
			default:
				return NULL;
		}
	}
}

struct Expr *apply_comm_disj_forward_at(struct Expr *expr, int *path) {
	return apply_law(expr, path, apply_comm_disj_forward);
}
struct Expr *apply_comm_disj_backward_at(struct Expr *expr, int *path) {
	return apply_law(expr, path, apply_comm_disj_backward);
}
struct Expr *apply_comm_conj_forward_at(struct Expr *expr, int *path) {
	return apply_law(expr, path, apply_comm_conj_forward);
}
struct Expr *apply_comm_conj_backward_at(struct Expr *expr, int *path) {
	return apply_law(expr, path, apply_comm_conj_backward);
}

struct Expr *apply_assoc_disj_forward_at(struct Expr *expr, int *path) {
	return apply_law(expr, path, apply_assoc_disj_forward);
}
struct Expr *apply_assoc_disj_backward_at(struct Expr *expr, int *path) {
	return apply_law(expr, path, apply_assoc_disj_backward);
}
struct Expr *apply_assoc_conj_forward_at(struct Expr *expr, int *path) {
	return apply_law(expr, path, apply_assoc_conj_forward);
}
struct Expr *apply_assoc_conj_backward_at(struct Expr *expr, int *path) {
	return apply_law(expr, path, apply_assoc_conj_backward);
}

struct Expr *apply_distr_disj_forward_at(struct Expr *expr, int *path) {
	return apply_law(expr, path, apply_distr_disj_forward);
}
struct Expr *apply_distr_disj_backward_at(struct Expr *expr, int *path) {
	return apply_law(expr, path, apply_distr_disj_backward);
}
struct Expr *apply_distr_conj_forward_at(struct Expr *expr, int *path) {
	return apply_law(expr, path, apply_distr_conj_forward);
}
struct Expr *apply_distr_conj_backward_at(struct Expr *expr, int *path) {
	return apply_law(expr, path, apply_distr_conj_backward);
}

struct Expr *apply_abs_disj_forward_at(struct Expr *expr, int *path) {
	return apply_law(expr, path, apply_abs_disj_forward);
}
struct Expr *apply_abs_conj_forward_at(struct Expr *expr, int *path) {
	return apply_law(expr, path, apply_abs_conj_forward);
}

struct Expr *apply_compl_disj_forward_at(struct Expr *expr, int *path) {
	return apply_law(expr, path, apply_compl_disj_forward);
}
struct Expr *apply_compl_conj_forward_at(struct Expr *expr, int *path) {
	return apply_law(expr, path, apply_compl_conj_forward);
}

/*******************************************/
/* Additional laws may be added here.      */
/*******************************************/

struct Expr *apply_domi_conj_forward_at(struct Expr *expr, int *path){
	return apply_law(expr, path, apply_domi_conj_forward);
}
struct Expr *apply_domi_disj_forward_at(struct Expr *expr, int *path){
	return apply_law(expr, path, apply_domi_disj_forward);
}

struct Expr *apply_dou_neg_forward_at(struct Expr *expr, int *path){
	return apply_law(expr, path, apply_dou_neg_forward);
}
struct Expr *apply_dou_neg_backward_at(struct Expr *expr, int *path){
	return apply_law(expr, path, apply_dou_neg_backward);
}

struct Expr *apply_f_neg_forward_at(struct Expr *expr,int *path){
	return apply_law(expr, path, apply_f_neg_forward);
}
struct Expr *apply_f_neg_backward_at(struct Expr *expr, int *path){
	return apply_law(expr, path, apply_f_neg_backward);
}
struct Expr *apply_idemp_conj_forward_at(struct Expr *expr, int *path){
	return apply_law(expr, path, apply_idemp_conj_forward);
}
/**
 * Part 3
 */
struct Expr *apply_mor_disj_forward_at(struct Expr *expr, int *path){
	return apply_law(expr, path, apply_mor_disj_forward);
}
struct Expr *apply_mor_conj_forward_at(struct Expr *expr, int *path){
	return apply_law(expr, path, apply_mor_conj_forward);
}
/*******************************************/
/* END ADDED                               */
/*******************************************/

/*******************************************/
/* For Part 1.                             */
/*******************************************/

LawSearch law_searches[] = {
	search_comm_disj_lhs,
	// omitted as redundant: search_comm_disj_rhs,
	search_comm_conj_lhs,
	// omitted as redundant: search_comm_conj_rhs,
	search_assoc_disj_lhs,
	search_assoc_disj_rhs,
	search_assoc_conj_lhs,
	search_assoc_conj_rhs,
	search_distr_disj_lhs,
	search_distr_disj_rhs,
	search_distr_conj_lhs,
	search_distr_conj_rhs,
	search_abs_disj_lhs,
	search_abs_conj_lhs,
	search_compl_disj_lhs,
	search_compl_conj_lhs
};

LawApplication law_applies[] = {
	apply_comm_disj_forward_at,
	// omitted as redundant: apply_comm_disj_backward_at,
	apply_comm_conj_forward_at,
	// omitted as redundant: apply_comm_conj_backward_at,
	apply_assoc_disj_forward_at,
	apply_assoc_disj_backward_at,
	apply_assoc_conj_forward_at,
	apply_assoc_conj_backward_at,
	apply_distr_disj_forward_at,
	apply_distr_disj_backward_at,
	apply_distr_conj_forward_at,
	apply_distr_conj_backward_at,
	apply_abs_disj_forward_at,
	apply_abs_conj_forward_at,
	apply_compl_disj_forward_at,
	apply_compl_conj_forward_at
};

char *law_names[] = {
	"commutative disj (forward)",
	"commutative conj (forward)",
	"associativity disj (forward)",
	"associativity disj (backward)",
	"associativity conj (forward)",
	"associativity conj (backward)",
	"distributivity disj (forward)",
	"distributivity disj (backward)",
	"distributivity conj (forward)",
	"distributivity conj (backward)",
	"absorption disj (forward)",
	"absorption conj (forward)",
	"complementation disj (forward)",
	"complementation conj (forward)"
};

int n_laws() {
	return sizeof(law_searches) / sizeof(LawSearch);
}

/*******************************************/
/* For Part 2.                             */
/*******************************************/

LawSearch extra_law_searches[] = {
	search_comm_disj_lhs,
	search_comm_conj_lhs,
	search_assoc_disj_lhs,
	search_assoc_disj_rhs,
	search_assoc_conj_lhs,
	search_assoc_conj_rhs,
	search_distr_disj_lhs,
	search_distr_disj_rhs,
	search_distr_conj_lhs,
	search_distr_conj_rhs,
	search_abs_disj_lhs,
	search_abs_conj_lhs,
	search_compl_disj_lhs,
	search_compl_conj_lhs,
	/*******************************************/
	/* Additional laws for Part 2 go here.     */
	/*******************************************/
	search_domi_conj_lhs,
	search_domi_disj_lhs,
	search_dou_neg_lhs,
	search_dou_neg_rhs,
	search_f_neg_lhs,
	search_f_neg_rhs,
	search_idemp_lhs
	/*******************************************/
	/* END ADDED                               */
	/*******************************************/
};

LawApplication extra_law_applies[] = {
	apply_comm_disj_forward_at,
	apply_comm_conj_forward_at,
	apply_assoc_disj_forward_at,
	apply_assoc_disj_backward_at,
	apply_assoc_conj_forward_at,
	apply_assoc_conj_backward_at,
	apply_distr_disj_forward_at,
	apply_distr_disj_backward_at,
	apply_distr_conj_forward_at,
	apply_distr_conj_backward_at,
	apply_abs_disj_forward_at,
	apply_abs_conj_forward_at,
	apply_compl_disj_forward_at,
	apply_compl_conj_forward_at,
	/*******************************************/
	/* Additional laws for Part 2 go here.     */
	/*******************************************/
	apply_domi_conj_forward_at,
	apply_domi_disj_forward_at,
	apply_dou_neg_forward_at,
	apply_dou_neg_backward_at,
	apply_f_neg_forward_at,
	apply_f_neg_backward_at,
	apply_idemp_conj_forward_at
	/*******************************************/
	/* END ADDED                               */
	/*******************************************/
};

char *extra_law_names[] = {
	"commutative disj (forward)",
	"commutative conj (forward)",
	"associativity disj (forward)",
	"associativity disj (backward)",
	"associativity conj (forward)",
	"associativity conj (backward)",
	"distributivity disj (forward)",
	"distributivity disj (backward)",
	"distributivity conj (forward)",
	"distributivity conj (backward)",
	"absorption disj (forward)",
	"absorption conj (forward)",
	"complementation disj (forward)",
	"complementation conj (forward)",
	/*******************************************/
	/* Additional laws for Part 2 go here.     */
	/*******************************************/
	"domination conj (forward)",
	"domination disj (forward)",
	"double negation (forward)",
	"double negation (backward)",
	"negation of constant -F (forward)",
	"negation of constant -F (backward)",
	"idempotence conj (forward)"
	/*******************************************/
	/* END ADDED                               */
	/*******************************************/
};

int n_extra_laws() {
	return sizeof(extra_law_searches) / sizeof(LawSearch);
}

/*******************************************/
/* For Part 3.                             */
/*******************************************/

LawSearch cnf_law_searches[] = {
	/*******************************************/
	/* Laws for Part 3 go here.                */
	/*******************************************/
	
	search_comm_disj_lhs,
	// omitted as redundant: search_comm_disj_rhs,
	search_comm_conj_lhs,
	// omitted as redundant: search_comm_conj_rhs,
	search_assoc_disj_lhs,
	search_assoc_disj_rhs,
	search_assoc_conj_lhs,
	search_assoc_conj_rhs,
	search_distr_disj_lhs,
	search_distr_disj_rhs,
	search_distr_conj_lhs,
	search_distr_conj_rhs,
	search_abs_disj_lhs,
	search_abs_conj_lhs,
	search_compl_disj_lhs,
	search_compl_conj_lhs,

	search_dou_neg_lhs,
	search_mor_conj_lhs,
	search_mor_disj_lhs,
	search_domi_conj_lhs,
	search_domi_disj_lhs,
	search_f_neg_lhs,
	search_idemp_lhs
	
	/*******************************************/
	/* END ADDED                               */
	/*******************************************/
};

LawApplication cnf_law_applies[] = {
	/*******************************************/
	/* Laws for Part 3 go here.                */
	/*******************************************/
	apply_comm_disj_forward_at,
	apply_comm_conj_forward_at,
	apply_assoc_disj_forward_at,
	apply_assoc_disj_backward_at,
	apply_assoc_conj_forward_at,
	apply_assoc_conj_backward_at,
	apply_distr_disj_forward_at,
	apply_distr_disj_backward_at,
	apply_distr_conj_forward_at,
	apply_distr_conj_backward_at,
	apply_abs_disj_forward_at,
	apply_abs_conj_forward_at,
	apply_compl_disj_forward_at,
	apply_compl_conj_forward_at,

	apply_dou_neg_forward_at,
	apply_mor_conj_forward_at,
	apply_mor_disj_forward_at,
	apply_domi_conj_forward_at,
	apply_domi_disj_forward_at,
	apply_f_neg_forward_at,
	apply_idemp_conj_forward_at
	/*******************************************/
	/* END ADDED                               */
	/*******************************************/
};

char *cnf_law_names[] = {
	/*******************************************/
	/* Laws for Part 3 go here.                */
	/*******************************************/
	"commutative disj (forward)",
	"commutative conj (forward)",
	"associativity disj (forward)",
	"associativity disj (backward)",
	"associativity conj (forward)",
	"associativity conj (backward)",
	"distributivity disj (forward)",
	"distributivity disj (backward)",
	"distributivity conj (forward)",
	"distributivity conj (backward)",
	"absorption disj (forward)",
	"absorption conj (forward)",
	"complementation disj (forward)",
	"complementation conj (forward)",

	"double negation (forward)",
	"de morgan conj (forward)",
	"de morgan disj (forward)",
	"domination conj (forward)",
	"domination disj (forward)",
	"negation of constant -F (forward)",
	"idempotence conj (forward)"
	/*******************************************/
	/* END ADDED                               */
	/*******************************************/
};

int n_cnf_laws() {
	return sizeof(cnf_law_searches) / sizeof(LawSearch);
}
