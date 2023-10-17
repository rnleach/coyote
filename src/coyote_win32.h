#ifndef _COYOTE_WIN32_H_
#define _COYOTE_WIN32_H_
/*---------------------------------------------------------------------------------------------------------------------------
 *                                                 Windows Implementation
 *-------------------------------------------------------------------------------------------------------------------------*/
#include <windows.h>

union WinTimePun
{
    FILETIME ft;
    ULARGE_INTEGER as_uint64;
};

static inline uint64_t
coy_current_time(void)
{
    SYSTEMTIME now_st = {0};
    GetSystemTime(&now_st);

    union WinTimePun now = {0};
    bool success = SystemTimeToFileTime(&now_st, &now.ft);
    PanicIf(!success);

    SYSTEMTIME epoch_st = { 
        .wYear=1970, 
        .wMonth=1,
        .wDayOfWeek=4,
        .wDay=1,
        .wHour=0,
        .wMinute=0,
        .wSecond=0,
        .wMilliseconds=0
    };

    union WinTimePun epoch = {0};
    success = SystemTimeToFileTime(&epoch_st, &epoch.ft);
    PanicIf(!success);

    return (now.as_uint64.QuadPart - epoch.as_uint64.QuadPart) / 10000000;
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

static char const coy_path_sep = '\\';

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
