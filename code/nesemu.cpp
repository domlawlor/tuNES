#include <windows.h>

// Included for sprintf, TODO: Remove later?
#include <stdio.h>

#define internal static 
#define local_static static 
#define global static

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#include <stdint.h>
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;

typedef size_t mem_idx;


// A, B, Select, Start, Up, Down, Left, Right
struct input {
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
    bool32 buttons[BUTTON_NUM];
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

global input WinInput = {};
global bool32 GlobalRunning;

internal real32 getMilliSeconds(uint64 PerfCountFrequency)
{
    LARGE_INTEGER Counter;
    QueryPerformanceCounter(&Counter);
    
    uint64 CounterElapsed = Counter.QuadPart;
    real32 MSElapsed = ((1000.0f * (real32)CounterElapsed) / (real32)PerfCountFrequency);

    return(MSElapsed);
}

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
                    // NOTE: Up and down changes the octave the keys are in
                    case VK_UP:
                    {
                        WinInput.buttons[input::B_UP] = !WinInput.buttons[input::B_UP];
                        break;
                    }
                    case VK_DOWN:
                    {
                        WinInput.buttons[input::B_DOWN] = !WinInput.buttons[input::B_DOWN];
                        break;
                    }
                    case VK_LEFT:
                    {
                        WinInput.buttons[input::B_LEFT] = !WinInput.buttons[input::B_LEFT];
                        break;
                    }
                    case VK_RIGHT:
                    {
                        WinInput.buttons[input::B_RIGHT] = !WinInput.buttons[input::B_RIGHT];
                        break;
                    }
                    case 'Z':
                    {
                        WinInput.buttons[input::B_A] = !WinInput.buttons[input::B_A];
                        break;
                    }
                    case 'X':
                    {
                        WinInput.buttons[input::B_B] = !WinInput.buttons[input::B_B];
                        break;
                    }
                    case VK_RETURN:
                    {
                        WinInput.buttons[input::B_START] = !WinInput.buttons[input::B_START];
                        break;
                    }
                    case VK_SHIFT:
                    {
                        WinInput.buttons[input::B_SELECT] = !WinInput.buttons[input::B_SELECT];
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
        default:
        {
            Result = DefWindowProc(WindowHandle, Message, wParam, lParam); 
        }
    }
    return Result;
}

