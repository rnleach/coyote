#ifndef _COYOTE_H_
#define _COYOTE_H_

#include <stdint.h>
#include <stddef.h>

#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>
#pragma warning(disable: 4142)

#elif defined(__linux__) || defined(__APPLE__)

#include <pthread.h>

#else
#error "Platform not supported by Coyote Library"
#endif

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

#define COY_ARRAY_SIZE(A) (sizeof(A) / sizeof(A[0]))

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
 *                                                     Terminal Info
 *-------------------------------------------------------------------------------------------------------------------------*/
typedef struct
{
    i32 columns;
    i32 rows;
} CoyTerminalSize;

static inline CoyTerminalSize coy_get_terminal_size(void);

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

typedef void (*CoyThreadFunc)(void *thread_data);

#if defined(_WIN32) || defined(_WIN64)

typedef struct 
{
    CoyThreadFunc func;
    void *thread_data;
    u32 thread_id;

    /* Win32 HANDLE = void * */
    _Alignas(32) void *thread_handle;
} CoyThread;

typedef struct 
{
    /* Win32 API calls them critical sections. It also has something called mutex, but it works for interprocess 
     * synchronization as well, and so it is slower. Within a single process, critical sections are a faster way to
     * do synchronization.
     */
    CRITICAL_SECTION mutex;
    bool valid;
} CoyMutex;

typedef struct
{
    CONDITION_VARIABLE cond_var;
    bool valid;
} CoyCondVar;

#elif defined(__linux__) || defined(__APPLE__)

typedef struct 
{
    CoyThreadFunc func;
    void *thread_data;

    _Alignas(32) pthread_t thread;
} CoyThread;

typedef struct 
{
    pthread_mutex_t mutex;
    bool valid;
} CoyMutex;


typedef struct
{
    pthread_cond_t cond_var;
    bool valid;
} CoyCondVar;

#endif

static inline bool coy_thread_create(CoyThread *thrd, CoyThreadFunc func, void *thread_data); /* Returns NULL on failure. */
static inline bool coy_thread_join(CoyThread *thread); /* Returns false if there was an error. */
static inline void coy_thread_destroy(CoyThread *thread);

static inline CoyMutex coy_mutex_create(void);
static inline bool coy_mutex_lock(CoyMutex *mutex);    /* Block, return false on failure. */
static inline bool coy_mutex_unlock(CoyMutex *mutex);  /* Return false on failure.        */
static inline void coy_mutex_destroy(CoyMutex *mutex); /* Must set valid member to false. */

static inline CoyCondVar coy_condvar_create(void);
static inline bool coy_condvar_sleep(CoyCondVar *cv, CoyMutex *mtx);
static inline bool coy_condvar_wake(CoyCondVar *cv);
static inline bool coy_condvar_wake_all(CoyCondVar *cv);
static inline void coy_condvar_destroy(CoyCondVar *cv); /* Must set valid member to false. */

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                  Thread Safe Channel
 *---------------------------------------------------------------------------------------------------------------------------
 * Threadsafe channel for sending / receiving pointers. Multiple Producer / Multiple Consumer (mpmc)
 */
#define COYOTE_CHANNEL_SIZE 16
typedef struct
{
    size head;
    size tail;
    size count;
    CoyMutex mtx;
    CoyCondVar space_available;
    CoyCondVar data_available;
    i32 num_producers;
    i32 num_consumers;
    bool finished;
    void *buf[COYOTE_CHANNEL_SIZE];
} CoyChannel;

static inline CoyChannel coy_channel_create(void);
static inline void coy_channel_destroy(CoyChannel *chan, void(*free_func)(void *ptr, void *ctx), void *free_ctx);
static inline void coy_channel_wait_until_ready_to_receive(CoyChannel *chan);
static inline void coy_channel_wait_until_ready_to_send(CoyChannel *chan);
static inline void coy_channel_register_sender(CoyChannel *chan);
static inline void coy_channel_register_receiver(CoyChannel *chan);
static inline void coy_channel_done_sending(CoyChannel *chan);
static inline void coy_channel_finish(CoyChannel *chan);
static inline void coy_channel_done_receiving(CoyChannel *chan);
static inline bool coy_channel_send(CoyChannel *chan, void *data);
static inline bool coy_channel_receive(CoyChannel *chan, void **out);

