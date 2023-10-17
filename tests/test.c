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

    coyote_time_tests();

    printf("\n\n*** Tests completed successfully. ***\n\n");
    return EXIT_SUCCESS;
}