internal void * LoadFile(char * Filename, uint32 *Size)
{
    void *FileData = 0;
    
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER Filesize;
        if(GetFileSizeEx(FileHandle, &Filesize))
        {
            FileData = VirtualAlloc(0, Filesize.LowPart, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(FileData)
            {
                DWORD BytesRead;
                if(ReadFile(FileHandle, FileData, Filesize.LowPart, &BytesRead, 0) &&
                   (Filesize.LowPart == BytesRead))
                {
                    *Size = (uint32)BytesRead;
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
        Assert(0);
    }
    return(FileData);
}


internal void cpyMemory(uint8 *Dest, uint8 *Src, uint16 Size)
{
    // NOTE: Very basic copy. Not bounds protection
    for(uint16 Byte = 0; Byte < Size; ++Byte)
        Dest[Byte] = Src[Byte];
}

internal void writeMemory8(uint8 Byte, uint16 Address, uint64 MemoryOffset)
{   
    uint8 *NewAddress = (uint8 *)(Address + MemoryOffset);
    *NewAddress = Byte;
}

internal uint8 readMemory8(uint16 Address, uint64 MemoryOffset)
{
    uint8 *NewAddress = (uint8 *)(Address + MemoryOffset);
    uint8 Value = *NewAddress;
    return(Value);
}

bool32 NMICalled = false;
bool32 ResetScrollIOAdrs = false;
bool32 ResetVRamIOAdrs = false;

uint8 VRamIOAdrsCount = 0;
uint8 PrevVRamIOAdrsCount;

uint8 VRamIOWriteCount = 0;
uint8 PrevVRamIOWriteCount;


// TODO: This will change location once other functions above get relocated.
#include "cpu.cpp"
#include "ppu.cpp"

internal void getWindowSize(HWND Window, uint16 *Width, uint16 *Height)
{
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    *Width = ClientRect.right - ClientRect.left;
    *Height = ClientRect.bottom - ClientRect.top;
}

    
internal void createBackBuffer(screen_buffer *Buffer, uint16 Width, uint16 Height)
{
    // TODO: This is based on Handmade Hero code. Will need to reference and look at licences later on
    //       website: handmadehero.org
    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;

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

internal void drawScreenBuffer(screen_buffer *BackBuffer, HDC DeviceContext,
                      uint16 WindowWidth, uint16 WindowHeight)
{                
    StretchDIBits(DeviceContext,
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, BackBuffer->Width, BackBuffer->Height,
                  BackBuffer->Memory,
                  &BackBuffer->Info,
                  DIB_RGB_COLORS, SRCCOPY);
}

int CALLBACK
WinMain(HINSTANCE WindowInstance, HINSTANCE PrevWindowInstance,
        LPSTR CommandLine, int CommandShow)
{
    // TODO: I could reduce the memory usage as the nes does not actually use 64 kbs,
    //       the nes mirrors certain sections of address space
    uint32 CpuMemorySize = Kilobytes(64);
    uint32 PpuMemorySize = Kilobytes(64);
    uint32 TotalMemorySize = CpuMemorySize + PpuMemorySize;

    // NOTE: Aiming to have one memory allocation for the whole program.
    // TODO: Loading the cartridge also creates memory. Figure out to include in this call.
    uint8 * Memory = (uint8 *)VirtualAlloc(0, (size_t)TotalMemorySize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

    cpu CpuData = {};
    CpuData.MemoryOffset = (uint64)Memory; 
    CpuData.Registers.StackPtr = 0xFF;
    
    ppu PpuData = {};    
    PpuData.MemoryOffset = (uint64)Memory + Kilobytes(64);
      
#define PPU_REG_ADRS 0x2000    
    PpuData.Registers = (ppu_registers *)(CpuData.MemoryOffset + PPU_REG_ADRS);
    PpuData.Registers->Status = (1 << 7) | (1 << 5); 
    
    
    // Reading rom file
    char * Filename = "Baseball.nes";
    uint32 FileSize;
    uint8 *RomData = (uint8 *)LoadFile(Filename, &FileSize);

    // NOTE: Check for correct header
    if(RomData[0] != 'N' || RomData[1] != 'E' || RomData[2] != 'S')
    {
        Assert(0);   
    }

    // NOTE: Read header
    uint8 RomPrgBankCount = RomData[4];
    uint8 RomChrBankCount = RomData[5];

    uint8 Flags6 = RomData[6];
    uint8 Flags7 = RomData[7];

    uint8 RomPrgRamSize = RomData[8];
    uint8 Flags9 = RomData[9];
    uint8 Flags10 = RomData[10];

    uint8 *RomPrgData;
    // NOTE: If trainer present. Data after header and before program data
    if(Flags6 & (1 << 2))
    {
        Assert(1); 
    }
    else
    {
        RomPrgData = RomData + 16;
    }

    uint8 *RomChrData = RomPrgData + (RomPrgBankCount * Kilobytes(16));
    // TODO: Implement Playchoice roms 



// NOTE: This is the two banks of memory that are currently loaded
//       The mapper number will specify which initial banks are loaded
//       Program will then change these banks while running.
//       These pointers reference the rom memory.
//       Offset is required to make relative to memory mapped address    

    
    // TODO: Mappers
    // NOTE: MY UNDERSTANDING OF HOW BANK REGISTERS WORK (SO FAR)
    //
    //       This is for M001. 
    //       On the catridge, 4 registers are stored, each is 5 bits wide
    //       These registers cannot be accessed directly, but can be set.
    //       To set, writing to the prg rom 5 times will save the value.
    //       To select the register to write too, the fifth write will be
    //       to one of the prg banks. Only the fifth write will write to the specified
    //       bank. If you write to 0x8000 4 times, then 0xE000 the fifth, only
    //       0xE000 register will be written too. 0xE004 counts as 0xE000, and 0x8001
    //       counts as 0x8000
    //       There is a temporary port too
    //       This is a byte with a reset bit and the bit just entered.
    //       If reset is hit, then bit entered, plus temporary reg is cleared.
    //       If 5 bits enter through port, then reg is saved

    // TODO: This will change as I add program mappers 
    uint16 MemPrgBank1 = 0x8000;
    uint16 MemPrgBank2 = 0xC000;
    
    uint8 MapperNumber = (Flags7 & 0xF0) | (Flags6 >> 4);
    switch(MapperNumber)
    {
        case 0:
        {
            if(RomPrgBankCount == 1) {
                cpyMemory((uint8 *)MemPrgBank1 + CpuData.MemoryOffset, RomPrgData, Kilobytes(16));
                cpyMemory((uint8 *)MemPrgBank2 + CpuData.MemoryOffset, RomPrgData, Kilobytes(16));
            }
            else
                Assert(0);
            break;
        }
        
        default:
        {
            char Buffer[8];
            sprintf(Buffer, "Error: Unknown mapper number = %d\n", MapperNumber);
            OutputDebugString(Buffer);
            Assert(0);
            break;
        }
    }
    
    // NOTE: Load the program counter with the reset vector
    CpuData.Registers.PrgCounter = readCpuMemory16(RESET_VEC, CpuData.MemoryOffset);

    
    // Screen back buffer creation
    uint16 RenderScaleWidth = 256, RenderScaleHeight = 240;
    uint8 ResScale = 5;
    uint16 WindowWidth = RenderScaleWidth * ResScale, WindowHeight = RenderScaleHeight * ResScale;
    screen_buffer ScreenBackBuffer = {};
    createBackBuffer(&ScreenBackBuffer, RenderScaleWidth, RenderScaleHeight);

    PpuData.ZeroPixel = (uint32 *)ScreenBackBuffer.Memory;


    
    // Window Creation
    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = WinInputCallback;
    WindowClass.hInstance = WindowInstance;
    WindowClass.lpszClassName = "NesEmu";

    LARGE_INTEGER WinPerfCountFrequency;
    QueryPerformanceFrequency(&WinPerfCountFrequency); 
    uint64 PerfCountFrequency = WinPerfCountFrequency.QuadPart;            

    
    if(RegisterClassA(&WindowClass))
    {
        HWND Window = CreateWindowExA(0, WindowClass.lpszClassName, "NesEmu",
                                      WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                      CW_USEDEFAULT, CW_USEDEFAULT,
                                      WindowWidth, WindowHeight,
                                      0, 0, WindowInstance, 0);
        if(Window) // If window was created successfully
        {
            LARGE_INTEGER LastCounter;
            QueryPerformanceCounter(&LastCounter);                        
            uint64 LastCycles = __rdtsc();
            
            // TODO: Must run the emulation at the same speed as the nes would.
            GlobalRunning = true;
            
                        
            real32 CurrentMS, PrevMS = getMilliSeconds(PerfCountFrequency);
            real32 ElapsedMS = 0;
            while(GlobalRunning)
            {
                MSG Message = {}; 
                while (PeekMessage(&Message, Window, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                cpuTick(&CpuData);
                // TODO: Timing is not complete
                //       Cpu opcodes execute in a different number of cycles
                //       Must multiply pputicks to stay synched.
                for(uint8 i = 0; i < 3; ++i)
                {
                    ppuTick(&ScreenBackBuffer, &PpuData);
                }
          
                getWindowSize(Window, &WindowWidth, &WindowHeight);
                
                // NOTE: Drawing the backbuffer to the window 
                HDC DeviceContext = GetDC(Window);
                drawScreenBuffer(&ScreenBackBuffer, DeviceContext,
                                 WindowWidth, WindowHeight);
                ReleaseDC(Window, DeviceContext);

                CurrentMS = getMilliSeconds(PerfCountFrequency);
                ElapsedMS = CurrentMS - PrevMS;
                PrevMS = CurrentMS;
            }
        }
        else
        {
            Assert(0);
        }
    }
    else
    {
        Assert(0);
    }
    return(0);
} 



#if 0
                uint64 EndCycles = __rdtsc();
                
                LARGE_INTEGER EndCounter;
                QueryPerformanceCounter(&EndCounter);
                
                uint64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart; 
                uint64 CyclesElapsed = EndCycles - LastCycles;
                
                real32 MSElapsed = ((1000.0f * (real32)CounterElapsed) / (real32)PerfCountFrequency);
                real32 FPSElapsed = (real32)PerfCountFrequency / (real32)CounterElapsed;
                real32 MCElapsed = (real32)CyclesElapsed / (1000.0f*1000.0f);                 
                
                char TextBuffer[256];
                _snprintf(TextBuffer, 256, "Cycles: %f, FPS: %f, DeltaTime: %f\n", MCElapsed, FPSElapsed, MSElapsed);
                OutputDebugString(TextBuffer);
                
                LastCounter = EndCounter;
                LastCycles = EndCycles;
#endif
