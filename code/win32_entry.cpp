/*
  Entry Point for window platform
*/

#include "globals.h"

#include <windows.h>
#include <stdio.h>
#include <DSound.h>

// A, B, Select, Start, Up, Down, Left, Right
struct input
{
    enum buttons {
        B_A = 0,
        B_B,
        B_SELECT,
        B_START,
        B_UP,
        B_DOWN,
        B_LEFT,
        B_RIGHT,
        BUTTON_NUM
    };    
    bool32 Buttons[BUTTON_NUM];
};

struct screen_buffer
{
    // NOTE: Memory Order BB GG RR XX
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};


// TODO: Investigate if all these need to be global
global screen_buffer GlobalScreenBackBuffer = {};

global input GlobalInput = {};

global bool32 GlobalRunning;
global bool32 GlobalDrawScreen = false;

global int64 GlobalPerfCountFrequency;

// Return the amount of clocks elapsed
inline LARGE_INTEGER
getClock(void)
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result);
}

// Take start and end clocks and return seconds elapsed. Uses ClockFrequency
inline real32
getSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    real32 Result = ((real32)(End.QuadPart - Start.QuadPart) /
                     (real32)GlobalPerfCountFrequency);
    return(Result);
}

// Copy Size amount of bytes from source to destination
static void copyMemory(uint8 *Dest, uint8 *Src, uint16 Size)
{
    // NOTE: Very basic copy. Not bounds protection
    for(uint16 Byte = 0; Byte < Size; ++Byte)
        Dest[Byte] = Src[Byte];
}

#include "file.cpp"
#include "log.cpp"


#define MAX_ROM_NAME_SIZE 256
global char RomFileName[MAX_ROM_NAME_SIZE]; 

#include "nes.cpp"

// Windows Menu Item command defines
#define ID_OPEN_ROM_ITEM        1001
#define ID_CLOSE_ROM_ITEM        1002
#define ID_QUIT_ITEM            1003

