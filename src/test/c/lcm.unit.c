#include "../../main/c/lcm.h"
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

void suite_lcm(unit_T* T)
{
    unit_run_test(T, test_log, provider_lua_state);
    unit_run_test(T, test_process, provider_lua_state);
}

//{ Callbacks used by test cases.
static void f_log(void* context, const lcm_LogEntry* entry);
static void f_batch(void* context, const lcm_Batch* batch);
//}

void test_log(unit_T* T, void* arg)
{
    lua_State* L = arg;

    // Setup LCM.
    lcm_LogEntry result_log = {.lambda_id = 0 };
    {
        lcm_openlib(L,
            &(lcm_Config){
                .closure_log = {
                    .context = &result_log,
                    .function = f_log,
                },
            });
    }
    // Register job.
    {
        const lcm_Lambda l = {
            .lambda_id = 1,
            .program = {
                .lua = "lcm:register(function (batch)\n"
                       "  lcm:log(batch)\n"
                       "end)",
                .length = 51,
            },
        };
        const int status = lcm_register(L, l);
        if (status != 0) {
            unit_failf(T, "[lcm_register] %s", lcm_errstr(status));
        }
    }
    // Process batch using registered job.
    lcm_Batch result_batch = {.lambda_id = 0 };
    {
        const lcm_Batch input_batch = {
            .lambda_id = 1,
            .batch_id = 2,
            .data = {
                .bytes = (uint8_t*)"hello",
                .length = 5,
            },
        };
        const lcm_ClosureBatch result_closure = {
            .context = &result_batch,
            .function = f_batch,
        };
        const int status = lcm_process(L, input_batch, result_closure);
        if (status != LCM_ERRNORESULT) {
            unit_failf(T, "[lcm_process] %s",
                status == 0
                    ? "Returned OK, expected LCM_ERRNORESULT."
                    : lcm_errstr(status));
        }
    }

    // Make sure result batch is untouched, as Lua job function never returned.
    {
        unit_assert(T, result_batch.lambda_id == 0);
        unit_assert(T, result_batch.batch_id == 0);
        unit_assert(T, result_batch.data.bytes == NULL);
        unit_assert(T, result_batch.data.length == 0);
    }
    // Make sure entry logged from Lua context was received.
    {
        unit_assert(T, result_log.lambda_id == 1);
        unit_assert(T, result_log.batch_id == 2);
        unit_assert(T, result_log.message.string != NULL
                && strcmp(result_log.message.string, "hello") == 0);
        unit_assert(T, result_log.message.length == 5);
    }
}

void test_process(unit_T* T, void* arg)
{
    lua_State* L = arg;

    // Setup LCM.
    {
        luaL_openlibs(L);
        lcm_openlib(L, NULL);
    }
    // Register job.
    {
        const lcm_Lambda l = {
            .lambda_id = 2,
            .program = {
                .lua = "lcm:register(function (batch)\n"
                       "  return batch:upper()\n"
                       "end)",
                .length = 57,
            },
        };
        const int status = lcm_register(L, l);
        if (status != 0) {
            unit_failf(T, "[lcm_register] %s", lcm_errstr(status));
        }
    }
    // Process batch using registered job.
    lcm_Batch result_batch = {.lambda_id = 0 };
    {
        const lcm_Batch input_batch = {
            .lambda_id = 2,
            .batch_id = 1,
            .data = {
                .bytes = (uint8_t*)"hello",
                .length = 5,
            },
        };
        const lcm_ClosureBatch result_closure = {
            .context = &result_batch,
            .function = f_batch,
        };
        const int status = lcm_process(L, input_batch, result_closure);
        if (status != 0) {
            unit_failf(T, "[lcm_process] %s", lcm_errstr(status));
        }
    }
    // Verify result batch state.
    {
        unit_assert(T, result_batch.lambda_id == 2);
        unit_assert(T, result_batch.batch_id == 1);
        unit_assert(T, result_batch.data.bytes != NULL
                && memcmp(result_batch.data.bytes, "HELLO", 5) == 0);
        unit_assert(T, result_batch.data.length == 5);
    }
}

static void f_log(void* context, const lcm_LogEntry* entry)
{
    lcm_LogEntry* result = context;

    static char message[64];
    message[sizeof(message) - 1] = '\0';

    size_t length = MIN(sizeof(message) - 1, entry->message.length);

    result->lambda_id = entry->lambda_id;
    result->batch_id = entry->batch_id;
    result->message.string = strncpy(message, entry->message.string, length);
    result->message.length = length;
}

static void f_batch(void* context, const lcm_Batch* batch)
{
    lcm_Batch* result = context;

    static char data[64];
    data[sizeof(data) - 1] = '\0';

    size_t length = MIN(sizeof(data) - 1, batch->data.length);

    result->lambda_id = batch->lambda_id;
    result->batch_id = batch->batch_id;
    result->data.bytes = memcpy(data, batch->data.bytes, length);
    result->data.length = length;
}
