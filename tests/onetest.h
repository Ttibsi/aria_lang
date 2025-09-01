#ifndef ONETEST_H
#define ONETEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ONETEST_MAX_ERRORS 32
#define ONETEST_MAX_ERROR_LEN 256
#define ONETEST_MAX_TESTS 128

/* Macro to create test entries */
#define ONETEST_TEST(func) {func, #func}

/* Test function pointer type */
typedef int (*onetest_func_t)(void);

/* Test structure */
typedef struct {
    onetest_func_t func;
    const char* name;
} onetest_test_t;

/* Error storage */
extern char onetest_errors[ONETEST_MAX_ERRORS][ONETEST_MAX_ERROR_LEN];
extern int onetest_error_count;

/* Main execution function */
int onetest_exec(const onetest_test_t* tests, int test_count);

#define onetest_assert(x) \
    do { \
        if (!(x)) { \
            snprintf(onetest_errors[onetest_error_count], ONETEST_MAX_ERROR_LEN, \
                    "%s:%d - assertion failed: %s", __FUNCTION__, __LINE__, #x); \
            onetest_error_count++; \
            if (onetest_error_count >= ONETEST_MAX_ERRORS) onetest_error_count = ONETEST_MAX_ERRORS - 1; \
        } \
    } while(0)

#ifdef ONETEST_IMPLEMENTATION

/* Global error storage */
char onetest_errors[ONETEST_MAX_ERRORS][ONETEST_MAX_ERROR_LEN];
int onetest_error_count = 0;

/* Check if running in CI environment */
static int onetest_is_ci(void) {
    return getenv("CI") != NULL;
}

/* Get current time in milliseconds */
static double onetest_get_time_ms(void) {
    clock_t c = clock();
    return ((double)c / CLOCKS_PER_SEC) * 1000.0;
}

int onetest_exec(const onetest_test_t* tests, int test_count) {
    int ci = onetest_is_ci();
    int fail_counter = 0;
    double start_time = onetest_get_time_ms();

    for (int i = 0; i < test_count; i++) {
        /* Clear errors for this test */
        onetest_error_count = 0;
        memset(onetest_errors, 0, sizeof(onetest_errors));

        /* Run the test */
        int retcode = tests[i].func();
        if (retcode) {
            snprintf(onetest_errors[onetest_error_count], ONETEST_MAX_ERROR_LEN,
                "Return code: %i", retcode);
            onetest_error_count++;
        }

        /* Calculate padding for output formatting */
        int name_len = strlen(tests[i].name);
        int padding = 79 - 6 - name_len;
        if (padding < 0) padding = 0;

        /* Print test name and dots */
        printf("%s", tests[i].name);
        for (int j = 0; j < padding; j++) {
            printf(".");
        }

        /* Print result */
        if (onetest_error_count > 0) {
            if (ci) {
                printf("Failed\n");
            } else {
                printf("\x1b[41mFailed\x1b[0m\n");
            }
        } else {
            if (ci) {
                printf("Passed\n");
            } else {
                printf("\x1b[42mPassed\x1b[0m\n");
            }
        }

        /* Print errors */
        for (int j = 0; j < onetest_error_count; j++) {
            if (ci) {
                printf("\t%s\n", onetest_errors[j]);
            } else {
                printf("\t\x1b[31m%s\x1b[0m\n", onetest_errors[j]);
            }
        }

        if (onetest_error_count > 0 || retcode > 0) {
            fail_counter++;
        }
    }

    /* Calculate runtime */
    double end_time = onetest_get_time_ms();
    double duration = end_time - start_time;

    /* Format summary message */
    char msg[128];
    switch (fail_counter) {
        case 0:
            snprintf(msg, sizeof(msg), " All tests passed in %.2fms ", duration);
            break;
        case 1:
            snprintf(msg, sizeof(msg), " 1 test failed in %.2fms ", duration);
            break;
        default:
            snprintf(msg, sizeof(msg), " %d tests failed in %.2fms ", fail_counter, duration);
    }

    /* Print summary line */
    int msg_len = strlen(msg);
    int line_size = (79 - msg_len) / 2;

    const char* color = fail_counter ? "\x1b[31m" : "\x1b[32m";
    const char* reset = "\x1b[0m";
    if (ci) {
        color = "";
        reset = "";
    }

    printf("%s", color);

    /* Print left side equals */
    int left_equals = line_size + (msg_len % 2 ? 0 : 1);
    for (int i = 0; i < left_equals; i++) {
        printf("=");
    }

    printf("%s", msg);

    /* Print right side equals */
    for (int i = 0; i < line_size; i++) {
        printf("=");
    }

    printf("%s\n", reset);

    return fail_counter;
}

#endif /* ONETEST_IMPLEMENTATION */
#endif /* ONETEST_H */
