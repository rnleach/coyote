#ifndef _COYOTE_H_
#define _COYOTE_H_

#include <stdint.h>
#include <stddef.h>

/*---------------------------------------------------------------------------------------------------------------------------
 * Define simpler types.
 *-------------------------------------------------------------------------------------------------------------------------*/

// stdbool.h
#ifndef bool
  #define bool int
  #define false 0
  #define true 1
#endif

/* Other libraries I may have already included may use these exact definitions too. */
#ifndef _TYPE_ALIASES_
#define _TYPE_ALIASES_
typedef char       byte;
typedef ptrdiff_t  size;
typedef size_t    usize;

typedef uintptr_t  uptr;
typedef intptr_t   iptr;

typedef float       f32;
typedef double      f64;

typedef uint8_t      u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef int8_t       i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;
#endif

/*---------------------------------------------------------------------------------------------------------------------------
 * Declare parts of the standard C library I use. These should almost always be implemented as compiler intrinsics anyway.
 *-------------------------------------------------------------------------------------------------------------------------*/

void *memset(void *buffer, int val, size_t num_bytes);

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                       Error Handling
 *-------------------------------------------------------------------------------------------------------------------------*/

// Crash immediately, useful with a debugger!
#ifndef HARD_EXIT
  #define HARD_EXIT (*(int volatile*)0) 
#endif

#ifndef PanicIf
  #define PanicIf(assertion) StopIf((assertion), HARD_EXIT)
#endif

#ifndef Panic
  #define Panic() HARD_EXIT
#endif

#ifndef StopIf
  #define StopIf(assertion, error_action) if (assertion) { error_action; }
#endif

#ifndef Assert
  #ifndef NDEBUG
    #define Assert(assertion) if(!(assertion)) { HARD_EXIT; }
  #else
    #define Assert(assertion)
  #endif
#endif

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                      Date and Time
 *-------------------------------------------------------------------------------------------------------------------------*/
static inline u64 coy_time_now(void); // Get the current system time in seconds since midnight, Jan. 1 1970.

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                     Files & Paths
 *---------------------------------------------------------------------------------------------------------------------------
 * Check the 'valid' member of the structs to check for errors!
 */

// Append new path to the path in path_buffer, return true on success or false on error. path_buffer must be zero terminated.
static inline bool coy_path_append(size buf_len, char path_buffer[], char const *new_path);

typedef struct
{
    iptr handle; // posix returns an int and windows a HANDLE (e.g. void*), this should work for all of them.
    bool valid;  // error indicator
} CoyFile;

static inline CoyFile coy_file_create(char const *filename); // Truncate if it already exists, otherwise create it.
static inline CoyFile coy_file_append(char const *filename); // Create file if it doesn't exist yet, otherwise append.
static inline CoyFile coy_file_open_read(char const *filename);
static inline void coy_file_close(CoyFile *file); /* Must set valid member to false on success or failure! */
static inline size coy_file_size(char const *filename); /* size of a file in bytes, -1 on error. */

static inline size coy_file_write(CoyFile *file, size nbytes_write, byte const *buffer); // return nbytes written or -1 on error
static inline bool coy_file_write_f64(CoyFile *file, f64 val);
static inline bool coy_file_write_i8(CoyFile *file, i8 val);
static inline bool coy_file_write_i16(CoyFile *file, i16 val);
static inline bool coy_file_write_i32(CoyFile *file, i32 val);
static inline bool coy_file_write_i64(CoyFile *file, i64 val);
static inline bool coy_file_write_u8(CoyFile *file, u8 val);
static inline bool coy_file_write_u16(CoyFile *file, u16 val);
static inline bool coy_file_write_u32(CoyFile *file, u32 val);
static inline bool coy_file_write_u64(CoyFile *file, u64 val);
static inline bool coy_file_write_str(CoyFile *file, size len, char *str);

