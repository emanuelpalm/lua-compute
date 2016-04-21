#include "../../main/c/lmr.h"
#include "unit.h"
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

//{ Data registered during calls to `lmr:log()`.
static lmr_LogEntry log_entry = {.message.string = "" };
//}

//{ Return value of registered Lua job function.
static lmr_Batch batch_result = {.job_id = 0 };
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

// Used to test the `lmr:log()` lua function.
static void f_log(const lmr_LogEntry* entry)
{
    static char message[64];
    message[sizeof(message) - 1] = '\0';

    size_t length = MIN(sizeof(message) - 1, entry->message.length);

    log_entry.job_id = entry->job_id;
    log_entry.batch_id = entry->batch_id;
    log_entry.message.string = strncpy(message, entry->message.string, length);
    log_entry.message.length = length;
}

static void f_batch(void* context, const lmr_Batch* batch)
{
    static char data[64];
    data[sizeof(data) - 1] = '\0';

    size_t length = MIN(sizeof(data) - 1, batch->data.length);

    batch_result.job_id = batch->job_id;
    batch_result.batch_id = batch->batch_id;
    batch_result.data.bytes = memcpy(data, batch->data.bytes, length);
    batch_result.data.length = length;
}

void test_log(unit_T* T, void* arg)
{
    lua_State* L = arg;

    lmr_openlib(L, &(lmr_Config){.log_function = f_log });

    const lmr_Job j = {
        .job_id = 1,
        .program = {
            .lua = "lmr:register(function (batch) lmr:log(batch) end)",
            .length = 49,
        },
    };
    const lmr_Batch b = {
        .job_id = 1,
        .batch_id = 2,
        .data = {
            .bytes = (uint8_t*)"hello",
            .length = 5,
        },
    };
    batch_result.job_id = 0;
    batch_result.batch_id = 0;
    batch_result.data.bytes = NULL;
    batch_result.data.length = 0;

    if (lmr_register(L, j) != 0) {
        unit_failf(T, "[lmr_register] %s", lua_tostring(L, -1));
    }
    lmr_ResultClosure c = {.function = f_batch };
    int status;
    if ((status = lmr_process(L, b, c)) != LMR_ERRNORESULT) {
        unit_failf(T, "[lmr_process] %s", status == 0
                ? "Returned OK, expected LMR_ERRNORESULT."
                : lua_tostring(L, -1));
    }

    //{ Since Lua job callback never returned, the out batch must be untouched.
    unit_assert(T, batch_result.job_id == 0);
    unit_assert(T, batch_result.batch_id == 0);
    unit_assert(T, batch_result.data.bytes == NULL);
    unit_assert(T, batch_result.data.length == 0);
    //}

    unit_assert(T, log_entry.job_id == 1);
    unit_assert(T, log_entry.batch_id == 2);
    unit_assert(T, strcmp(log_entry.message.string, "hello") == 0);
}

void test_process(unit_T* T, void* arg)
{
    lua_State* L = arg;

    luaL_openlibs(L);
    lmr_openlib(L, &(lmr_Config){.log_function = logf });

    const lmr_Job j = {
        .job_id = 2,
        .program = {
            .lua = "lmr:register(function (batch) return batch:upper() end)",
            .length = 55,
        },
    };
    const lmr_Batch b = {
        .job_id = 2,
        .batch_id = 1,
        .data = {
            .bytes = (uint8_t*)"hello",
            .length = 5,
        },
    };
    batch_result.job_id = 0;
    batch_result.batch_id = 0;
    batch_result.data.bytes = (uint8_t*)"";
    batch_result.data.length = 0;

    if (lmr_register(L, j) != 0) {
        unit_failf(T, "[lmr_register] %s", lua_tostring(L, -1));
    }
    lmr_ResultClosure c = {.function = f_batch };
    int status;
    if ((status = lmr_process(L, b, c)) != 0) {
        unit_failf(T, "[lmr_process] %s", lua_tostring(L, -1));
    }

    unit_assert(T, batch_result.job_id == 2);
    unit_assert(T, batch_result.batch_id == 1);
    unit_assert(T, memcmp(batch_result.data.bytes, "HELLO", 5) == 0);
    unit_assert(T, batch_result.data.length == 5);
}
