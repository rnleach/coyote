#ifndef _COYOTE_H_
#define _COYOTE_H_

#include <stdint.h>

/*---------------------------------------------------------------------------------------------------------------------------
 * Define the few parts of the standard headers that I need.
 *-------------------------------------------------------------------------------------------------------------------------*/

// stdbool.h
#ifndef bool
  #define bool int
  #define false 0
  #define true 1
#endif

#if 0
// string.h
void *memcpy(void *dst, void const *src, size_t num_bytes);
void *memset(void *buffer, int val, size_t num_bytes);
int memcmp(const void *s1, const void *s2, size_t num_bytes);
#endif

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
static inline uint64_t coy_time_now(void); // Get the current system time in seconds since midnight, Jan. 1 1970.

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                     Files & Paths
 *---------------------------------------------------------------------------------------------------------------------------
 * Check the 'valid' member of the structs to check for errors!
 */

// Append new path to the path in path_buffer, return true on success or false on error. path_buffer must be zero terminated.
static inline bool coy_path_append(intptr_t buf_len, char path_buffer[], char const *new_path);

typedef struct
{
    intptr_t handle; // posix returns an int and windows a HANDLE (e.g. void*), this should work for all of them.
    bool valid;      // error indicator
} CoyFile;

static inline CoyFile coy_file_create(char const *filename); // Truncate if it already exists, otherwise create it.
static inline CoyFile coy_file_append(char const *filename); // Create file if it doesn't exist yet, otherwise append.
static inline intptr_t coy_file_write(CoyFile *file, intptr_t nbytes_write, unsigned char *buffer); // return nbytes written or -1 on error

static inline CoyFile coy_file_open_read(char const *filename);
static inline intptr_t coy_file_read(CoyFile *file, intptr_t buf_size, unsigned char *buffer); // return nbytes read or -1 on error
static inline void coy_file_close(CoyFile *file); // Must set valid member to false on success or failure!

static inline intptr_t coy_file_size(char const *filename); // size of a file in bytes, -1 on error.

// return size in bytes of the loaded data or -1 on error. If buffer is too small, load nothing and return -1
static inline intptr_t coy_file_slurp(char const *filename, intptr_t buf_size, unsigned char *buffer);

typedef struct
{
    intptr_t size_in_bytes;     // size of the file
    unsigned char const *data; 
    intptr_t _internal[2];      // implementation specific data
    bool valid;                 // error indicator
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
    intptr_t os_handle;         // for internal use only
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
    int64_t size;
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

static inline CoyMemoryBlock coy_memory_allocate(intptr_t minimum_num_bytes);
static inline void coy_memory_free(CoyMemoryBlock *mem);

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

    while(*l && *r)
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

static inline intptr_t 
coy_file_slurp(char const *filename, intptr_t buf_size, unsigned char *buffer)
{
    intptr_t file_size = coy_file_size(filename);
    StopIf(file_size < 1 || file_size > buf_size, goto ERR_RETURN);

    CoyFile file = coy_file_open_read(filename);
    StopIf(!file.valid, goto ERR_RETURN);

    intptr_t num_bytes = coy_file_read(&file, buf_size, buffer);
    coy_file_close(&file);
    StopIf(num_bytes != file_size, goto ERR_RETURN);

    return num_bytes;

ERR_RETURN:
    return -1;
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