/*---------------------------------------------------------------------------------------------------------------------------
 *                                                    Profiling Tools
 *---------------------------------------------------------------------------------------------------------------------------
 * Macro enabled tools for profiling code.
 */

#ifndef COY_PROFILE
#define COY_PROFILE 0
#endif

#if COY_PROFILE
#define COY_PROFILE_NUM_BLOCKS 64
#else
#define COY_PROFILE_NUM_BLOCKS 1
#endif

typedef struct
{
    u64 tsc_elapsed_inclusive;
    u64 tsc_elapsed_exclusive;

    u64 hit_count;
    i32 ref_count;

    u64 bytes;

    char const *label;

    double exclusive_pct;
    double inclusive_pct;
    double gibibytes_per_second;
} CoyBlockProfiler;

#if COY_PROFILE
typedef struct
{
    u64 start;
    u64 old_tsc_elapsed_inclusive;
    i32 index;
    i32 parent_index;
} CoyProfileAnchor;
#else
typedef u32 CoyProfileAnchor;
#endif

typedef struct
{
    CoyBlockProfiler blocks[COY_PROFILE_NUM_BLOCKS];
    i32 current_block;

    u64 start;

    double total_elapsed;
    u64 freq;
} GlobalProfiler;

static GlobalProfiler coy_global_profiler;

/* CPU & timing */
static inline void coy_profile_begin(void);
static inline void coy_profile_end(void);
static inline u64 coy_profile_read_cpu_timer(void);
static inline u64 coy_profile_estimate_cpu_timer_freq(void);

/* OS Counters (page faults, etc.) */
static inline void coy_profile_initialize_os_metrics(void);
static inline void coy_profile_finalize_os_metrics(void);
static inline u64 coy_profile_read_os_page_fault_count(void);

#define COY_START_PROFILE_FUNCTION_BYTES(bytes) coy_profile_start_block(__func__, __COUNTER__, bytes)
#define COY_START_PROFILE_FUNCTION coy_profile_start_block(__func__, __COUNTER__, 0)
#define COY_START_PROFILE_BLOCK_BYTES(name, bytes) coy_profile_start_block(name, __COUNTER__, bytes)
#define COY_START_PROFILE_BLOCK(name) coy_profile_start_block(name, __COUNTER__, 0)
#define COY_END_PROFILE(ap) coy_profile_end_block(&ap)

#if COY_PROFILE
#define COY_PROFILE_STATIC_CHECK _Static_assert(__COUNTER__ <= COY_ARRAY_SIZE(coy_global_profiler.blocks), "not enough profiler blocks")
#else
#define COY_PROFILE_STATIC_CHECK
#endif
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

static inline CoyChannel 
coy_channel_create(void)
{
    CoyChannel chan = 
        {
            .head = 0,
            .tail = 0,
            .count = 0,
            .mtx = coy_mutex_create(),
            .space_available = coy_condvar_create(),
            .data_available = coy_condvar_create(),
            .num_producers = 0,
            .num_consumers = 0,
            .finished = false,
            .buf = {0},
        };

    Assert(chan.mtx.valid);
    Assert(chan.space_available.valid);
    Assert(chan.data_available.valid);

    return chan;
}

static inline void 
coy_channel_destroy(CoyChannel *chan, void(*free_func)(void *ptr, void *ctx), void *free_ctx)
{
    Assert(chan->num_producers == 0);
    Assert(chan->num_consumers == 0);
    if(free_func)
    {
        while(chan->count > 0)
        {
            free_func(chan->buf[chan->head % COYOTE_CHANNEL_SIZE], free_ctx);
            chan->head += 1;
            chan->count -= 1;
        }

        Assert(chan->head == chan->tail && chan->count == 0);
    }

    coy_mutex_destroy(&chan->mtx);
    coy_condvar_destroy(&chan->space_available);
    coy_condvar_destroy(&chan->data_available);
    *chan = (CoyChannel){0};
}

