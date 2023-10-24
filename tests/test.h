#ifndef _COYOTE_TESTS_
#define _COYOTE_TESTS_
//
// For testing, ensure we have some debugging tools activated.
//

// We must have asserts working for the tests to work.
#ifdef NDEBUG
#    undef NDEBUG
#endif

#include <stdio.h>
#include "../src/coyote.h"

void coyote_time_tests(void);
void coyote_file_tests(void);
void coyote_file_name_iterator_tests(void);
void coyote_memory_tests(void);

#if defined(_WIN32) || defined(_WIN64)
static char const *test_data_dir = "tmp_output";
#elif defined(__linux__) || defined(__APPLE__)
static char const *test_data_dir = "tmp_output";
#else
  #error "Platform not supported by Coyote Library"
#endif

#endif
