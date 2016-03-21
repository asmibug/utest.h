// The latest version of this library is available on GitHub;
//   https://github.com/sheredom/utest.h

// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org/>

#ifndef SHEREDOM_UTEST_H_INCLUDED
#define SHEREDOM_UTEST_H_INCLUDED

#ifdef _MSC_VER
#pragma warning(push, 1)
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#if defined(_MSC_VER)
#if defined(_M_IX86)
#define _X86_
#endif

#if defined(_M_AMD64)
#define _AMD64_
#endif

#pragma warning(push, 1)
#include <winbase.h>
#include <windef.h>
#pragma warning(pop)

#elif defined(__linux__)

// slightly obscure include here - we need to include glibc's features.h, but
// we don't want to just include a header that might not be defined for other
// c libraries like musl. Instead we include limits.h, which we know on all
// glibc distributions includes features.h
#include <limits.h>

#if defined(__GLIBC__) && defined(__GLIBC_MINOR__)
#include <time.h>

#if ((2 < __GLIBC__) || ((2 == __GLIBC__) && (17 <= __GLIBC_MINOR__)))
// glibc is version 2.17 or above, so we can just use clock_gettime
#define UTEST_USE_CLOCKGETTIME
#else // ((2 < __GLIBC__) || ((2 == __GLIBC__) && (17 <= __GLIBC_MINOR__)))
#include <sys/syscall.h>
#include <unistd.h>
#endif // ((2 < __GLIBC__) || ((2 == __GLIBC__) && (17 <= __GLIBC_MINOR__)))
#endif // defined(__GLIBC__) && defined(__GLIBC_MINOR__)

#elif defined(__APPLE__)
#include <mach/mach_time.h>
#endif

#if defined(_MSC_VER)
#define UTEST_PRId64 "I64d"
#define UTEST_PRIu64 "I64u"
#define UTEST_INLINE __forceinline

#pragma section(".CRT$XCU", read)
#define UTEST_INITIALIZER(f)                                                   \
  static void __cdecl f(void);                                                 \
  __declspec(allocate(".CRT$XCU")) void(__cdecl * f##_)(void) = f;             \
  static void __cdecl f(void)
#else
#include <inttypes.h>

#define UTEST_PRId64 PRId64
#define UTEST_PRIu64 PRIu64
#define UTEST_INLINE inline

#define UTEST_INITIALIZER(f)                                                   \
  static void f(void) __attribute__((constructor));                            \
  static void f(void)
#endif

#if defined(__cplusplus)
#define UTEST_CAST(type, x) static_cast<type>(x)
#define UTEST_PTR_CAST(type, x) reinterpret_cast<type>(x)
#define UTEST_EXTERN extern "C"
#else
#define UTEST_CAST(type, x) ((type)x)
#define UTEST_PTR_CAST(type, x) ((type)x)
#define UTEST_EXTERN extern
#endif

static UTEST_INLINE int64_t utest_ns(void) {
#ifdef _MSC_VER
  LARGE_INTEGER counter;
  LARGE_INTEGER frequency;
  QueryPerformanceCounter(&counter);
  QueryPerformanceFrequency(&frequency);
  return UTEST_CAST(int64_t,
                    (counter.QuadPart * 1000000000) / frequency.QuadPart);
#elif defined(__linux)
  struct timespec ts;
  const clockid_t cid = CLOCK_REALTIME;
#if defined(UTEST_USE_CLOCKGETTIME)
  clock_gettime(cid, &ts);
#else
  syscall(SYS_clock_gettime, cid, &ts);
#endif
  return UTEST_CAST(int64_t, ts.tv_sec) * 1000 * 1000 * 1000 + ts.tv_nsec;
#elif __APPLE__
  return UTEST_CAST(int64_t, mach_absolute_time());
#endif
}

typedef void (*utest_testcase_t)(int *);

struct utest_test_state_s {
  utest_testcase_t func;
  const char *name;
};

struct utest_state_s {
  struct utest_test_state_s *tests;
  size_t tests_length;
};

#if defined(_MSC_VER)
#define UTEST_WEAK __forceinline
#else
#define UTEST_WEAK __attribute__((weak))
#endif

#if defined(__cplusplus)
// if we are using c++ we can use overloaded methods (its in the language)
#define UTEST_OVERLOADABLE
#elif defined(__clang__)
// otherwise, if we are using clang with c we can use the overloadable attribute
#define UTEST_OVERLOADABLE __attribute__((overloadable))
#endif

#if defined(UTEST_OVERLOADABLE)
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(float f);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(float f) {
  printf("%f", UTEST_CAST(double, f));
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(double d);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(double d) {
  printf("%f", d);
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long double d);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long double d) {
  printf("%Lf", d);
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(int i);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(int i) {
  printf("%d", i);
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(unsigned int i);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(unsigned int i) {
  printf("%u", i);
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long int i);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long int i) {
  printf("%ld", i);
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long unsigned int i);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long unsigned int i) {
  printf("%lu", i);
}

// long long is a c++11 extension
// TODO: grok for c++11 version here
#if !defined(__cplusplus)
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long long int i);
UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long long int i) {
  printf("%lld", i);
}