static inline size coy_file_read(CoyFile *file, size buf_size, byte *buffer); // return nbytes read or -1 on error
static inline bool coy_file_read_f64(CoyFile *file, f64 *val);
static inline bool coy_file_read_i8(CoyFile *file, i8 *val);
static inline bool coy_file_read_i16(CoyFile *file, i16 *val);
static inline bool coy_file_read_i32(CoyFile *file, i32 *val);
static inline bool coy_file_read_i64(CoyFile *file, i64 *val);
static inline bool coy_file_read_u8(CoyFile *file, u8 *val);
static inline bool coy_file_read_u16(CoyFile *file, u16 *val);
static inline bool coy_file_read_u32(CoyFile *file, u32 *val);
static inline bool coy_file_read_u64(CoyFile *file, u64 *val);

static inline bool coy_file_read_str(CoyFile *file, size *len, char *str); /* set len to buffer lenght, updated to actual size on return. */

// return size in bytes of the loaded data or -1 on error. If buffer is too small, load nothing and return -1
static inline size coy_file_slurp(char const *filename, size buf_size, byte *buffer);

typedef struct
{
    size size_in_bytes;     // size of the file
    byte const *data; 
    iptr _internal[2];      // implementation specific data
    bool valid;             // error indicator
} CoyMemMappedFile;

static inline CoyMemMappedFile coy_memmap_read_only(char const *filename);
static inline void coy_memmap_close(CoyMemMappedFile *file);

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                File System Interactions
 *---------------------------------------------------------------------------------------------------------------------------
 * Check the 'valid' member of the structs to check for errors!
 *
 * WARNING: NONE OF THESE ARE THREADSAFE.
 */

typedef struct
{
    iptr os_handle;         // for internal use only
    char const *file_extension;
    bool valid;
} CoyFileNameIter;

// Create an iterator. file_extension can be NULL if you want all files. Does not list directories. NOT THREADSAFE.
static inline CoyFileNameIter coy_file_name_iterator_open(char const *directory_path, char const *file_extension);

// Returns NULL when done. Copy the string if you need it, it will be overwritten on the next call. NOT THREADSAFE.
static inline char const *coy_file_name_iterator_next(CoyFileNameIter *cfni);
static inline void coy_file_name_iterator_close(CoyFileNameIter *cfin); // should leave the argument zeroed. NOT THREADSAFE.

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                         Memory
 *---------------------------------------------------------------------------------------------------------------------------
 * Request big chunks of memory from the OS, bypassing the CRT. The system may round up your requested memory size, but it
 * will return an error instead of rounding down if there isn't enough memory.
 */
typedef struct
{
    void *mem;
    size size;
    bool valid;
} CoyMemoryBlock;

#define COY_KB(a) ((a) * INT64_C(1000))
#define COY_MB(a) (COY_KB(a) * INT64_C(1000))
#define COY_GB(a) (COY_MB(a) * INT64_C(1000))
#define COY_TB(a) (COY_GB(a) * INT64_C(1000))

#define COY_KiB(a) ((a) * (1024))
#define COY_MiB(a) (COY_KiB(a) * INT64_C(1024))
#define COY_GiB(a) (COY_MiB(a) * INT64_C(1024))
#define COY_TiB(a) (COY_GiB(a) * INT64_C(1024))

static inline CoyMemoryBlock coy_memory_allocate(size minimum_num_bytes);
static inline void coy_memory_free(CoyMemoryBlock *mem);

/*---------------------------------------------------------------------------------------------------------------------------
 *                                        Multi-threading & Syncronization
 *---------------------------------------------------------------------------------------------------------------------------
 * Basic threading and syncronization.
 */

/* Windows requires a u32 return type while Linux (pthreads) requires a void*. Just return 0 or 1 to indicate success
 * and Coyote will cast it to the correct type for the API.
 */
#if defined(_WIN32) || defined(_WIN64)

typedef u32 CoyThreadFunReturnType;

typedef struct 
{
    /* Win32 HANDLE = void * */
    void *thread_handle;
    CoyThreadFunReturnType ret_val;
    u32 thread_id;
    bool valid;
} CoyThread;

typedef struct 
{
    /* Win32 HANDLE = void * */
    void *mutex;
    bool valid;
} CoyMutex;

#elif defined(__linux__) || defined(__APPLE__)
#include <pthread.h>

