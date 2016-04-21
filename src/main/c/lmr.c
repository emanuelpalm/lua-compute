#include "lmr.h"
#include "lmrlua.h"
#include <lauxlib.h>
#include <stdlib.h>

/**
 * LMR state object.
 *
 * Makes LMR context data available to LMR Lua/C functions.
 */
typedef struct {
    int32_t job_id;
    int32_t batch_id;
    lmr_ClosureLog closure_log;
} lmr_State;

LMR_API void lmr_openlib(lua_State* L, const lmr_Config* c)
{
    // Add global LMR state object.
    {
        lmr_State* state = lua_newuserdata(L, sizeof(lmr_State));
        state->batch_id = 0;
        state->job_id = 0;
        if (c != NULL) {
            state->closure_log = c->closure_log;
        }
        luaL_newmetatable(L, "LMR.state");

        // Add LMR Lua methods to state object.
        lua_newtable(L);
        {
            lua_pushcfunction(L, lmr_l_register);
            lua_setfield(L, -2, "register");

            lua_pushcfunction(L, lmr_l_log);
            lua_setfield(L, -2, "log");
        }
        lua_setfield(L, -2, "__index");

        // Create jobs table.
        lua_newtable(L);
        lua_setfield(L, -2, "jobs");

        lua_setmetatable(L, -2);
        lua_setglobal(L, "lmr");
    }
}

LMR_API int lmr_register(lua_State* L, const lmr_Job j)
{
    // Save job identifier to Lua registry.
    {
        lua_getglobal(L, "lmr");
        if (lua_type(L, -1) != LUA_TUSERDATA) {
            return LMR_ERRINIT;
        }
        lmr_State* state = luaL_checkudata(L, -1, "LMR.state");
        state->job_id = j.job_id;
        state->batch_id = 0;
        lua_pop(L, 1);
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
    // Ensure that the executed job actually called `lmr:job()` with a function
    // as argument.
    {
        lua_getglobal(L, "lmr");
        luaL_getmetafield(L, -1, "jobs");
        lua_pushinteger(L, j.job_id);
        lua_gettable(L, -2);
        const int status = lua_type(L, -1);
        lua_pop(L, 3);
        if (status != LUA_TFUNCTION) {
            return LMR_ERRNOCALL;
        }
    }
    return 0;
}

LMR_API int lmr_process(lua_State* L, const lmr_Batch b, lmr_ClosureBatch c)
{
    // Get and setup LMR context object.
    lmr_State* state;
    {
        lua_getglobal(L, "lmr");
        if (lua_type(L, -1) != LUA_TUSERDATA) {
            return LMR_ERRINIT;
        }
        state = luaL_checkudata(L, -1, "LMR.state");
        state->job_id = b.job_id;
        state->batch_id = b.batch_id;
    }
    // Get job function.
    {
        luaL_getmetafield(L, -1, "jobs");
        lua_pushinteger(L, b.job_id);
        lua_gettable(L, -2);
    }
    // Call job function.
    {
        lua_pushlstring(L, (char*)b.data.bytes, b.data.length);
        lua_pcall(L, 1, 1, 0);
    }
    int status;
    switch (lua_type(L, -1)) {
    case LUA_TNIL:
    case LUA_TNONE:
        status = LMR_ERRNORESULT;
        break;

    case LUA_TSTRING: {
        lmr_Batch result = {
            .job_id = b.job_id,
            .batch_id = b.batch_id,
        };
        result.data.bytes = (uint8_t*)lua_tolstring(L, -1, &result.data.length);
        c.function(c.context, &result);
        status = 0;
    } break;

    default:
        status = LMR_ERRRUN;
    }
    lua_pop(L, 3);
    if (status == LMR_ERRRUN) {
        lua_pushliteral(L, "Must return `string`.");
    }
    return status;
}

int lmr_l_register(lua_State* L)
{
    if (lua_type(L, 1) != LUA_TUSERDATA || lua_type(L, 2) != LUA_TFUNCTION) {
        luaL_error(L, "Bad arguments. (`LMR.state`, `function`) expected.");
    }
    // Load job identifier from registry.
    int32_t job_id;
    {
        const lmr_State* state = luaL_checkudata(L, 1, "LMR.state");
        job_id = state->job_id;
    }
    // Save job function to registry.
    {
        luaL_getmetafield(L, -2, "jobs");
        lua_pushinteger(L, job_id);
        lua_pushvalue(L, -3); // Copy function reference to top of stack.
        lua_settable(L, -3);
    }
    return 0;
}

int lmr_l_log(lua_State* L)
{
    const lmr_State* state = luaL_checkudata(L, 1, "LMR.state");
    size_t message_length;
    const char* message = luaL_checklstring(L, 2, &message_length);

    const lmr_ClosureLog c = state->closure_log;
    if (c.function != NULL) {
        c.function(
            c.context,
            &(lmr_LogEntry){
                .job_id = state->job_id,
                .batch_id = state->batch_id,
                .message = {.string = message, .length = message_length },
            });
    }
    return 0;
}
