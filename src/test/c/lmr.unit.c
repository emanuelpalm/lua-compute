#include "../../main/c/lmr.h"
#include "unit.h"
#include <lauxlib.h>
#include <lua.h>

void provider_lua_state(unit_T* T, unit_TestFunction t)
{
    lua_State* L = luaL_newstate();
    if (L == NULL) {
        unit_fatal(T, "Failed to create new Lua state object.");
    }
    t(T, L);
    lua_close(L);
}

// Test cases.
void test_dummy(unit_T* T, void* arg);

void suite_lmr(unit_T* T)
{
    unit_run_test(T, &test_dummy, provider_lua_state);
}

void test_dummy(unit_T* T, void* arg)
{
    lua_State* L = arg;
    // Do nothing.
}
