#include <windows.h>
#include <stdio.h>

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

#define PPU_REG_ADRS 0x2000    
#define OAM_SIZE 0x100
#define OAM_SPRITE_TOTAL 64

bool32 IOReadFromCpu;
bool32 IOWriteFromCpu;
bool32 ScrollAdrsChange;
bool32 VRamAdrsChange;
bool32 ResetScrollIOAdrs;
bool32 ResetVRamIOAdrs;

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

global bool32 DrawScreen = false;

global input WinInput = {};
global bool32 GlobalRunning;

static real32 getMilliSeconds(uint64 PerfCountFrequency)
{
    LARGE_INTEGER Counter;
    QueryPerformanceCounter(&Counter);
    
    uint64 CounterElapsed = Counter.QuadPart;
    real32 MSElapsed = ((1000.0f * (real32)CounterElapsed) / (real32)PerfCountFrequency);

    return(MSElapsed);
}

static void cpyMemory(uint8 *Dest, uint8 *Src, uint16 Size)
{
    // NOTE: Very basic copy. Not bounds protection
    for(uint16 Byte = 0; Byte < Size; ++Byte)
        Dest[Byte] = Src[Byte];
}


#define ID_OPEN_ROM_ITEM        1001
#define ID_CLOSE_ROM_ITEM        1002
#define ID_QUIT_ITEM            1003

#define MAX_ROM_NAME_SIZE 256

global bool32 MapperExtWrite = false;

global bool32 PowerOn = true;
global bool32 PowerHit = false;
global bool32 ResetHit = false;
global char RomFileName[MAX_ROM_NAME_SIZE]; 

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
                        WinInput.buttons[input::B_UP] = IsDown ? 1 : 0;
                        break;
                    }
                    case VK_DOWN:
                    {
                        WinInput.buttons[input::B_DOWN] = IsDown ? 1 : 0;
                        break;
                    }
                    case VK_LEFT:
                    {
                        WinInput.buttons[input::B_LEFT] = IsDown ? 1 : 0;
                        break;
                    }
                    case VK_RIGHT:
                    {
                        WinInput.buttons[input::B_RIGHT] = IsDown ? 1 : 0;
                        break;
                    }
                    case 'Z':
                    {
                        WinInput.buttons[input::B_A] = IsDown ? 1 : 0;
                        break;
                    }
                    case 'X':
                    {
                        WinInput.buttons[input::B_B] = IsDown ? 1 : 0;
                        break;
                    }
                    case VK_RETURN:
                    {
                        WinInput.buttons[input::B_START] = IsDown ? 1 : 0;
                        break;
                    }
                    case VK_SHIFT:
                    {
                        WinInput.buttons[input::B_SELECT] = IsDown ? 1 : 0;
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
                    newRom.nFilterIndex =1;
                    newRom.lpstrFileTitle = NULL ;
                    newRom.nMaxFileTitle = 0 ;
                    newRom.lpstrInitialDir=NULL ;
                    newRom.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST ;

                    bool32 FileOpened = GetOpenFileName(&newRom); 
                    
                    if(FileOpened) // If exists then restart emulator with new file 
                    {
                        ZeroMemory(&RomFileName, sizeof(RomFileName));
                        uint8 NameSize = strlen(tempFileName);
                        cpyMemory((uint8 *)RomFileName, (uint8 *)tempFileName, NameSize);
                        
                        if(PowerOn)
                        {
                            ResetHit = true;
                        }
                        else
                        {
                            PowerHit = true;
                        }
                    }
                    
                    break;
                }
                case ID_CLOSE_ROM_ITEM:
                {
                    ZeroMemory(&RomFileName, sizeof(RomFileName));
                    PowerHit = true;
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

static void * LoadFile(char * Filename, uint32 *Size)
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
//        Assert(0);
    }
    return(FileData);
}

global uint8 *OamData = 0;

bool32 TriggerNmi = false;
bool32 NmiTriggered = false;
bool32 IrqTriggered = false;

bool32 OamDataChange = false;

