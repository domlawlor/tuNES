/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

static HANDLE createLog(char *logName)
{
    HANDLE handle = CreateFileA(logName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    return(handle);
}

static void closeLog(HANDLE handle)
{
    CloseHandle(handle);
}

static b32 writeLog(char *str, u32 byteCount, u32 *bytesWritten, HANDLE fileHandle)
{
    return(WriteFile(fileHandle, str, byteCount, (LPDWORD)bytesWritten, 0));
}

