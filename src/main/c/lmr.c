#include "lmr.h"
#include "lmrlua.h"
#include <stdlib.h>

static const char* lmr_register_key = "process_key";

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
        if ((status = luaL_loadbuffer(L, buffer, size, "lmr_job")) != 0) {
            return status;
        }
        if ((status = lua_pcall(L, 0, 0, 0)) != 0) {
            return status;
        }
    }
    // Ensure that the executed job actually called `lmr:job()`.
    {
        lua_pushlightuserdata(L, (void*)lmr_register_key);
        lua_gettable(L, LUA_REGISTRYINDEX);
        const int type = lua_type(L, -1);
        if (type != LUA_TNUMBER || lua_tointeger(L, -1) != j.job_id) {
            return LMR_ERRNOCALL;
        }
    }
    return 0;
}

LMR_API int lmr_process(lua_State* L, const lmr_Batch in, lmr_Batch* out)
{
    return -1;
}

int lmr_l_job(lua_State* L) { return 0; }

int lmr_l_log(lua_State* L) { return 0; }
