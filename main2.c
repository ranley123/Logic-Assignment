#include "simplify.h"

int main(void) {
	int max_depth = 6;
	find_derivations_for_strings(max_depth,
			extra_law_searches, extra_law_applies, extra_law_names, n_extra_laws());
}
