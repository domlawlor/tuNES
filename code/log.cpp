#include "globals.h"

static HANDLE CreateLog(char *logName)
{
	HANDLE handle = CreateFileA(logName, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	return(handle);
}

static void CloseLog(HANDLE handle)
{
	CloseHandle(handle);
}

static bool WriteLog(char *str, u32 byteCount, u32 *bytesWritten, HANDLE fileHandle)
{
	return(WriteFile(fileHandle, str, byteCount, (LPDWORD)bytesWritten, 0));
}