LRESULT CALLBACK
WinInputCallback(HWND WindowHandle, UINT Message,
                 WPARAM wParam, LPARAM lParam)
{
    LRESULT Result = 0;
    
    switch(Message) 
    { 
        case WM_CREATE:
        {
            // Initialize the window. 
            break; 
        }
        
        case WM_SIZE:
        {
            // Set the size and position of the window. 
            break;
        }
        case WM_CLOSE:
        {
            GlobalRunning = false;
            break;
        }
        case WM_DESTROY:
        {
            break;
        }
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            bool32 IsDown = ((lParam & (1<<31)) == 0);
            bool32 WasDown = ((lParam & (1<<30)) != 0);

            // NOTE: Alt only on SYSDOWN messages
            bool32 AltPressed = ((lParam & (1<<29)) != 0);
            
            if(IsDown != WasDown)
            {  
                switch(wParam)
                {
                    case VK_UP:
                    {
                        GlobalInput.Buttons[input::B_UP] = IsDown ? 1 : 0;
                        break;
                    }
                    case VK_DOWN:
                    {
                        GlobalInput.Buttons[input::B_DOWN] = IsDown ? 1 : 0;
                        break;
                    }
                    case VK_LEFT:
                    {
                        GlobalInput.Buttons[input::B_LEFT] = IsDown ? 1 : 0;
                        break;
                    }
                    case VK_RIGHT:
                    {
                        GlobalInput.Buttons[input::B_RIGHT] = IsDown ? 1 : 0;
                        break;
                    }
                    case 'Z':
                    {
                        GlobalInput.Buttons[input::B_A] = IsDown ? 1 : 0;
                        break;
                    }
                    case 'X':
                    {
                        GlobalInput.Buttons[input::B_B] = IsDown ? 1 : 0;
                        break;
                    }
                    case VK_RETURN:
                    {
                        GlobalInput.Buttons[input::B_START] = IsDown ? 1 : 0;
                        break;
                    }
                    case VK_SHIFT:
                    {
                        GlobalInput.Buttons[input::B_SELECT] = IsDown ? 1 : 0;
                        break;
                    }
                    case VK_SPACE:
                    {                        
                        break;
                    }
                    case VK_ESCAPE:
                    {
                        GlobalRunning = false;
                        break;
                    }
                    case VK_F4:
                    {
                        if(AltPressed)
                        {
                            GlobalRunning = false;
                        }
                        break;
                    }
                }
            }
            break;
        }
        
        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case ID_OPEN_ROM_ITEM:
                {
                    char tempFileName[256];
                    
                    OPENFILENAMEA newRom = {};
                    newRom.lStructSize = sizeof(OPENFILENAME);
                    newRom.hwndOwner = WindowHandle;
                    newRom.lpstrFile = tempFileName;
                    newRom.lpstrFile[0] = '\0';
                    newRom.nMaxFile = sizeof(tempFileName);
                    newRom.lpstrFilter = ".nes\0*.nes\0";
                    newRom.nFilterIndex = 1;
                    newRom.lpstrFileTitle = NULL;
                    newRom.nMaxFileTitle = 0;
                    newRom.lpstrInitialDir= NULL;
                    newRom.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

                    bool32 FileOpened = GetOpenFileName(&newRom); 
                    
                    if(FileOpened) // If exists then restart emulator with new file 
                    {
                        ZeroMemory(&RomFileName, sizeof(RomFileName));
                        uint16 NameSize = (uint16)strlen(tempFileName);
                        copyMemory((uint8 *)RomFileName, (uint8 *)tempFileName, NameSize);
                        
                        if(GlobalNes->PowerOn)
                        {
                            reset(GlobalNes);
                        }
                        else
                        {
                            power(GlobalNes);
                        }
                    }
                    
                    break;
                }
                case ID_CLOSE_ROM_ITEM:
                {
                    ZeroMemory(&RomFileName, sizeof(RomFileName));
                    power(GlobalNes);
                    break;
                }
                case ID_QUIT_ITEM:
                {
                    GlobalRunning = false;
                    break;
                }
            }
            break;
        }
                
        default:
        {
            Result = DefWindowProc(WindowHandle, Message, wParam, lParam);
            break;
        }
    }
    return Result;
}

static void getWindowSize(HWND Window, uint16 *Width, uint16 *Height)
{
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    *Width = (uint16)(ClientRect.right - ClientRect.left);
    *Height = (uint16)(ClientRect.bottom - ClientRect.top);
}

static void createBackBuffer(screen_buffer *Buffer, uint16 Width, uint16 Height)
{
    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4; // TODO: Should it be 3 instead because of no alpha?

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Width;
    Buffer->Info.bmiHeader.biHeight = -Height; // Negative tells windows that we raster top to bottom
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int MemorySize = Width * Height * Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, MemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE); 
    Buffer->Pitch = Width * Buffer->BytesPerPixel;
}

static void drawScreenBuffer(screen_buffer *BackBuffer, HDC DeviceContext,
                               uint16 WindowWidth, uint16 WindowHeight)
{                
    StretchDIBits(DeviceContext,
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, BackBuffer->Width, BackBuffer->Height,
                  BackBuffer->Memory,
                  &BackBuffer->Info,
                  DIB_RGB_COLORS, SRCCOPY);
}


struct win_sound
{    
    uint16 SamplesPerSecond;
    uint8 Channels;
    uint8 BitsPerSample;
    uint16 BytesPerSample;
    uint16 BufferSize;
    uint32 SampleIndex;
    uint32 SafetyBytes;
    int16 *Samples;

    bool32 Valid;
};

struct nes_sound
{
    uint16 SamplesPerSecond;
    uint32 SampleCount;
    uint32 BytesToWrite;
    void *Samples;
};

