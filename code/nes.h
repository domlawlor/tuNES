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

static const u8 MapperTotal = 8;

struct Cartridge
{
    u8 * fileName;
    u32 fileSize;
    u8 * data;

    u8 prgBankCount;
	u8 chrBankCount;

    u8 * prgData;
    u8 * chrData;
    
    u8 prgRamSize;

    u8 mapperNum;

    u16 extRegister;
    
    b32 useVertMirror;
    b32 hasBatteryRam;
    b32 hasTrainer;
    b32 useFourScreenMirror;

    u8 mapperInternalReg;
    u8 mapperWriteCount;
};

struct Nes
{
    Cpu cpu;
    Ppu ppu;
    Apu apu;
    Cartridge cartridge;

    u8 * cpuMemoryBase;
    u8 * ppuMemoryBase;
    
    b32 powerOn;

    r32 cpuHz;
    
    r32 frameClocksElapsed;
    r32 frameClockTotal;
};

global Nes *globalNes;

global u8 globalOpenBus;



#define NES_H
#endif
