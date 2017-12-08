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
};

struct nes
{
    cpu Cpu;
    ppu Ppu;
    apu Apu;
    cartridge Cartridge;
};


global uint64 GlobalCpuMemoryBase = 0;
global uint64 GlobalPpuMemoryBase = 0;
global cpu *GlobalCpu;
global ppu *GlobalPpu;
global apu *GlobalApu;

#define NES_H
#endif
