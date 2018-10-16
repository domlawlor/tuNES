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
        else if(Apu->Square1.ShiftCount > 0)
        {
            Apu->Square1.PeriodHigh = (ResultPeriod >> 8) & 0x7;
            Apu->Square1.PeriodLow = (uint8)ResultPeriod; 
        }
        
        
    }

    if(Apu->Square1.SweepReset || Apu->Square1.SweepDivider == 0)
    {
        Apu->Square1.SweepReset = false;
        Apu->Square1.SweepDivider = Apu->Square1.SweepPeriod + 1;
    }    
}

static void
clockSequencer(apu *Apu)
{
    /*
    d = 0
    _ - _ _ _ _ _ _
    d = 1
    _ - - _ _ _ _ _
    d = 2
    _ - - - - _ _ _
    d = 3
    - _ _ - - - - -
    */

    // Period is decreased each frame. When it reaches zero(goes from 0 to t) it is clocked. 

    // Sequencer starts at zero, and is decremented. Goes from 0 to 7, 6, 5, 4 so on
    
/*    
      The mixer receives the current envelope volume except when
      The sequencer output is zero, or
      overflow from the sweep unit's adder is silencing the channel, or
      the length counter is zero, or
      the timer has a value less than eight.
*/  
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

            // TODO:
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

                if(!Apu->IRQInhibit)
                {
                    TriggerIRQ = true;
                }

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
        
    uint8 Square1Out = 0; 
    
    // Square 2
    uint8 Square2Out = 0;
    
    // Triangle
    uint8 TriangleOut = 0;
    
    // Noise
    uint8 NoiseOut = 0;
    
    // DMC
    uint8 DmcOut = 0;
    
    // Mixing
    Apu->FinalOutput = ((0.00752f * (Square1Out + Square2Out)) +
                        (0.00851f * TriangleOut) +
                        (0.00494f * NoiseOut) +
                        (0.00335f * DmcOut));
}    


static void
initApu(apu *Apu)
{
    *Apu = {};
    // All registers are clear
}
