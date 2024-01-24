#include <stdlib.h>

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

    printf("coyote_time_tests()..");
    coyote_time_tests();
    printf(".complete.\n");

    printf("coyote_file_tests()..");
    coyote_file_tests();
    printf(".complete.\n");

    printf("coyote_file_name_iterator_tests()..");
    coyote_file_name_iterator_tests();
    printf(".complete.\n");

    printf("coyote_memory_tests()..");
    coyote_memory_tests();
    printf(".complete.\n");

    printf("coyote_terminal_tests()..");
    coyote_terminal_tests();
    printf(".complete.\n");

    printf("coyote_threads_tests()..");
    coyote_threads_tests();
    printf(".complete.\n");

    printf("\n\n*** Tests completed successfully. ***\n\n");
    return EXIT_SUCCESS;
}
