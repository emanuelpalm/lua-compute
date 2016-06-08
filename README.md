![icon](design/icon/web-270dpi.png)

# Lua/compute

The Lua/compute C library facilitates the use of [Lua][lua] programs as compute
contexts. In Lua/compute, a compute context is a set of functions that all
accept byte array arguments, and return byte array results. These functions are
referred to as *lambdas*, and the byte array input/output values are called
*batches*. One could think of Lua/compute as one big function, that takes
batches and lambda identifiers as input, and return processed batches as
output, as illustrated by the below diagram.

[lua]: http://www.lua.org/

![diagram](design/docs/lua-compute-diagram.png)

Read more about the details of Lua/compute C API [here](src/main/c/lcm.h).

## Manual

### Initializing Lua State Context

From the perspective of Lua, Lua/compute is just a C library that exposes Lua
functions. In order to use Lua/compute, an existing Lua state is required.
Setting up a Lua state and then attaching Lua/compute to it can be done as in
the below example.

```c
// Lua/compute main header.
#include <lcm.h>

// Lua headers.
#include <lua.h>
#include <lualib.h>

int main()
{
    lua_State* L = luaL_newstate();

    // Attach Lua/compute functions to Lua state.
    lcm_openlib(L, NULL);

    // Use Lua state and Lua/compute ...

    lua_close(L);
}
```

### Registering Lambdas

After attaching Lua/compute to a Lua state, it has to be provided with lambda
functions before it can be used to process batches. This is done by providing
lua programs that invoke `lcm:register()` with a Lua function as argument. This
function can later be called in order to process batches, and should, because
of this, accept a single `batch` parameter, and return a new batch at some
point.

```c
// Headers ...

int main()
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L); // Loads Lua standard library.
    lcm_openlib(L, NULL);

    // Registers program that turns batch characters to uppercase.
    const char* to_uppercase = ""
        "lcm:register(function (batch)\n"
        "  batch:upper()\n"
        "end)";

    lcm_register(L, (lcm_Lambda){
        .lambda_id = 1,
        .program = { .lua = to_uppercase, .length = strlen(to_uppercase) },
    });

    // Use Lua state and Lua/compute ...

    lua_close(L);
}
```

### Processing Batches

When a properly set up Lua state contains at least one registered lambda, it
can be used to process batches. This is done using the `lcm_process` function,
which accepts a lambda identifier, a batch identifier, and any byte array. It
calls a referenced function with the result of the batch processing, if it was
successful.

```c
// Headers ...

// This function is called with any `batch_process` results.
static void on_batch(void* context, const lcm_Batch* batch);

int main()
{
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    lcm_openlib(L, NULL);

    const char* to_uppercase = ""
        "lcm:register(function (batch)\n"
        "  batch:upper()\n"
        "end)";

    lcm_register(L, (lcm_Lambda){
        .lambda_id = 1,
        .program = { .lua = to_uppercase, .length = strlen(to_uppercase) },
    });

    // Right before this function returns, the `on_batch` function is called
    // with the batch result `"HELLO"`.
    lcm_process(L, (lcm_Batch){
        .lambda_id = 1,
        .batch_id = 100,
        .data = { .bytes = (uint8_t*)"hello", .length = 5 },
    }, (lcm_ClosureBatch){
        .context = NULL,
        .function = on_batch,
    });

    lua_close(L);
}
```

### Capturing Log Output

To make it more straightforward to handle log output, the Lua/compute library
provides the Lua function `lcm:log()`, which accepts a single string argument.
Calling this while a lambda is being registered or a batch being processed will
cause a C function, if properly referenced, to be called with the ID of the
lambda and/or batch currently active.

```c
// Headers ...

// This function is called whenever the `lcm:log()` function is called.
static void on_log(void* context, const lcm_LogEntry* entry);

int main()
{
    lua_State* L = luaL_newstate();

    lcm_openlib(L, &(lcm_Config){
        .closure_log = { .context = null, .function = on_log },
    });

    // Use Lua state and Lua/compute ...

    lua_close(L);
}
```

### Structured Batch Data, Libraries, etc.

If wanting to decode JSON structured batches, perform vector calculations, or
any other task that isn't facilitated directly by the standard Lua library,
there are two primary ways to get that functionality.

1. Provide the functionality as Lua code inside registered lambdas.
2. Attach C libraries that expose Lua functions to the used Lua state.

## Building and Installing

The library requires LuaJIT 2.0 headers to be available to build the library,
and the LuaJIT 2.0 library if building the test suite. Building is done using
GNU Make from the repository root directory, as shown below.

```bash
$ make all
```

OS X users should note that it is assumed that all libraries and headers are
available via the `/usr/local` prefix. The prefix is used by [Homebrew][brew],
which provides a convenient way to install the required headers and libraries.

[brew]: http://brew.sh/

### Using Regular Lua 5.1

If wishing to build using regular Lua 5.1, the below example commands could be
used to force its usage.

#### Linux (Ubuntu 14.04)

```bash
$ make all CFLAGS="-std=c99 `pkg-config --cflags lua5.1`" \
    LDFLAGS="`pkg-config --libs-only-L lua5.1`" \
    LIBS="`pkg-config --libs-only-l lua5.1`"
```

#### Mac OS X (10.11 El Capitan)

```bash
$ make all CFLAGS="-std=c99 -I/usr/local/include/lua-5.1" \
    LDFLAGS="-L/usr/local/lib" LIBS="-llua5.1"
```

## Limitations

The library provides no networking utilities whatsoever, despite that the
intended use case was for the library to be used for distributed computations.
The reason for this is to make the library as portable as possible, and to not
limits its use to distributed applications.

## Contributing

Bug fixes and minor enhancements are most appreciated. If wanting to add some
more significant functionality, please create a question issue and discuss it
first.

All contributions are expected to adhere to the code style and conventions
adhered to by the project.

All code was written using the Atom editor using the plug-ins `clang-format`,
`autocomplete-clang`, `docblockr`, `linter` and `linter-clang`. The repository
already contains the proper configuration files for these plug-ins to work out
as intended.
