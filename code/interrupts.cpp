/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "cpu.h"

bool nmiFlag = false;
bool lastNmiFlag = false;
bool triggerNmi = false;
bool nmiInterruptSet = false;

bool irqFlag = false;
bool lastIRQFlag = false;
bool triggerIRQ = false;
bool irqInterruptSet = false;

static void RunPpuCatchup(u8 clocksIntoCurrentOp);

static void SetNmi(bool newNmiFlag)
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

	bool interruptFlag = (INTERRUPT_BIT & cpu->flags) != 0;

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