UTEST_WEAK UTEST_OVERLOADABLE void utest_type_printer(long long unsigned int i);
UTEST_WEAK UTEST_OVERLOADABLE void
utest_type_printer(long long unsigned int i) {
  printf("%llu", i);
}
#endif
#else
// we don't have the ability to print the values we got, so we create a macro to
// tell our users we can't do anything fancy
#define utest_type_printer(...) printf("undef")
#endif

#define UTEST_EXPECT(x, y, cond)                                               \
  if (!((x)cond(y))) {                                                         \
    printf("%s:%u: Failure\n", __FILE__, __LINE__);                            \
    *utest_result = 1;                                                         \
  }

#define EXPECT_TRUE(x)                                                         \
  if (!(x)) {                                                                  \
    printf("%s:%u: Failure\n", __FILE__, __LINE__);                            \
    printf("  Expected : true\n");                                             \
    printf("    Actual : %s\n", (x) ? "true" : "false");                       \
    *utest_result = 1;                                                         \
  }

#define EXPECT_FALSE(x)                                                        \
  if (x) {                                                                     \
    printf("%s:%u: Failure\n", __FILE__, __LINE__);                            \
    printf("  Expected : false\n");                                            \
    printf("    Actual : %s\n", (x) ? "true" : "false");                       \
    *utest_result = 1;                                                         \
  }

#define EXPECT_EQ(x, y) UTEST_EXPECT(x, y, ==)
#define EXPECT_NE(x, y) UTEST_EXPECT(x, y, !=)
#define EXPECT_LT(x, y) UTEST_EXPECT(x, y, <)
#define EXPECT_LE(x, y) UTEST_EXPECT(x, y, <=)
#define EXPECT_GT(x, y) UTEST_EXPECT(x, y, >)
#define EXPECT_GE(x, y) UTEST_EXPECT(x, y, >=)

#define EXPECT_STREQ(x, y)                                                     \
  if (0 != strcmp(x, y)) {                                                     \
    printf("%s:%u: Failure\n", __FILE__, __LINE__);                            \
    printf("  Expected : \"%s\"\n", x);                                        \
    printf("    Actual : \"%s\"\n", y);                                        \
    *utest_result = 1;                                                         \
  }

#define EXPECT_STRNE(x, y)                                                     \
  if (0 == strcmp(x, y)) {                                                     \
    printf("%s:%u: Failure\n", __FILE__, __LINE__);                            \
    printf("  Expected : \"%s\"\n", x);                                        \
    printf("    Actual : \"%s\"\n", y);                                        \
    *utest_result = 1;                                                         \
  }

#define UTEST_ASSERT(x, y, cond)                                               \
  UTEST_EXPECT(x, y, cond);                                                    \
  if (!((x)cond(y))) {                                                         \
    return;                                                                    \
  }

#define ASSERT_TRUE(x)                                                         \
  EXPECT_TRUE(x);                                                              \
  if (!(x)) {                                                                  \
    return;                                                                    \
  }

#define ASSERT_FALSE(x)                                                        \
  EXPECT_FALSE(x);                                                             \
  if (x) {                                                                     \
    return;                                                                    \
  }

#define ASSERT_EQ(x, y) UTEST_ASSERT(x, y, ==)
#define ASSERT_NE(x, y) UTEST_ASSERT(x, y, !=)
#define ASSERT_LT(x, y) UTEST_ASSERT(x, y, <)
#define ASSERT_LE(x, y) UTEST_ASSERT(x, y, <=)
#define ASSERT_GT(x, y) UTEST_ASSERT(x, y, >)
#define ASSERT_GE(x, y) UTEST_ASSERT(x, y, >=)

