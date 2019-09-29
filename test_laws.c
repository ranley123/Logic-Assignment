#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "logic.h"
#include "laws.h"
#include "test_laws.h"

/* In expression in string 'str', test finding all paths of occurrences of rewrite 
 * rule with number 'law'.
 */
static void test_search_of(char *str, int law) {
	printf("%s\n  Law: %s\n", str, law_names[law]);
	struct Expr *expr = read_expr(str);
	int *path = non_path();
	while (path != NULL) {
		int *next_path = law_searches[law](expr, path);
		free(path);
		path = next_path;
		if (path != NULL) {
			printf("    found at: ");
			print_path(path);
		}
	}
}

void test_search() {
	test_search_of("a|b|c", 0);
	test_search_of("a|(b|c)", 0);
	test_search_of("a|b|(c|d)", 0);
	test_search_of("a&b|c&d|e&f", 0);
	test_search_of("a|(b|c)", 0);
	test_search_of("d&(a|(b|c))", 0);
	test_search_of("(a|((b|c)|(d|e)))&(f&g|h&i)", 0);
	test_search_of("a&b&c", 1);
	test_search_of("a&b&c", 4);
	test_search_of("a&b&c", 5);
	test_search_of("a&(a|b&c)", 7);
	test_search_of("a|(a&d|b)&(a&d|c)", 7);
	test_search_of("a|(a&d|b)&(a&e|c)", 7);
	test_search_of("a|(a&d|b)&(a&d|c)", 8);
	test_search_of("a&b|-(a&b)", 12);
	test_search_of("a&b|-(a&c)", 12);
}

/* In expression in string 'str', test finding all paths of occurrences of rewrite 
 * rule with number 'law', and apply the rewrite rule.
 */
static void test_apply_of(char *str, int law) {
	printf("%s\n  Law: %s\n", str, law_names[law]);
	struct Expr *expr = read_expr(str);
	int *path = non_path();
	while (path != NULL) {
		int *next_path = law_searches[law](expr, path);
		free(path);
		path = next_path;
		if (path != NULL) {
			printf("    found at: ");
			print_path(path);
			struct Expr *new_expr = law_applies[law](expr, path);
			printf("    ");
			print_expr(new_expr);
			printf("\n");
			free_expr(new_expr);
		}
	}
}

void test_apply() {
	test_search_of("a|(a&d|b)&(a&d|c)", 7);
	test_apply_of("a|b|c", 0);
	test_apply_of("a&b|-(a&b)", 12);
}
