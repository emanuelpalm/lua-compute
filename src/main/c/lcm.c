#include "lcm.h"
#include "lcmlua.h"
#include "lauxlib.h"
#include <stdlib.h>

#define LCM_STATE_METAFIELD_LAMBDAS "lambdas"
#define LCM_STATE_METATYPE "LCM.state"
#define LCM_STATE_NAME "lcm"

/**
 * LCM state object.
 *
 * Makes LCM context data available to LCM Lua/C functions.
 */
typedef struct {
    int32_t lambda_id;
    int32_t batch_id;
    lcm_ClosureLog closure_log;
} lcm_State;

LCM_API void lcm_openlib(lua_State* L, const lcm_Config* c)
{
    // Create global LCM state object.
    {
        lcm_State* state = lua_newuserdata(L, sizeof(lcm_State));
        state->batch_id = 0;
        state->lambda_id = 0;
        if (c != NULL) {
            state->closure_log = c->closure_log;
        }
    }
    // Attach Lua meta table to state object.
    luaL_newmetatable(L, LCM_STATE_METATYPE);
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

        // Create lambdas table.
        lua_newtable(L);
        lua_setfield(L, -2, LCM_STATE_METAFIELD_LAMBDAS);
    }
    lua_setmetatable(L, -2);

    // Bind state object to global Lua variable.
    lua_setglobal(L, LCM_STATE_NAME);
}

LCM_API int lcm_register(lua_State* L, const lcm_Lambda l)
{
    const int bottom = lua_gettop(L);
    int status = 0;

    // Save job identifier to Lua registry.
    {
        lua_getglobal(L, LCM_STATE_NAME);
        if (lua_type(L, -1) != LUA_TUSERDATA) {
            status = LCM_ERRINIT;
            goto end;
        }
        lcm_State* state = luaL_checkudata(L, -1, LCM_STATE_METATYPE);
        state->lambda_id = l.lambda_id;
        state->batch_id = 0;
    }
    // Load job into Lua state and execute it.
    {
        const char* buffer = l.program.lua;
        const size_t size = l.program.length;
        if ((status = luaL_loadbuffer(L, buffer, size, "lambda")) != 0) {
            goto end;
        }
        if ((status = lua_pcall(L, 0, 0, 0)) != 0) {
            goto end;
        }
    }
    // Ensure that the executed job actually called `lcm:job()` with a function
    // as argument.
    {
        luaL_getmetafield(L, bottom + 1, LCM_STATE_METAFIELD_LAMBDAS);
        lua_pushinteger(L, l.lambda_id);
        lua_gettable(L, -2);
        if (lua_type(L, -1) != LUA_TFUNCTION) {
            status = LCM_ERRNOCALL;
            goto end;
        }
    }

end:
    lua_settop(L, bottom);
    return status;
}

LCM_API int lcm_process(lua_State* L, const lcm_Batch b, lcm_ClosureBatch c)
{
    const int bottom = lua_gettop(L);
    int status = 0;

    // Get and setup LCM context object.
    lcm_State* state;
    {
        lua_getglobal(L, LCM_STATE_NAME);
        if (lua_type(L, -1) != LUA_TUSERDATA) {
            status = LCM_ERRINIT;
            goto end;
        }
        state = luaL_checkudata(L, -1, LCM_STATE_METATYPE);
        state->lambda_id = b.lambda_id;
        state->batch_id = b.batch_id;
    }
    // Get job function.
    {
        luaL_getmetafield(L, -1, LCM_STATE_METAFIELD_LAMBDAS);
        lua_pushinteger(L, b.lambda_id);
        lua_gettable(L, -2);
        if (lua_type(L, -1) != LUA_TFUNCTION) {
            status = LCM_ERRNOLAMBDA;
            goto end;
        }
    }
    // Call job function.
    {
        lua_pushlstring(L, (char*)b.data.bytes, b.data.length);
        if ((status = lua_pcall(L, 1, 1, 0)) != 0) {
            goto end;
        }
        if (lua_type(L, -1) != LUA_TSTRING) {
            status = LCM_ERRNORESULT;
            goto end;
        }
    }
    // Handle job results.
    lcm_Batch r = {.lambda_id = b.lambda_id, .batch_id = b.batch_id };
    r.data.bytes = (uint8_t*)lua_tolstring(L, -1, &r.data.length);
    c.function(c.context, &r);

end:
    lua_settop(L, bottom);
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
    case LCM_ERRNOLAMBDA:
        return "LCM: Required lambda not available.";
    case LCM_ERRNORESULT:
        return "LCM: No result produced.";
    default:
        return "LCM: ?";
    }
}

int lcm_l_register(lua_State* L)
{
    if (lua_type(L, 1) != LUA_TUSERDATA || lua_type(L, 2) != LUA_TFUNCTION) {
        luaL_error(L, "Expected arguments [`LCM.state`, `function`]");
    }
    // Load job identifier from registry.
    int32_t lambda_id;
    {
        const lcm_State* state = luaL_checkudata(L, 1, LCM_STATE_METATYPE);
        lambda_id = state->lambda_id;
    }
    // Save job function to registry.
    {
        luaL_getmetafield(L, -2, LCM_STATE_METAFIELD_LAMBDAS);
        lua_pushinteger(L, lambda_id);
        lua_pushvalue(L, -3); // Copy function reference to top of stack.
        lua_settable(L, -3);
    }
    return 0;
}

int lcm_l_log(lua_State* L)
{
    const lcm_State* state = luaL_checkudata(L, 1, LCM_STATE_METATYPE);
    size_t message_length;
    const char* message = luaL_checklstring(L, 2, &message_length);

    const lcm_ClosureLog c = state->closure_log;
    if (c.function != NULL) {
        c.function(
            c.context,
            &(lcm_LogEntry){
                .lambda_id = state->lambda_id,
                .batch_id = state->batch_id,
                .message = {.string = message, .length = message_length },
            });
    }
    return 0;
}
