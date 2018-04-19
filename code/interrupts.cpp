/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "cpu.h"

bool32 NmiFlag = false;
bool32 LastNmiFlag = false;
bool32 TriggerNmi = false;
bool32 NmiInterruptSet = false;

bool32 IRQFlag = false;
bool32 LastIRQFlag = false;
bool32 TriggerIRQ = false;
bool32 IRQInterruptSet = false;

static void runPpuCatchup(uint8 ClocksIntoCurrentOp);

static void setNmi(bool32 newNmiFlag)
{
    NmiFlag = newNmiFlag;

    if(NmiFlag && !LastNmiFlag)
        TriggerNmi = true;

    LastNmiFlag = NmiFlag;
}

static void pollInterrupts(cpu *Cpu)
{
    // NMI
    if(NmiFlag && !LastNmiFlag)
    {
        TriggerNmi = true;
    }
    LastNmiFlag = NmiFlag;
    
    if(TriggerNmi)
    {
        TriggerNmi = false;
        NmiInterruptSet = true;
    }

    bool32 InterruptFlag = (INTERRUPT_BIT & Cpu->Flags) != 0;
    
    if(TriggerIRQ && !InterruptFlag)
    {
        IRQInterruptSet = true;
    }
    TriggerIRQ = false;

}

static void pollInterrupts(cpu *Cpu, uint8 CurrentCycle)
{
    runPpuCatchup(CurrentCycle);
    pollInterrupts(Cpu);
}
