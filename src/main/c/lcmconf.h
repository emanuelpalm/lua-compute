/**
 * Lua/compute configuration header.
 *
 * @file
 */
#ifndef lcmconf_h
#define lcmconf_h

#include "lua.h"

/** Public API function linkage. */
#define LCM_API extern

///{ Error codes. Use `lcm_strerr() to turn into strings.`
#define LCM_ERRRUN LUA_ERRRUN ///< Lua runtime error.
#define LCM_ERRSYNTAX LUA_ERRSYNTAX ///< Lua syntax error.
#define LCM_ERRMEM LUA_ERRMEM ///< Memory allocation failure.
#define LCM_ERRERR LUA_ERRERR ///< Error in Lua error handler.
#define LCM_ERR (LUA_ERRERR + 100)
#define LCM_ERRINIT (LCM_ERR + 1) ///< Lua context not setup with LCM.
#define LCM_ERRNOCALL (LCM_ERR + 2) ///< `lcm:register()` never called.
#define LCM_ERRNOLAMBDA (LCM_ERR + 3) //< Required lambda not available.
#define LCM_ERRNORESULT (LCM_ERR + 4) ///< No result produced.
///}

#endif