typedef void* CoyThreadFunReturnType;

typedef struct 
{
    pthread_t thread;
    CoyThreadFunReturnType ret_val;
    bool valid;
} CoyThread;

typedef struct 
{
    pthread_mutex_t mutex;
    bool valid;
} CoyMutex;

#else
#error "Platform not supported by Coyote Library"
#endif

typedef CoyThreadFunReturnType (*CoyThreadFunc)(void *thread_data);

static inline CoyThread coy_thread_create(CoyThreadFunc func, void *thread_data); /* Returns NULL on failure. */
static inline bool coy_thread_join(CoyThread *thread); /* Returns false if there was an error. */
static inline void coy_thread_destroy(CoyThread *thread);
#define coy_thread_get_result(thread) (thread).ret_val

static inline CoyMutex coy_mutex_create();
static inline bool coy_mutex_lock(CoyMutex *mutex);   /* Block, return false on failure. */
static inline bool coy_mutex_unlock(CoyMutex *mutex); /* Return false on failure. */

/*---------------------------------------------------------------------------------------------------------------------------
 *
 *
 *
 *                                          Implementation of `inline` functions.
 *                                                      Internal Only
 *
 *
 *
 *-------------------------------------------------------------------------------------------------------------------------*/

// assumes zero terminated string returned from OS - not for general use.
static inline char const *
coy_file_extension(char const *path)
{
    char const *extension = path;
    char const *next_char = path;
    while(*next_char)
    {
        if(*next_char == '.')
        {
            extension = next_char + 1;
        }
        ++next_char;
    }
    return extension;
}

// assumes zero terminated string returned from OS - not for general use.
static inline bool
coy_null_term_strings_equal(char const *left, char const *right)
{
    char const *l = left;
    char const *r = right;

    while(*l || *r)
    {
        if(*l != *r)
        {
            return false;
        }

        ++l;
        ++r;
    }

    return true;
}

static inline size 
coy_file_slurp(char const *filename, size buf_size, byte *buffer)
{
    size file_size = coy_file_size(filename);
    StopIf(file_size < 1 || file_size > buf_size, goto ERR_RETURN);

    CoyFile file = coy_file_open_read(filename);
    StopIf(!file.valid, goto ERR_RETURN);

    size num_bytes = coy_file_read(&file, buf_size, buffer);
    coy_file_close(&file);
    StopIf(num_bytes != file_size, goto ERR_RETURN);

    return num_bytes;

ERR_RETURN:
    return -1;
}

static inline bool 
coy_file_write_f64(CoyFile *file, f64 val)
{
    size nbytes = coy_file_write(file, sizeof(val), (byte *)&val);
    if(nbytes != sizeof(val)) { return false; }
    return true;
}

static inline bool 
coy_file_write_i8(CoyFile *file, i8 val)
{
    size nbytes = coy_file_write(file, sizeof(val), (byte *)&val);
    if(nbytes != sizeof(val)) { return false; }
    return true;
}

static inline bool 
coy_file_write_i16(CoyFile *file, i16 val)
{
    size nbytes = coy_file_write(file, sizeof(val), (byte *)&val);
    if(nbytes != sizeof(val)) { return false; }
    return true;
}

static inline bool 
coy_file_write_i32(CoyFile *file, i32 val)
{
    size nbytes = coy_file_write(file, sizeof(val), (byte *)&val);
    if(nbytes != sizeof(val)) { return false; }
    return true;
}

static inline bool 
coy_file_write_i64(CoyFile *file, i64 val)
{
    size nbytes = coy_file_write(file, sizeof(val), (byte *)&val);
    if(nbytes != sizeof(val)) { return false; }
    return true;
}

static inline bool 
coy_file_write_u8(CoyFile *file, u8 val)
{
    size nbytes = coy_file_write(file, sizeof(val), (byte *)&val);
    if(nbytes != sizeof(val)) { return false; }
    return true;
}

static inline bool 
coy_file_write_u16(CoyFile *file, u16 val)
{
    size nbytes = coy_file_write(file, sizeof(val), (byte *)&val);
    if(nbytes != sizeof(val)) { return false; }
    return true;
}