#define ASSERT_STREQ(x, y)                                                     \
  EXPECT_STREQ(x, y);                                                          \
  if (0 != strcmp(x, y)) {                                                     \
    return;                                                                    \
  }

#define ASSERT_STRNE(x, y)                                                     \
  EXPECT_STRNE(x, y);                                                          \
  if (0 == strcmp(x, y)) {                                                     \
    return;                                                                    \
  }

#define UTEST(SET, NAME)                                                       \
  UTEST_EXTERN struct utest_state_s utest_state;                               \
  static void utest_run_##SET##_##NAME(int *utest_result);                     \
  UTEST_INITIALIZER(utest_register_##SET##_##NAME) {                           \
    const size_t index = utest_state.tests_length++;                           \
    utest_state.tests =                                                        \
        UTEST_PTR_CAST(struct utest_test_state_s *,                            \
                       realloc(UTEST_PTR_CAST(void *, utest_state.tests),      \
                               sizeof(struct utest_test_state_s) *             \
                                   utest_state.tests_length));                 \
    utest_state.tests[index].func = &utest_run_##SET##_##NAME;                 \
    utest_state.tests[index].name = #SET "." #NAME;                            \
  }                                                                            \
  void utest_run_##SET##_##NAME(int *utest_result)

#define UTEST_F_SETUP(FIXTURE)                                                 \
  static void utest_setup_##FIXTURE(int *utest_result, struct FIXTURE *fixture)

#define UTEST_F_TEARDOWN(FIXTURE)                                              \
  static void utest_teardown_##FIXTURE(int *utest_result,                      \
                                       struct FIXTURE *fixture)

#define UTEST_F(FIXTURE, NAME)                                                 \
  UTEST_EXTERN struct utest_state_s utest_state;                               \
  static void utest_run_##FIXTURE##_##NAME(int *, struct FIXTURE *);           \
  static void utest_fixture_##FIXTURE##_##NAME(int *utest_result) {            \
    struct FIXTURE fixture = {0};                                              \
    utest_setup_##FIXTURE(utest_result, &fixture);                             \
    if (0 != *utest_result) {                                                  \
      return;                                                                  \
    }                                                                          \
    utest_run_##FIXTURE##_##NAME(utest_result, &fixture);                      \
    utest_teardown_##FIXTURE(utest_result, &fixture);                          \
  }                                                                            \
  UTEST_INITIALIZER(utest_register_##FIXTURE##_##NAME) {                       \
    const size_t index = utest_state.tests_length++;                           \
    utest_state.tests =                                                        \
        UTEST_PTR_CAST(struct utest_test_state_s *,                            \
                       realloc(UTEST_PTR_CAST(void *, utest_state.tests),      \
                               sizeof(struct utest_test_state_s) *             \
                                   utest_state.tests_length));                 \
    utest_state.tests[index].func = &utest_fixture_##FIXTURE##_##NAME;         \
    utest_state.tests[index].name = #FIXTURE "." #NAME;                        \
  }                                                                            \
  void utest_run_##FIXTURE##_##NAME(int *utest_result, struct FIXTURE *fixture)

// extern to the global state utest needs to execute
UTEST_EXTERN struct utest_state_s utest_state;

UTEST_WEAK int utest_should_filter_test(const char *filter,
                                        const char *testcase);
UTEST_WEAK int utest_should_filter_test(const char *filter,
                                        const char *testcase) {
  if (filter) {
    const char *filter_cur = filter;
    const char *testcase_cur = testcase;
    const char *filter_wildcard = 0;

    while (('\0' != *filter_cur) && ('\0' != *testcase_cur)) {
      if ('*' == *filter_cur) {
        // store the position of the wildcard
        filter_wildcard = filter_cur;

        // skip the wildcard character
        filter_cur++;

        while (('\0' != *filter_cur) && ('\0' != *testcase_cur)) {
          if ('*' == *filter_cur) {
            // we found another wildcard (filter is something like *foo*) so we
            // exit the current loop, and return to the parent loop to handle
            // the wildcard case
            break;
          } else if (*filter_cur != *testcase_cur) {
            // otherwise our filter didn't match, so reset it
            filter_cur = filter_wildcard;
          }

          // move testcase along
          testcase_cur++;

          // move filter along
          filter_cur++;
        }

        if (('\0' == *filter_cur) && ('\0' == *testcase_cur)) {
          return 0;
        }

        // if the testcase has been exhausted, we don't have a match!
        if ('\0' == *testcase_cur) {
          return 1;
        }
      } else {
        if (*testcase_cur != *filter_cur) {
          // test case doesn't match filter
          return 1;
        } else {
          // move our filter and testcase forward
          testcase_cur++;
          filter_cur++;
        }
      }
    }

    if (('\0' != *filter_cur) ||
        (('\0' != *testcase_cur) &&
         ((filter == filter_cur) || ('*' != filter_cur[-1])))) {
      // we have a mismatch!
      return 1;
    }
  }

  return 0;
}

