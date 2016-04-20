#include "lmr.h"
#include "lmrlua.h"
#include <stdlib.h>

LMR_API void lmr_openlib(lua_State* L, const lmr_Config* c) { return -1; }

LMR_API int lmr_register(lua_State* L, const lmr_Job j) { return -1; }

LMR_API int lmr_process(lua_State* L, const lmr_Batch in, lmr_Batch* out)
{
    return -1;
}

int lmr_l_job(lua_State* L) { return 0; }

int lmr_l_log(lua_State* L) { return 0; }
