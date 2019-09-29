#include "simplify.h"

int main(void) {
	int max_depth = 6;
	find_derivations_for_strings(max_depth,
			law_searches, law_applies, law_names, n_laws());
	return 0;
}
