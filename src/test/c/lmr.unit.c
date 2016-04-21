#include "../../main/c/lmr.h"
#include "unit.h"
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

void provider_lua_state(unit_T* T, unit_TestFunction t)
{
    lua_State* L = luaL_newstate();
    if (L == NULL) {
        unit_fatal(T, "Failed to create new Lua state object.");
    }
    t(T, L);
    lua_close(L);
}

//{ Test cases.
void test_log(unit_T* T, void* arg);
void test_process(unit_T* T, void* arg);
//}

void suite_lmr(unit_T* T)
{
    unit_run_test(T, test_log, provider_lua_state);
    unit_run_test(T, test_process, provider_lua_state);
}

//{ Callbacks used by test cases.
static void f_log(void* context, const lmr_LogEntry* entry);
static void f_batch(void* context, const lmr_Batch* batch);
//}

void test_log(unit_T* T, void* arg)
{
    lua_State* L = arg;

    // Setup LMR.
    lmr_LogEntry result_log = {.job_id = 0 };
    {
        lmr_openlib(L,
            &(lmr_Config){
                .closure_log = {
                    .context = &result_log,
                    .function = f_log,
                },
            });
    }
    // Register job.
    {
        const lmr_Job j = {
            .job_id = 1,
            .program = {
                .lua = "lmr:register(function (batch)\n"
                       "  lmr:log(batch)\n"
                       "end)",
                .length = 51,
            },
        };
        if (lmr_register(L, j) != 0) {
            unit_failf(T, "[lmr_register] %s", lua_tostring(L, -1));
        }
    }
    // Process batch using registered job.
    lmr_Batch result_batch = {.job_id = 0 };
    {
        const lmr_Batch input_batch = {
            .job_id = 1,
            .batch_id = 2,
            .data = {
                .bytes = (uint8_t*)"hello",
                .length = 5,
            },
        };
        const lmr_ClosureBatch result_closure = {
            .context = &result_batch,
            .function = f_batch,
        };
        const int status = lmr_process(L, input_batch, result_closure);
        if (status != LMR_ERRNORESULT) {
            unit_failf(T, "[lmr_process] %s", status == 0
                    ? "Returned OK, expected LMR_ERRNORESULT."
                    : lua_tostring(L, -1));
        }
    }

    // Make sure result batch is untouched, as Lua job function never returned.
    {
        unit_assert(T, result_batch.job_id == 0);
        unit_assert(T, result_batch.batch_id == 0);
        unit_assert(T, result_batch.data.bytes == NULL);
        unit_assert(T, result_batch.data.length == 0);
    }
    // Make sure entry logged from Lua context was received.
    {
        unit_assert(T, result_log.job_id == 1);
        unit_assert(T, result_log.batch_id == 2);
        unit_assert(T, result_log.message.string != NULL
                && strcmp(result_log.message.string, "hello") == 0);
        unit_assert(T, result_log.message.length == 5);
    }
}

void test_process(unit_T* T, void* arg)
{
    lua_State* L = arg;

    // Setup LMR.
    {
        luaL_openlibs(L);
        lmr_openlib(L, NULL);
    }
    // Register job.
    {
        const lmr_Job j = {
            .job_id = 2,
            .program = {
                .lua = "lmr:register(function (batch)\n"
                       "  return batch:upper()\n"
                       "end)",
                .length = 57,
            },
        };
        if (lmr_register(L, j) != 0) {
            unit_failf(T, "[lmr_register] %s", lua_tostring(L, -1));
        }
    }
    // Process batch using registered job.
    lmr_Batch result_batch = {.job_id = 0 };
    {
        const lmr_Batch input_batch = {
            .job_id = 2,
            .batch_id = 1,
            .data = {
                .bytes = (uint8_t*)"hello",
                .length = 5,
            },
        };
        const lmr_ClosureBatch result_closure = {
            .context = &result_batch,
            .function = f_batch,
        };
        int status;
        if ((status = lmr_process(L, input_batch, result_closure)) != 0) {
            unit_failf(T, "[lmr_process] %s", lua_tostring(L, -1));
        }
    }
    // Verify result batch state.
    {
        unit_assert(T, result_batch.job_id == 2);
        unit_assert(T, result_batch.batch_id == 1);
        unit_assert(T, result_batch.data.bytes != NULL
                && memcmp(result_batch.data.bytes, "HELLO", 5) == 0);
        unit_assert(T, result_batch.data.length == 5);
    }
}

static void f_log(void* context, const lmr_LogEntry* entry)
{
    lmr_LogEntry* result = context;

    static char message[64];
    message[sizeof(message) - 1] = '\0';

    size_t length = MIN(sizeof(message) - 1, entry->message.length);

    result->job_id = entry->job_id;
    result->batch_id = entry->batch_id;
    result->message.string = strncpy(message, entry->message.string, length);
    result->message.length = length;
}

static void f_batch(void* context, const lmr_Batch* batch)
{
    lmr_Batch* result = context;

    static char data[64];
    data[sizeof(data) - 1] = '\0';

    size_t length = MIN(sizeof(data) - 1, batch->data.length);

    result->job_id = batch->job_id;
    result->batch_id = batch->batch_id;
    result->data.bytes = memcpy(data, batch->data.bytes, length);
    result->data.length = length;
}