static inline void 
coy_channel_wait_until_ready_to_receive(CoyChannel *chan)
{
    bool success = coy_mutex_lock(&chan->mtx);
    Assert(success);

    while(chan->num_producers == 0 && chan->count == 0 && !chan->finished) 
    {
        coy_condvar_sleep(&chan->data_available, &chan->mtx); 
    }

    success = coy_mutex_unlock(&chan->mtx);
    Assert(success);
}

static inline void 
coy_channel_wait_until_ready_to_send(CoyChannel *chan)
{
    bool success = coy_mutex_lock(&chan->mtx);
    Assert(success);
    while(chan->num_consumers == 0) { coy_condvar_sleep(&chan->space_available, &chan->mtx); }
    success = coy_mutex_unlock(&chan->mtx);
    Assert(success);
}

static inline void 
coy_channel_register_sender(CoyChannel *chan)
{
    bool success = coy_mutex_lock(&chan->mtx);
    Assert(success);
    chan->num_producers += 1;

    if(chan->num_producers == 1)
    {
        /* Broadcast here so any threads blocked on coy_channel_wait_until_ready_to_receive can progress. If the number of
         * producers is greater than 1, then this was already sent.
         */
        success = coy_condvar_wake_all(&chan->data_available);
        Assert(success);
    }

    success = coy_mutex_unlock(&chan->mtx);
    Assert(success);
}

static inline void 
coy_channel_register_receiver(CoyChannel *chan)
{
    bool success = coy_mutex_lock(&chan->mtx);
    Assert(success);

    chan->num_consumers += 1;

    if(chan->num_consumers == 1)
    {
        /* Broadcast here so any threads blocked on coy_channel_wait_until_ready_to_send can progress. If the number of
         * consumers is greater than 1, then this was already sent.
         */
        success = coy_condvar_wake_all(&chan->space_available);
        Assert(success);
    }

    success = coy_mutex_unlock(&chan->mtx);
    Assert(success);
}

static inline void 
coy_channel_done_sending(CoyChannel *chan)
{
    bool success = coy_mutex_lock(&chan->mtx);
    Assert(success);
    Assert(chan->num_producers > 0);

    chan->num_producers -= 1;

    if(chan->num_producers == 0)
    {
        /* Broadcast in case any thread is waiting for data to become available that won't because there are no more
         * producers. This will let them check the number of producers and realize nothing is coming so they too can quit.
         */
        success = coy_condvar_wake_all(&chan->data_available);
        Assert(success);
    }
    else
    {
        /* Deadlock may occur if I don't do this. This thread got signaled when others should have. */
        success = coy_condvar_wake_all(&chan->space_available);
        Assert(success);
    }

    success = coy_mutex_unlock(&chan->mtx);
    Assert(success);
}

static inline void 
coy_channel_finish(CoyChannel *chan)
{
    bool success = coy_mutex_lock(&chan->mtx);
    Assert(success);
    Assert(chan->num_producers == 0);

    chan->finished = true;

    success = coy_condvar_wake_all(&chan->data_available);
    Assert(success);

    success = coy_mutex_unlock(&chan->mtx);
    Assert(success);
}

static inline void 
coy_channel_done_receiving(CoyChannel *chan)
{
    bool success = coy_mutex_lock(&chan->mtx);
    Assert(success);
    Assert(chan->num_consumers > 0);

    chan->num_consumers -= 1;

    if(chan->num_consumers == 0)
    {
        /* Broadcast in case any thread is waiting to send data that they will never be able to send because there are no
         * consumers to receive it! This will let them check the number of consumers and realize space will never become
         * available.
         */
        success = coy_condvar_wake_all(&chan->space_available);
        Assert(success);
    }
    else
    {
        /* Deadlock may occur if I don't do this. This thread got signaled when others should have. */
        success = coy_condvar_wake_all(&chan->data_available);
        Assert(success);
    }

    success = coy_mutex_unlock(&chan->mtx);
    Assert(success);
}

