#if !defined(NES_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "cpu.h"
#include "ppu.h"
#include "apu.h"

#define MAPPER_TOTAL 8

struct cartridge
{
    char * FileName;
    uint32 FileSize;
    uint8 * Data;

    uint8 PrgBankCount;
    uint8 * PrgData;

    uint8 ChrBankCount;
    uint8 * ChrData;
    
    uint8 PrgRamSize;

    uint8 MapperNum;

    uint16 ExtRegister;
    
    bool32 UseVertMirror;
    bool32 HasBatteryRam;
    bool32 HasTrainer;
    bool32 UseFourScreenMirror;

    uint8 MapperInternalReg;
    uint8 MapperWriteCount;
};

struct nes
{
    cpu Cpu;
    ppu Ppu;
    apu Apu;
    cartridge Cartridge;

    uint64 CpuMemoryBase;
    uint64 PpuMemoryBase;
    
    bool32 PowerOn;

    real32 CpuHz;
    
    real32 FrameClocksElapsed;
    real32 FrameClockTotal;
};

global nes *GlobalNes;

global uint8 GlobalOpenBus;



#define NES_H
#endif
