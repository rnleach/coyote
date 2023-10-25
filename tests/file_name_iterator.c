#include "test.h"

int strcmp(const char *s1, const char *s2);
/*--------------------------------------------------------------------------------------------------------------------------
 *
 *                                               Tests for FileNameIterator
 *
 *-------------------------------------------------------------------------------------------------------------------------*/
static void
test_file_name_iterator(void)
{
    char const *src_files[] = 
    {"coyote.h", "coyote_win32.h", "coyote_apple_osx.h", "coyote_linux.h", "coyote_linux_apple_common.h"};

    CoyFileNameIter iter_ = coy_file_name_iterator_open("src", NULL);
    CoyFileNameIter *iter = &iter_;

    int count = 0;
    const char *fname = coy_file_name_iterator_next(iter);
    while(fname)
    {
        bool found = false;
        char const *f = 0;
        for(int i = 0; i < sizeof(src_files) / sizeof(src_files[0]); ++i)
        {
            f = src_files[i];
            if(strcmp(fname, f) == 0)
            {
                found = true;
                break;
            }
        }

        Assert(found);
        count += 1;

        fname = coy_file_name_iterator_next(iter);
    }

    Assert(count == sizeof(src_files) / sizeof(src_files[0]));

    coy_file_name_iterator_close(iter);
    Assert(iter->os_handle == 0 && iter->file_extension == NULL && iter->valid == false);
}

static void
test_file_name_iterator_filtering(void)
{
    char const *readme = "README.md"; 

    CoyFileNameIter iter_ = coy_file_name_iterator_open(".", "md");
    CoyFileNameIter *iter = &iter_;

    const char *fname = coy_file_name_iterator_next(iter);
    int count = 0;
    while(fname)
    {
        Assert(strcmp(fname, readme) == 0);

        count += 1;
        fname = coy_file_name_iterator_next(iter);
    }

    Assert(count == 1);

    coy_file_name_iterator_close(iter);
    Assert(iter->os_handle == 0 && iter->file_extension == NULL && iter->valid == false);
}

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                    All file IO tests
 *-------------------------------------------------------------------------------------------------------------------------*/
void
coyote_file_name_iterator_tests(void)
{
    test_file_name_iterator();
    test_file_name_iterator_filtering();
}
