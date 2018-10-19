/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

// TODO: Check: Is memory allocation a good idea here?

static void * readFileData(char * Filename, u32 *Size)
{
    void *FileData = 0;
    
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER Filesize;
        if(GetFileSizeEx(FileHandle, &Filesize))
        {
            // TODO: Do I want to allocate memory here? Or delegate to some sort of manager?
            FileData = VirtualAlloc(0, Filesize.LowPart, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(FileData)
            {
                DWORD BytesRead;
                if(ReadFile(FileHandle, FileData, Filesize.LowPart, &BytesRead, 0) &&
                   (Filesize.LowPart == BytesRead))
                {
                    *Size = (u32)BytesRead;
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

    CloseHandle(FileHandle);
    
    return(FileData);
}

