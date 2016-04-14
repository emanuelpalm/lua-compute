/*!
** Lua Map/Reduce Lua API header.
**
** This file exists primarilt for documentation purposes. It lists and
** documents all the functions provided to Lua states via the `lmr_openlib()`
** function.
**
** @file
*/
#ifndef lmrlua_h
#define lmrlua_h

/**
** Registers new job callback.
**
** The provided callback must accept one argument, being the batch to process
** as a Lua string. Typically, the first thing the callback would do is to
** decode the contents of the string.
**
** This function must be called exactly once by each job being registered in an
** LMR Lua state.
**
** @function job
** @param lmr LMR context reference.
** @param callback Function called with job batches.
*/
int lmr_l_job(lua_State *L);

/**
** Reports job execution results.
**
** Typically, this function would be called when a job has finished processing
** some batch of data, with the results encoded as a Lua string.
**
** This function must be called no more than once by each registered job
** callback.
**
** @function report
** @param lmr LMR context reference.
** @param data A Lua string, or a number that is converted to a string.
*/
int lmr_l_report(lua_State *L);

/**
** Logs arbitrary string.
**
** This function may be called any amount of times to inform whoever provided
** some job batch about any occurrence of interest.
**
** @function log
** @param lmr LMR context reference.
** @param message A Lua string, or a number that is converted to a string.
*/
int lmr_l_log(lua_State *L);

#endif

