#include <stdio.h>
#include <stdbool.h>

#include "logic.h"
#include "test_logic.h"
#include "laws.h"
#include "test_laws.h"

/* Run a number of tests.
 */
int main(void) {
	// logic
	test_expr_io();
	test_expr_copy();
	// laws
	test_search();
	test_apply();
}