// TODO: This will change location once other functions above get relocated.
#include "ppu.cpp"
#include "cpu.cpp"

#include "nes.h"

static void getWindowSize(HWND Window, uint16 *Width, uint16 *Height)
{
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    *Width = ClientRect.right - ClientRect.left;
    *Height = ClientRect.bottom - ClientRect.top;
}

static void createBackBuffer(screen_buffer *Buffer, uint16 Width, uint16 Height)
{
    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4; // TODO: Check if this is wrong. Should it be 3 instead? No alpha value

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

    
static void
initCpu(cpu *Cpu, uint64 MemoryBase)
{
    Cpu->MemoryBase = MemoryBase;
}

static void
initPpu(ppu *Ppu, uint64 MemoryBase, uint32 * BasePixel)
{
    OamData = Ppu->Oam;

    Ppu->MemoryBase = MemoryBase;
    Ppu->BasePixel = BasePixel;
}



void nromInit(cartridge *Cartridge, cpu *Cpu, ppu *Ppu)
{
    uint16 MemPrgBank1 = 0x8000;
    uint16 MemPrgBank2 = 0xC000;

    uint8 * BankToCpy1;
    uint8 * BankToCpy2;
        
    if(Cartridge->PrgBankCount == 1)
    {
        BankToCpy1 = Cartridge->PrgData;
        BankToCpy2 = Cartridge->PrgData;
    }
    else if(Cartridge->PrgBankCount == 2)
    {
        BankToCpy1 = Cartridge->PrgData;
        BankToCpy2 = Cartridge->PrgData + Kilobytes(16);
    }
        
    cpyMemory((uint8 *)MemPrgBank1 + Cpu->MemoryBase, BankToCpy1, Kilobytes(16));
    cpyMemory((uint8 *)MemPrgBank2 + Cpu->MemoryBase, BankToCpy2, Kilobytes(16));

    // Map CHR Data to Ppu
    if(Cartridge->ChrBankCount == 1)
    {
        cpyMemory((uint8 *)Ppu->MemoryBase, Cartridge->ChrData, Kilobytes(8));
    }
}

void mmc1Init(cartridge *Cartridge, cpu *Cpu, ppu *Ppu)
{
    uint16 MemPrgBank1 = 0x8000;
    uint16 MemPrgBank2 = 0xC000;

    uint8 * BankToCpy1 = Cartridge->PrgData;
    uint8 * BankToCpy2 = Cartridge->PrgData + ((Cartridge->PrgBankCount - 1) * Kilobytes(16));
           
    cpyMemory((uint8 *)MemPrgBank1 + Cpu->MemoryBase, BankToCpy1, Kilobytes(16));
    cpyMemory((uint8 *)MemPrgBank2 + Cpu->MemoryBase, BankToCpy2, Kilobytes(16));
 }

void unromInit(cartridge *Cartridge, cpu *Cpu, ppu *Ppu)
{
    uint16 MemPrgBank1 = 0x8000;
    uint16 MemPrgBank2 = 0xC000;

    uint8 * BankToCpy1 = Cartridge->PrgData;
    uint8 * BankToCpy2 = Cartridge->PrgData + ((Cartridge->PrgBankCount - 1) * Kilobytes(16));
           
    cpyMemory((uint8 *)MemPrgBank1 + Cpu->MemoryBase, BankToCpy1, Kilobytes(16));
    cpyMemory((uint8 *)MemPrgBank2 + Cpu->MemoryBase, BankToCpy2, Kilobytes(16));
}

void axromInit(cartridge *Cartridge, cpu *Cpu, ppu *Ppu)
{
    uint16 MemoryPrgBank = 0x8000;
    uint8 *BankToCpy = Cartridge->PrgData + ((Cartridge->PrgBankCount) * Kilobytes(16)) - Kilobytes(32);
    cpyMemory((uint8 *)MemoryPrgBank + Cpu->MemoryBase, BankToCpy, Kilobytes(32));

    Ppu->MirrorType = SINGLE_SCREEN_BANK_A;
}

#define MAPPER_TOTAL 8

void (*mapperInit[MAPPER_TOTAL])(cartridge *Cartridge, cpu *Cpu, ppu *Ppu) =
{
    nromInit, mmc1Init, unromInit, 0, 0, 0, 0, axromInit 
};

void nromUpdate(cartridge *Cartridge, cpu *Cpu, ppu *Ppu)
{
    Assert(0);
}

void mmc1Update(cartridge *Cartridge, cpu *Cpu, ppu *Ppu)
{
    uint16 PrgRomBank1 = 0x8000;
    uint16 PrgRomBank2 = 0xC000;

    bool32 IsLargePrg = (Cartridge->PrgBankCount > 16);
    bool32 IsLargeChr = (Cartridge->ChrBankCount > 1);

    // TODO: Figure a way to deal with these static values
    static uint8 PrgRomMode;
    static uint8 ChrRomMode;
    
    if(Cpu->MapperWrite)
    {
        Cpu->MapperWrite = false;
        
        bool32 IsClearBitSet = (Cpu->MapperReg & (1 << 7)) != 0;
        if(IsClearBitSet)
        {
            Cpu->MapperWriteCount = 0;
            Cpu->MapperReg = 0;
            Cartridge->MapperInternalReg = 0;
        }
        else
        {
            ++Cpu->MapperWriteCount;
            
            Cartridge->MapperInternalReg = (Cartridge->MapperInternalReg << 1);
            Cartridge->MapperInternalReg |= (Cpu->MapperReg & 1);
            
            if(Cpu->MapperWriteCount == 5) // On 5th write
            {
                uint8 DataReg = Cartridge->MapperInternalReg;
                
                bool32 bit13Set = (Cpu->MapperWriteAddress & (1 << 13)) != 0;
                bool32 bit14Set = (Cpu->MapperWriteAddress & (1 << 14)) != 0;

                if(!bit13Set && !bit14Set)     // Control Reg
                {
                    uint8 Mirror = DataReg & 3;
                    if(Mirror == 0)
                        Ppu->MirrorType = SINGLE_SCREEN_BANK_A;
                    if(Mirror == 1)
                        Ppu->MirrorType = SINGLE_SCREEN_BANK_B;
                    if(Mirror == 2)
                        Ppu->MirrorType = VERTICAL_MIRROR;
                    if(Mirror == 3)
                        Ppu->MirrorType = HORIZONTAL_MIRROR;
                                        
                    PrgRomMode = (DataReg & 12) >> 2;
                    ChrRomMode = (DataReg & (1 << 4)) >> 4;
                }
                else if(bit13Set && !bit14Set) // CHR Bank 0
                {
                    uint8 SizeToCpy = 0;
                    
                    if(ChrRomMode == 1) // 8kb mode Low bit ignored
                    {
                        DataReg = DataReg >> 1;
                        SizeToCpy = Kilobytes(8);
                    }
                    else
                    {
                        SizeToCpy = Kilobytes(4);
                    }
                    uint8 * BankToCpy = Cartridge->ChrData + (DataReg * SizeToCpy);              
                    cpyMemory((uint8 *)Ppu->MemoryBase, BankToCpy, SizeToCpy);
                }
                else if(!bit13Set && bit14Set) // CHR Bank 1
                {
                    if(ChrRomMode == 1) // 4kb mode
                    {
                        uint8 * BankToCpy = Cartridge->ChrData + (DataReg * Kilobytes(4));              
                        cpyMemory((uint8 *)Ppu->MemoryBase + 0x1000, BankToCpy, Kilobytes(4));
                    }
                }
                else if(bit13Set && bit14Set) // PRG bank
                {
                    uint8 * BankToCpy;
                    if(PrgRomMode == 0 || PrgRomMode == 1) // 32kib Mode
                    {
                        DataReg = DataReg >> 1;
                        BankToCpy = Cartridge->PrgData + (DataReg * Kilobytes(32));              
                        cpyMemory((uint8 *)Cpu->MemoryBase + 0x8000, BankToCpy, Kilobytes(32));
                    }
                    else if(PrgRomMode == 2) // 16kb low bank
                    {
                        BankToCpy = Cartridge->PrgData + (DataReg * Kilobytes(32));              
                        cpyMemory((uint8 *)Cpu->MemoryBase + 0x8000, BankToCpy, Kilobytes(16));
                        
                    }
                    else if(PrgRomMode == 3) // 16kb high bank
                    {
                        BankToCpy = Cartridge->PrgData + (DataReg * Kilobytes(32));              
                        cpyMemory((uint8 *)Cpu->MemoryBase + 0xC000, BankToCpy, Kilobytes(16));
                    }
                }
                
                Cpu->MapperWriteCount = 0;
                Cpu->MapperReg = 0;
                Cartridge->MapperInternalReg = 0;
            }
        }
    }
}

void unromUpdate(cartridge *Cartridge, cpu *Cpu, ppu *Ppu)
{
    uint16 MemPrgBank1 = 0x8000;
    uint8 BankNumber = Cpu->MapperReg;
    
    uint8 * BankToCpy = Cartridge->PrgData + (BankNumber * Kilobytes(16));
    cpyMemory((uint8 *)MemPrgBank1 + Cpu->MemoryBase, BankToCpy, Kilobytes(16));
}

void axromUpdate(cartridge *Cartridge, cpu *Cpu, ppu *Ppu)
{
    uint16 MemoryPrgBank = 0x8000;
    
    uint8 SelectedBank = Cpu->MapperReg & 7;
    uint8 *BankToCpy = Cartridge->PrgData + (SelectedBank * Kilobytes(32));
    
    cpyMemory((uint8 *)MemoryPrgBank + Cpu->MemoryBase, BankToCpy, Kilobytes(32));

    // Nametable Single Screen bank select
    if(Cpu->MapperReg & 0x10)
    {
        Ppu->MirrorType = SINGLE_SCREEN_BANK_B;   
    }
    else
    {
        Ppu->MirrorType = SINGLE_SCREEN_BANK_A;
    }
}

void (*mapperUpdate[MAPPER_TOTAL])(cartridge *Cartridge, cpu *Cpu, ppu *Ppu) =
{
    nromUpdate, mmc1Update, unromUpdate, 0, 0, 0, 0, axromUpdate 
};

static void loadCartridge(nes *Nes, char * FileName)
{
    cartridge *Cartridge = &Nes->Cartridge;
    cpu *Cpu = &Nes->Cpu;
    ppu *Ppu = &Nes->Ppu;
        
    // Reading rom file
    Cartridge->FileName = FileName;
    Cartridge->FileSize;
    Cartridge->Data = (uint8 *)LoadFile(FileName, &Cartridge->FileSize);

    if(Cartridge->FileSize == 0)
    {
        PowerOn = false;
        return;
    }
    else
    {
        PowerOn = true;
    
        uint8 * RomData = Cartridge->Data;
        
        // NOTE: Check for correct header
        if(RomData[0] != 'N' || RomData[1] != 'E' || RomData[2] != 'S' || RomData[3] != 0x1A)
            Assert(0);   

        // NOTE: Read header
        Cartridge->PrgBankCount = RomData[4];
        Cartridge->ChrBankCount = RomData[5];
        uint8 Flags6            = RomData[6];        
        uint8 Flags7            = RomData[7];
        Cartridge->PrgRamSize   = RomData[8];
        
        Cartridge->UseVertMirror       = (Flags6 & (1)) != 0;
        Cartridge->HasBatteryRam       = (Flags6 & (1 << 1)) != 0;
        Cartridge->HasTrainer          = (Flags6 & (1 << 2)) != 0;
        Cartridge->UseFourScreenMirror = (Flags6 & (1 << 3)) != 0;
        Cartridge->MapperNum           = (Flags7 & 0xF0) | (Flags6 >> 4);

        Assert(Cartridge->UseFourScreenMirror == 0);
        
        if(Cartridge->UseFourScreenMirror)
            Nes->Ppu.MirrorType = FOUR_SCREEN_MIRROR;
        else if(Cartridge->UseVertMirror)
            Nes->Ppu.MirrorType = VERTICAL_MIRROR;
        else
            Nes->Ppu.MirrorType = HORIZONTAL_MIRROR;      
        
        Cartridge->PrgData = RomData + 16; // PrgData starts after the header info(16 bytes)

        if(Cartridge->HasTrainer)
        {
            Cartridge->PrgData += 512; // Trainer size 512 bytes
        }

        Cartridge->ChrData = Cartridge->PrgData + (Cartridge->PrgBankCount * Kilobytes(16));

        mapperInit[Cartridge->MapperNum](Cartridge, Cpu, Ppu);
    }
}

static void
power(nes *Nes)
{
    PowerOn = !PowerOn;

    if(PowerOn)
    {
        loadCartridge(Nes, RomFileName);
        Nes->Cpu.PrgCounter = (read8(RESET_VEC+1, Nes->Cpu.MemoryBase) << 8) | read8(RESET_VEC, Nes->Cpu.MemoryBase);
    }
    else
    {
        uint64 MemoryBase = Nes->Cpu.MemoryBase;
        Nes->Cpu = {};
        Nes->Cpu.MemoryBase = MemoryBase;

        MemoryBase = Nes->Ppu.MemoryBase;
        uint32 *BasePixel = Nes->Ppu.BasePixel;
        Nes->Ppu = {};
        Nes->Ppu.MemoryBase = MemoryBase;
        Nes->Ppu.BasePixel = BasePixel;
    }
}

static void
reset(nes *Nes)
{
    Nes->Cpu.PrgCounter = readCpu16(RESET_VEC, &Nes->Cpu);

    // NOTE: The status after reset was taken from nesdev
    Nes->Cpu.StackPtr -= 3;
    setInterrupt(&Nes->Cpu.Flags);

    /*
    PpuReg->Ctrl1 = 0;
    PpuReg->Ctrl2 = 0;
    PpuReg->ScrollAddress = 0;
    PpuReg->VRamAddress = 0;

    vram_io *PpuIO = &Nes->Ppu.VRamIO;
    PpuIO->TempVRamAdrs = 0;
    PpuIO->LatchWrite = 0;
    PpuIO->FineX = 0;
    */
    }

int CALLBACK
WinMain(HINSTANCE WindowInstance, HINSTANCE PrevWindowInstance,
        LPSTR CommandLine, int CommandShow)
{
    LARGE_INTEGER WinPerfCountFrequency;
    QueryPerformanceFrequency(&WinPerfCountFrequency); 
    uint64 PerfCountFrequency = WinPerfCountFrequency.QuadPart;            

    /**************************************/
    /* NOTE : Screen back buffer creation */
    
    uint16 RenderScaleWidth = 256, RenderScaleHeight = 240;
    uint8 ResScale = 4;
    uint16 WindowWidth = RenderScaleWidth * ResScale, WindowHeight = RenderScaleHeight * ResScale;
    screen_buffer ScreenBackBuffer = {};
    createBackBuffer(&ScreenBackBuffer, RenderScaleWidth, RenderScaleHeight);

    /**************************/
    /* NOTE : Window creation */

    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = WinInputCallback;
    WindowClass.hInstance = WindowInstance;
    WindowClass.lpszClassName = "Nes Emulator";

    uint16 InitialWindowPosX = 0;
    uint16 InitialWindowPosY = 0;
    
    if(RegisterClassA(&WindowClass))
    {        
        HWND Window = CreateWindowExA(0, WindowClass.lpszClassName, "NesEmu", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                      InitialWindowPosX, InitialWindowPosY, WindowWidth, WindowHeight,
                                      0, 0, WindowInstance, 0);

        if(Window) // If window was created successfully
        {
            HMENU WindowMenu = CreateMenu();
            HMENU SubMenu = CreatePopupMenu();
            
            AppendMenu(SubMenu, MF_STRING, ID_OPEN_ROM_ITEM, "&Open Rom");
            AppendMenu(SubMenu, MF_STRING, ID_CLOSE_ROM_ITEM, "&Close Rom");
            AppendMenu(SubMenu, MF_STRING, ID_QUIT_ITEM, "&Quit");
            AppendMenu(WindowMenu, MF_STRING | MF_POPUP, (uint64)SubMenu, "&File");

            SetMenu(Window, WindowMenu);

            /**************************************************************************/
            /* NOTE : creation and initialization of Emulators Cpu, Ppu, and Cartridge structures */


            // Memory allocation for the Cpu and Ppu.
            uint32 CpuMemorySize = Kilobytes(64);
            uint32 PpuMemorySize = Kilobytes(64);

            uint8 * Memory = (uint8 *)VirtualAlloc(0, (size_t)(CpuMemorySize + PpuMemorySize), MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

            uint64 CpuMemoryBase = (uint64)Memory;
            uint64 PpuMemoryBase = (uint64)Memory + CpuMemorySize;
            GlobalCpuMemoryBase = CpuMemoryBase;
            GlobalPpuMemoryBase = PpuMemoryBase;

            char MemoryInfoBuffer[64];
            sprintf(MemoryInfoBuffer, "Cpu Base = %X , Ppu Base = %X\n", (uint32)CpuMemoryBase, (uint32)PpuMemoryBase);
            OutputDebugString(MemoryInfoBuffer);
            
            nes Nes = {};
            initCpu(&Nes.Cpu, CpuMemoryBase);
            initPpu(&Nes.Ppu, PpuMemoryBase, (uint32 *)ScreenBackBuffer.Memory);
            GlobalCpu = &Nes.Cpu;
            GlobalPpu = &Nes.Ppu;
            
            loadCartridge(&Nes, "Mario Bros.nes");

            // NOTE: Load the program counter with the reset vector
            Nes.Cpu.PrgCounter = readCpu16(RESET_VEC, &Nes.Cpu);

            /*****************/
            /* NOTE : Timing */
            
            real32 CpuClockRateHz = 1789772.727272728;
            real32 CpuCyclesPerMS = CpuClockRateHz / 1000.0;
            
            uint32 CpuCyclesElapsed = 0;
            uint32 TickCycles = 0;
            
            real32 ElapsedSecs = 0;
            real32 CurrentSecs, PrevSecs = getMilliSeconds(PerfCountFrequency) / 1000.0f;

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

                if(PowerHit)
                {
                    PowerHit = false;
                    power(&Nes);
                }
                
                if(ResetHit)
                {
                    ResetHit = false;
                    loadCartridge(&Nes, RomFileName);
                    reset(&Nes);
                }

                if(Nes.Cpu.MapperWrite)
                {
                    Nes.Cpu.MapperWrite = false;
                    mapperUpdate[Nes.Cartridge.MapperNum](&Nes.Cartridge, &Nes.Cpu, &Nes.Ppu);
                }
                
                if(PowerOn)
                {
                    TickCycles = cpuTick(&Nes.Cpu, &WinInput);
                    
                    for(uint8 i = 0; i < (3*TickCycles); ++i)
                    {
                        ppuTick(&ScreenBackBuffer, &Nes.Ppu);
                    }

                    CpuCyclesElapsed += TickCycles;
                }
                
                if(DrawScreen) // NOTE: Gets called everytime the vblank happens in Ppu TODO: Should it be the end of vblank?
                {
                    DrawScreen = false; 
                    getWindowSize(Window, &WindowWidth, &WindowHeight);
                
                    // NOTE: Drawing the backbuffer to the window 
                    HDC DeviceContext = GetDC(Window);
                    drawScreenBuffer(&ScreenBackBuffer, DeviceContext,
                                     WindowWidth, WindowHeight);
                    ReleaseDC(Window, DeviceContext);
                }
           
                CurrentSecs = getMilliSeconds(PerfCountFrequency) / 1000.0f;
                ElapsedSecs = CurrentSecs - PrevSecs;
                PrevSecs = CurrentSecs;
            }

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
