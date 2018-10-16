/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "nes.h"

#include "mapper.cpp"
#include "interrupts.cpp"
#include "memory.cpp"

#include "apu.cpp"
#include "ppu.cpp"
#include "cpu.cpp"

static void runNes(nes *Nes, input *NewInput)
{
    // TODO: If power is off, could we just run the nes on an empty cartridge?
    //       Or is this the best check to stop extra work being done
    if(!Nes->PowerOn)
    {
        return;
    }

    cpu *Cpu = &Nes->Cpu;
    
    // Input read // TODO: Only run when reading input??
    // TODO: Move this to where it happens in memory read.
    if(Cpu->PadStrobe)
    {
        for(uint8 idx = 0; idx < input::BUTTON_NUM; ++idx)
        {
            Cpu->InputPad1.Buttons[idx] = NewInput->Buttons[idx];
        }
    }
    


    Nes->FrameClocksElapsed += runCpu(Cpu, NewInput);
}

static void loadCartridge(nes *Nes, char * FileName)
{
    cartridge *Cartridge = &Nes->Cartridge;
    cpu *Cpu = &Nes->Cpu;
    ppu *Ppu = &Nes->Ppu;
        
    // Reading rom file
    Cartridge->FileName = FileName;
    Cartridge->FileSize;
    Cartridge->Data = (uint8 *)readFileData(FileName, &Cartridge->FileSize);

    if(Cartridge->FileSize == 0)
    {
        Nes->PowerOn = false;
        return;
    }
    else
    {
        Nes->PowerOn = true;
    
        uint8 * RomData = Cartridge->Data;
        
        // NOTE: Check for correct header
        Assert(RomData[0] == 'N' && RomData[1] == 'E' && RomData[2] == 'S' && RomData[3] == 0x1A);
        
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
    Nes->PowerOn = !(Nes->PowerOn);

    if(Nes->PowerOn)
    {
        initCpu(&Nes->Cpu, Nes->CpuMemoryBase);
        initPpu(&Nes->Ppu, Nes->PpuMemoryBase, (uint32 *)GlobalScreenBackBuffer.Memory);
           
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

    Nes->Cpu.PrgCounter = readCpu16(RESET_VEC, &Nes->Cpu);
}

static void
reset(nes *Nes)
{
    initCpu(&Nes->Cpu, Nes->CpuMemoryBase);
    initPpu(&Nes->Ppu, Nes->PpuMemoryBase, (uint32 *)GlobalScreenBackBuffer.Memory);

    loadCartridge(Nes, RomFileName);
    
    Nes->Cpu.PrgCounter = readCpu16(RESET_VEC, &Nes->Cpu);

    // NOTE: The status after reset was taken from nesdev
    Nes->Cpu.StackPtr -= 3;
    setInterrupt(&Nes->Cpu.Flags);

    Nes->Cpu.PrgCounter = readCpu16(RESET_VEC, &Nes->Cpu);
}


/****************************************************************/
/* NOTE : Initialization of Cpu, Ppu, and Cartridge structures */
static nes createNes(char *RomName)
{    
    //nes *Nes = (nes *)VirtualAlloc(0, sizeof(nes), MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    nes Nes = {};
    
    // Memory allocation for the Cpu and Ppu. TODO: Different Allocation in the future?
    uint32 CpuMemorySize = Kilobytes(64);
    uint32 PpuMemorySize = Kilobytes(64);
    uint8 * Memory = (uint8 *)VirtualAlloc(0, (size_t)(CpuMemorySize + PpuMemorySize), MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    
    Nes.CpuMemoryBase = (uint64)Memory;
    Nes.PpuMemoryBase = (uint64)Memory + CpuMemorySize;

    initCpu(&Nes.Cpu, Nes.CpuMemoryBase);
    initPpu(&Nes.Ppu, Nes.PpuMemoryBase, (uint32 *)GlobalScreenBackBuffer.Memory);
    initApu(&Nes.Apu);

    // TODO: Check ref?
    loadCartridge(&Nes, RomName);

    // NOTE: Load the program counter with the reset vector
    Nes.Cpu.PrgCounter = readCpu16(RESET_VEC, &Nes.Cpu);

    Nes.CpuHz = 1789772.727272728f;            
    
    return(Nes);
}
