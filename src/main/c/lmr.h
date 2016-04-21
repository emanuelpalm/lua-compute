/**
 * Lua Map/Reduce main header.
 *
 * Its important to note that all of the methods provided have no thread
 * safety guarantees whatsoever. It is expected that this API is used
 * exclusively by a single thread at all times.
 *
 * @file
 */
#ifndef lmr_h
#define lmr_h

#include "lmrconf.h"
#include <stdint.h>

/**
 * LMR log entry.
 *
 * Provided when receiving Lua `lmr:log()` calls.
 */
typedef struct {
    int32_t job_id;
    int32_t batch_id;
    struct {
        const char* string;
        size_t length;
    } message;
} lmr_LogEntry;

/**
 * Function used to receive `lmr:log()` calls.
 *
 * Provided log entries are destroyed right after a log function returns.
 */
typedef void (*lmr_LogFunction)(void* context, const lmr_LogEntry* entry);

/**
 * Closure holding some arbitrary context pointer and a function for
 * `lmr:log()` calls.
 *
 * When `function` is called, the `context` should be provided as argument.
 */
typedef struct {
    void* context;
    lmr_LogFunction function;
} lmr_LogClosure;

/**
 * LMR Lua library configuration.
 *
 * Provided when initializing an LMR Lua context using `lmr_openlib()` in order
 * to configure LMR behavior.
 */
typedef struct {
    /// Log closure used when forwarding `lmr:log()` calls. May be NULL.
    lmr_LogClosure closure_log;
} lmr_Config;

/**
 * Job definition, containing a job ID and a lua program able to process data
 * batches.
 *
 * A job is essentially an arbitrary Lua program that is required to call the
 * function `lmr:job(callback)` with a provided callback, which is subsequently
 * called whenever the job receives a new batch of data to process.
 */
typedef struct {
    int32_t job_id;
    struct {
        char* lua;
        size_t length;
    } program;
} lmr_Job;

/**
 * A job batch, containing a job ID, a batch ID, and arbitrary data.
 *
 * Batches are fed as input into jobs, and are also received as output when
 * jobs complete.
 */
typedef struct {
    int32_t job_id, batch_id;
    struct {
        uint8_t* bytes;
        size_t length;
    } data;
} lmr_Batch;

/**
 * Function used to receive batch processing results.
 *
 * Provided `result` is only guaranteed to point to valid memory during the
 * invocation of this function.
 */
typedef void (*lmr_ResultFunction)(void* context, const lmr_Batch* result);

/**
 * Closure holding some arbitrary context pointer and a function for receiving
 * batch processing results.
 *
 * When `function` is called, the `context` should be provided as argument.
 */
typedef struct {
    void* context;
    lmr_ResultFunction function;
} lmr_ResultClosure;

/**
 * Adds LMR library functions to provided lua state, with their behavior
 * customized using provided configuration, if given.
 */
LMR_API void lmr_openlib(lua_State* L, const lmr_Config* c);

/**
 * Registers provided job in Lua state.
 *
 * The job is safe to destroy at any point after the function returns.
 *
 * Returns `0` (OK), `LMR_ERRRUN`, `LMR_ERRSYNTAX`, `LMR_ERRMEM`, `LMR_ERRERR`,
 * `LMR_ERRINIT`, or `LMR_ERRNOCALL`. The last is returned only if the provided
 * job fails to call `lmr:job()` when evaluated.
 */
LMR_API int lmr_register(lua_State* L, const lmr_Job j);

/**
 * Processes, using referenced Lua state, batch `b` and provides any results to
 * the function in closure `c`.
 *
 * The batch provided to `c` is destroyed after the closure function returns.
 *
 * Returns `0` (OK), `LMR_ERRRUN`, `LMR_ERRMEM`, `LMR_ERRERR`, `LMR_ERRINIT` or
 * `LMR_ERRNORESULT`. The last is returned only if the job processing the batch
 * fails to return a batch result, in which case `c` is never called.
 */
LMR_API int lmr_process(lua_State* L, const lmr_Batch b, lmr_ResultClosure c);

#endif
