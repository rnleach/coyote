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

    _Static_assert(sizeof(intptr_t) == sizeof(DWORD));
    StopIf(attr.nFileSizeLow > INTPTR_MAX, goto ERR_RETURN);

    return (intptr_t) attr.nFileSizeLow;

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
    DWORD size_in_bytes = GetFileSize((HANDLE)cf.handle, NULL);
    StopIf(size_in_bytes == INVALID_FILE_SIZE, goto CLOSE_FMH_AND_ERR);

    return (CoyMemMappedFile){
	  .size_in_bytes = (intptr_t)size_in_bytes, 
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

static inline CoyMemoryBlock 
coy_memory_allocate(int64_t minimum_num_bytes)
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
