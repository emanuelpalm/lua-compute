/*!
** Lua Map/Reduce main header.
**
** @file
*/
#ifndef lmr_h
#define lmr_h

#include "lmrconf.h"
#include <stdint.h>

/*!
** A function used to receive `lmr:log()` calls.
**
** Called with `job_id`, `batch_id`, and the logged string.
*/
typedef void (*lmr_LogFunction)(const uint32_t, const uint32_t, const char*);

/*!
** LMR Lua library configuration.
**
** Provided when initializing an LMR Lua context using `lmr_openlib()` in order
** to configure LMR behavior.
*/
typedef struct {
    //! Log function used when forwarding `lmr:log()` calls. Must not be NULL.
    lmr_LogFunction log_function;
} lmr_Config;

/*!
** A job definition, containing a job ID and a lua program able to process data
** batches.
**
** A job is essentially an arbitrary Lua program that is required to call the
** function `lmr:job(callback)` with a provided callback, which is subsequently
** called whenever the job receives a new batch of data to process.
**
** Instances of `lmr_Job` must to be destroyed using `lmr_freejob()` once no
** longer used.
*/
typedef struct {
    uint32_t job_id;
    uint8_t *prog_lua;
} lmr_Job;

/*!
** A job batch, containing a job ID, a batch ID, and arbitrary data.
**
** Batches are fed as input into jobs, and are also received as output when
** jobs complete.
**
** Instances of `lmr_Batch` must be destroyed using `lmr_freebatch()` once no
** longer used.
*/
typedef struct {
    uint32_t job_id, batch_id;
    uint8_t *data;
} lmr_Batch;

/*! Adds LMR library functions to provided lua state. */
LMR_API void lmr_openlib(lua_State *L, const lmr_Config *c);

/*!
** Registers provided job in Lua state.
**
** The job is safe to destroy at any point after the function returns.
**
** Returns 0 if registration was successful.
*/
LMR_API int lmr_register(lua_State *L, const lmr_Job j);

/*!
** Processes, using referenced Lua state, in-batch and writes any results to
** out-batch.
**
** Both batches are safe to destroy at any point after the function returns.
*/
LMR_API int lmr_process(lua_State *L, const lmr_Batch in, lmr_Batch *out);

/*! Destroys provided job, freeing any resources held. */
LMR_API void lmr_freejob(lmr_Job j);

/*! Destroys provided batch, freeing any resources held. */
LMR_API void lmr_freebatch(lmr_Batch b);

#endif
