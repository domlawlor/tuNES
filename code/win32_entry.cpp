/*
  Entry Point for window platform
*/

#include "globals.h"

#include <windows.h>
#include <stdio.h>
#include <DSound.h>

// A, B, Select, Start, Up, Down, Left, Right
struct Input
{
    enum Buttons {
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
    b32 buttons[BUTTON_NUM];
};

struct ScreenBuffer
{
    // NOTE: Memory Order BB GG RR XX
    BITMAPINFO info;
    void *memory;
    s16 width;
    s16 height;
    s16 pitch;
    s16 bytesPerPixel;
};


// TODO: Investigate if all these need to be global
global ScreenBuffer globalScreenBackBuffer = {};

global Input globalInput = {};

global b32 globalRunning;
global b32 globalDrawScreen = false;

global s64 globalPerfCountFrequency;

// Return the amount of clocks elapsed
inline LARGE_INTEGER GetClock()
{
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return(result);
}

// Take start and end clocks and return seconds elapsed. Uses ClockFrequency
inline r32 GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
    r32 result = ((r32)(end.QuadPart - start.QuadPart) /
                     (r32)globalPerfCountFrequency);
    return(result);
}

// Copy Size amount of bytes from source to destination
static void MemoryCopy(u8 *dest, u8 *src, u16 size)
{
    // NOTE: Very basic copy. Not bounds protection
    for(u16 byte = 0; byte < size; ++byte)
        dest[byte] = src[byte];
}

#include "file.cpp"
#include "log.cpp"


global const u16 romNameMaxSize = 256;
global u8 romFileName[romNameMaxSize];

#include "nes.cpp"

// Windows Menu Item command defines
#define ID_OPEN_ROM_ITEM        1001
#define ID_CLOSE_ROM_ITEM        1002
#define ID_QUIT_ITEM            1003

LRESULT CALLBACK WinInputCallback(HWND windowHandle, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    
    switch(msg) 
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
            globalRunning = false;
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
            b32 isDown = ((lParam & (1<<31)) == 0);
            b32 wasDown = ((lParam & (1<<30)) != 0);

            // NOTE: Alt only on SYSDOWN messages
            b32 altPressed = ((lParam & (1<<29)) != 0);
            
            if(isDown != wasDown)
            {  
                switch(wParam)
                {
                    case VK_UP:
                    {
                        globalInput.buttons[Input::B_UP] = isDown ? 1 : 0;
                        break;
                    }
                    case VK_DOWN:
                    {
						globalInput.buttons[Input::B_DOWN] = isDown ? 1 : 0;
                        break;
                    }
                    case VK_LEFT:
                    {
						globalInput.buttons[Input::B_LEFT] = isDown ? 1 : 0;
                        break;
                    }
                    case VK_RIGHT:
                    {
						globalInput.buttons[Input::B_RIGHT] = isDown ? 1 : 0;
                        break;
                    }
                    case 'Z':
                    {
						globalInput.buttons[Input::B_A] = isDown ? 1 : 0;
                        break;
                    }
                    case 'X':
                    {
						globalInput.buttons[Input::B_B] = isDown ? 1 : 0;
                        break;
                    }
                    case VK_RETURN:
                    {
						globalInput.buttons[Input::B_START] = isDown ? 1 : 0;
                        break;
                    }
                    case VK_SHIFT:
                    {
						globalInput.buttons[Input::B_SELECT] = isDown ? 1 : 0;
                        break;
                    }
                    case VK_SPACE:
                    {                        
                        break;
                    }
                    case VK_ESCAPE:
                    {
                        globalRunning = false;
                        break;
                    }
                    case VK_F4:
                    {
                        if(altPressed)
                        {
                            globalRunning = false;
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
                    newRom.hwndOwner = windowHandle;
                    newRom.lpstrFile = tempFileName;
                    newRom.lpstrFile[0] = '\0';
                    newRom.nMaxFile = sizeof(tempFileName);
                    newRom.lpstrFilter = ".nes\0*.nes\0";
                    newRom.nFilterIndex = 1;
                    newRom.lpstrFileTitle = NULL;
                    newRom.nMaxFileTitle = 0;
                    newRom.lpstrInitialDir= NULL;
                    newRom.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

                    b32 fileOpened = GetOpenFileName(&newRom); 
                    
                    if(fileOpened) // If exists then restart emulator with new file 
                    {
                        ZeroMemory(&romFileName, sizeof(romFileName));
                        u16 nameSize = (u16)strlen(tempFileName);
                        MemoryCopy((u8 *)romFileName, (u8 *)tempFileName, nameSize);
                        
                        if(globalNes->powerOn)
                        {
                            Reset(globalNes);
                        }
                        else
                        {
                            Power(globalNes);
                        }
                    }
                    
                    break;
                }
                case ID_CLOSE_ROM_ITEM:
                {
                    ZeroMemory(&romFileName, sizeof(romFileName));
                    Power(globalNes);
                    break;
                }
                case ID_QUIT_ITEM:
                {
                    globalRunning = false;
                    break;
                }
            }
            break;
        }
                
        default:
        {
            result = DefWindowProc(windowHandle, msg, wParam, lParam);
            break;
        }
    }
    return result;
}

