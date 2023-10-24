#ifndef _COYOTE_WIN32_H_
#define _COYOTE_WIN32_H_
/*---------------------------------------------------------------------------------------------------------------------------
 *                                                 Windows Implementation
 *-------------------------------------------------------------------------------------------------------------------------*/
#include <windows.h>

_Static_assert(UINT32_MAX < INTPTR_MAX, "DWORD cannot be cast to intptr_t safely.");

union WinTimePun
{
    FILETIME ft;
    ULARGE_INTEGER as_uint64;
};

static inline uint64_t
coy_time_now(void)
{
    SYSTEMTIME now_st = {0};
    GetSystemTime(&now_st);

    union WinTimePun now = {0};
    bool success = SystemTimeToFileTime(&now_st, &now.ft);
    StopIf(!success, goto ERR_RETURN);

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

ERR_RETURN:
    return UINT64_MAX;
}

static char const coy_path_sep = '\\';

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

static inline CoyFile
coy_file_create(char const *filename)
{
    HANDLE fh = CreateFileA(filename,              // [in]           LPCSTR                lpFileName,
                            GENERIC_WRITE,         // [in]           DWORD                 dwDesiredAccess,
                            0,                     // [in]           DWORD                 dwShareMode,
                            NULL,                  // [in, optional] LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                            CREATE_ALWAYS,         // [in]           DWORD                 dwCreationDisposition,
                            FILE_ATTRIBUTE_NORMAL, // [in]           DWORD                 dwFlagsAndAttributes,
                            NULL);                 // [in, optional] HANDLE                hTemplateFile

    if(fh != INVALID_HANDLE_VALUE)
    {
        return (CoyFile){.handle = (intptr_t)fh, .valid = true};
    }
    else
    {
        return (CoyFile){.handle = (intptr_t)INVALID_HANDLE_VALUE, .valid = false};
    }
}

static inline 
CoyFile coy_file_append(char const *filename)
{
    HANDLE fh = CreateFileA(filename,              // [in]           LPCSTR                lpFileName,
                            FILE_APPEND_DATA,      // [in]           DWORD                 dwDesiredAccess,
                            0,                     // [in]           DWORD                 dwShareMode,
                            NULL,                  // [in, optional] LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                            OPEN_ALWAYS,           // [in]           DWORD                 dwCreationDisposition,
                            FILE_ATTRIBUTE_NORMAL, // [in]           DWORD                 dwFlagsAndAttributes,
                            NULL);                 // [in, optional] HANDLE                hTemplateFile

    if(fh != INVALID_HANDLE_VALUE)
    {
        return (CoyFile){.handle = (intptr_t)fh, .valid = true};
    }
    else
    {
        return (CoyFile){.handle = (intptr_t)INVALID_HANDLE_VALUE, .valid = false};
    }
}

static inline intptr_t 
coy_file_write(CoyFile *file, intptr_t nbytes_write, unsigned char *buffer)
{
    StopIf(!file->valid, goto ERR_RETURN);

    DWORD nbytes_written = 0;
    BOOL success = WriteFile(
        (HANDLE)file->handle,     // [in]                HANDLE       hFile,
        buffer,                   // [in]                LPCVOID      lpBuffer,
        nbytes_write,             // [in]                DWORD        nNumberOfBytesToWrite,
        &nbytes_written,          // [out, optional]     LPDWORD      lpNumberOfBytesWritten,
        NULL                      // [in, out, optional] LPOVERLAPPED lpOverlapped
    );

    StopIf(!success, goto ERR_RETURN);
    return (intptr_t)nbytes_written;
    
ERR_RETURN:
    return -1;
}

static inline CoyFile 
coy_file_open_read(char const *filename)
{
    HANDLE fh = CreateFileA(filename,              // [in]           LPCSTR                lpFileName,
                            GENERIC_READ,          // [in]           DWORD                 dwDesiredAccess,
                            FILE_SHARE_READ,       // [in]           DWORD                 dwShareMode,
                            NULL,                  // [in, optional] LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                            OPEN_EXISTING,         // [in]           DWORD                 dwCreationDisposition,
                            FILE_ATTRIBUTE_NORMAL, // [in]           DWORD                 dwFlagsAndAttributes,
                            NULL);                 // [in, optional] HANDLE                hTemplateFile

    if(fh != INVALID_HANDLE_VALUE)
    {
        return (CoyFile){.handle = (intptr_t)fh, .valid = true};
    }
    else
    {
        return (CoyFile){.handle = (intptr_t)INVALID_HANDLE_VALUE, .valid = false};
    }
}

