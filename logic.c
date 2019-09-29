#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "logic.h"

/* Create new node for disjunction.
 */
struct Expr *make_disj(struct Expr *expr1, struct Expr *expr2) {
	struct Expr *expr = malloc(sizeof(struct Expr));
	expr->tag = isDisj;
	expr->expr1 = expr1;
	expr->expr2 = expr2;
	return expr;
}

struct Expr *make_conj(struct Expr *expr1, struct Expr *expr2) {
	struct Expr *expr = malloc(sizeof(struct Expr));
	expr->tag = isConj;
	expr->expr1 = expr1;
	expr->expr2 = expr2;
	return expr;
}

struct Expr *make_neg(struct Expr *expr1) {
	struct Expr *expr = malloc(sizeof(struct Expr));
	expr->tag = isNeg;
	expr->expr1 = expr1;
	return expr;
}

struct Expr *make_true() {
	struct Expr *expr = malloc(sizeof(struct Expr));
	expr->tag = isTrue;
	return expr;
}

struct Expr *make_false() {
	struct Expr *expr = malloc(sizeof(struct Expr));
	expr->tag = isFalse;
	return expr;
}

struct Expr *make_var(char var) {
	struct Expr *expr = malloc(sizeof(struct Expr));
	expr->tag = isVar;
	expr->var = var;
	return expr;
}

/* Make deep copy of expression.
 */
struct Expr *copy_expr(struct Expr *expr) {
	switch (expr->tag) {
		case isDisj:
			return make_disj(copy_expr(expr->expr1), copy_expr(expr->expr2));
		case isConj:
			return make_conj(copy_expr(expr->expr1), copy_expr(expr->expr2));
		case isNeg:
			return make_neg(copy_expr(expr->expr1));
		case isTrue:
			return make_true();
		case isFalse:
			return make_false();
		case isVar:
			return make_var(expr->var);
	}
}

/* Free all space recursively in expression.
 */
void free_expr(struct Expr *expr) {
	switch (expr->tag) {
		case isDisj:
		case isConj:
			free_expr(expr->expr1);
			free_expr(expr->expr2);
			break;
		case isNeg:
			free_expr(expr->expr1);
			break;
		default:
			break;
	}
	free(expr);
}

/* Equality of two expressions.
 */
bool equal_expr(struct Expr *expr1, struct Expr *expr2) {
	if (expr1->tag != expr2->tag)
		return false;
	else {
		switch (expr1->tag) {
			case isDisj:
			case isConj:
				return equal_expr(expr1->expr1, expr2->expr1) &&
						equal_expr(expr1->expr2, expr2->expr2);
			case isNeg:
				return equal_expr(expr1->expr1, expr2->expr1);
			case isTrue:
				return true;
			case isFalse:
				return true;
			case isVar:
				return expr1->var == expr2->var;
		}
	}
}

/* Auxiliary functions for printing Boolean expressions.
 */
static void print_expr_nested(struct Expr *expr, bool in_conj);
static void print_disj(struct Expr *expr, bool in_conj);
static void print_conj(struct Expr *expr);
static void print_neg(struct Expr *expr);

void print_expr(struct Expr *expr) {
	print_expr_nested(expr, false);
}

/* Print nested expression. Is this expression a direct subexpression of 
 * a conjunction? Then brackets are needed if this is a disjunction.
 */
static void print_expr_nested(struct Expr *expr, bool in_conj) {
	switch (expr->tag) {
		case isDisj:
			print_disj(expr, in_conj);
			break;
		case isConj:
			print_conj(expr);
			break;
		case isNeg:
			print_neg(expr);
			break;
		case isTrue:
			printf("T");
			break;
		case isFalse:
			printf("F");
			break;
		case isVar:
			printf("%c", expr->var);
			break;
	}
}

/* Add brackets if needed to override operator precedence.
 */
static void print_disj(struct Expr *expr, bool in_conj) {
	if (in_conj)
		printf("(");
	print_expr_nested(expr->expr1, false);
	printf("|");
	print_expr_nested(expr->expr2, false);
	if (in_conj)
		printf(")");
}

static void print_conj(struct Expr *expr) {
	print_expr_nested(expr->expr1, true);
	printf("&");
	print_expr_nested(expr->expr2, true);
}

/* A negation of a disjunction or conjunction requires a pair of brackets.
 */
static void print_neg(struct Expr *expr) {
	printf("-");
	if (expr->expr1->tag == isDisj || expr->expr1->tag == isConj) {
		printf("(");
		print_expr_nested(expr->expr1, false);
		printf(")");
	} else {
		print_expr_nested(expr->expr1, false);
	}
}

/* Auxiliary functions for parsing Boolean expression.
 */
static struct Expr *read_expr_from(char *str, int *pos);
static struct Expr *read_conj_from(char *str, int *pos);
static struct Expr *read_base_from(char *str, int *pos);
static bool force_read(char *str, int *pos, char c);

/* Read expression from string.
 * Return NULL if this fails.
 */
struct Expr *read_expr(char *str) {
	int pos = 0;
	struct Expr *expr = read_expr_from(str, &pos);
	if (str[pos] != '\0') {
		fprintf(stderr, "Unexpected %c at %d in %s\n", str[pos], pos, str);
		return NULL;
	}
	return expr;
}

/* Read expression from position in string.
 */
static struct Expr *read_expr_from(char *str, int *pos) {
	struct Expr *expr = read_conj_from(str, pos);
	if (expr == NULL)
		return NULL;
	while (str[*pos] == '|') {
		(*pos)++;
		struct Expr *next = read_conj_from(str, pos);
		if (next == NULL)
			return NULL;
		expr = make_disj(expr, next);
	}
	return expr;
}

static struct Expr *read_conj_from(char *str, int *pos) {
	struct Expr *expr = read_base_from(str, pos);
	if (expr == NULL)
		return NULL;
	while (str[*pos] == '&') {
		(*pos)++;
		struct Expr *next = read_base_from(str, pos);
		if (next == NULL)
			return NULL;
		expr = make_conj(expr, next);
	}
	return expr;
}

/* Read base expression.
 */
static struct Expr *read_base_from(char *str, int *pos) {
	struct Expr *expr = NULL;
	switch (str[*pos]) {
		case '(':
			(*pos)++;
			expr = read_expr_from(str, pos);
			if (expr == NULL || !force_read(str, pos, ')'))
				return NULL;
			break;
		case '-':
			(*pos)++;
			struct Expr *expr1 = read_base_from(str, pos);
			if (expr1 == NULL)
				return NULL;
			expr = make_neg(expr1);
			break;
		case 'F':
			(*pos)++;
			expr = make_false();
			break;
		case 'T':
			(*pos)++;
			expr = make_true();
			break;
		default:
			if ('a' <= str[*pos] && str[*pos] <= 'z') {
				char var = str[*pos];
				(*pos)++;
				expr = make_var(var);
			} else {
				fprintf(stderr, "Unexpected %c at %d in %s\n", str[*pos], *pos, str);
			}
			break;
	}
	return expr;
}

/* Read character from position. If not found, report error and return false.
 */
static bool force_read(char *str, int *pos, char c) {
	if (str[*pos] == c) {
		(*pos)++;
		return true;
	} else {
		fprintf(stderr, "Expected %c not %c at %d in %s\n", c, str[*pos], *pos, str);
		return false;
	}
}