static void GetWindowSize(HWND window, u16 *width, u16 *height)
{
    RECT clientRect;
    GetClientRect(window, &clientRect);
    *width = (u16)(clientRect.right - clientRect.left);
    *height = (u16)(clientRect.bottom - clientRect.top);
}

static void CreateBackBuffer(ScreenBuffer *buffer, u16 width, u16 height)
{
    if(buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

	buffer->width = width;
    buffer->height = height;
    buffer->bytesPerPixel = 4; // TODO: Should it be 3 instead because of no alpha?
	
	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = width;
	buffer->info.bmiHeader.biHeight = -height; // Negative tells windows that we raster top to bottom
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
	buffer->info.bmiHeader.biCompression = BI_RGB;

    int memorySize = width * height * buffer->bytesPerPixel;
	buffer->memory = VirtualAlloc(0, memorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	buffer->pitch = width * buffer->bytesPerPixel;
}

static void DrawScreenBuffer(ScreenBuffer *backBuffer, HDC deviceContext,
                               u16 windowWidth, u16 windowHeight)
{                
    StretchDIBits(deviceContext,
                  0, 0, windowWidth, windowHeight,
                  0, 0, backBuffer->width, backBuffer->height,
				  backBuffer->memory,
                  &backBuffer->info,
                  DIB_RGB_COLORS, SRCCOPY);
}


struct WinSound
{    
    u16 samplesPerSecond;
    u8 channels;
    u8 bitsPerSample;
    u16 bytesPerSample;
    u16 bufferSize;
    u32 sampleIndex;
    u32 safetyBytes;
    s16 *samples;

    b32 valid;
};

struct NesSound
{
    u16 samplesPerSecond;
    u32 sampleCount;
    u32 bytesToWrite;
    void *samples;
};

static void ClearSoundBuffer(LPDIRECTSOUNDBUFFER buffer, u32 bufferSize)
{
    VOID *firstSection;
    DWORD firstSectionSize;
    VOID *secondSection;
    DWORD secondSectionSize;
    
    HRESULT locked = buffer->Lock(0, bufferSize,
                                  &firstSection, &firstSectionSize,
                                  &secondSection, &secondSectionSize, 0);
    if(SUCCEEDED(locked))
    {
        Assert((firstSectionSize + secondSectionSize) == bufferSize);

		ZeroMemory(firstSection, firstSectionSize);
		ZeroMemory(secondSection, secondSectionSize);

        buffer->Unlock(firstSection, firstSectionSize, secondSection, secondSectionSize);
    }
}

// NOTE: Taken from Handmade hero and adjusted
static void FillSoundBuffer(LPDIRECTSOUNDBUFFER buffer, u16 bytesPerSample, DWORD byteToLock, DWORD bytesToWrite,
                s16 *sourceSample, u32 *sourceRunningIndex)
{
    VOID *firstSection;
    DWORD firstSectionSize;
    VOID *secondSection;
    DWORD secondSectionSize;

    HRESULT locked = buffer->Lock(byteToLock, bytesToWrite,
                                      &firstSection, &firstSectionSize,
                                      &secondSection, &secondSectionSize, 0);
    
    if(SUCCEEDED(locked))
    {
        DWORD firstSectionSampleCount = firstSectionSize / bytesPerSample;
        s16 *destSample = (s16 *)firstSection;

        for(DWORD Index = 0; Index < firstSectionSampleCount; ++Index)
        {
            *destSample++ = *sourceSample++;
            *destSample++ = *sourceSample++;
            ++(*sourceRunningIndex);
        }

        DWORD secondSectionSampleCount = secondSectionSize / bytesPerSample;
        destSample = (s16 *)secondSection;
        for(DWORD Index = 0; Index < secondSectionSampleCount; ++Index)
        {
            *destSample++ = *sourceSample++;
            *destSample++ = *sourceSample++;
            ++(*sourceRunningIndex);
        }

        buffer->Unlock(firstSection, firstSectionSize, secondSection, secondSectionSize);
    }
}

// NOTE: Taken and adapted Handmade hero code
static LPDIRECTSOUNDBUFFER CreateSoundBuffer(HWND WindowHandle, u32 BufferSize, u8 Channels,
                                             u32 SamplesPerSecond, u8 BitsPerSample)
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

static void UpdateAudio(LPDIRECTSOUNDBUFFER SoundBuffer, WinSound *soundOut,
                        u32 FrameUpdateHz, r32 FrameTargetSeconds, r32 FrameTimeElapsed)
{
    DWORD PlayCursor;
    DWORD WriteCursor;                
    if(SoundBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK)
    {
        if(!soundOut->valid)
        {
			soundOut->sampleIndex = WriteCursor / soundOut->bytesPerSample;
			soundOut->valid = true;
        }
                    
        DWORD LockByte = ((soundOut->sampleIndex * soundOut->bytesPerSample) % soundOut->bufferSize);

        DWORD FrameExpectedSoundBytes = (int)((r32)(soundOut->samplesPerSecond * soundOut->bytesPerSample) /
                                              FrameUpdateHz);

        r32 FrameTimeLeft = (FrameTargetSeconds - FrameTimeElapsed);
        DWORD FrameExpectedBytesLeft = (DWORD)((FrameTimeLeft / FrameTargetSeconds) * (r32)FrameExpectedSoundBytes);

        DWORD ExpectedFrameBoundaryByte = PlayCursor + FrameExpectedBytesLeft;

        DWORD SafeWriteCursor = WriteCursor;
        if(SafeWriteCursor < PlayCursor)
        {
            SafeWriteCursor += soundOut->bufferSize;
        }
        Assert(SafeWriteCursor >= PlayCursor);
        SafeWriteCursor += soundOut->safetyBytes;

        b32 AudioCardIsLowLatency = (SafeWriteCursor < ExpectedFrameBoundaryByte);

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
				soundOut->safetyBytes);
        }

        // Mod to reposition in ring buffer
        TargetCursor = (TargetCursor % soundOut->bufferSize);

        DWORD BytesToWrite = 0;
        if(LockByte > TargetCursor)
        {
            BytesToWrite = (soundOut->bufferSize - LockByte);
            BytesToWrite += TargetCursor;
        }
        else
        {
            BytesToWrite = TargetCursor - LockByte;
        }

		NesSound nesSound = {};
		nesSound.samplesPerSecond = soundOut->samplesPerSecond;
		nesSound.sampleCount = Align8(BytesToWrite / soundOut->bytesPerSample);
		nesSound.bytesToWrite = nesSound.sampleCount * soundOut->bytesPerSample;
		nesSound.samples = soundOut->samples;

        /*
        if(Game.GetSoundSamples)
        {
            Game.GetSoundSamples(&GameMemory, &SoundBuffer);
        }
        */

        FillSoundBuffer(SoundBuffer, soundOut->bytesPerSample, LockByte, BytesToWrite, soundOut->samples, &soundOut->sampleIndex);
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
    globalPerfCountFrequency = WinPerfCountFrequency.QuadPart;            

    printf("Test\n");
    
    /**************************************/
    /* NOTE : Screen back buffer creation */
    
    u16 RenderScaleWidth = 256, RenderScaleHeight = 240;
    u8 ResScale = 2;
    u16 WindowWidth = RenderScaleWidth * ResScale, WindowHeight = RenderScaleHeight * ResScale;
    globalScreenBackBuffer = {};
    CreateBackBuffer(&globalScreenBackBuffer, RenderScaleWidth, RenderScaleHeight);

    /**************************/
    /* NOTE : Window creation */
    
    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = WinInputCallback;
    WindowClass.hInstance = GetModuleHandle(NULL);//WindowInstance;
    WindowClass.lpszClassName = "tuNES";

    u16 InitialWindowPosX = 700;
    u16 InitialWindowPosY = 400;
    
    if(RegisterClassA(&WindowClass))
    {        
        HWND Window = CreateWindowExA(0, WindowClass.lpszClassName, "tuNES", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                      InitialWindowPosX, InitialWindowPosY, WindowWidth, WindowHeight,
                                      0, 0, WindowClass.hInstance, 0);

        if(Window) // If window was created successfully
        {
            r32 FrameHz = 60.0988f; // aka fps. // TODO: Will be different for PAL
            r32 FrameTargetSeconds = 1.0f / FrameHz;
            
            /********************************/
            /* NOTE : Window Menu Creation  */            
            
            HMENU WindowMenu = CreateMenu();
            HMENU SubMenu = CreatePopupMenu();
            
            AppendMenu(SubMenu, MF_STRING, ID_OPEN_ROM_ITEM, "&Open Rom");
            AppendMenu(SubMenu, MF_STRING, ID_CLOSE_ROM_ITEM, "&Close Rom");
            AppendMenu(SubMenu, MF_STRING, ID_QUIT_ITEM, "&Quit");
            AppendMenu(WindowMenu, MF_STRING | MF_POPUP, (u64)SubMenu, "&File");

            SetMenu(Window, WindowMenu);

            /********************************/
            /* NOTE : Sound Buffer Creation */

            WinSound winSound = {};
			winSound.samplesPerSecond = 48000;
			winSound.channels = 2; // Sterio
			winSound.bitsPerSample = 16; // Bit depth
			winSound.bytesPerSample = sizeof(s16) * winSound.channels;
			winSound.bufferSize = winSound.samplesPerSecond * winSound.bytesPerSample;
			winSound.safetyBytes = (int)(((r32)winSound.samplesPerSecond * (r32)winSound.bytesPerSample
                                          / FrameHz) / 3.0f);

            winSound.samples = (s16 *)VirtualAlloc(0, (size_t)winSound.bufferSize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
            
            LPDIRECTSOUNDBUFFER soundBuffer = CreateSoundBuffer(Window, winSound.bufferSize, winSound.channels,
				winSound.samplesPerSecond, winSound.bitsPerSample);

            ClearSoundBuffer(soundBuffer, winSound.bufferSize);
			soundBuffer->Play(0, 0, DSBPLAY_LOOPING);
                        
            /********************************************************/
            /* NOTE : Timing */
            // Using the vertical scan rate of ~60Hz or 16ms a frame.
            // We run cpu and ppu 16ms worth of ticks. After 16ms
            // elapsed, we then reset the counters that tracks how
            // ticks have been. Repeat
            
            nes Nes = createNes("../roms/Zelda.nes");
            Nes.FrameClockTotal = Nes.CpuHz * FrameTargetSeconds; // TODO: Put in create nes?
            
            GlobalNes = &Nes;

            ////
            
            r32 ElapsedTime = 0.0;
            u32 ClocksRun = 0;
            r32 FrameTime = 0.0;
            
            LARGE_INTEGER LastClock = GetClock();
            LARGE_INTEGER FrameFlippedClock = GetClock();
            LARGE_INTEGER FrameLastFlippedClock = GetClock();
            
            /********************/
            /* NOTE : Main Loop */

            globalRunning = true; 
            while(globalRunning)
            {
                MSG Message = {}; 
                while (PeekMessage(&Message, Window, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                
                if(Nes.FrameClocksElapsed < Nes.FrameClockTotal)
                {
                    runNes(&Nes, &globalInput);
                }
                else
                {
                    Sleep(2);
                }

                /*
                LARGE_INTEGER FrameSoundClock = getClock();
                r32 FrameTimeElapsed = getSecondsElapsed(FrameFlippedClock, FrameSoundClock);
                UpdateAudio(SoundBuffer, &WinSound, FrameHz, FrameTargetSeconds, FrameTimeElapsed);
                */

                // NOTE: TODO: The cpu might run several more cycles before we get to drawn screen.
                // This could lead to pixels from the next frame overwriting other pixels we haven't displayed
                // To fix, I could implement a double buffer. Two buffers, one is drawn on buy the ppu while the other is the displayed.
                // They are then swapped. This will let the complete frame to be displayed while the new one can created.
                
                if(globalDrawScreen)
                {
					globalDrawScreen = false;
                                                              
                    GetWindowSize(Window, &WindowWidth, &WindowHeight);
                
                    // NOTE: Drawing the backbuffer to the window 
                    HDC DeviceContext = GetDC(Window);
                    DrawScreenBuffer(&globalScreenBackBuffer, DeviceContext,
                                     WindowWidth, WindowHeight);
                    ReleaseDC(Window, DeviceContext);

                    FrameFlippedClock = GetClock();

                    FrameTime = GetSecondsElapsed(FrameLastFlippedClock, FrameFlippedClock);
                    FrameLastFlippedClock = GetClock();
                }

                LARGE_INTEGER EndClock = GetClock();
                
                r32 LoopTime = GetSecondsElapsed(LastClock, EndClock);
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
