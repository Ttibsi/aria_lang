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

/* Internal assert implementations */
static inline void onetest_assert_eq_int_impl(int x, int y) {
    if (x != y) {
        snprintf(onetest_errors[onetest_error_count], ONETEST_MAX_ERROR_LEN,
                "%s:%d - %d does not equal %d", __FUNCTION__, __LINE__, x, y);
        onetest_error_count++;
        if (onetest_error_count >= ONETEST_MAX_ERRORS) onetest_error_count = ONETEST_MAX_ERRORS - 1;
    }
}

static inline void onetest_assert_ne_int_impl(int x, int y) {
    if (x == y) {
        snprintf(onetest_errors[onetest_error_count], ONETEST_MAX_ERROR_LEN,
                "%s:%d - %d is equal to %d", __FUNCTION__, __LINE__, x, y);
        onetest_error_count++;
        if (onetest_error_count >= ONETEST_MAX_ERRORS) onetest_error_count = ONETEST_MAX_ERRORS - 1;
    }
}

static inline void onetest_assert_eq_float_impl(double x, double y) {
    if (x != y) {
        snprintf(onetest_errors[onetest_error_count], ONETEST_MAX_ERROR_LEN,
                "%s:%d - %g does not equal %g", __FUNCTION__, __LINE__, x, y);
        onetest_error_count++;
        if (onetest_error_count >= ONETEST_MAX_ERRORS) onetest_error_count = ONETEST_MAX_ERRORS - 1;
    }
}

static inline void onetest_assert_ne_float_impl(double x, double y) {
    if (x == y) {
        snprintf(onetest_errors[onetest_error_count], ONETEST_MAX_ERROR_LEN,
                "%s:%d - %g is equal to %g", __FUNCTION__, __LINE__, x, y);
        onetest_error_count++;
        if (onetest_error_count >= ONETEST_MAX_ERRORS) onetest_error_count = ONETEST_MAX_ERRORS - 1;
    }
}

static inline void onetest_assert_eq_char_impl(char x, char y) {
    if (x != y) {
        snprintf(onetest_errors[onetest_error_count], ONETEST_MAX_ERROR_LEN,
                "%s:%d - '%c' does not equal '%c'", __FUNCTION__, __LINE__, x, y);
        onetest_error_count++;
        if (onetest_error_count >= ONETEST_MAX_ERRORS) onetest_error_count = ONETEST_MAX_ERRORS - 1;
    }
}

static inline void onetest_assert_ne_char_impl(char x, char y) {
    if (x == y) {
        snprintf(onetest_errors[onetest_error_count], ONETEST_MAX_ERROR_LEN,
                "%s:%d - '%c' is equal to '%c'", __FUNCTION__, __LINE__, x, y);
        onetest_error_count++;
        if (onetest_error_count >= ONETEST_MAX_ERRORS) onetest_error_count = ONETEST_MAX_ERRORS - 1;
    }
}

static inline void onetest_assert_eq_str_impl(const char* x, const char* y) {
    if (strcmp(x, y) != 0) {
        snprintf(onetest_errors[onetest_error_count], ONETEST_MAX_ERROR_LEN,
                "%s:%d - \"%s\" does not equal \"%s\"", __FUNCTION__, __LINE__, x, y);
        onetest_error_count++;
        if (onetest_error_count >= ONETEST_MAX_ERRORS) onetest_error_count = ONETEST_MAX_ERRORS - 1;
    }
}

static inline void onetest_assert_ne_str_impl(const char* x, const char* y) {
    if (strcmp(x, y) == 0) {
        snprintf(onetest_errors[onetest_error_count], ONETEST_MAX_ERROR_LEN,
                "%s:%d - \"%s\" is equal to \"%s\"", __FUNCTION__, __LINE__, x, y);
        onetest_error_count++;
        if (onetest_error_count >= ONETEST_MAX_ERRORS) onetest_error_count = ONETEST_MAX_ERRORS - 1;
    }
}

