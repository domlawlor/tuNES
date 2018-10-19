/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "cpu.h"

b32 NmiFlag = false;
b32 LastNmiFlag = false;
b32 TriggerNmi = false;
b32 NmiInterruptSet = false;

b32 IRQFlag = false;
b32 LastIRQFlag = false;
b32 TriggerIRQ = false;
b32 IRQInterruptSet = false;

static void runPpuCatchup(u8 ClocksIntoCurrentOp);

static void setNmi(b32 newNmiFlag)
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

    b32 InterruptFlag = (INTERRUPT_BIT & Cpu->Flags) != 0;
    
    if(TriggerIRQ && !InterruptFlag)
    {
        IRQInterruptSet = true;
    }
    TriggerIRQ = false;

}

static void pollInterrupts(cpu *Cpu, u8 CurrentCycle)
{
    runPpuCatchup(CurrentCycle);
    pollInterrupts(Cpu);
}
