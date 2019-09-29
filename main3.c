#include "simplify.h"

int main(void) {
	int max_depth = 7;
	find_derivations_for_strings(max_depth,
			cnf_law_searches, cnf_law_applies, cnf_law_names, n_cnf_laws());
}
