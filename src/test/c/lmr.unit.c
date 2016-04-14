#include "../../main/c/lmr.h"
#include "unit.h"

// Test cases.
void test_dummy(unit_T *T, void *arg);

void suite_lmr(unit_T *T) {
    unit_run_test(T, &test_dummy, NULL);
}

void test_dummy(unit_T *T, void *arg) {
    // Do nothing.
}
