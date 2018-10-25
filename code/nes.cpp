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

static void RunNes(Nes *nes, Input *newInput)
{
    // TODO: If power is off, could we just run the nes on an empty cartridge?
    //       Or is this the best check to stop extra work being done
    if(!nes->powerOn)
    {
        return;
    }

    Cpu *cpu = &nes->cpu;
    
    // Input read // TODO: Only run when reading input??
    // TODO: Move this to where it happens in memory read.
    if(cpu->padStrobe)
    {
        for(u8 idx = 0; idx < Input::BUTTON_NUM; ++idx)
        {
            cpu->inputPad1.buttons[idx] = newInput->buttons[idx];
        }
    }

    nes->frameClocksElapsed += RunCpu(cpu, newInput);
}

static void LoadCartridge(Nes *nes, u8 *fileName)
{
    Cartridge *cartridge = &nes->cartridge;
    Cpu *cpu = &nes->cpu;
    Ppu *ppu = &nes->ppu;
        
    // Reading rom file
    cartridge->fileName = fileName;
    cartridge->fileSize;
    cartridge->data = (u8 *)ReadFileData(fileName, &cartridge->fileSize);

    if(cartridge->fileSize == 0)
    {
        nes->powerOn = false;
        return;
    }
    else
    {
        nes->powerOn = true;
    
        u8 * romData = cartridge->data;
        
        // NOTE: Check for correct header
        Assert(romData[0] == 'N' && romData[1] == 'E' && romData[2] == 'S' && romData[3] == 0x1A);
        
        // NOTE: Read header
        cartridge->prgBankCount = romData[4];
        cartridge->chrBankCount = romData[5];
        u8 flags6            = romData[6];        
        u8 flags7            = romData[7];
        cartridge->prgRamSize   = romData[8];
        
        cartridge->useVertMirror       = (flags6 & (1)) != 0;
        cartridge->hasBatteryRam       = (flags6 & (1 << 1)) != 0;
        cartridge->hasTrainer          = (flags6 & (1 << 2)) != 0;
        cartridge->useFourScreenMirror = (flags6 & (1 << 3)) != 0;
        cartridge->mapperNum           = (flags7 & 0xF0) | (flags6 >> 4);

        Assert(cartridge->useFourScreenMirror == 0);
        
        if(cartridge->useFourScreenMirror)
            nes->ppu.mirrorType = FOUR_SCREEN_MIRROR;
        else if(cartridge->useVertMirror)
            nes->ppu.mirrorType = VERTICAL_MIRROR;
        else
            nes->ppu.mirrorType = HORIZONTAL_MIRROR;      
        
        cartridge->prgData = romData + 16; // PrgData starts after the header info(16 bytes)

        if(cartridge->hasTrainer)
        {
            cartridge->prgData += 512; // Trainer size 512 bytes
        }

        cartridge->chrData = cartridge->prgData + (cartridge->prgBankCount * Kilobytes(16));

        MapperInit[cartridge->mapperNum](cartridge, cpu, ppu);
    }
}

static void Power(Nes *nes)
{
    nes->powerOn = !(nes->powerOn);

    if(nes->powerOn)
    {
        InitCpu(&nes->cpu, nes->cpuMemoryBase);
        InitPpu(&nes->ppu, nes->ppuMemoryBase, (u32 *)globalScreenBackBuffer.memory);
           
        LoadCartridge(nes, romFileName);
        nes->cpu.prgCounter = (Read8(nes->cpu.memoryBase + (RESET_VEC + 1)) << 8) | Read8(nes->cpu.memoryBase + RESET_VEC);
    }
    else
    {
        u8 * memoryBase = nes->cpu.memoryBase;
        nes->cpu = {};
        nes->cpu.memoryBase = memoryBase;

        memoryBase = nes->ppu.memoryBase;
        u32 *basePixel = nes->ppu.basePixel;
        nes->ppu = {};
        nes->ppu.memoryBase = memoryBase;
        nes->ppu.basePixel = basePixel;
    }

    nes->cpu.prgCounter = ReadCpu16(RESET_VEC, &nes->cpu);
}

static void Reset(Nes *nes)
{
    InitCpu(&nes->cpu, nes->cpuMemoryBase);
    InitPpu(&nes->ppu, nes->ppuMemoryBase, (u32 *)globalScreenBackBuffer.memory);

    LoadCartridge(nes, romFileName);
    
    nes->cpu.prgCounter = ReadCpu16(RESET_VEC, &nes->cpu);

    // NOTE: The status after reset was taken from nesdev
    nes->cpu.stackPtr -= 3;
    SetInterrupt(&nes->cpu.flags);

    nes->cpu.prgCounter = ReadCpu16(RESET_VEC, &nes->cpu);
}


/****************************************************************/
/* NOTE : Initialization of cpu, Ppu, and cartridge structures */
static Nes CreateNes(u8 *romName)
{    
    Nes nes = {};
    
    // Memory allocation for the cpu and Ppu
    u32 cpuMemorySize = Kilobytes(64);
    u32 ppuMemorySize = Kilobytes(64);
    u8 * memory = (u8 *)VirtualAlloc(0, (size_t)(cpuMemorySize + ppuMemorySize), MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    
    nes.cpuMemoryBase = memory;
    nes.ppuMemoryBase = memory + cpuMemorySize;

    InitCpu(&nes.cpu, nes.cpuMemoryBase);
    InitPpu(&nes.ppu, nes.ppuMemoryBase, (u32 *)globalScreenBackBuffer.memory);
    InitApu(&nes.apu);

    // TODO: Check ref?
    LoadCartridge(&nes, romName);

    // NOTE: Load the program counter with the reset vector
    nes.cpu.prgCounter = ReadCpu16(RESET_VEC, &nes.cpu);

    nes.cpuHz = 1789772.727272728f;            
    
    return(nes);
}
