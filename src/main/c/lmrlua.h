/**
 * Lua Map/Reduce Lua API header.
 *
 * This file exists primarily for documentation purposes. It lists and
 * documents all the functions provided to Lua states via the `lmr_openlib()`
 * function.
 *
 * @file
 */
#ifndef lmrlua_h
#define lmrlua_h

#include <lua.h>

/**
 * Registers new job callback.
 *
 * The provided callback must accept one argument, being the batch to process
 * as a Lua string. Typically, the first thing the callback would do is to
 * decode the contents of the string.
 *
 * This function must be called exactly once by each job being registered in an
 * LMR Lua state.
 *
 * @function register
 * @param lmr LMR context reference.
 * @param callback Function called with job batches.
 */
int lmr_l_register(lua_State* L);

/**
 * Logs arbitrary string.
 *
 * This function may be called any amount of times to inform whoever provided
 * some job batch about any occurrence of interest.
 *
 * @function log
 * @param lmr LMR context reference.
 * @param message A Lua string, or a number that is converted to a string.
 */
int lmr_l_log(lua_State* L);

#endif
