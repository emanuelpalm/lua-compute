/**
 * Lua/compute main header.
 *
 * Its important to note that the library provides no thread safety guarantees
 * whatsoever. If wishing to use the library in a multi-threaded setting, make
 * sure that no more than one thread access each used Lua context.
 *
 * @file
 */
#ifndef lcm_h
#define lcm_h

#include "lcmconf.h"
#include <stdint.h>

typedef struct lcm_Config lcm_Config;
typedef struct lcm_Job lcm_Job;
typedef struct lcm_Batch lcm_Batch;
typedef struct lcm_LogEntry lcm_LogEntry;

/**
 * Function used to receive `lcm:log()` calls.
 *
 * Provided `entry` is only guaranteed to point to valid memory during the
 * invocation of the function.
 */
typedef void (*lcm_FunctionLog)(void* context, const lcm_LogEntry* entry);

/**
 * Function used to receive batch processing results.
 *
 * Provided `result` is only guaranteed to point to valid memory during the
 * invocation of the function.
 */
typedef void (*lcm_FunctionBatch)(void* context, const lcm_Batch* result);

/**
 * Closure holding some arbitrary context pointer and a function for
 * `lcm:log()` calls.
 *
 * When `function` is called, the `context` should be provided as argument.
 */
typedef struct lcm_ClosureLog {
    void* context;
    lcm_FunctionLog function;
} lcm_ClosureLog;

/**
 * Closure holding some arbitrary context pointer and a function for receiving
 * batch processing results.
 *
 * When `function` is called, the `context` should be provided as argument.
 */
typedef struct lcm_ClosureBatch {
    void* context;
    lcm_FunctionBatch function;
} lcm_ClosureBatch;

/**
 * LCM Lua library configuration.
 */
struct lcm_Config {
    /// Log closure used when forwarding `lcm:log()` calls. May be NULL.
    lcm_ClosureLog closure_log;
};

/**
 * Job definition, containing a job ID and a lua program able to process data
 * batches.
 *
 * A job is essentially an arbitrary Lua program that is required to call the
 * function `lcm:job(callback)` with a provided callback, which is subsequently
 * called whenever the job receives a new batch of data to process.
 */
struct lcm_Job {
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
struct lcm_Batch {
    int32_t job_id, batch_id;
    struct {
        uint8_t* bytes;
        size_t length;
    } data;
};

/**
 * LCM log entry.
 *
 * Provided when receiving Lua `lcm:log()` calls.
 */
struct lcm_LogEntry {
    int32_t job_id;
    int32_t batch_id;
    struct {
        const char* string;
        size_t length;
    } message;
};

/**
 * Adds LCM library functions to provided lua state, with their behavior
 * customized using provided configuration, if given.
 */
LCM_API void lcm_openlib(lua_State* L, const lcm_Config* c);

/**
 * Registers provided job in Lua state.
 *
 * The job is safe to destroy at any point after the function returns.
 *
 * Returns `0` (OK), `LCM_ERRRUN`, `LCM_ERRSYNTAX`, `LCM_ERRMEM`, `LCM_ERRERR`,
 * `LCM_ERRINIT`, or `LCM_ERRNOCALL`. The last is returned only if the provided
 * job fails to call `lcm:job()` when evaluated.
 */
LCM_API int lcm_register(lua_State* L, const lcm_Job j);

/**
 * Processes, using referenced Lua state, batch `b` and provides any results to
 * the function in closure `c`.
 *
 * The batch provided to `c` is destroyed after the closure function returns.
 *
 * Returns `0` (OK), `LCM_ERRRUN`, `LCM_ERRMEM`, `LCM_ERRERR`, `LCM_ERRINIT` or
 * `LCM_ERRNORESULT`. The last is returned only if the job processing the batch
 * fails to return a batch result, in which case `c` is never called.
 */
LCM_API int lcm_process(lua_State* L, const lcm_Batch b, lcm_ClosureBatch c);

/** Returns string representation of provided LCM error code. */
LCM_API const char* lcm_errstr(const int err);

#endif