static inline bool 
coy_channel_send(CoyChannel *chan, void *data)
{
    bool success = coy_mutex_lock(&chan->mtx);
    Assert(success);
    Assert(chan->num_producers > 0);

    while(chan->count == COYOTE_CHANNEL_SIZE && chan->num_consumers > 0)
    {
        success = coy_condvar_sleep(&chan->space_available, &chan->mtx);
        Assert(success);
    }

    Assert(chan->count < COYOTE_CHANNEL_SIZE || chan->num_consumers == 0);

    if(chan->num_consumers > 0)
    {
        chan->buf[(chan->tail) % COYOTE_CHANNEL_SIZE] = data;
        chan->tail += 1;
        chan->count += 1;

        /* If the count was increased to , then someone may have been waiting to be notified! */
        if(chan->count == 1)
        {
            success = coy_condvar_wake(&chan->data_available);
            Assert(success);
        }

        success = coy_mutex_unlock(&chan->mtx);
        Assert(success);
        return true;
    }
    else
    {
        /* Space will never become available. */
        success = coy_mutex_unlock(&chan->mtx);
        Assert(success);
        return false;
    }
}

static inline bool 
coy_channel_receive(CoyChannel *chan, void **out)
{
    bool success = coy_mutex_lock(&chan->mtx);
    Assert(success);

    while(chan->count == 0 && chan->num_producers > 0)
    {
        success = coy_condvar_sleep(&chan->data_available, &chan->mtx);
        Assert(success);
    }

    Assert(chan->count > 0 || chan->num_producers == 0);

    *out = NULL;
    if(chan->count > 0)
    {
        *out = chan->buf[(chan->head) % COYOTE_CHANNEL_SIZE];
        chan->head += 1;
        chan->count -= 1;
    }
    else
    {
        /* Nothing more is coming, to get here num_producers must be 0. */
        success = coy_mutex_unlock(&chan->mtx);
        Assert(success);
        return false;
    }

    /* If the queue was full before, send a signal to let other's know there is room now. */
    if( chan->count + 1 == COYOTE_CHANNEL_SIZE)
    {
        success = coy_condvar_wake(&chan->space_available);
        Assert(success);
    }
    
    success = coy_mutex_unlock(&chan->mtx);
    Assert(success);

    return true;
}

typedef struct
{
    bool initialized;
#if defined(_WIN64)
    HANDLE proc;
#else
    int perf_fd;
#endif
} CoyOsMetrics;

static CoyOsMetrics coy_global_os_metrics;

/* NOTE: force the __COUNTER__ to start at 1, so I can offset and NOT use position 0 of global_profiler.blocks array */
i64 dummy =  __COUNTER__; 

static inline u64 coy_profile_get_os_timer_freq(void);
static inline u64 coy_profile_read_os_timer(void);

static inline u64
coy_profile_estimate_cpu_timer_freq(void)
{
	u64 milliseconds_to_wait = 100;
	u64 os_freq = coy_profile_get_os_timer_freq();

	u64 cpu_start = coy_profile_read_cpu_timer();
	u64 os_start = coy_profile_read_os_timer();
	u64 os_end = 0;
	u64 os_elapsed = 0;
	u64 os_wait_time = os_freq * milliseconds_to_wait / 1000;
	while(os_elapsed < os_wait_time)
	{
		os_end = coy_profile_read_os_timer();
		os_elapsed = os_end - os_start;
	}
	
	u64 cpu_end = coy_profile_read_cpu_timer();
	u64 cpu_elapsed = cpu_end - cpu_start;
	
	u64 cpu_freq = 0;
	if(os_elapsed)
	{
		cpu_freq = os_freq * cpu_elapsed / os_elapsed;
	}
	
	return cpu_freq;
}

