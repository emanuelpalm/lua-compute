#include "lmr.h"
#include "lmrlua.h"
#include <stdlib.h>

static const char* KEY_JOB_ID = "lmr_job_id";

LMR_API void lmr_openlib(lua_State* L, const lmr_Config* c)
{
    static const luaL_Reg functions[] = {
        {.name = "register", .func = lmr_l_register },
        {.name = "log", .func = lmr_l_log },
        {.name = NULL, .func = NULL },
    };
    luaL_register(L, "lmr", functions);
}

LMR_API int lmr_register(lua_State* L, const lmr_Job j)
{
    // Save job identifier to Lua registry.
    {
        lua_pushlightuserdata(L, (void*)KEY_JOB_ID);
        lua_pushinteger(L, j.job_id);
        lua_settable(L, LUA_REGISTRYINDEX);
    }
    // Load job into Lua state and execute it.
    {
        const char* buffer = j.program.lua;
        const size_t size = j.program.length;

        int status;
        if ((status = luaL_loadbuffer(L, buffer, size, "lmr_job")) != 0) {
            return status;
        }
        if ((status = lua_pcall(L, 0, 0, 0)) != 0) {
            return status;
        }
    }
    // Ensure that the executed job actually called `lmr.job()` with a function
    // as argument.
    {
        lua_pushfstring(L, "lmr_job_%d", j.job_id);
        lua_gettable(L, LUA_REGISTRYINDEX);
        if (lua_type(L, -1) != LUA_TFUNCTION) {
            return LMR_ERRNOCALL;
        }
    }
    return 0;
}

LMR_API int lmr_process(lua_State* L, const lmr_Batch in, lmr_Batch* out)
{
    return -1;
}

int lmr_l_register(lua_State* L)
{
    if (lua_type(L, 1) != LUA_TFUNCTION) {
        lua_pushstring(L, "Bad job arguments. Must be [function].");
        lua_error(L);
    }
    // Load job identifier from registry.
    lua_Integer job_id;
    {
        lua_pushlightuserdata(L, (void*)KEY_JOB_ID);
        lua_gettable(L, LUA_REGISTRYINDEX);
        job_id = lua_tointeger(L, -1);
        lua_pop(L, 1);
    }
    // Save job function to registry.
    {
        lua_pushfstring(L, "lmr_job_%d", job_id);
        lua_insert(L, 1);
        lua_settable(L, LUA_REGISTRYINDEX);
    }
    return 0;
}

int lmr_l_log(lua_State* L) { return 0; }
