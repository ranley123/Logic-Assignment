#ifndef LOGIC_H
#define LOGIC_H

#include <stdbool.h>

/* The different kinds of expression.
 */
enum ExprTag {isDisj, isConj, isNeg, isTrue, isFalse, isVar};

/* An expression can be of different kinds.
 * If it is a disjunction or conjunction, it has two subexpressions
 * expr1 and expr2.
 * If it is a negation, it has one subexpression expr1.
 * If it is a variable, then it has a name var.
 */
struct Expr {
	enum ExprTag tag;
	union {
		struct {
			struct Expr *expr1;
			struct Expr *expr2;
		};
		char var;
	};
};

struct Expr *make_disj(struct Expr *expr1, struct Expr *expr2);
struct Expr *make_conj(struct Expr *expr1, struct Expr *expr2);
struct Expr *make_neg(struct Expr *expr);
struct Expr *make_true();
struct Expr *make_false();
struct Expr *make_var(char name);

struct Expr *copy_expr(struct Expr *expr);

void free_expr(struct Expr *expr);

bool equal_expr(struct Expr *expr1, struct Expr *expr2);

void print_expr(struct Expr *expr);

struct Expr *read_expr(char *str);

#endif // LOGIC_H
