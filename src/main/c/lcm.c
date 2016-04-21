#include "lcm.h"
#include "lcmlua.h"
#include <lauxlib.h>
#include <stdlib.h>

/**
 * LCM state object.
 *
 * Makes LCM context data available to LCM Lua/C functions.
 */
typedef struct {
    int32_t job_id;
    int32_t batch_id;
    lcm_ClosureLog closure_log;
} lcm_State;

LCM_API void lcm_openlib(lua_State* L, const lcm_Config* c)
{
    // Create global LCM state object.
    {
        lcm_State* state = lua_newuserdata(L, sizeof(lcm_State));
        state->batch_id = 0;
        state->job_id = 0;
        if (c != NULL) {
            state->closure_log = c->closure_log;
        }
    }
    // Attach Lua meta table to state object.
    luaL_newmetatable(L, "LCM.state");
    {
        // Add LCM Lua methods to state object.
        lua_newtable(L);
        {
            lua_pushcfunction(L, lcm_l_register);
            lua_setfield(L, -2, "register");

            lua_pushcfunction(L, lcm_l_log);
            lua_setfield(L, -2, "log");
        }
        lua_setfield(L, -2, "__index");

        // Create jobs table.
        lua_newtable(L);
        lua_setfield(L, -2, "jobs");
    }
    lua_setmetatable(L, -2);

    // Bind state object to global Lua variable.
    lua_setglobal(L, "lcm");
}

LCM_API int lcm_register(lua_State* L, const lcm_Job j)
{
    // Save job identifier to Lua registry.
    {
        lua_getglobal(L, "lcm");
        if (lua_type(L, -1) != LUA_TUSERDATA) {
            return LCM_ERRINIT;
        }
        lcm_State* state = luaL_checkudata(L, -1, "LCM.state");
        state->job_id = j.job_id;
        state->batch_id = 0;
        lua_pop(L, 1);
    }
    // Load job into Lua state and execute it.
    {
        const char* buffer = j.program.lua;
        const size_t size = j.program.length;

        int status;
        if ((status = luaL_loadbuffer(L, buffer, size, "lcm_job")) != 0) {
            return status;
        }
        if ((status = lua_pcall(L, 0, 0, 0)) != 0) {
            return status;
        }
    }
    // Ensure that the executed job actually called `lcm:job()` with a function
    // as argument.
    {
        lua_getglobal(L, "lcm");
        luaL_getmetafield(L, -1, "jobs");
        lua_pushinteger(L, j.job_id);
        lua_gettable(L, -2);
        const int status = lua_type(L, -1);
        lua_pop(L, 3);
        if (status != LUA_TFUNCTION) {
            return LCM_ERRNOCALL;
        }
    }
    return 0;
}

LCM_API int lcm_process(lua_State* L, const lcm_Batch b, lcm_ClosureBatch c)
{
    // Get and setup LCM context object.
    lcm_State* state;
    {
        lua_getglobal(L, "lcm");
        if (lua_type(L, -1) != LUA_TUSERDATA) {
            return LCM_ERRINIT;
        }
        state = luaL_checkudata(L, -1, "LCM.state");
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
    int status;
    {
        lua_pushlstring(L, (char*)b.data.bytes, b.data.length);
        status = lua_pcall(L, 1, 1, 0);
        if (status == 0 && lua_type(L, -1) != LUA_TSTRING) {
            lua_pushliteral(L, "Must return `string`.");
            status = LCM_ERRNORESULT;
        }
    }
    // Handle job results.
    if (status == 0) {
        lcm_Batch r = {.job_id = b.job_id, .batch_id = b.batch_id };
        r.data.bytes = (uint8_t*)lua_tolstring(L, -1, &r.data.length);
        c.function(c.context, &r);

    } else {
        // Make sure error message is at bottom of stack before popping.
        lua_insert(L, 1);
    }
    lua_pop(L, 3);
    return status;
}

LCM_API const char* lcm_errstr(const int err)
{
    switch (err) {
    case LCM_ERRRUN:
        return "LCM: Lua runtime error.";
    case LCM_ERRSYNTAX:
        return "LCM: Lua syntax error.";
    case LCM_ERRMEM:
        return "LCM: Memory allocation failure.";
    case LCM_ERRERR:
        return "LCM: Error in Lua error handler.";
    case LCM_ERRINIT:
        return "LCM: Lua context not setup with LCM.";
    case LCM_ERRNOCALL:
        return "LCM: `lcm:register()` never called.";
    case LCM_ERRNORESULT:
        return "LCM: No result produced.";
    default:
        return "?";
    }
}

int lcm_l_register(lua_State* L)
{
    if (lua_type(L, 1) != LUA_TUSERDATA || lua_type(L, 2) != LUA_TFUNCTION) {
        luaL_error(L, "Bad arguments. (`LCM.state`, `function`) expected.");
    }
    // Load job identifier from registry.
    int32_t job_id;
    {
        const lcm_State* state = luaL_checkudata(L, 1, "LCM.state");
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

int lcm_l_log(lua_State* L)
{
    const lcm_State* state = luaL_checkudata(L, 1, "LCM.state");
    size_t message_length;
    const char* message = luaL_checklstring(L, 2, &message_length);

    const lcm_ClosureLog c = state->closure_log;
    if (c.function != NULL) {
        c.function(
            c.context,
            &(lcm_LogEntry){
                .job_id = state->job_id,
                .batch_id = state->batch_id,
                .message = {.string = message, .length = message_length },
            });
    }
    return 0;
}
