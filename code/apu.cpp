/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

#include "apu.h"

static void
clockLengthCounters(apu *Apu)
{
    if(!Apu->Square1.LengthCounterHalt && Apu->Square1.LengthCounter > 0)
        --Apu->Square1.LengthCounter;

    if(!Apu->Square2.LengthCounterHalt && Apu->Square2.LengthCounter > 0)
        --Apu->Square2.LengthCounter;
    if(!Apu->Triangle.LinearCtrl && Apu->Triangle.LengthCounter > 0)
        --Apu->Triangle.LengthCounter;
    if(!Apu->Noise.LengthCounterHalt && Apu->Noise.LengthCounter > 0)
        --Apu->Noise.LengthCounter;
}

static void
clockSweep(apu *Apu)
{
    --Apu->Square1.SweepDivider;
    
    // Output clock
    if(Apu->Square1.EnableSweep && Apu->Square1.SweepDivider == 0)
    {
        uint16 Period = ((uint16)Apu->Square1.PeriodHigh << 8) | Apu->Square1.PeriodLow; 

        uint16 Shift = Period >> Apu->Square1.ShiftCount;

        uint16 ResultPeriod;
        
        if(Apu->Square1.Negative)
        {
            ResultPeriod = Period - Shift;            
            // NOTE: TODO: Only on the square two, the shift is subtracted then incremented by 1.
            //   See all the details, not completely sure yet
        }
        else
        {
            ResultPeriod = Period + Shift;            
        }

        // TODO: MORE WORK ON MUTING IS REQUIRED. Something about it happening even when disabled??
        
        // Mute
        if(Period < 8 || ResultPeriod > 0x7FF)
        {
            // Channel DAC receives 0 and sweep unit doesn't change channel period.
        }
        else if(Apu->Square1.Shift > 0)
        {
            Apu->Square1.PeriodHigh = (ResultPeriod >> 8) & 0x7;
            Apu->Square1.PeriodLow = (uint8)ResultPeriod; 
        }
        
        
    }

    if(SweepReset || Apu->Square1.SweepDivider == 0)
    {
        Apu->Square1.SweepReset = false;
        Apu->Square1.SweepDivider = Apu->Square1.SweepPeriod + 1;
    }    
}

static void
clockEnvelope(apu *Apu)
{
    if(Apu->Square1.RestartEnv)
    {
        Apu->Square1.RestartEnv = false;
        Apu->Square1.EnvDivider = Apu->Square1.VolumePeriod + 1;
        Apu->Square1.EnvCounter = 15;
    }
    else
    {
        --Apu->Square1.EnvDivider;

        if(Apu->Square1.EnvDivider == 0)
        {
            Apu->Square1.EnvDivider = Apu->Square1.VolumePeriod + 1;

            if(Apu->Square1.EnvCounter > 0)
                --Apu->Square1.EnvCounter;
            else if(Apu->Square1.LengthCounterHalt)
                Apu->Square1.EnvCounter = 15;

            // NOTE TODO: Output CLOCK here!!
        }
    }
}

static void
apuTick(apu *Apu)
{    
    // Frame Sequencer clocks everything
    if(!Apu->Mode)
    {
        switch(Apu->FrameCounter)
        {
            case 0:
            {
                clockEnvelope(Apu);
                break;
            }
            case 1:
            {
                clockLengthCounters(Apu);
                clockSweep(Apu);
                clockEnvelope(Apu);
                break;
            } 
            case 2:
            {
                clockEnvelope(Apu);
                break;
            }
            case 3:
            {
                clockLengthCounters(Apu);
                clockSweep(Apu);
                clockEnvelope(Apu);
                //  irq
                break;
            }
        }

        ++Apu->FrameCounter;
        
        if(Apu->FrameCounter > 3)
            Apu->FrameCounter = 0;
    }
    else
    {
        switch(Apu->FrameCounter)
        {
            case 0:
            {
                clockLengthCounters(Apu);
                clockSweep(Apu);
                clockEnvelope(Apu);
                break;
            }
            case 1:
            {
                clockEnvelope(Apu);
                break;
            } 
            case 2:
            {
                clockLengthCounters(Apu);
                clockSweep(Apu);
                clockEnvelope(Apu);
                break;
            }
            case 3:
            {
                clockEnvelope(Apu);
                break;
            }
            case 4:
            {
                // NOTHING HAPPENS ON THIS CLOCK
                break;
            }
        }

        ++Apu->FrameCounter;
        
        if(Apu->FrameCounter > 4)
            Apu->FrameCounter = 0;
    }
    
// Square 1
    // Envelop picks volume
    
    
    uint8 Square1Out; 
    
    // Square 2
    uint8 Square2Out;
    
    // Triangle
    uint8 TriangleOut;
    
    // Noise
    uint8 NoiseOut;
    
    // DMC
    uint8 DmcOut;
    

    // Mixing
    real32 FinalOutput = ((0.00752 * (Square1Out + Square2Out)) +
                          (0.00851 * TriangleOut) +
                          (0.00494 * NoiseOut) +
                          (0.00335 * DmcOut));
}    
