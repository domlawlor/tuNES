
#include <windows.h>

// Included for sprintf
#include <stdio.h>

#define internal static 
#define local_persist static 
#define global_variable static

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

typedef size_t mem_idx;

struct input {
    enum buttons {
        B_UP = 0,
        B_DOWN,
        B_LEFT,
        B_RIGHT,
        B_A,
        B_B,
        B_START,
        B_SELECT,

        BUTTON_NUM
    };
    
    bool32 buttons[BUTTON_NUM];
};

global_variable input WinInput = {};
global_variable bool32 GlobalRunning;

global_variable uint64 MemoryStartOffset;

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
                        OutputDebugString("Up");
                        WinInput.buttons[input::B_UP] = !WinInput.buttons[input::B_UP];
                        break;
                    }
                    case VK_DOWN:
                    {
                        OutputDebugString("Up");
                        WinInput.buttons[input::B_DOWN] = !WinInput.buttons[input::B_DOWN];
                        break;
                    }
                    case VK_LEFT:
                    {
                        OutputDebugString("Up");
                        WinInput.buttons[input::B_LEFT] = !WinInput.buttons[input::B_LEFT];
                        break;
                    }
                    case VK_RIGHT:
                    {
                        OutputDebugString("Up");
                        WinInput.buttons[input::B_RIGHT] = !WinInput.buttons[input::B_RIGHT];
                        break;
                    }
                    case 'Z':
                    {
                        OutputDebugString("Up");
                        WinInput.buttons[input::B_A] = !WinInput.buttons[input::B_A];
                        break;
                    }
                    case 'X':
                    {
                        OutputDebugString("Up");
                        WinInput.buttons[input::B_B] = !WinInput.buttons[input::B_B];
                        break;
                    }
                    case VK_RETURN:
                    {
                        OutputDebugString("Up");
                        WinInput.buttons[input::B_START] = !WinInput.buttons[input::B_START];
                        break;
                    }
                    case VK_SHIFT:
                    {
                        OutputDebugString("Up");
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

void * LoadRomData(char * Filename, uint32 *Size)
{
    void *RomData = 0;
    
    HANDLE RomHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(RomHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER Filesize;
        if(GetFileSizeEx(RomHandle, &Filesize))
        {
            RomData = VirtualAlloc(0, Filesize.LowPart, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(RomData)
            {
                DWORD BytesRead;
                if(ReadFile(RomHandle, RomData, Filesize.LowPart, &BytesRead, 0) &&
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
    return(RomData);
}

inline void setNegative(uint8 Value, uint8 *Flags)
{  
    if(Value >= 0x00 && Value <= 0x7F)
        *Flags = *Flags & ~(1 << 7); // clear negative flag
    else
        *Flags = *Flags | (1 << 7); // set negative flag
}
inline void setZero(uint8 Value, uint8 *Flags)
{
    if(Value == 0x00)
        *Flags = *Flags | (1 << 1); // Set zero flag
    else
        *Flags = *Flags & ~(1 << 1);
}
inline void setCarry(uint8 *Flags)
{
    *Flags = *Flags | 1;
}
inline void clearCarry(uint8 *Flags)
{
    *Flags = *Flags & ~1;
}

void PushByte(uint8 Byte, uint8 *StackPointer, uint8 *StackLocation)
{
    // NOTE:
    // Stack pointer is commonly initialised to 0xFF.
    // Adding a byte will decrease the pointer.
    // Poping increases the pointer.
    // The pointer does wrap if under 0x100

    uint8 *Address = (uint8 *)(*StackPointer + (uint64) StackLocation);
    *Address = Byte;

    --(*StackPointer);  
}
uint8 PopByte(uint8 *StackPointer, uint8 *StackLocation)
{
    uint8 *Address = (uint8 *)(*StackPointer + (uint64) StackLocation);
    uint8 Value = *Address;

    ++(*StackPointer);
    
    return(Value);
}

void writeMemByte(uint16 Address, uint8 Byte)
{
    // NOTE: Mirrors the address for the 2kb ram 
    if(0x800 <= Address && Address < 0x2000)
        Address = Address & (Kilobytes(2) - 1);  // Modulus for values power of 2
    // NOTE: Mirror for PPU Registers
    if(0x2008 <= Address && Address < 0x4000)
        Address = Address & 7; // Modulus, repeates every 8 bytes

    
    uint8 *NewAddress = (uint8 *)(Address + MemoryStartOffset);
    *NewAddress = Byte;
}

uint8 readMemory(uint16 Address)
{
    // NOTE: Mirrors the address for the 2kb ram 
    if(0x800 <= Address && Address < 0x2000)
        Address = Address & (Kilobytes(2) - 1);  // Modulus for values power of 2
    // NOTE: Mirror for PPU Registers
    if(0x2008 <= Address && Address < 0x4000)
        Address = Address & 7; // Modulus, repeates every 8 bytes

    uint8 *NewAddress = (uint8 *)(Address + MemoryStartOffset);
    return(*NewAddress);
}


uint16 readMemory16(uint16 Address)
{
    // NOTE: Little Endian
    uint8 LowByte = readMemory(Address);
    uint8 HighByte = readMemory(Address+1);
        
    uint16 NewAddress = (HighByte << 8) | LowByte;
    return(NewAddress);
}

int CALLBACK
WinMain(HINSTANCE WindowInstance, HINSTANCE PrevWindowInstance,
        LPSTR CommandLine, int CommandShow)
{
    struct registers {
        uint8 A;
        uint8 X;
        uint8 Y; 
        uint8 Flags = 0;
        uint8 StackPtr;
        uint16 PrgCounter; // not yet used
    };
    
    registers Registers = {};

    // Program will have these
    uint16 NMIVec = 0xFFFA;
    uint16 ResetVec = 0xFFFC;
    uint16 IRQnBRKVec = 0xFFFE; 
    
    /*
      CPU Memory Map - taken from nesdev.com
      $0000-$07FF   $0800   2KB internal RAM
      $0800-$0FFF   $0800   Mirrors of $0000-$07FF
      $1000-$17FF   $0800
      $1800-$1FFF   $0800
      $2000-$2007   $0008   NES PPU registers
      $2008-$3FFF   $1FF8   Mirrors of $2000-2007 (repeats every 8 bytes)
      $4000-$4017   $0018   NES APU and I/O registers
      $4018-$401F   $0008   APU and I/O functionality that is normally disabled. See CPU Test Mode.
      $4020-$FFFF   $BFE0   Cartridge space: PRG ROM, PRG RAM, and mapper registers (See Note)
     */

    /*
      2KB RAM Rough Layout
      0x0000-0x00FF  Zero Page
      0x0100-0x01FF  Stack
      0x0200-0x07FF  Unassigned Memory(free for program to use)
     */

    uint8 * Memory = (uint8 *)VirtualAlloc(0, Kilobytes(64), MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    MemoryStartOffset = (uint64)Memory; // TODO: Remove from gobal scope?
    
    // Ram Memory
    uint16 RAMAdrs = 0x0;
    uint16 ZeroPageAdrs = 0x0;
    uint16 StackAdrs = 0x100;

        
    uint8 MemPrgBank1 = 0x8000;
    uint8 MemPrgBank2 = 0xC000;


    // NOTE: Not sure if needed yet.
    //uint16 PPURegAdrs = 0x2000;
    //uint16 IORegAdrs = 0x4000;
    
    // Reading rom file
    char * Filename = "ZeldaT.nes";
    uint32 FileSize;
    uint8 *RomData = (uint8 *)LoadRomData(Filename, &FileSize);

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
#define PRG_BANK_NUM 4
    uint8 PrgRomBanks[PRG_BANK_NUM] = {};

    uint8 *PrgRamBank;
    uint8 *ChrBank1;
    uint8 *ChrBank2;


    uint8 BankRegisters[PRG_BANK_NUM];


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
    
    uint8 MapperNumber = (Flags7 & 0xF0) | (Flags6 >> 4);
    switch(MapperNumber)
    {
        case 0:
        {
            PrgRomBanks[0] = RomPrgData;
            PrgRomBanks[2] = RomPrgData;
            break;
        }
        case 1:
        {
            
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
    Registers.PrgCounter = readMemory16(ResetVec);


    
    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = WinInputCallback;
    WindowClass.hInstance = WindowInstance;
    WindowClass.lpszClassName = "NesEmu";
    
    if(RegisterClassA(&WindowClass))
    {
        HWND Window = CreateWindowExA(0, WindowClass.lpszClassName, "NesEmu",
                                      WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                      CW_USEDEFAULT, CW_USEDEFAULT,
                                      CW_USEDEFAULT,CW_USEDEFAULT,
                                      0, 0, WindowInstance, 0);
        if(Window)
        {            
            LARGE_INTEGER LastCounter;
            QueryPerformanceCounter(&LastCounter);                        
            uint64 LastCycles = __rdtsc();

            
            uint8 BytesRead = 1;
            
            GlobalRunning = true;            
            while(GlobalRunning)
            {
                MSG Message = {}; 
                while (PeekMessage(&Message, Window, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                /*
                switch(*CurrentInstr)
                {
                    case 0x10: // BPL - Branch if plus.
                    {
                        int8 RelAddress = readNextByte(CurrentInstr, &BytesRead);
                        // NOTE: Will change program counter if negative flag is clear.
                        if(!(Registers.Flags & (1 << 7)))
                        {
                            Registers.PrgCounter += (RelAddress + 2); // Plus two to next instruction
                            CurrentInstr = (uint8 *)((uint64)PrgStart + Registers.PrgCounter);
                            BytesRead = 0;
                        }
                        break;
                    }
                    case 0x20: // JSR - Jump to subroutine
                    {
                        // last byte of jrs (next instruction minus 1) is pushed onto stack
                        // program counter jumps to address

                        // TODO: Is absolute addressing from cartridge 0x00, or where program bank starts?
                        uint16 NewAddress = readNextUInt16(CurrentInstr, &BytesRead);
                        
                        uint16 PrevAdrs = ((uint64)(CurrentInstr + 2) - (uint64)PrgStart);                              
                        uint8 HighByte = (uint8)(PrevAdrs >> 8);
                        uint8 LowByte = (uint8)PrevAdrs;
      
                        // Push onto stack, little endian
                        PushByte(HighByte, &Registers.StackPtr, Stack);
                        PushByte(LowByte, &Registers.StackPtr, Stack);

                        Registers.PrgCounter = NewAddress;
                        CurrentInstr = (uint8 *)((uint64)PrgStart + Registers.PrgCounter);
                        BytesRead = 0; // set to zero so does move forward

                        break;
                    }
                    case 0x29: // AND(Immediate) - Logical AND with value and A, stores in A
                    {
                        uint8 ByteValue = readNextByte(CurrentInstr, &BytesRead);

                        uint8 ANDValue = Registers.A & ByteValue;

                        setNegative(ANDValue, &Registers.Flags);
                        setZero(ANDValue, &Registers.Flags);

                        Registers.A = ANDValue;
                        break;
                    }
                    case 0x4C: // JMP(Absolute) - Jump
                    {
                        uint16 NewAddress = readNextUInt16(CurrentInstr, &BytesRead);
                        Registers.PrgCounter = NewAddress;
                        CurrentInstr = (uint8 *)((uint64)PrgStart + Registers.PrgCounter);
                        BytesRead = 0;
                        break;
                    }
                    case 0x78: // Set Interrupt Disable Flag
                    {
                        Registers.Flags = Registers.Flags | (1 << 2); // Set interrupt flag
                        break;
                    }
                    case 0x8D: // STA(Absolute) - Store Accumulator in Memory
                    {
                        uint16 Address = readNextUInt16(CurrentInstr, &BytesRead);
                        
                        writeMemByte(Registers.A, Address, MemStart);                        
                        break;
                    }
                    case 0x9A: // TXS - Transfer X to Stack Pointer
                    {
                        uint8 Value = Registers.X;
                        setNegative(Value, &Registers.Flags);
                        setZero(Value, &Registers.Flags);
                        Registers.StackPtr = Value;
                        break;
                    }
                    case 0xA2: // LDX(Immediate) - Load X index with memory
                    {
                        uint8 Value = readNextByte(CurrentInstr, &BytesRead);

                        setNegative(Value, &Registers.Flags);
                        setZero(Value, &Registers.Flags);

                        Registers.X = Value;
                        break;
                    }
                    case 0xAD: // LDA(Absolute) - Load A with 16bit address
                    {
                        uint16 Address = readNextUInt16(CurrentInstr, &BytesRead);                                                
                        uint8 Value = readMemByte(Address, MemStart);
                        
                        setNegative(Value, &Registers.Flags);
                        setZero(Value, &Registers.Flags);
                        Registers.A = Value;                    
                        break;
                    }
                    case 0xA5: // LDA(Zero Page) - load A from zero page
                    {
                        uint8 Address = readNextByte(CurrentInstr, &BytesRead);

                        uint8 Value = readMemByte(Address, ZeroPage);
                        
                        setNegative(Value, &Registers.Flags);
                        setZero(Value, &Registers.Flags);
                        
                        Registers.A = Value;
                        break;
                    }
                    case 0xA9: // LDA(Immediate) - Load accumulator with memory
                    {
                        uint8 Value = readNextByte(CurrentInstr, &BytesRead);
   
                        setNegative(Value, &Registers.Flags);
                        setZero(Value, &Registers.Flags);
                        
                        Registers.A = Value;          
                        break;
                    }
                    case 0xB0: // BCS - Branch if Carry is set
                    {
                        uint8 RelAddress = readNextByte(CurrentInstr, &BytesRead);
                        if(Registers.Flags & 1)
                        {
                            Registers.PrgCounter += (RelAddress + 2); // Plus two to next instruction
                            CurrentInstr = (uint8 *)((uint64)PrgStart + Registers.PrgCounter);
                            BytesRead = 0;
                        }
                        break;
                    };
                    case 0xC9: // CMP(Immediate) - Compare value against A
                    {
                        uint8 ByteValue = readNextByte(CurrentInstr, &BytesRead);

                        uint8 CmpValue = Registers.A - ByteValue;
                        
                        setNegative(ByteValue, &Registers.Flags);
                        setZero(ByteValue, &Registers.Flags);
                        
                        if(Registers.A < ByteValue)
                        {
                            clearCarry(&Registers.Flags);
                        }
                        else
                            setCarry(&Registers.Flags);

                        break;
                    }
                    case 0xD0: // BNE - Branch if not equal
                    {
                        // TODO: Might be wrong, where do we branch from, currentinstr or next??
                        uint8 RelAddress = readNextByte(CurrentInstr, &BytesRead);
                        if((Registers.Flags & (1 << 1)))
                        {
                            Registers.PrgCounter += (RelAddress); // Plus two to next instruction
                            CurrentInstr = (uint8 *)((uint64)PrgStart + Registers.PrgCounter);
                            BytesRead = 0;
                        }
                        break;
                    }
                    case 0xD8: // Clear decimal flag
                    {
                        Registers.Flags = Registers.Flags & ~(1 << 3); // clear decimal flag
                        break;
                    }
                    case 0xF0: // BEQ - Brach if Equal
                    {
                        uint8 RelAddress = readNextByte(CurrentInstr, &BytesRead);
                        if(Registers.Flags & (1 << 1))
                        {
                            Registers.PrgCounter += (RelAddress + 2); // Plus two to next instruction
                            CurrentInstr = (uint8 *)((uint64)PrgStart + Registers.PrgCounter);
                            BytesRead = 0;
                        }
                        break;    
                    }
                    default:
                    {
                        uint8 MissingValue = *CurrentInstr;
                        char Buffer[8];
                        sprintf(Buffer, "%X\n", MissingValue);
                        OutputDebugString(Buffer);
                        Assert(0);
                        break;
                    }

                }
                
                // NOTE: When updating bytesRead, some instruction like branch do not want to advance the progcounter
                CurrentInstr = getnextInstruction(CurrentInstr, &Registers.PrgCounter, &BytesRead);
                BytesRead = 1;
                */
                
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
