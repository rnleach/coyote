#include <stdio.h>

#include "../src/coyote.h"

void *memcpy(void *dest, const void *src, size_t n);

static inline bool
str_eq(char const *left, char const *right, intptr_t len)
{
    for (intptr_t i = 0; i < len; ++i)
    {
        if (left[i] != right[i]) { return false; }
    }

    return true;
}


static bool
load_file(char const *fname, intptr_t buf_size, char *buffer, intptr_t *read)
{
    intptr_t expected_size = coy_file_size(fname);
    StopIf(expected_size < 0, fprintf(stderr, "Error getting file size %s\n", fname); return false);

    CoyFile f_ = coy_file_open_read(fname);
    CoyFile *f = &f_;
    StopIf(!f->valid, fprintf(stderr, "Error opening file %s\n", fname); return false);

     *read = coy_file_read(f, buf_size, (unsigned char *)buffer);
    StopIf(*read != expected_size, fprintf(stderr, "File Size Mismatch %s\n", fname); return false);

    coy_file_close(f);

    return true;
}

static void
insert_buffer(intptr_t out_size, intptr_t *out_idx, char *out, intptr_t in_size, char *in)
{
    Assert(*out_idx + in_size < out_size);

    char *start = &out[*out_idx];
    memcpy(start, in, in_size);
    *out_idx += in_size;
}

int
main(int argc, char *argv[])
{
    char main_buffer[COY_KiB(100)] = {0};
    intptr_t mb_size = 0;
    char win32_buffer[COY_KiB(100)] = {0};
    intptr_t w32_size = 0;
    char apple_buffer[COY_KiB(20)] = {0};
    intptr_t ap_size = 0;
    char linux_buffer[COY_KiB(20)] = {0};
    intptr_t li_size = 0;
    char common_buffer[COY_KiB(100)] = {0};
    intptr_t co_size = 0;

    char finished_lib[COY_MiB(1)] = {0};

    // Load all the files
    char const *fname = "../src/coyote.h";
    bool success = load_file(fname, sizeof(main_buffer), main_buffer, &mb_size);
    StopIf(!success, return 1);

    char const *fname_win32 = "../src/coyote_win32.h";
    success = load_file(fname_win32, sizeof(win32_buffer), win32_buffer, &w32_size);
    StopIf(!success, return 1);

    char const *fname_apple = "../src/coyote_apple_osx.h";
    success = load_file(fname_apple, sizeof(apple_buffer), apple_buffer, &ap_size);
    StopIf(!success, return 1);

    char const *fname_linux = "../src/coyote_linux.h";
    success = load_file(fname_linux, sizeof(linux_buffer), linux_buffer, &li_size);
    StopIf(!success, return 1);

    char const *fname_common = "../src/coyote_linux_apple_common.h";
    success = load_file(fname_common, sizeof(common_buffer), common_buffer, &co_size);
    StopIf(!success, return 1);

    // Merge them in a buffer
    intptr_t oi = 0; //output index
    for(char *c = main_buffer; *c; ++c)
    {
        bool did_insert = false;
        if(*c == '#' && *(c + 1) == 'i' && *(c + 2) == 'n' && *(c + 3) == 'c' && *(c + 4) == 'l' && *(c + 5) == 'u')
        {
            if(str_eq("#include \"coyote_win32.h\"", c, 25))
            {
                insert_buffer(sizeof(finished_lib), &oi, finished_lib, w32_size, win32_buffer);
                c += 24;
                did_insert = true;
            }
            else if(str_eq("#include \"coyote_linux.h\"", c, 25))
            {
                insert_buffer(sizeof(finished_lib), &oi, finished_lib, li_size, linux_buffer);
                c += 24;
                did_insert = true;
            }
            else if(str_eq("#include \"coyote_apple_osx.h\"", c, 29))
            {
                insert_buffer(sizeof(finished_lib), &oi, finished_lib, ap_size, apple_buffer);
                c += 29;
                did_insert = true;
            }
            else if(str_eq("#include \"coyote_linux_apple_common.h\"", c, 38))
            {
                insert_buffer(sizeof(finished_lib), &oi, finished_lib, co_size, common_buffer);
                c += 38;
                did_insert = true;
            }
        }
        if(!did_insert)
        {
            finished_lib[oi++] = *c;
        }
    }

    // Write out the buffer
    CoyFile coyote_ = coy_file_create("coyote.h");
    CoyFile *coyote = &coyote_;
    StopIf(!coyote->valid, fprintf(stderr, "Error opening coyote.h for output\n"); return 1);

    intptr_t coyote_wrote = coy_file_write(coyote, oi, (unsigned char *)finished_lib);

    coy_file_close(coyote);
}
