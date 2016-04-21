/**
 * Lua Map/Reduce main header.
 *
 * Its important to note that the library provides no thread safety guarantees
 * whatsoever. If wishing to use the library in a multi-threaded setting, make
 * sure that no more than one thread access each used Lua context.
 *
 * @file
 */
#ifndef lmr_h
#define lmr_h

#include "lmrconf.h"
#include <stdint.h>

typedef struct lmr_Config lmr_Config;
typedef struct lmr_Job lmr_Job;
typedef struct lmr_Batch lmr_Batch;
typedef struct lmr_LogEntry lmr_LogEntry;

/**
 * Function used to receive `lmr:log()` calls.
 *
 * Provided `entry` is only guaranteed to point to valid memory during the
 * invocation of the function.
 */
typedef void (*lmr_FunctionLog)(void* context, const lmr_LogEntry* entry);

/**
 * Function used to receive batch processing results.
 *
 * Provided `result` is only guaranteed to point to valid memory during the
 * invocation of the function.
 */
typedef void (*lmr_FunctionBatch)(void* context, const lmr_Batch* result);

/**
 * Closure holding some arbitrary context pointer and a function for
 * `lmr:log()` calls.
 *
 * When `function` is called, the `context` should be provided as argument.
 */
typedef struct lmr_ClosureLog {
    void* context;
    lmr_FunctionLog function;
} lmr_ClosureLog;

/**
 * Closure holding some arbitrary context pointer and a function for receiving
 * batch processing results.
 *
 * When `function` is called, the `context` should be provided as argument.
 */
typedef struct lmr_ClosureBatch {
    void* context;
    lmr_FunctionBatch function;
} lmr_ClosureBatch;

/**
 * LMR Lua library configuration.
 */
struct lmr_Config {
    /// Log closure used when forwarding `lmr:log()` calls. May be NULL.
    lmr_ClosureLog closure_log;
};

/**
 * Job definition, containing a job ID and a lua program able to process data
 * batches.
 *
 * A job is essentially an arbitrary Lua program that is required to call the
 * function `lmr:job(callback)` with a provided callback, which is subsequently
 * called whenever the job receives a new batch of data to process.
 */
struct lmr_Job {
    int32_t job_id;
    struct {
        char* lua;
        size_t length;
    } program;
};

/**
 * A job batch, containing a job ID, a batch ID, and arbitrary data.
 *
 * Batches are fed as input into jobs, and are also received as output when
 * jobs complete.
 */
struct lmr_Batch {
    int32_t job_id, batch_id;
    struct {
        uint8_t* bytes;
        size_t length;
    } data;
};

/**
 * LMR log entry.
 *
 * Provided when receiving Lua `lmr:log()` calls.
 */
struct lmr_LogEntry {
    int32_t job_id;
    int32_t batch_id;
    struct {
        const char* string;
        size_t length;
    } message;
};

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
LMR_API int lmr_process(lua_State* L, const lmr_Batch b, lmr_ClosureBatch c);

/** Returns string representation of provided LMR error code. */
LMR_API const char* lmr_errstr(const int err);

#endif