static inline intptr_t 
coy_file_read(CoyFile *file, intptr_t buf_size, unsigned char *buffer)
{
    StopIf(!file->valid, goto ERR_RETURN);

    DWORD nbytes_read = 0;
    BOOL success =  ReadFile((HANDLE) file->handle, //  [in]                HANDLE       hFile,
                             buffer,                //  [out]               LPVOID       lpBuffer,
                             buf_size,              //  [in]                DWORD        nNumberOfBytesToRead,
                             &nbytes_read,          //  [out, optional]     LPDWORD      lpNumberOfBytesRead,
                             NULL);                 //  [in, out, optional] LPOVERLAPPED lpOverlapped

    StopIf(!success, goto ERR_RETURN);
    return (intptr_t)nbytes_read;
    
ERR_RETURN:
    return -1;
}

static inline void 
coy_file_close(CoyFile *file)
{
    CloseHandle((HANDLE)file->handle);
    file->valid = false;

    return;
}

static inline intptr_t 
coy_file_size(char const *filename)
{
    WIN32_FILE_ATTRIBUTE_DATA attr = {0};
    BOOL success = GetFileAttributesExA(filename, GetFileExInfoStandard, &attr);
    StopIf(!success, goto ERR_RETURN);

    _Static_assert(sizeof(uintptr_t) == 2 * sizeof(DWORD) && sizeof(DWORD) == 4);
    uintptr_t file_size = ((uintptr_t)attr.nFileSizeHigh << 32) | attr.nFileSizeLow;
    StopIf(file_size > INTPTR_MAX, goto ERR_RETURN);

    return (intptr_t) file_size;

ERR_RETURN:
    return -1;
}

static inline CoyMemMappedFile 
coy_memmap_read_only(char const *filename)
{
    CoyFile cf = coy_file_open_read(filename);
    StopIf(!cf.valid, goto ERR_RETURN);

    HANDLE fmh =  CreateFileMappingA((HANDLE)cf.handle, // [in]           HANDLE                hFile,
                                     NULL,              // [in, optional] LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
                                     PAGE_READONLY,     // [in]           DWORD                 flProtect,
                                     0,                 // [in]           DWORD                 dwMaximumSizeHigh,
                                     0,                 // [in]           DWORD                 dwMaximumSizeLow,
                                     NULL);             // [in, optional] LPCSTR                lpName
    StopIf(fmh == INVALID_HANDLE_VALUE, goto CLOSE_CF_AND_ERR);

    LPVOID ptr =  MapViewOfFile(fmh,           // [in] HANDLE hFileMappingObject,
                                FILE_MAP_READ, // [in] DWORD  dwDesiredAccess,
                                0,             // [in] DWORD  dwFileOffsetHigh,
                                0,             // [in] DWORD  dwFileOffsetLow,
                                0);            // [in] SIZE_T dwNumberOfBytesToMap
    StopIf(!ptr, goto CLOSE_FMH_AND_ERR);

    // Get the size of the file mapped.
    DWORD file_size_high = 0;
    DWORD file_size_low = GetFileSize((HANDLE)cf.handle, &file_size_high);
    StopIf(file_size_low == INVALID_FILE_SIZE, goto CLOSE_FMH_AND_ERR);

    uintptr_t file_size = ((uintptr_t)file_size_high << 32) | file_size_low;
    StopIf(file_size > INTPTR_MAX, goto CLOSE_FMH_AND_ERR);


    return (CoyMemMappedFile){
      .size_in_bytes = (intptr_t)file_size, 
        .data = ptr, 
        ._internal = { cf.handle, (intptr_t)fmh }, 
        .valid = true 
    };

CLOSE_FMH_AND_ERR:
    CloseHandle(fmh);
CLOSE_CF_AND_ERR:
    coy_file_close(&cf);
ERR_RETURN:
    return (CoyMemMappedFile) { .valid = false };
}

