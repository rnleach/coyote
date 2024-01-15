#ifndef _COYOTE_LINUX_APPLE_OSX_COMMON_H_
#define _COYOTE_LINUX_APPLE_OSX_COMMON_H_
/*---------------------------------------------------------------------------------------------------------------------------
 *                                         Apple/MacOSX Linux Common Implementation
 *-------------------------------------------------------------------------------------------------------------------------*/

#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

static inline u64
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

static char const coy_path_sep = '/';

static inline bool 
coy_path_append(size buf_len, char path_buffer[], char const *new_path)
{
    // Find first '\0'
    size position = 0;
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

static inline CoyFile
coy_file_create(char const *filename)
{
    int fd = open( filename,                                        // char const *pathname
                   O_WRONLY | O_CREAT | O_TRUNC,                    // Write only, create if needed, or truncate if needed.
                   S_IRWXU | S_IRGRP | S_IXGRP |S_IROTH | S_IXOTH); // Default permissions 0755

    if (fd >= 0)
    {
        return (CoyFile){ .handle = (iptr) fd, .valid = true  };
    }
    else
    {
        return (CoyFile){ .handle = (iptr) fd, .valid = false };
    }
}

static inline 
CoyFile coy_file_append(char const *filename)
{
    int fd = open( filename,                                        // char const *pathname
                   O_WRONLY | O_CREAT | O_APPEND,                   // Write only, create if needed, and append.
                   S_IRWXU | S_IRGRP | S_IXGRP |S_IROTH | S_IXOTH); // Default permissions 0755

    if (fd >= 0) {
        return (CoyFile){ .handle = (iptr) fd, .valid = true  };
    }
    else
    {
        return (CoyFile){ .handle = (iptr) fd, .valid = false };
    }
}

static inline size 
coy_file_write(CoyFile *file, size nbytes_to_write, byte const *buffer)
{
    StopIf(!file->valid, goto ERR_RETURN);

    _Static_assert(sizeof(ssize_t) == sizeof(size), "oh come on people. ssize_t != intptr_t!? Really!");
    Assert(nbytes_to_write >= 0);

    ssize_t num_bytes_written = write((int)file->handle, buffer, nbytes_to_write);
    StopIf(num_bytes_written < 0, goto ERR_RETURN);
    return (size) num_bytes_written;

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
        return (CoyFile){ .handle = (iptr) fd, .valid = true  };
    }
    else
    {
        return (CoyFile){ .handle = (iptr) fd, .valid = false };
    }
}

static inline size 
coy_file_read(CoyFile *file, size buf_size, byte *buffer)
{
    _Static_assert(sizeof(ssize_t) <= sizeof(size), "oh come on people. ssize_t != intptr_t!? Really!");
    Assert(buf_size > 0);

    StopIf(!file->valid, goto ERR_RETURN);

    size bytes_remaining = buf_size;
    ssize_t num_bytes_read = 0;
    ssize_t total_num_bytes_read = 0;
    while(bytes_remaining)
    {
        num_bytes_read = read((int)file->handle, buffer + total_num_bytes_read, bytes_remaining);
        bytes_remaining -= num_bytes_read;
        total_num_bytes_read += num_bytes_read;

        StopIf(num_bytes_read < 0, goto ERR_RETURN);

        if(num_bytes_read == 0) { break; }
    }

    return (size) total_num_bytes_read;

ERR_RETURN:
    return -1;
}

static inline void 
coy_file_close(CoyFile *file)
{
    /* int err_code = */ close((int)file->handle);
    file->valid = false;
}

static inline size 
coy_file_size(char const *filename)
{
    _Static_assert(sizeof(off_t) == 8 && sizeof(size) == sizeof(off_t), "Need 64-bit off_t");
    struct stat statbuf = {0};
    int success = stat(filename, &statbuf);
    StopIf(success != 0, return -1);

    return (size)statbuf.st_size;
}

static inline CoyMemMappedFile 
coy_memmap_read_only(char const *filename)
{
    _Static_assert(sizeof(usize) == sizeof(size), "sizeof(size_t) != sizeof(intptr_t)");

    size size_in_bytes = coy_file_size(filename);
    StopIf(size_in_bytes == -1, goto ERR_RETURN);

    int fd = open( filename, // char const *pathname
                   O_RDONLY, // Read only
                   0);       // No mode information needed.
    StopIf(fd < 0, goto ERR_RETURN);

    byte const *data = mmap(NULL,           // Starting address - OS chooses
                            size_in_bytes,  // The size of the mapping,should be the file size for this
                            PROT_READ,      // Read only access
                            MAP_PRIVATE,    // flags - not sure if anything is necessary for read only
                            fd,             // file descriptor for the file to map
                            0);             // offset into the file to read

    close(fd);
    StopIf(data == MAP_FAILED, goto ERR_RETURN);

    return (CoyMemMappedFile){ .size_in_bytes = (size)size_in_bytes, 
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

static inline CoyFileNameIter 
coy_file_name_iterator_open(char const *directory_path, char const *file_extension)
{
    DIR *d = opendir(directory_path);
    StopIf(!d, goto ERR_RETURN);

    return (CoyFileNameIter){ .os_handle = (iptr)d, .file_extension=file_extension, .valid=true};

ERR_RETURN:
    return (CoyFileNameIter) {.valid=false};
}

static inline char const *
coy_file_name_iterator_next(CoyFileNameIter *cfni)
{
    if(cfni->valid)
    {
        DIR *d = (DIR *)cfni->os_handle;
        struct dirent *entry = readdir(d);
        while(entry)
        {
            if(entry->d_type == DT_REG) {
                if(cfni->file_extension == NULL) 
                {
                    return entry->d_name;
                }
                else 
                {
                    char const *ext = coy_file_extension(entry->d_name);
                    if(coy_null_term_strings_equal(ext, cfni->file_extension))
                    {
                        return entry->d_name;
                    }
                }
            }
            entry = readdir(d);
        }
    }

    cfni->valid = false;
    return NULL;
}

static inline void 
coy_file_name_iterator_close(CoyFileNameIter *cfin)
{
    DIR *d = (DIR *)cfin->os_handle;
    /*int rc = */ closedir(d);
    *cfin = (CoyFileNameIter){0};
    return;
}

static inline CoyThread
coy_thread_create(CoyThreadFunc func, void *thread_data)
{
    pthread_t thread = {0};
    int status = pthread_create(&thread, NULL, func, thread_data);
    if(status == 0)
    {
        return (CoyThread){ .thread = thread, .ret_val = NULL, .valid = true };
    }
    else
    {
        return (CoyThread){ .valid = false };
    }
}

static inline bool
coy_thread_join(CoyThread *thread)
{

    int status = pthread_join(thread->thread, &thread->ret_val);
    if(status == 0) { return true; }
    return false;
}

static inline void 
coy_destroy_thread(CoyThread *thread)
{
    thread->valid = false;
}

static inline CoyMutex 
coy_mutex_create()
{
    pthread_mutex_t mut = {0};
    int status = pthread_mutex_init(&mut, NULL);
    if(status == 0)
    {
        return (CoyMutex){ .mutex=mut, .valid=true };
    }

    return (CoyMutex){ .valid=false };
}

static inline bool 
coy_mutex_lock(CoyMutex *mutex)
{
    int status = pthread_mutex_lock(&mutex->mutex);
    if(status == 0) { return true; }
    return false;
}

static inline bool 
coy_mutex_unlock(CoyMutex *mutex)
{
    int status = pthread_mutex_unlock(&mutex->mutex);
    if(status == 0) { return true; }
    return false;
}

#endif
