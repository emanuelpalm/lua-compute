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
    // Create global LMR state object.
    {
        lmr_State* state = lua_newuserdata(L, sizeof(lmr_State));
        state->batch_id = 0;
        state->job_id = 0;
        if (c != NULL) {
            state->closure_log = c->closure_log;
        }
    }
    // Attach Lua meta table to state object.
    luaL_newmetatable(L, "LMR.state");
    {
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
    }
    lua_setmetatable(L, -2);

    // Bind state object to global Lua variable.
    lua_setglobal(L, "lmr");
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
    int status;
    {
        lua_pushlstring(L, (char*)b.data.bytes, b.data.length);
        status = lua_pcall(L, 1, 1, 0);
        if (status == 0 && lua_type(L, -1) != LUA_TSTRING) {
            lua_pushliteral(L, "Must return `string`.");
            status = LMR_ERRNORESULT;
        }
    }
    // Handle job results.
    if (status == 0) {
        lmr_Batch r = {.job_id = b.job_id, .batch_id = b.batch_id };
        r.data.bytes = (uint8_t*)lua_tolstring(L, -1, &r.data.length);
        c.function(c.context, &r);

    } else {
        // Make sure error message is at bottom of stack before popping.
        lua_insert(L, 1);
    }
    lua_pop(L, 3);
    return status;
}

LMR_API const char* lmr_errstr(const int err)
{
    switch (err) {
    case LMR_ERRRUN:
        return "LMR: Lua runtime error.";
    case LMR_ERRSYNTAX:
        return "LMR: Lua syntax error.";
    case LMR_ERRMEM:
        return "LMR: Memory allocation failure.";
    case LMR_ERRERR:
        return "LMR: Error in Lua error handler.";
    case LMR_ERRINIT:
        return "LMR: Lua context not setup with LMR.";
    case LMR_ERRNOCALL:
        return "LMR: `lmr:register()` never called.";
    case LMR_ERRNORESULT:
        return "LMR: No result produced.";
    default:
        return "?";
    }
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