static inline void
coy_profile_begin(void)
{
    coy_global_profiler.start = coy_profile_read_cpu_timer();
    coy_global_profiler.blocks[0].label = "Global";
    coy_global_profiler.blocks[0].hit_count++;
}

static inline void
coy_profile_end(void)
{
    u64 end = coy_profile_read_cpu_timer();
    u64 total_elapsed = end - coy_global_profiler.start;

    CoyBlockProfiler *gp = coy_global_profiler.blocks;
    gp->tsc_elapsed_inclusive = total_elapsed;
    gp->tsc_elapsed_exclusive += total_elapsed;

    u64 freq = coy_profile_estimate_cpu_timer_freq();
    f64 const ZERO = 0.0;
    f64 const A_NAN = 0.0 / ZERO;
    if(freq)
    {
        coy_global_profiler.total_elapsed = (double)total_elapsed / (double)freq;
        coy_global_profiler.freq = freq;
    }
    else
    {
        coy_global_profiler.total_elapsed = A_NAN;
        coy_global_profiler.freq = A_NAN;
    }

    for(i32 i = 0; i < COY_ARRAY_SIZE(coy_global_profiler.blocks); ++i)
    {
        CoyBlockProfiler *block = coy_global_profiler.blocks + i;
        if(block->tsc_elapsed_inclusive)
        {
            block->exclusive_pct = (double)block->tsc_elapsed_exclusive / (double) total_elapsed * 100.0;
            block->inclusive_pct = (double)block->tsc_elapsed_inclusive / (double) total_elapsed * 100.0;
            if(block->bytes && freq)
            {
                double gib = (double)block->bytes / (1024 * 1024 * 1024);
                block->gibibytes_per_second = gib * (double) freq / (double)block->tsc_elapsed_inclusive;
            }
            else
            {
                block->gibibytes_per_second = A_NAN;
            }
        }
        else
        {
            block->exclusive_pct = A_NAN;
            block->inclusive_pct = A_NAN;
            block->gibibytes_per_second = A_NAN;
        }
    }
}

static inline CoyProfileAnchor 
coy_profile_start_block(char const *label, i32 index, u64 bytes_processed)
{
#if COY_PROFILE
    /* Update global state */
    i32 parent_index = coy_global_profiler.current_block;
    coy_global_profiler.current_block = index;

    /* update block profiler for this block */
    CoyBlockProfiler *block = coy_global_profiler.blocks + index;
    block->hit_count++;
    block->ref_count++;
    block->bytes += bytes_processed;
    block->label = label; /* gotta do it every time */

    /* build anchor to track this block */
    u64 start = coy_profile_read_cpu_timer();
    return (CoyProfileAnchor)
        { 
            .index = index, 
            .parent_index = parent_index, 
            .start=start, 
            .old_tsc_elapsed_inclusive = block->tsc_elapsed_inclusive 
        };
#else
    return -1;
#endif
}

static inline void 
coy_profile_end_block(CoyProfileAnchor *anchor)
{
#if COY_PROFILE
    /* read the end time, hopefully before the rest of this stuff */
    u64 end = coy_profile_read_cpu_timer();
    u64 elapsed = end - anchor->start;

    /* update global state */
    coy_global_profiler.current_block = anchor->parent_index;

    /* update the parent block profilers state */
    CoyBlockProfiler *parent = coy_global_profiler.blocks + anchor->parent_index;
    parent->tsc_elapsed_exclusive -= elapsed;

    /* update block profiler state */
    CoyBlockProfiler *block = coy_global_profiler.blocks + anchor->index;
    block->tsc_elapsed_exclusive += elapsed;
    block->tsc_elapsed_inclusive = anchor->old_tsc_elapsed_inclusive + elapsed;
    block->ref_count--;
#endif
}


#if defined(_WIN32) || defined(_WIN64)

#include "coyote_win32.h"
#pragma warning(default: 4142)

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
