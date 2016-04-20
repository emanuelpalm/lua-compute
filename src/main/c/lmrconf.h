/**
 * Lua Map/Reduce configuration header.
 *
 * @file
 */
#ifndef lmrconf_h
#define lmrconf_h

#include <lauxlib.h>
#include <lua.h>

/** Public API function linkage. */
#define LMR_API extern

#define LMR_ERRUN LUA_ERRRUN ///< Lua runtime error.
#define LMR ERRSYNTAX LUA_ERRSYNTAX ///< Lua syntax error.
#define LMR_ERRMEM LUA_ERRMEM ///< Memory allocation failure.
#define LMR_ERRERR LUA_ERRERR ///< Runtime error in error handler.
#define LMR_ERRFILE LUA_ERRFILE ///< File operation failure.
#define LMR_ERR (LUA_ERRFILE + 100)
#define LMR_ERRCONFIG (LMR_ERR + 1) ///< Invalid configuration.
#define LMR_ERRNOCALL (LMR_ERR + 1) ///< Mandatory function call not made.
#define LMR_ERRREPORT (LMR_ERR + 2) ///< Failed to generate report.

#endif