/* Pointer comparison implementations */
static inline void onetest_assert_eq_ptr_impl(const void* x, const void* y) {
    if (x != y) {
        snprintf(onetest_errors[onetest_error_count], ONETEST_MAX_ERROR_LEN,
                "%s:%d - %p does not equal %p", __FUNCTION__, __LINE__, x, y);
        onetest_error_count++;
        if (onetest_error_count >= ONETEST_MAX_ERRORS) onetest_error_count = ONETEST_MAX_ERRORS - 1;
    }
}

static inline void onetest_assert_ne_ptr_impl(const void* x, const void* y) {
    if (x == y) {
        snprintf(onetest_errors[onetest_error_count], ONETEST_MAX_ERROR_LEN,
                "%s:%d - %p is equal to %p", __FUNCTION__, __LINE__, x, y);
        onetest_error_count++;
        if (onetest_error_count >= ONETEST_MAX_ERRORS) onetest_error_count = ONETEST_MAX_ERRORS - 1;
    }
}

/* Generic assert macros using _Generic (C23) */
#define onetest_assert_eq(x, y) \
    _Generic((x), \
        int: onetest_assert_eq_int_impl, \
        long: onetest_assert_eq_int_impl, \
        long long: onetest_assert_eq_int_impl, \
        unsigned int: onetest_assert_eq_int_impl, \
        unsigned long: onetest_assert_eq_int_impl, \
        unsigned long long: onetest_assert_eq_int_impl, \
        short: onetest_assert_eq_int_impl, \
        unsigned short: onetest_assert_eq_int_impl, \
        char: onetest_assert_eq_char_impl, \
        signed char: onetest_assert_eq_char_impl, \
        unsigned char: onetest_assert_eq_char_impl, \
        float: onetest_assert_eq_float_impl, \
        double: onetest_assert_eq_float_impl, \
        long double: onetest_assert_eq_float_impl, \
        char*: onetest_assert_eq_str_impl, \
        const char*: onetest_assert_eq_str_impl, \
        void*: onetest_assert_eq_ptr_impl, \
        const void*: onetest_assert_eq_ptr_impl, \
        default: onetest_assert_eq_ptr_impl \
    )(x, y)

#define onetest_assert_ne(x, y) \
    _Generic((x), \
        int: onetest_assert_ne_int_impl, \
        long: onetest_assert_ne_int_impl, \
        long long: onetest_assert_ne_int_impl, \
        unsigned int: onetest_assert_ne_int_impl, \
        unsigned long: onetest_assert_ne_int_impl, \
        unsigned long long: onetest_assert_ne_int_impl, \
        short: onetest_assert_ne_int_impl, \
        unsigned short: onetest_assert_ne_int_impl, \
        char: onetest_assert_ne_char_impl, \
        signed char: onetest_assert_ne_char_impl, \
        unsigned char: onetest_assert_ne_char_impl, \
        float: onetest_assert_ne_float_impl, \
        double: onetest_assert_ne_float_impl, \
        long double: onetest_assert_ne_float_impl, \
        char*: onetest_assert_ne_str_impl, \
        const char*: onetest_assert_ne_str_impl, \
        void*: onetest_assert_ne_ptr_impl, \
        const void*: onetest_assert_ne_ptr_impl, \
        default: onetest_assert_ne_ptr_impl \
    )(x, y)

#define onetest_assert_true(x) \
    do { \
        if (!(x)) { \
            snprintf(onetest_errors[onetest_error_count], ONETEST_MAX_ERROR_LEN, \
                    "%s:%d - assertion failed: %s", __FUNCTION__, __LINE__, #x); \
            onetest_error_count++; \
            if (onetest_error_count >= ONETEST_MAX_ERRORS) onetest_error_count = ONETEST_MAX_ERRORS - 1; \
        } \
    } while(0)

#define onetest_assert_false(x) \
    do { \
        if ((x)) { \
            snprintf(onetest_errors[onetest_error_count], ONETEST_MAX_ERROR_LEN, \
                    "%s:%d - assertion failed: !(%s)", __FUNCTION__, __LINE__, #x); \
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
        if (onetest_error_count > 0 || retcode > 0) {
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
