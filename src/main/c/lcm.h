/**
 * Lua/compute main header.
 *
 * Its important to note that the library provides no thread safety guarantees
 * whatsoever. If wishing to use the library in a multi-threaded setting, make
 * sure that no more than one thread accesses each used Lua context.
 *
 * @file
 */
#ifndef lcm_h
#define lcm_h

#include "lcmconf.h"
#include <stdint.h>

typedef struct lcm_Config lcm_Config;
typedef struct lcm_Lambda lcm_Lambda;
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
 * Lambda definition, containing a lambda ID and a lua program able to process
 * data batches.
 *
 * The lua program is required to call `lcm:register(lambda)` with a lambda
 * function. The registered function is subsequently called whenever a data
 * batch is processed with a matching lambda identifier.
 */
struct lcm_Lambda {
    int32_t lambda_id;
    struct {
        char* lua;
        size_t length;
    } program;
};

/**
 * A lambda batch, containing a lambda ID, a batch ID, and arbitrary data.
 *
 * Batches are fed as input into lambdas, and are also received as output when
 * lambdas complete.
 */
struct lcm_Batch {
    int32_t lambda_id, batch_id;
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
    int32_t lambda_id;
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
 * Registers provided lambda in Lua state.
 *
 * The provided lambda object is safe to destroy at any point after the
 * function returns.
 *
 * Returns `0` (OK), `LCM_ERRRUN`, `LCM_ERRSYNTAX`, `LCM_ERRMEM`, `LCM_ERRERR`,
 * `LCM_ERRINIT`, or `LCM_ERRNOCALL`. The last is returned only if the provided
 * lambda fails to call `lcm:lambda()` when evaluated.
 */
LCM_API int lcm_register(lua_State* L, const lcm_Lambda l);

/**
 * Processes, using referenced Lua state, batch `b` and provides any results to
 * the function in closure `c`.
 *
 * The batch provided to `c` is destroyed after the closure function returns.
 *
 * Returns `0` (OK), `LCM_ERRRUN`, `LCM_ERRMEM`, `LCM_ERRERR`, `LCM_ERRINIT` or
 * `LCM_ERRNORESULT`. The last is returned only if the lambda processing the
 * batch fails to return a batch result, in which case `c` is never called.
 */
LCM_API int lcm_process(lua_State* L, const lcm_Batch b, lcm_ClosureBatch c);

/** Returns string representation of provided LCM error code. */
LCM_API const char* lcm_errstr(const int err);

#endif
