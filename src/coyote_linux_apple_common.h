#ifndef _COYOTE_LINUX_APPLE_OSX_COMMON_H_
#define _COYOTE_LINUX_APPLE_OSX_COMMON_H_
/*---------------------------------------------------------------------------------------------------------------------------
 *                                         Apple/MacOSX Linux Common Implementation
 *-------------------------------------------------------------------------------------------------------------------------*/

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

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
    int fd = open( filename,                                        // char const *pathname
                   O_WRONLY | O_CREAT | O_TRUNC,                    // Write only, create if needed, or truncate if needed.
                   S_IRWXU | S_IRGRP | S_IXGRP |S_IROTH | S_IXOTH); // Default permissions 0755

    if (fd >= 0)
    {
        return (CoyFile){ .handle = (intptr_t) fd, .valid = true  };
    }
    else
    {
        return (CoyFile){ .handle = (intptr_t) fd, .valid = false };
    }
}

static inline 
CoyFile coy_file_append(char const *filename)
{
    int fd = open( filename,                                        // char const *pathname
                   O_WRONLY | O_CREAT | O_APPEND,                   // Write only, create if needed, and append.
                   S_IRWXU | S_IRGRP | S_IXGRP |S_IROTH | S_IXOTH); // Default permissions 0755

    if (fd >= 0) {
        return (CoyFile){ .handle = (intptr_t) fd, .valid = true  };
    }
    else
    {
        return (CoyFile){ .handle = (intptr_t) fd, .valid = false };
    }
}

static inline intptr_t 
coy_file_write(CoyFile *file, intptr_t nbytes_to_write, unsigned char *buffer)
{
    StopIf(!file->valid, goto ERR_RETURN);

    _Static_assert(sizeof(ssize_t) == sizeof(intptr_t), "oh come on people. ssize_t != intptr_t!? Really!");
    Assert(nbytes_to_write >= 0);

    ssize_t num_bytes_written = write((int)file->handle, buffer, nbytes_to_write);
    StopIf(num_bytes_written < 0, goto ERR_RETURN);
    return (intptr_t) num_bytes_written;

ERR_RETURN:
    return -1;
}

static inline CoyFile 
coy_file_open_read(char const *filename)
{
    int fd = open( filename, // char const *pathname
                   O_RDONLY, // Read only
                   0);       // No mode information needed.

    if (fd >= 0)
    {
        return (CoyFile){ .handle = (intptr_t) fd, .valid = true  };
    }
    else
    {
        return (CoyFile){ .handle = (intptr_t) fd, .valid = false };
    }
}

static inline intptr_t 
coy_file_read(CoyFile *file, intptr_t buf_size, unsigned char *buffer)
{
    _Static_assert(sizeof(ssize_t) == sizeof(intptr_t), "oh come on people. ssize_t != intptr_t!? Really!");
    Assert(buf_size > 0);

    StopIf(!file->valid, goto ERR_RETURN);
    ssize_t num_bytes_read = read((int)file->handle, buffer, buf_size);
    StopIf(num_bytes_read < 0, goto ERR_RETURN);
    return (intptr_t) num_bytes_read;

ERR_RETURN:
    return -1;
}

static inline void 
coy_file_close(CoyFile *file)
{
    /* int err_code = */ close((int)file->handle);
    file->valid = false;
}

static inline intptr_t 
coy_file_size(char const *filename)
{
    _Static_assert(sizeof(off_t) == 8 && sizeof(intptr_t) == sizeof(off_t), "Need 64-bit off_t");
    struct stat statbuf = {0};
    int success = stat(filename, &statbuf);
    StopIf(success != 0, return -1);

    return (intptr_t)statbuf.st_size;
}

static inline CoyMemMappedFile 
coy_memmap_read_only(char const *filename)
{
    _Static_assert(sizeof(size_t) == sizeof(intptr_t), "sizeof(size_t) != sizeof(intptr_t)");

    intptr_t size_in_bytes = coy_file_size(filename);
    StopIf(size_in_bytes == -1, goto ERR_RETURN);

    int fd = open( filename, // char const *pathname
                   O_RDONLY, // Read only
                   0);       // No mode information needed.
    StopIf(fd < 0, goto ERR_RETURN);

    unsigned char const *data = mmap(NULL,           // Starting address - OS chooses
                                     size_in_bytes,  // The size of the mapping,should be the file size for this
                                     PROT_READ,      // Read only access
                                     MAP_PRIVATE,    // flags - not sure if anything is necessary for read only
                                     fd,             // file descriptor for the file to map
                                     0);             // offset into the file to read

    close(fd);
    StopIf(data == MAP_FAILED, goto ERR_RETURN);

    return (CoyMemMappedFile){ .size_in_bytes = (intptr_t)size_in_bytes, 
                               .data = data, 
                               ._internal = {0}, 
                               .valid = true 
    };

ERR_RETURN:
    return (CoyMemMappedFile) { .valid = false };
}

static inline void 
coy_memmap_close(CoyMemMappedFile *file)
{
    /*BOOL success = */ munmap((void *)file->data, file->size_in_bytes);
    file->valid = false;

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
    Assert(minimum_num_bytes > 0);

    long page_size = sysconf(_SC_PAGESIZE);
    StopIf(page_size == -1, goto ERR_RETURN);
    size_t nbytes = minimum_num_bytes + page_size - (minimum_num_bytes % page_size);

    void *ptr = mmap(NULL,                     // the starting address, NULL = don't care
                     nbytes,                   // the amount of memory to allocate
                     PROT_READ | PROT_WRITE,   // we should have read and write access to the memory
                     MAP_PRIVATE | MAP_ANON,   // not backed by a file, this is pure memory
                     -1,                       // recommended file descriptor for portability
                     0);                       // offset into what? this isn't a file, so should be zero

    StopIf(ptr == MAP_FAILED, goto ERR_RETURN);

    return (CoyMemoryBlock){ .mem = ptr, .size = nbytes, .valid = true };

ERR_RETURN:
    return (CoyMemoryBlock){ .mem = NULL, .size = 0, .valid = false};
}

static inline void
coy_memory_free(CoyMemoryBlock *mem)
{
    /* int success = */ munmap(mem->mem, (size_t)mem->size);
    mem->valid = false;
    return;
}

#endif
