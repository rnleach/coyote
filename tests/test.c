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
    printf("\n\n***        Starting Tests.        ***\n\n");
    coy_profile_begin();

    printf("coyote_time_tests()..");
    CoyProfileAnchor ap = COY_START_PROFILE_BLOCK("time_tests");
    coyote_time_tests();
    COY_END_PROFILE(ap);
    printf(".complete.\n");

    printf("coyote_file_tests()..");
    ap = COY_START_PROFILE_BLOCK("file_tests");
    coyote_file_tests();
    COY_END_PROFILE(ap);
    printf(".complete.\n");

    printf("coyote_file_name_iterator_tests()..");
    ap = COY_START_PROFILE_BLOCK("file_name_iterator_tests");
    coyote_file_name_iterator_tests();
    COY_END_PROFILE(ap);
    printf(".complete.\n");

    printf("coyote_memory_tests()..");
    ap = COY_START_PROFILE_BLOCK("memory_tests");
    coyote_memory_tests();
    COY_END_PROFILE(ap);
    printf(".complete.\n");

    printf("coyote_terminal_tests()..");
    ap = COY_START_PROFILE_BLOCK("terminal_tests");
    coyote_terminal_tests();
    COY_END_PROFILE(ap);
    printf(".complete.\n");

    printf("coyote_threads_tests()..");
    ap = COY_START_PROFILE_BLOCK("threads");
    coyote_threads_tests();
    COY_END_PROFILE(ap);
    printf(".complete.\n");

    printf("\n\n*** Tests completed successfully. ***\n\n");
    coy_profile_end();

#if COY_PROFILE
    printf("Total Runtime = %.3lf seconds at a frequency of %"PRIu64"\n",
            coy_global_profiler.total_elapsed, coy_global_profiler.freq);

    for(i32 i = 1; i < COY_PROFILE_NUM_BLOCKS; ++i)
    {
        CoyBlockProfiler *block = &coy_global_profiler.blocks[i];
        if(block->hit_count)
        {
            printf("%-24s Hits: %3"PRIu64" Exclusive: %5.2lf%% Inclusive: %5.2lf%%\n", 
                    block->label, block->hit_count, block->exclusive_pct, block->inclusive_pct);
        }
    }
#endif
    return EXIT_SUCCESS;
}