static void
ClearSoundBuffer(LPDIRECTSOUNDBUFFER Buffer, uint32 BufferSize)
{
    VOID *FirstSection;
    DWORD FirstSectionSize;
    VOID *SecondSection;
    DWORD SecondSectionSize;
    
    HRESULT Locked = Buffer->Lock(0, BufferSize,
                                  &FirstSection, &FirstSectionSize,
                                  &SecondSection, &SecondSectionSize, 0);
    if(SUCCEEDED(Locked))
    {
        Assert((FirstSectionSize + SecondSectionSize) == BufferSize);

        uint8 *BytePtr = (uint8 *)FirstSection;
        for(uint32 ByteNum = 0; ByteNum < FirstSectionSize; ++ByteNum)
        {
            BytePtr[ByteNum] = 0;
        }

        BytePtr = (uint8 *)SecondSection;        
        for(uint32 ByteNum = 0; ByteNum < SecondSectionSize; ++ByteNum)
        {
            BytePtr[ByteNum] = 0;
        }
        
        Buffer->Unlock(FirstSection, FirstSectionSize, SecondSection, SecondSectionSize);
    }
}

// NOTE: Taken from Handmade hero and adjusted
static void
FillSoundBuffer(LPDIRECTSOUNDBUFFER Buffer, uint16 BytesPerSample, DWORD ByteToLock, DWORD BytesToWrite,
                int16 *SourceSample, uint32 *SourceRunningIndex)
{
    // TODO(casey): More strenuous test!
    VOID *FirstSection;
    DWORD FirstSectionSize;
    VOID *SecondSection;
    DWORD SecondSectionSize;

    HRESULT LockResult = Buffer->Lock(ByteToLock, BytesToWrite,
                                      &FirstSection, &FirstSectionSize,
                                      &SecondSection, &SecondSectionSize, 0);
    
    if(SUCCEEDED(LockResult))
    {
        // TODO(casey): assert that FirstSectionSize/SecondSectionSize is valid

        // TODO(casey): Collapse these two loops
        DWORD FirstSectionSampleCount = FirstSectionSize/BytesPerSample;
        int16 *DestSample = (int16 *)FirstSection;

        for(DWORD Index = 0; Index < FirstSectionSampleCount; ++Index)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++(*SourceRunningIndex);
        }

        DWORD SecondSectionSampleCount = SecondSectionSize/BytesPerSample;
        DestSample = (int16 *)SecondSection;
        for(DWORD Index = 0; Index < SecondSectionSampleCount; ++Index)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++(*SourceRunningIndex);
        }

        Buffer->Unlock(FirstSection, FirstSectionSize, SecondSection, SecondSectionSize);
    }
}

// NOTE: Taken and adapted Handmade hero code
static LPDIRECTSOUNDBUFFER CreateSoundBuffer(HWND WindowHandle, uint32 BufferSize, uint8 Channels,
                                             uint32 SamplesPerSecond, uint8 BitsPerSample)
{
    // Create DirectSound Buffer for us to fill and play from    
    IDirectSound8 *DSoundInterface;
    HRESULT DSoundResult = DirectSoundCreate8(0, &DSoundInterface, 0);
    Assert(DSoundResult == DS_OK);

    WAVEFORMATEX WaveFormat = {};
    WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
    WaveFormat.nChannels = Channels;
    WaveFormat.nSamplesPerSec = SamplesPerSecond;
    WaveFormat.wBitsPerSample = BitsPerSample;
    WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
    WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;

    // Create Primary Sound buffer first, that we set the wave
    // format too. We can then create the secondary buffer
    // that we fill. Primary buffer can be discarded.
    DSoundResult = DSoundInterface->SetCooperativeLevel(WindowHandle, DSSCL_PRIORITY);
            
    if(SUCCEEDED(DSoundResult))
    {
        DSBUFFERDESC BufferDescription = {};
        BufferDescription.dwSize = sizeof(DSBUFFERDESC);
        BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

        LPDIRECTSOUNDBUFFER PrimaryBuffer;

        DSoundResult = DSoundInterface->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0);
        if(SUCCEEDED(DSoundResult))
        {
            DSoundResult = PrimaryBuffer->SetFormat(&WaveFormat);
            Assert(DSoundResult == DS_OK);
        }        
    }

    // Secondary buffer is the buffer we use. We create it here and return it
    DSBUFFERDESC BufferDescription = {};
    BufferDescription.dwSize = sizeof(DSBUFFERDESC);
    BufferDescription.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
    BufferDescription.dwBufferBytes = BufferSize;
    BufferDescription.lpwfxFormat = &WaveFormat;

    LPDIRECTSOUNDBUFFER SoundBuffer; 
    DSoundResult = DSoundInterface->CreateSoundBuffer(&BufferDescription, &SoundBuffer, 0);
    Assert(DSoundResult == DS_OK);

    return(SoundBuffer);
}