UTEST_WEAK int utest_main(int argc, const char *const argv[]);
UTEST_WEAK int utest_main(int argc, const char *const argv[]) {
  size_t failed = 0;
  size_t index = 0;
  size_t *failed_testcases = 0;
  size_t failed_testcases_length = 0;
  const char *filter = 0;
  size_t ran_tests = 0;

  // loop through all arguments looking for our options
  for (index = 1; index < UTEST_CAST(size_t, argc); index++) {
    const char filter_str[] = "--filter=";

    if (0 < strcmp(argv[index], filter_str)) {
      // user wants to filter what test cases run!
      filter = argv[index] + strlen(filter_str);
    }
  }

  for (index = 0; index < utest_state.tests_length; index++) {
    if (utest_should_filter_test(filter, utest_state.tests[index].name)) {
      continue;
    }

    ran_tests++;
  }

  printf("\033[32m[==========]\033[0m Running %" UTEST_PRIu64 " test cases.\n",
         UTEST_CAST(uint64_t, ran_tests));

  for (index = 0; index < utest_state.tests_length; index++) {
    int result = 0;
    int64_t ns = 0;

    if (utest_should_filter_test(filter, utest_state.tests[index].name)) {
      continue;
    }

    printf("\033[32m[ RUN      ]\033[0m %s\n", utest_state.tests[index].name);
    ns = utest_ns();
    utest_state.tests[index].func(&result);
    ns = utest_ns() - ns;
    if (0 != result) {
      const size_t failed_testcase_index = failed_testcases_length++;
      failed_testcases = UTEST_PTR_CAST(
          size_t *, realloc(UTEST_PTR_CAST(void *, failed_testcases),
                            sizeof(size_t) * failed_testcases_length));
      failed_testcases[failed_testcase_index] = index;
      failed++;
      printf("\033[31m[  FAILED  ]\033[0m %s (%" UTEST_PRId64 "ns)\n",
             utest_state.tests[index].name, ns);
    } else {
      printf("\033[32m[       OK ]\033[0m %s (%" UTEST_PRId64 "ns)\n",
             utest_state.tests[index].name, ns);
    }
  }
  printf("\033[32m[==========]\033[0m %" UTEST_PRIu64 " test cases ran.\n",
         UTEST_CAST(uint64_t, ran_tests));
  printf("\033[32m[  PASSED  ]\033[0m %" UTEST_PRIu64 " tests.\n",
         UTEST_CAST(uint64_t, ran_tests - failed));
  if (0 != failed) {
    printf("\033[31m[  FAILED  ]\033[0m %" UTEST_PRIu64
           " tests, listed below:\n",
           UTEST_CAST(uint64_t, failed));
    for (index = 0; index < failed_testcases_length; index++) {
      printf("\033[31m[  FAILED  ]\033[0m %s\n",
             utest_state.tests[failed_testcases[index]].name);
    }
  }
  free(UTEST_PTR_CAST(void *, failed_testcases));
  free(UTEST_PTR_CAST(void *, utest_state.tests));
  return UTEST_CAST(int, failed);
}

// we need, in exactly one source file, define the global struct that will hold
// the data we need to run utest. This macro allows the user to declare the
// data without having to use the UTEST_MAIN macro, thus allowing them to write
// their own main() function.
#define UTEST_STATE() struct utest_state_s utest_state

// define a main() function to call into utest.h and start executing tests! A
// user can optionally not use this macro, and instead define their own main()
// function and manually call utest_main. The user must, in exactly one source
// file, use the UTEST_STATE macro to declare a global struct variable that
// utest requires.
#define UTEST_MAIN()                                                           \
  UTEST_STATE();                                                               \
  int main(int argc, const char *const argv[]) {                               \
    return utest_main(argc, argv);                                             \
  }

#endif // SHEREDOM_UTEST_H_INCLUDED
