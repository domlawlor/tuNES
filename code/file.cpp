/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

   // TODO: Check: Is memory allocation a good idea here?

static void *ReadFileData(u8 *filename, u32 *size)
{
	void *fileData = 0;

	HANDLE fileHandle = CreateFileA((char *)filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if(fileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER filesize;
		if(GetFileSizeEx(fileHandle, &filesize))
		{
			// TODO: Do I want to allocate memory here? Or delegate to some sort of manager?
			fileData = VirtualAlloc(0, filesize.LowPart, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if(fileData)
			{
				DWORD bytesRead;
				if(ReadFile(fileHandle, fileData, filesize.LowPart, &bytesRead, 0) &&
					(filesize.LowPart == bytesRead))
				{
					*size = (u32)bytesRead;
					// It worked!
				}
				else
				{
					Assert(0);
				}
			}
			else
			{
			}
		}
		else
		{
		}
	}
	else
	{
	}

	CloseHandle(fileHandle);

	return(fileData);
}