static void UpdateAudio(LPDIRECTSOUNDBUFFER SoundBuffer, win_sound *SoundOut,
                        uint32 FrameUpdateHz, real32 FrameTargetSeconds, real32 FrameTimeElapsed)
{
    DWORD PlayCursor;
    DWORD WriteCursor;                
    if(SoundBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK)
    {
        if(!SoundOut->Valid)
        {
            SoundOut->SampleIndex = WriteCursor / SoundOut->BytesPerSample;
            SoundOut->Valid = true;
        }
                    
        DWORD LockByte = ((SoundOut->SampleIndex * SoundOut->BytesPerSample) % SoundOut->BufferSize);

        DWORD FrameExpectedSoundBytes = (int)((real32)(SoundOut->SamplesPerSecond * SoundOut->BytesPerSample) /
                                              FrameUpdateHz);

        real32 FrameTimeLeft = (FrameTargetSeconds - FrameTimeElapsed);
        DWORD FrameExpectedBytesLeft = (DWORD)((FrameTimeLeft / FrameTargetSeconds) * (real32)FrameExpectedSoundBytes);

        DWORD ExpectedFrameBoundaryByte = PlayCursor + FrameExpectedBytesLeft;

        DWORD SafeWriteCursor = WriteCursor;
        if(SafeWriteCursor < PlayCursor)
        {
            SafeWriteCursor += SoundOut->BufferSize;
        }
        Assert(SafeWriteCursor >= PlayCursor);
        SafeWriteCursor += SoundOut->SafetyBytes;

        bool32 AudioCardIsLowLatency = (SafeWriteCursor < ExpectedFrameBoundaryByte);

        DWORD TargetCursor = 0;
        if(AudioCardIsLowLatency)
        {
            // From the start of next frame until the end of next frame, fill
            TargetCursor = (ExpectedFrameBoundaryByte + FrameExpectedSoundBytes);
        }
        else
        {
            // From the current write cursor, add a frames worth of audio plus saftey bytes
            TargetCursor = (WriteCursor + FrameExpectedSoundBytes +
                            SoundOut->SafetyBytes);
        }

        // Mod to reposition in ring buffer
        TargetCursor = (TargetCursor % SoundOut->BufferSize);

        DWORD BytesToWrite = 0;
        if(LockByte > TargetCursor)
        {
            BytesToWrite = (SoundOut->BufferSize - LockByte);
            BytesToWrite += TargetCursor;
        }
        else
        {
            BytesToWrite = TargetCursor - LockByte;
        }

        nes_sound NesSound = {};
        NesSound.SamplesPerSecond = SoundOut->SamplesPerSecond;
        NesSound.SampleCount = Align8(BytesToWrite / SoundOut->BytesPerSample);
        NesSound.BytesToWrite = NesSound.SampleCount * SoundOut->BytesPerSample;
        NesSound.Samples = SoundOut->Samples;

        /*
        if(Game.GetSoundSamples)
        {
            Game.GetSoundSamples(&GameMemory, &SoundBuffer);
        }
        */

        FillSoundBuffer(SoundBuffer, SoundOut->BytesPerSample, LockByte, BytesToWrite, SoundOut->Samples, &SoundOut->SampleIndex);
    }
}

