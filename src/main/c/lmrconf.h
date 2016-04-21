/**
 * Lua Map/Reduce configuration header.
 *
 * @file
 */
#ifndef lmrconf_h
#define lmrconf_h

#include <lua.h>

/** Public API function linkage. */
#define LMR_API extern

#define LMR_ERRRUN LUA_ERRRUN ///< Lua runtime error.
#define LMR_ERRSYNTAX LUA_ERRSYNTAX ///< Lua syntax error.
#define LMR_ERRMEM LUA_ERRMEM ///< Memory allocation failure.
#define LMR_ERRERR LUA_ERRERR ///< Error in Lua error handler.
#define LMR_ERR (LUA_ERRERR + 100)
#define LMR_ERRINIT (LMR_ERR + 1) ///< Lua context not setup with LMR.
#define LMR_ERRNOCALL (LMR_ERR + 2) ///< `lmr:register()` never called.
#define LMR_ERRNORESULT (LMR_ERR + 3) ///< No result produced.

#endif
