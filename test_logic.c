#include <stdio.h>

#include "logic.h"
#include "laws.h"

/* Test parsing of expression.
 */
static void test_expr_io_str(char* str) {
	struct Expr *expr = read_expr(str);
	if (expr == NULL) {
		printf("Cannot parse: %s\n", str);
		return;
	}
	printf("%s should be same as:\n", str);
	print_expr(expr);
	printf("\n");
	free_expr(expr);
}

void test_expr_io() {
	test_expr_io_str("T");
	test_expr_io_str("F");
	test_expr_io_str("a");
	test_expr_io_str("a&b");
	test_expr_io_str("a&b&c");
	test_expr_io_str("a|b");
	test_expr_io_str("a|b|c");
	test_expr_io_str("a|b&c");
	test_expr_io_str("a|b&c|d");
	test_expr_io_str("a&b|c");
	test_expr_io_str("a&b|c&d");
	test_expr_io_str("a&b|c&d");
	test_expr_io_str("(a|b)&c");
	test_expr_io_str("(a|T)&c");
	test_expr_io_str("(F|T)&c");
	test_expr_io_str("b");
	test_expr_io_str("--b");
	test_expr_io_str("-(b&c)");
	test_expr_io_str("-(b|c)");
	test_expr_io_str("-(b|c&d)");
	test_expr_io_str("(a|b)&(c|d)&(f|j|l)|a");
	test_expr_io_str("(a&b)&(c&d)&(f&j&l)&a");
	test_expr_io_str("-(a&b)&-(((c|d))&(f&j&l))&a");
}

/* Test making copy of expression.
 */
void test_expr_copy() {
	char *str = "-d&-(a|T)&c|z";
	struct Expr *e1 = read_expr(str);
	print_expr(e1);
	printf("\n");
	struct Expr *e2 = copy_expr(e1);
	print_expr(e2);
	printf("\n");
	if (equal_expr(e1, e2))
		printf("found equal expressions (OK)\n");
	if (equal_expr(e1, e2->expr1))
		printf("found equal expressions (NOT OK)\n");
	free_expr(e1);
	free_expr(e2);
}