static inline void 
coy_memmap_close(CoyMemMappedFile *file)
{
    void const*data = file->data;
    intptr_t fh = file->_internal[0];
    HANDLE fmh = (HANDLE)file->_internal[1];

    /*BOOL success = */UnmapViewOfFile(data);
    CloseHandle(fmh);
    CoyFile cf = { .handle = fh, .valid = true };
    coy_file_close(&cf);

    file->valid = false;

    return;
}

// I don't normally use static storage, but the linux interface uses it internally, so I'm stuck with those semantics.
// I'll use my own here.
static WIN32_FIND_DATA coy_file_name_iterator_data;
static char coy_file_name[1024];

static inline CoyFileNameIter
coy_file_name_iterator_open(char const *directory_path, char const *file_extension)
{
    char path_buf[1024] = {0};
    int i = 0;
    for(i = 0; i < sizeof(path_buf) && directory_path[i]; ++i)
    {
        path_buf[i] = directory_path[i];
    }
    StopIf(i + 2 >= sizeof(path_buf), goto ERR_RETURN);
    path_buf[i] = '\\';
    path_buf[i + 1] = '*';
    path_buf[i + 2] = '\0';

    HANDLE finder = FindFirstFile(path_buf, &coy_file_name_iterator_data);
    StopIf(finder == INVALID_HANDLE_VALUE, goto ERR_RETURN);
    return (CoyFileNameIter) { .os_handle=(intptr_t)finder, .file_extension=file_extension, .valid=true };

ERR_RETURN:
    return (CoyFileNameIter) { .valid=false };
}

static inline char const *
coy_file_name_iterator_next(CoyFileNameIter *cfni)
{
    if(cfni->valid)
    {
        // The first call to coy_file_name_iterator_open() should have populated
        char const *fname = coy_file_name_iterator_data.cFileName;
        bool found = false;
        while(!found)
        {
            if(!(coy_file_name_iterator_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                if(cfni->file_extension == NULL)
                {
                    found = true;
                }
                else
                {
                    char const *ext = coy_file_extension(fname);
                    if(coy_null_term_strings_equal(ext, cfni->file_extension))
                    {
                        found = true;
                    }
                }
            }
            if(found)
            {
                int i = 0;
                for(i = 0; i < sizeof(coy_file_name) && fname[i]; ++i)
                {
                    coy_file_name[i] = fname[i];
                }
                coy_file_name[i] = '\0';
            }

            BOOL foundnext = FindNextFileA((HANDLE)cfni->os_handle, &coy_file_name_iterator_data);

            if(!foundnext)
            {
                cfni->valid = false;
                break;
            }
        }

        if(found) 
        {
            return coy_file_name;
        }
    }

    return NULL;
}

static inline void 
coy_file_name_iterator_close(CoyFileNameIter *cfin)
{
    HANDLE finder = (HANDLE)cfin->os_handle;
    FindClose(finder);
    *cfin = (CoyFileNameIter) {0};
    return;
}

static inline CoyMemoryBlock 
coy_memory_allocate(intptr_t minimum_num_bytes)
{
    if(minimum_num_bytes <= 0)
    {
         return (CoyMemoryBlock){.mem = 0, .size = minimum_num_bytes, .valid = false };
    }

    SYSTEM_INFO info = {0};
    GetSystemInfo(&info);
    DWORD page_size = info.dwPageSize;
    DWORD alloc_gran = info.dwAllocationGranularity;

    DWORD target_granularity = minimum_num_bytes > alloc_gran ? alloc_gran : page_size;

    DWORD allocation_size = minimum_num_bytes + target_granularity - (minimum_num_bytes % target_granularity);

    if(allocation_size > INTPTR_MAX)
    {
         return (CoyMemoryBlock){.mem = 0, .size = INTPTR_MAX, .valid = false };
    }
    int64_t size = (int64_t)allocation_size;

    void *mem = VirtualAlloc(NULL, allocation_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if(!mem)
    {
         return (CoyMemoryBlock){.mem = 0, .size = 0, .valid = false };
    }

    return (CoyMemoryBlock){.mem = mem, .size = size, .valid = true };
}

static inline void
coy_memory_free(CoyMemoryBlock *mem)
{
    Assert(mem);
    if(mem->valid)
    {
         /*BOOL success =*/ VirtualFree(mem->mem, 0, MEM_RELEASE);
         mem->valid = false;
    }

    return;
}

#endif
