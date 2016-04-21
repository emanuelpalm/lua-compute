#include "unit.h"

// Test suite function prototypes.
void suite_lcm(unit_T* T);

int main()
{
    unit_State u;
    unit_init(&u);

    // Test suite invocations.
    unit_run_suite(&u, "lcm", suite_lcm);

    unit_exit(&u);
}
