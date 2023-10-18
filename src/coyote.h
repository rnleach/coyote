#ifndef _COYOTE_H_
#define _COYOTE_H_

#include <stddef.h>
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
static inline uint64_t coy_current_time(void); // Get the current system time in seconds since midnight, Jan. 1 1970.

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                     Files & Paths
 *---------------------------------------------------------------------------------------------------------------------------
 * Check the 'valid' member of the structs to check for errors!
 */
typedef struct
{
    intptr_t handle; // posix returns an int and windows a HANDLE (e.g. void*), this should work for all of them.
    bool valid;      // error indicator
} CoyFile;

static inline CoyFile coy_create_file(char const *filename);
static inline CoyFile coy_append_to_file(char const *filename);
static inline intptr_t coy_write_to_file(CoyFile *file, intptr_t nbytes_to_write, unsigned char *buffer); // return nbytes written

static inline CoyFile coy_open_file_read(char const *filename);
static inline intptr_t coy_read_from_file(CoyFile *file, intptr_t buf_size, unsigned char *buffer); // return nbytes read
static inline intptr_t coy_file_size(char const *filename); // size of a file in bytes

typedef struct
{
    intptr_t size_in_bytes; // size of the file
    unsigned char *data; 
    intptr_t _internal[2];  // implementation specific data
    bool valid;             // error indicator
} CoyMemMappedFile;

static inline CoyMemMappedFile coy_memmap_read_only(char const *filename);
static inline void coy_memmap_close(CoyMemMappedFile *file);

// Append new path to the path in path_buffer, return true on success or false on error.
static inline bool coy_append_to_path(ptrdiff_t buf_len, char path_buffer[], char const *new_path);

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

static inline CoyMemoryBlock coy_allocate_memory(int64_t minimum_num_bytes);
static inline void coy_free_memory(CoyMemoryBlock *mem);

/*---------------------------------------------------------------------------------------------------------------------------
 *
 *
 *
 *                                          Implementation of `inline` functions.
 *
 *
 *
 *-------------------------------------------------------------------------------------------------------------------------*/
#if defined(_WIN32) || defined(_WIN64)
  #include "coyote_win32.h"
#elif defined(__linux__)
  #include "coyote_linux.h"
#elif defined(__APPLE__)
  #include "coyote_apple_osx.h"
#else
  #error "Platform not supported by Coyote Library"
#endif

#endif
