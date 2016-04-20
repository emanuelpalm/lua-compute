#include "lmr.h"
#include "lmrlua.h"
#include <stdlib.h>

LMR_API void lmr_openlib(lua_State* L, const lmr_Config* c)
{
    static const luaL_Reg functions[] = {
        {.name = "job", .func = lmr_l_job },
        {.name = "log", .func = lmr_l_log },
        {.name = NULL, .func = NULL },
    };
    luaL_register(L, "lmr", functions);
}

LMR_API int lmr_register(lua_State* L, const lmr_Job j)
{
    // Load job into Lua state and execute it.
    {
        const char* buffer = j.program.lua;
        const size_t size = j.program.length;
        int status;

        status = luaL_loadbuffer(L, buffer, size, "lmr_job");
        if (status != 0) {
            return status;
        }

        lua_pushinteger(L, j.job_id);
        lua_setglobal(L, "lmr");

        status = lua_pcall(L, 0, 0, 0);

        lua_pushnil(L);
        lua_setglobal(L, "lmr");

        if (status != 0) {
            return status;
        }
    }
    // Ensure that the executed job actually called `lmr:job()`.
    {
        lua_pushfstring(L, "lmr_job_%u", j.job_id);
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

int lmr_l_job(lua_State* L)
{
    if (lua_type(L, 1) != LUA_TNUMBER || lua_type(L, 1) != LUA_TFUNCTION) {
        lua_pushstring(L, "Bad job arguments. Must be [LMR, function].");
        lua_error(L);
    }
    // Save job function, which at this point is at stack index 2, to registry.
    {
        lua_pushfstring(L, "lmr_job_%d", lua_tointeger(L, 1));
        lua_replace(L, 1);
        lua_settable(L, LUA_REGISTRYINDEX);
    }
    return 0;
}

int lmr_l_log(lua_State* L) { return 0; }
