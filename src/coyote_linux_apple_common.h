#ifndef _COYOTE_LINUX_APPLE_OSX_COMMON_H_
#define _COYOTE_LINUX_APPLE_OSX_COMMON_H_
/*---------------------------------------------------------------------------------------------------------------------------
 *                                         Apple/MacOSX Linux Common Implementation
 *-------------------------------------------------------------------------------------------------------------------------*/

#include <sys/time.h>

static inline uint64_t
coy_current_time(void)
{
    struct timeval tv = {0};
    int errcode = gettimeofday(&tv, NULL);

    StopIf(errcode != 0, goto ERR_RETURN);
    StopIf(tv.tv_sec < 0, goto ERR_RETURN);

    return tv.tv_sec;

ERR_RETURN:
    return UINT64_MAX;
}

static inline CoyFile
coy_create_file(char const *filename)
{
    CoyFile result = {0};
    // TODO implement
    Assert(false);
    return result;
}

static inline 
CoyFile coy_append_to_file(char const *filename)
{
    CoyFile result = {0};
    // TODO implement
    Assert(false);
    return result;
}

static inline intptr_t 
coy_write_to_file(CoyFile *file, intptr_t nbytes_to_write, unsigned char *buffer)
{
    // TODO implement
    Assert(false);
    return 0;
}

static inline CoyFile 
coy_open_file_read(char const *filename)
{
    CoyFile result = {0};
    // TODO implement
    Assert(false);
    return result;
}

static inline intptr_t 
coy_read_from_file(CoyFile *file, intptr_t buf_size, unsigned char *buffer)
{
    // TODO implement
    Assert(false);
    return 0;
}

static inline intptr_t 
coy_file_size(char const *filename)
{
    // TODO implement
    Assert(false);
    return 0;
}

static inline CoyMemMappedFile 
coy_memmap_read_only(char const *filename)
{
    CoyMemMappedFile result = {0};
    // TODO implement
    Assert(false);
    return result;
}

static inline void 
coy_memmap_close(CoyMemMappedFile *file)
{
    // TODO implement
    Assert(false);
    return;
}

static char const coy_path_sep = '/';

static inline bool 
coy_append_to_path(ptrdiff_t buf_len, char path_buffer[], char const *new_path)
{
    // TODO implement
    Assert(false);
    return false;
}

static inline CoyMemoryBlock 
coy_allocate_memory(int64_t minimum_num_bytes)
{
    CoyMemoryBlock result = {0};
    // TODO implement
    Assert(false);
    return result;
}

static inline void
coy_free_memory(CoyMemoryBlock *mem)
{
    // TODO implement
    Assert(false);
    return;
}

#endif
