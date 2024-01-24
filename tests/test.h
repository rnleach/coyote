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
#include "../build/coyote.h"

void coyote_time_tests(void);
void coyote_file_tests(void);
void coyote_file_name_iterator_tests(void);
void coyote_memory_tests(void);
void coyote_terminal_tests(void);
void coyote_threads_tests(void);

static char const *test_data_dir = "tmp_output";

#endif