// NOTE: Entry Point

/*int CALLBACK
WinMain(HINSTANCE WindowInstance, HINSTANCE PrevWindowInstance,
        LPSTR CommandLine, int CommandShow)
{
*/
int main()
{
    LARGE_INTEGER WinPerfCountFrequency;
    QueryPerformanceFrequency(&WinPerfCountFrequency); 
    GlobalPerfCountFrequency = WinPerfCountFrequency.QuadPart;            

    printf("Test\n");
    
    /**************************************/
    /* NOTE : Screen back buffer creation */
    
    uint16 RenderScaleWidth = 256, RenderScaleHeight = 240;
    uint8 ResScale = 2;
    uint16 WindowWidth = RenderScaleWidth * ResScale, WindowHeight = RenderScaleHeight * ResScale;
    GlobalScreenBackBuffer = {};
    createBackBuffer(&GlobalScreenBackBuffer, RenderScaleWidth, RenderScaleHeight);

    /**************************/
    /* NOTE : Window creation */
    
    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = WinInputCallback;
    WindowClass.hInstance = GetModuleHandle(NULL);//WindowInstance;
    WindowClass.lpszClassName = "tuNES";

    uint16 InitialWindowPosX = 700;
    uint16 InitialWindowPosY = 400;
    
    if(RegisterClassA(&WindowClass))
    {        
        HWND Window = CreateWindowExA(0, WindowClass.lpszClassName, "tuNES", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                      InitialWindowPosX, InitialWindowPosY, WindowWidth, WindowHeight,
                                      0, 0, WindowClass.hInstance, 0);

        if(Window) // If window was created successfully
        {
            real32 FrameHz = 60.0988f; // aka fps. // TODO: Will be different for PAL
            real32 FrameTargetSeconds = 1.0f / FrameHz;
            
            /********************************/
            /* NOTE : Window Menu Creation  */            
            
            HMENU WindowMenu = CreateMenu();
            HMENU SubMenu = CreatePopupMenu();
            
            AppendMenu(SubMenu, MF_STRING, ID_OPEN_ROM_ITEM, "&Open Rom");
            AppendMenu(SubMenu, MF_STRING, ID_CLOSE_ROM_ITEM, "&Close Rom");
            AppendMenu(SubMenu, MF_STRING, ID_QUIT_ITEM, "&Quit");
            AppendMenu(WindowMenu, MF_STRING | MF_POPUP, (uint64)SubMenu, "&File");

            SetMenu(Window, WindowMenu);

            /********************************/
            /* NOTE : Sound Buffer Creation */

            win_sound WinSound = {};
            WinSound.SamplesPerSecond = 48000;
            WinSound.Channels = 2; // Sterio
            WinSound.BitsPerSample = 16; // Bit depth
            WinSound.BytesPerSample = sizeof(int16) * WinSound.Channels;
            WinSound.BufferSize = WinSound.SamplesPerSecond * WinSound.BytesPerSample;
            WinSound.SafetyBytes = (int)(((real32)WinSound.SamplesPerSecond * (real32)WinSound.BytesPerSample
                                          / FrameHz) / 3.0f);

            WinSound.Samples = (int16 *)VirtualAlloc(0, (size_t)WinSound.BufferSize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
            
            LPDIRECTSOUNDBUFFER SoundBuffer = CreateSoundBuffer(Window, WinSound.BufferSize, WinSound.Channels,
                                                                WinSound.SamplesPerSecond, WinSound.BitsPerSample);

            ClearSoundBuffer(SoundBuffer, WinSound.BufferSize);
            SoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
                        
            /********************************************************/
            /* NOTE : Timing */
            // Using the vertical scan rate of ~60Hz or 16ms a frame.
            // We run cpu and ppu 16ms worth of ticks. After 16ms
            // elapsed, we then reset the counters that tracks how
            // ticks have been. Repeat
            
            nes Nes = createNes("games/Contra (USA).nes");
            Nes.FrameClockTotal = Nes.CpuHz * FrameTargetSeconds; // TODO: Put in create nes?
            
            GlobalNes = &Nes;

            ////
            
            real32 ElapsedTime = 0.0;
            uint32 ClocksRun = 0;
            real32 FrameTime = 0.0;
            
            LARGE_INTEGER LastClock = getClock();
            LARGE_INTEGER FrameFlippedClock = getClock();
            LARGE_INTEGER FrameLastFlippedClock = getClock();
            
            /********************/
            /* NOTE : Main Loop */

            GlobalRunning = true; 
            while(GlobalRunning)
            {
                MSG Message = {}; 
                while (PeekMessage(&Message, Window, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                
                if(Nes.FrameClocksElapsed < Nes.FrameClockTotal)
                {
                    runNes(&Nes, &GlobalInput);
                }
                else
                {
                    Sleep(2);
                }

                /*
                LARGE_INTEGER FrameSoundClock = getClock();
                real32 FrameTimeElapsed = getSecondsElapsed(FrameFlippedClock, FrameSoundClock);
                UpdateAudio(SoundBuffer, &WinSound, FrameHz, FrameTargetSeconds, FrameTimeElapsed);
                */

                // NOTE: TODO: The cpu might run several more cycles before we get to drawn screen.
                // This could lead to pixels from the next frame overwriting other pixels we haven't displayed
                // To fix, I could implement a double buffer. Two buffers, one is drawn on buy the ppu while the other is the displayed.
                // They are then swapped. This will let the complete frame to be displayed while the new one can created.
                
                if(GlobalDrawScreen)
                {
                    GlobalDrawScreen = false;
                                                              
                    getWindowSize(Window, &WindowWidth, &WindowHeight);
                
                    // NOTE: Drawing the backbuffer to the window 
                    HDC DeviceContext = GetDC(Window);
                    drawScreenBuffer(&GlobalScreenBackBuffer, DeviceContext,
                                     WindowWidth, WindowHeight);
                    ReleaseDC(Window, DeviceContext);

                    FrameFlippedClock = getClock();

                    FrameTime = getSecondsElapsed(FrameLastFlippedClock, FrameFlippedClock);
                    FrameLastFlippedClock = getClock();
                }

                LARGE_INTEGER EndClock = getClock();
                
                real32 LoopTime = getSecondsElapsed(LastClock, EndClock);
                ElapsedTime += LoopTime;
                
                if(ElapsedTime >= FrameTargetSeconds)
                {
                    /*
                    char TextBuffer[256];
                    _snprintf(TextBuffer, 256, "CpuCycles %f, FrameTime %f, ElapsedTime %f\n",
                              Nes.FrameClocksElapsed, FrameTime, ElapsedTime);
                    printf("%s\n", TextBuffer);
                    */
                    
                    ElapsedTime -= FrameTargetSeconds;

                    // The extra clocks we need to add to the next frame. Is 0 or more
                    if(Nes.FrameClocksElapsed >= Nes.FrameClockTotal)
                    {
                        Nes.FrameClocksElapsed = Nes.FrameClocksElapsed - Nes.FrameClockTotal;
                    }
                    else
                    {
                        Nes.FrameClocksElapsed = 0;
                    }
                }
                
                LastClock = EndClock;
            }
     
#if CPU_LOG
            closeLog(Nes->Cpu.LogHandle);
#endif            
        }
        else
        {
            // NOTE: Window failed to create
            // TODO: Handle this in a better way
            Assert(0);
        }
    }
    else
    {
        // NOTE: Failed to register window
        // TODO: Handle this in a better way
        Assert(0);
    }

    return(0);
} 
