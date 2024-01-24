#include <stdlib.h>
#include <inttypes.h>

#include "test.h"
/*-------------------------------------------------------------------------------------------------
 *
 *                                       Main - Run the tests
 *
 *-----------------------------------------------------------------------------------------------*/
int
main(int argc, char *argv[])
{
    fprintf(stderr, "\n\n***        Starting Tests.        ***\n\n");
    coy_profile_begin();

    fprintf(stderr, "coyote_time_tests()..");
    CoyProfileAnchor ap = COY_START_PROFILE_BLOCK("time_tests");
    coyote_time_tests();
    COY_END_PROFILE(ap);
    fprintf(stderr, ".complete.\n");

    fprintf(stderr, "coyote_file_tests()..");
    ap = COY_START_PROFILE_BLOCK("file_tests");
    coyote_file_tests();
    COY_END_PROFILE(ap);
    fprintf(stderr, ".complete.\n");

    fprintf(stderr, "coyote_file_name_iterator_tests()..");
    ap = COY_START_PROFILE_BLOCK("file_name_iterator_tests");
    coyote_file_name_iterator_tests();
    COY_END_PROFILE(ap);
    fprintf(stderr, ".complete.\n");

    fprintf(stderr, "coyote_memory_tests()..");
    ap = COY_START_PROFILE_BLOCK("memory_tests");
    coyote_memory_tests();
    COY_END_PROFILE(ap);
    fprintf(stderr, ".complete.\n");

    fprintf(stderr, "coyote_terminal_tests()..");
    ap = COY_START_PROFILE_BLOCK("terminal_tests");
    coyote_terminal_tests();
    COY_END_PROFILE(ap);
    fprintf(stderr, ".complete.\n");

    fprintf(stderr, "coyote_threads_tests()..");
    ap = COY_START_PROFILE_BLOCK("threads");
    coyote_threads_tests();
    COY_END_PROFILE(ap);
    fprintf(stderr, ".complete.\n");

    coy_profile_end();
    fprintf(stderr, "\n\n*** Tests completed successfully. ***\n\n");

#if COY_PROFILE
    printf("Total Runtime = %.3lf seconds at a frequency of %"PRIu64"\n",
            coy_global_profiler.total_elapsed, coy_global_profiler.freq);

    for(i32 i = 0; i < COY_PROFILE_NUM_BLOCKS; ++i)
    {
        CoyBlockProfiler *block = &coy_global_profiler.blocks[i];
        if(block->hit_count)
        {
            printf("%-28s Hits: %3"PRIu64" Exclusive: %6.2lf%%", block->label, block->hit_count, block->exclusive_pct);
            
            if(block->inclusive_pct != block->exclusive_pct)
            {
                printf(" Inclusive: %6.2lf%%\n", block->inclusive_pct);
            }
            else
            {
                printf("\n");
            }
        }
    }
#endif
    return EXIT_SUCCESS;
}

#include "fileio.c"
#include "file_name_iterator.c"
#include "memory.c"
#include "terminal.c"
#include "threads.c"
#include "time.c"

