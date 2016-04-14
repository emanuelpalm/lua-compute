#include "unit.h"

// Test suite function prototypes.
void suite_lmr(unit_T *T);

int main() {
    unit_State u;
    unit_init(&u);

    // Test suite invocations.
    unit_run_suite(&u, "lmr", suite_lmr);

    unit_exit(&u);
}
