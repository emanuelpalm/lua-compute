#include "../../main/c/lmr.h"
#include "unit.h"
#include <lauxlib.h>
#include <lua.h>
#include <string.h>

//{ Data registered during calls to `logf()`.
static uint32_t logf_job_id = 0;
static uint32_t logf_batch_id = 0;
static const char* logf_s = "";
//}

void provider_lua_state(unit_T* T, unit_TestFunction t)
{
    lua_State* L = luaL_newstate();
    if (L == NULL) {
        unit_fatal(T, "Failed to create new Lua state object.");
    }
    t(T, L);
    lua_close(L);
}

// Test cases.
void test_log(unit_T* T, void* arg);
void test_process(unit_T* T, void* arg);

void suite_lmr(unit_T* T)
{
    unit_run_test(T, test_log, provider_lua_state);
    unit_run_test(T, test_process, provider_lua_state);
}

// Used to test `lmr:log()` lua function.
static void logf(const uint32_t job_id, const uint32_t batch_id, const char* s)
{
    logf_job_id = job_id;
    logf_batch_id = batch_id;
    logf_s = s;
}

void test_log(unit_T* T, void* arg)
{
    lua_State* L = arg;

    const lmr_Config c = {
        .log_function = logf,
    };
    const lmr_Job j = {
        .job_id = 1,
        .program = {
            .lua = "lmr:job(function (batch) lmr:log(batch) end)",
            .length = 44,
        },
    };
    const lmr_Batch b_in = {
        .job_id = 1,
        .batch_id = 2,
        .data = {
            .bytes = (uint8_t*)"hello",
            .length = 5,
        },
    };
    lmr_Batch b_out = {
        .job_id = 0,
        .batch_id = 0,
        .data = {
            .bytes = NULL,
            .length = 0,
        },
    };
    unit_assert(T, lmr_openlib(L, &c) == 0);
    unit_assert(T, lmr_register(L, j) == 0);
    unit_assert(T, lmr_process(L, b_in, &b_out) == LMR_ERRNORESULT);

    //{ Since Lua job callback never returned, the out batch must be untouched.
    unit_assert(T, b_out.job_id == 0);
    unit_assert(T, b_out.batch_id == 0);
    unit_assert(T, b_out.data.bytes == NULL);
    unit_assert(T, b_out.data.length == 0);
    //}

    unit_assert(T, logf_job_id == 1);
    unit_assert(T, logf_batch_id == 2);
    unit_assert(T, strcmp(logf_s, "hello") == 0);
}

void test_process(unit_T* T, void* arg)
{
    lua_State* L = arg;

    const lmr_Config c = {
        .log_function = logf,
    };
    const lmr_Job j = {
        .job_id = 2,
        .program = {
            .lua = "lmr:job(function (batch) return batch:upper() end)",
            .length = 50,
        },
    };
    const lmr_Batch b_in = {
        .job_id = 2,
        .batch_id = 1,
        .data = {
            .bytes = (uint8_t*)"hello",
            .length = 5,
        },
    };
    lmr_Batch b_out = {
        .job_id = 0,
        .batch_id = 0,
        .data = {
            .bytes = (uint8_t*)"",
            .length = 0,
        },
    };
    unit_assert(T, lmr_openlib(L, &c) == 0);
    unit_assert(T, lmr_register(L, j) == 0);
    unit_assert(T, lmr_process(L, b_in, &b_out) == 0);

    unit_assert(T, b_out.job_id == 2);
    unit_assert(T, b_out.batch_id == 1);
    unit_assert(T, memcmp(b_out.data.bytes, "HELLO", 5) == 0);
    unit_assert(T, b_out.data.length == 5);
}
