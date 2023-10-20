#ifndef _COYOTE_LINUX_APPLE_OSX_COMMON_H_
#define _COYOTE_LINUX_APPLE_OSX_COMMON_H_
/*---------------------------------------------------------------------------------------------------------------------------
 *                                         Apple/MacOSX Linux Common Implementation
 *-------------------------------------------------------------------------------------------------------------------------*/

#include <sys/time.h>

static inline uint64_t
coy_time_now(void)
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
coy_file_create(char const *filename)
{
    CoyFile result = {0};
    // TODO implement
    Assert(false);
    return result;
}

static inline 
CoyFile coy_file_append(char const *filename)
{
    CoyFile result = {0};
    // TODO implement
    Assert(false);
    return result;
}

static inline intptr_t 
coy_file_write(CoyFile *file, intptr_t nbytes_to_write, unsigned char *buffer)
{
    // TODO implement
    Assert(false);
    return 0;
}

static inline CoyFile 
coy_file_open_read(char const *filename)
{
    CoyFile result = {0};
    // TODO implement
    Assert(false);
    return result;
}

static inline intptr_t 
coy_file_read(CoyFile *file, intptr_t buf_size, unsigned char *buffer)
{
    // TODO implement
    Assert(false);
    return 0;
}

static inline void 
coy_file_close(CoyFile *file)
{
    // TODO implement
    Assert(false);
    return;
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
coy_path_append(intptr_t buf_len, char path_buffer[], char const *new_path)
{
    // Find first '\0'
    intptr_t position = 0;
    char *c = path_buffer;
    while(position < buf_len && *c)
    {
	  ++c;
	  position += 1;
    }

    StopIf(position >= buf_len, goto ERR_RETURN);

    // Add a path separator - unless the buffer is empty or the last path character was a path separator.
    if(position > 0 && path_buffer[position - 1] != coy_path_sep)
    {
	  path_buffer[position] = coy_path_sep;
	  position += 1;
	  StopIf(position >= buf_len, goto ERR_RETURN);
    }

    // Copy in the new path part.
    char const *new_c = new_path;
    while(position < buf_len && *new_c)
    {
	  path_buffer[position] = *new_c;
	  ++new_c;
	  position += 1;
    }

    StopIf(position >= buf_len, goto ERR_RETURN);

    // Null terminate the path.
    path_buffer[position] = '\0';
    
    return true;

ERR_RETURN:
    path_buffer[buf_len - 1] = '\0';
    return false;
}

static inline CoyMemoryBlock 
coy_memory_allocate(intptr_t minimum_num_bytes)
{
    CoyMemoryBlock result = {0};
    // TODO implement
    Assert(false);
    return result;
}

static inline void
coy_memory_free(CoyMemoryBlock *mem)
{
    // TODO implement
    Assert(false);
    return;
}

#endif
