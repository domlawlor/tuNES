/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "cpu.h"

b32 nmiFlag = false;
b32 lastNmiFlag = false;
b32 triggerNmi = false;
b32 nmiInterruptSet = false;

b32 irqFlag = false;
b32 lastIRQFlag = false;
b32 triggerIRQ = false;
b32 irqInterruptSet = false;

static void RunPpuCatchup(u8 clocksIntoCurrentOp);

static void SetNmi(b32 newNmiFlag)
{
    nmiFlag = newNmiFlag;

    if(nmiFlag && !lastNmiFlag)
        triggerNmi = true;

    lastNmiFlag = nmiFlag;
}

static void PollInterrupts(Cpu *cpu)
{
    // NMI
    if(nmiFlag && !lastNmiFlag)
    {
        triggerNmi = true;
    }
    lastNmiFlag = nmiFlag;
    
    if(triggerNmi)
    {
        triggerNmi = false;
        nmiInterruptSet = true;
    }

    b32 interruptFlag = (INTERRUPT_BIT & cpu->flags) != 0;
    
    if(triggerIRQ && !interruptFlag)
    {
        irqInterruptSet = true;
    }
    triggerIRQ = false;

}

static void PollInterrupts(Cpu *cpu, u8 currentCycle)
{
    RunPpuCatchup(currentCycle);
    PollInterrupts(cpu);
}
