/*!
** Lua Map/Reduce main header.
**
** @file
*/
#ifndef lmr_h
#define lmr_h

#include "lmrconf.h"
#include <lua.h>

/*! Adds LMR library functions to provided lua state. */
LMR_API void lmr_openlib(lua_State *L);

/*!
** Gets serialized results, available at the top of the result stack, from
** previous calls to the Lua functions `lmr:map()` and `lmr:reduce()`.
**
** This operation does not modify the result stack. Use `lmr_popresults()` to
** remove unwanted result entries.
** 
** The returned pointer points to dynamic memory and should be `free()`d once
** no longer needed. Returns `NULL` only in case of memory allocation failure.
*/
LMR_API char *lmr_getresults(lua_State *L);

/*! Removes any entry at the top of the result stack. */
LMR_API void lmr_popresults(lua_State *L);

#endif
