/*!
** Unit testing framework.
**
** Simplistic testing framework which divides tests into suites.
**
** @file unit.h
*/
#ifndef unit_h
#define unit_h

#include <setjmp.h>
#include <stdlib.h>

/*!
** Global unit test context.
**
** An instance of this type represents the results of all performed unit tests.
*/
typedef struct {
    struct {
        size_t total, failed;
    } suites;
} unit_State;

/*!
** Unit test suite context.
**
** An instance of this type is passed around to all tests within the same suite
** in order for them to report on their results. The convention is to name
** instances of this type T when passing them around.
*/
typedef struct {
    struct {
        size_t total, failed, skipped;
    } tests;
    struct {
        size_t failures;
    } test;
    jmp_buf jmp_skip, jmp_fatal;
} unit_T;

/*! Unit test suite function pointer type. */
typedef void (*unit_SuiteFunction)(unit_T *);

/*!
** Unit test function pointer type.
**
** The void pointer argument is only given if a provider function is used, and
** it passes on anything of relevance.
*/
typedef void (*unit_TestFunction)(unit_T *, void *);

/*! Unit test provider function pointer type. */
typedef void (*unit_ProviderFunction)(unit_T *, unit_TestFunction);

/*! Initializes unit test suite data structure. */
void unit_init(unit_State *u);

/*!
** Reports on unit test suite results and terminates application.
**
** The application is terminated with the EXIT_FAILURE exit code if any test
** failed. In case all tests passed then EXIT_SUCCESS is used.
*/
void unit_exit(unit_State *u);

/*! Prints name of given suite, runs suite, and reports results. */
void unit_run_suite(unit_State *u, const char *name, unit_SuiteFunction s);

/*!
** Runs given test and stores any results into T.
**
** If a provider function is given, it is run instead. It is called with a
** reference to given test.
*/
void unit_run_test(unit_T *T, unit_TestFunction t, unit_ProviderFunction p);

/*!
** Fails current test, reporting given message.
**
** Do note that execution of the current function continues after the call.
*/
#define unit_fail(T, msg) _unit_failf(T, UNIX_CTX, msg)
#define unit_failf(T, fmt, ...) _unit_failf(T, UNIX_CTX, fmt, __VA_ARGS__)

/*!
** Skips current test, reporting given message.
**
** The execution of the current test is stopped. Any clean-up required ought to
** be performed prior to issuing this call.
*/
#define unit_skip(T, msg) _unit_skipf(T, UNIX_CTX, msg)
#define unit_skipf(T, fmt, ...) _unit_skipf(T, UNIX_CTX, fmt, __VA_ARGS__)

/*!
** Skips current test suite, failing it, reporting given message.
**
** The execution of the current test suite is stopped. Any clean-up required
** ought to be performed prior to issuing this call.
*/
#define unit_fatal(T, msg) _unit_fatalf(T, UNIX_CTX, msg)
#define unit_fatalf(T, fmt, ...) _unit_fatalf(T, UNIX_CTX, fmt, __VA_ARGS__)

/*!
** Asserts that given test is true.
**
** In case of test is evaluated to false, the test is printed as a string and a
** failure is registered.
*/
#define unit_assert(T, test)                  \
    if (!(test)) {                            \
        _unit_failf(T, UNIX_CTX, "! " #test); \
    }

/*!
** Unit call context.
**
** Represents the function, file and line at which a fail, skip or fatal call
** is made. The type is typically instantiated using the UNIX_CTX macro.
*/
typedef struct {
    const char *func;
    const char *file;
    const int line;
} unit_Ctx;

/*!
** Unit call context macro.
**
** This macro expands into a call context which describes the function, file
** and line at which the macro is placed.
**
** The macro is intended to be use together with the _unit_failf(),
** _unit_skipf(), and _unit_fatalf() functions, in order for these to be able
** to properly print the contexts in which they are called.
*/
#define UNIX_CTX \
    &(unit_Ctx) { __func__, __FILE__, __LINE__ }

void _unit_failf(unit_T *T, const unit_Ctx *X, const char *fmt, ...);
void _unit_skipf(unit_T *T, const unit_Ctx *X, const char *fmt, ...);
void _unit_fatalf(unit_T *T, const unit_Ctx *X, const char *fmt, ...);

#endif