static inline bool 
coy_file_write_u32(CoyFile *file, u32 val)
{
    size nbytes = coy_file_write(file, sizeof(val), (byte *)&val);
    if(nbytes != sizeof(val)) { return false; }
    return true;
}

static inline bool 
coy_file_write_u64(CoyFile *file, u64 val)
{
    size nbytes = coy_file_write(file, sizeof(val), (byte *)&val);
    if(nbytes != sizeof(val)) { return false; }
    return true;
}

static inline bool 
coy_file_write_str(CoyFile *file, size len, char *str)
{
    _Static_assert(sizeof(size) == sizeof(i64), "must not be on 64 bit!");
    bool success = coy_file_write_i64(file, len);
    StopIf(!success, return false);
    if(len > 0)
    {
        size nbytes = coy_file_write(file, len, (byte *)str);
        if(nbytes != len) { return false; }
    }
    return true;
}

static inline bool 
coy_file_read_f64(CoyFile *file, f64 *val)
{
    size nbytes = coy_file_read(file, sizeof(*val), (byte *)val);
    if(nbytes != sizeof(*val)) { return false; }
    return true;
}

static inline bool 
coy_file_read_i8(CoyFile *file, i8 *val)
{
    size nbytes = coy_file_read(file, sizeof(*val), (byte *)val);
    if(nbytes != sizeof(*val)) { return false; }
    return true;
}

static inline bool 
coy_file_read_i16(CoyFile *file, i16 *val)
{
    size nbytes = coy_file_read(file, sizeof(*val), (byte *)val);
    if(nbytes != sizeof(*val)) { return false; }
    return true;
}

static inline bool 
coy_file_read_i32(CoyFile *file, i32 *val)
{
    size nbytes = coy_file_read(file, sizeof(*val), (byte *)val);
    if(nbytes != sizeof(*val)) { return false; }
    return true;
}

static inline bool 
coy_file_read_i64(CoyFile *file, i64 *val)
{
    size nbytes = coy_file_read(file, sizeof(*val), (byte *)val);
    if(nbytes != sizeof(*val)) { return false; }
    return true;
}

static inline bool 
coy_file_read_u8(CoyFile *file, u8 *val)
{
    size nbytes = coy_file_read(file, sizeof(*val), (byte *)val);
    if(nbytes != sizeof(*val)) { return false; }
    return true;
}

static inline bool 
coy_file_read_u16(CoyFile *file, u16 *val)
{
    size nbytes = coy_file_read(file, sizeof(*val), (byte *)val);
    if(nbytes != sizeof(*val)) { return false; }
    return true;
}

static inline bool 
coy_file_read_u32(CoyFile *file, u32 *val)
{
    size nbytes = coy_file_read(file, sizeof(*val), (byte *)val);
    if(nbytes != sizeof(*val)) { return false; }
    return true;
}

static inline bool 
coy_file_read_u64(CoyFile *file, u64 *val)
{
    size nbytes = coy_file_read(file, sizeof(*val), (byte *)val);
    if(nbytes != sizeof(*val)) { return false; }
    return true;
}

static inline bool 
coy_file_read_str(CoyFile *file, size *len, char *str)
{
    i64 str_len = 0;
    bool success = coy_file_read_i64(file, &str_len);
    StopIf(!success || str_len > *len, return false);

    if(str_len > 0)
    {
        success = coy_file_read(file, str_len, (byte *)str);
        StopIf(!success, return false);
    }
    else
    {
        /* Clear the provided buffer. */
        memset(str, 0, *len);
    }

    *len = str_len;
    return true;
}

#if defined(_WIN32) || defined(_WIN64)
#include "coyote_win32.h"
#elif defined(__linux__)
#include "coyote_linux.h"
#elif defined(__APPLE__)
#include "coyote_apple_osx.h"
#else
#error "Platform not supported by Coyote Library"
#endif

#if defined(__linux__) || defined(__APPLE__)
#include "coyote_linux_apple_common.h"
#endif

#endif
