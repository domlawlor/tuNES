#include "nes.h"
#include "apu.h"

constexpr u32 AudioSampleRate = 44100;
constexpr u8 AudioSampleSize = 16;
constexpr u8 AudioChannelCount = 1;

constexpr u32 SamplesNeededPerAudioFrame = 480;

// Audio input processing callback
void AudioInputCallback(void *bufferToFill, u32 samplesNeeded)
{
	Nes::GetApu()->FillAudioBuffer(bufferToFill, samplesNeeded);
}

void Apu::FillAudioBuffer(void *bufferToFill, u32 samplesNeeded)
{
	u32 samplesAvailable = blip_samples_avail(sampleBuffer);

	constexpr u32 bufferFrameAmount = 2;
	u32 samplesToBuffer = samplesNeeded * bufferFrameAmount;

	if(samplesAvailable < samplesToBuffer)
	{
		return;
	}

	Assert(samplesNeeded <= MaxSamplesInTempBuffer);


	s32 samplesRead = blip_read_samples(sampleBuffer, tempSampleBuffer, samplesNeeded, 0);

	if(samplesRead != samplesNeeded)
	{
		TraceLog(LOG_INFO, "NOT ENOUGH SAMPLES READY");
	}

	s16 *output = (s16 *)bufferToFill;
	for(u32 sampleNum = 0; sampleNum < samplesRead; ++sampleNum)
	{
		output[sampleNum] = tempSampleBuffer[sampleNum];
	}
}


void Apu::FillAudioBuffer()
{
	u32 samplesAvailable = blip_samples_avail(sampleBuffer);

	constexpr u32 bufferFrameAmount = 3;
	u32 samplesToBuffer = SamplesNeededPerAudioFrame * bufferFrameAmount;

	if(samplesAvailable < samplesToBuffer)
	{
		return;
	}

	Assert(SamplesNeededPerAudioFrame <= MaxSamplesInTempBuffer);


	s32 samplesRead = blip_read_samples(sampleBuffer, tempSampleBuffer, SamplesNeededPerAudioFrame, 0);

	if(samplesRead != SamplesNeededPerAudioFrame)
	{
		TraceLog(LOG_INFO, "NOT ENOUGH SAMPLES READY");
	}

	if(!IsAudioStreamProcessed(audioStream))
	{
		TraceLog(LOG_INFO, "Failed IsAudioStreamProcessed");
	}

	UpdateAudioStream(audioStream, tempSampleBuffer, SamplesNeededPerAudioFrame);
}

Apu::Apu()
{
	square1.isSquare1 = true;

	tempSampleBuffer = (s16 *)MemAlloc(MaxSamplesInTempBuffer * sizeof(s16));

	sampleBuffer = blip_new(MaxSamplesInBlipBuffer);
	blip_set_rates(sampleBuffer, gNesCpuClockRate, AudioSampleRate);
	

	InitAudioDevice();

	
	SetAudioStreamBufferSizeDefault(SamplesNeededPerAudioFrame);

	audioStream = LoadAudioStream(AudioSampleRate, AudioSampleSize, AudioChannelCount);

	SetAudioStreamCallback(audioStream, AudioInputCallback);

	PlayAudioStream(audioStream);        // Start processing stream buffer (no data loaded currently)
}

Apu::~Apu()
{
	MemFree(tempSampleBuffer);

	StopAudioStream(audioStream);
	UnloadAudioStream(audioStream);
	blip_delete(sampleBuffer);
}



void Apu::WriteRegisters(u16 address, u8 value)
{
	UpdateCycles();

	Nes::SetOpenBus(value);

	switch(address)
	{
	case 0x4000:
	case 0x4004:
	{
		SquareChannel *square = (address == 0x4004) ? &square2 : &square1;
		square->dutyCycle = ((value & 0xC0) >> 6);
		square->haltLengthCounter = ((value & 0x20) != 0);
		square->envelope.SetLoop(square->haltLengthCounter);
		square->envelope.SetConstVolume((value & 0x10) != 0);
		square->envelope.SetVolume(value & 0xF);

		forceUpdateSet = true;
		break;
	}
	case 0x4001:
	case 0x4005:
	{
		SquareChannel *square = (address == 0x4005) ? &square2 : &square1;
		square->enabledSweep = ((value & 0x80) != 0);
		square->sweepPeriod = ((value & 0x70) >> 4) + 1; // Period + 1
		square->sweepNegateSet = ((value & 0x8) != 0);
		square->sweepShiftCount = value & 0x7;
		square->sweepResetSet = true;

		square->UpdatePeriodData();
		break;
	}
	case 0x4002:
	case 0x4006:
	{
		SquareChannel *square = (address == 0x4006) ? &square2 : &square1;
		square->period = (square->period & 0x700) | value;
		square->UpdatePeriodData();
		break;
	}
	case 0x4003:
	case 0x4007:
	{
		SquareChannel *square = (address == 0x4007) ? &square2 : &square1;

		if(square->enabled)
		{
			square->lengthCounter = LengthCounterTable[((value & 0xF8) >> 3)];
		}
		square->period = ((value & 0x7) << 8) | (square->period & 0xFF);
		square->UpdatePeriodData();

		square->currentDutyPos = 0;

		square->envelope.RegisterReset();
		forceUpdateSet = true;
		break;
	}
	case 0x4008:
	{
		triangle.haltLengthCounter = ((value & 0x80) != 0);
		triangle.linearCounterOnReset = value & 0x7F;

		forceUpdateSet = true;
		break;
	}
	case 0x400A:
	{
		triangle.period = (triangle.period & 0x700) | value;
		break;
	}
	case 0x400B:
	{
		if(triangle.enabled) { triangle.lengthCounter = LengthCounterTable[((value & 0xF8) >> 3)]; }

		triangle.period = (value & 0x7) << 8 | (triangle.period & 0x00FF);

		triangle.resetCounter = true;
		forceUpdateSet = true;
		break;
	}
	case 0x400C:
	{
		noise.haltLengthCounter = ((value & 0x20) != 0);
		noise.envelope.SetLoop(noise.haltLengthCounter);
		noise.envelope.SetConstVolume((value & 0x10) != 0);
		noise.envelope.SetVolume(value & 0xF);

		forceUpdateSet = true;
		break;
	}
	case 0x400E:
	{
		u8 periodIndex = value & 0xF;
		noise.period = NoisePeriodTable[periodIndex] - 1;

		noise.modeFlag = ((value & 0x80) != 0);
		break;
	}
	case 0x400F:
	{
		if(noise.enabled) 
		{
			u8 lengthCounterIndex = value >> 3;
			noise.lengthCounter = LengthCounterTable[lengthCounterIndex];
			forceUpdateSet = true;
		}

		noise.envelope.RegisterReset();
		
		break;
	}
	case 0x4010:
	{
		dmc.irqEnable = ((value & 0x80) != 0);
		if(!dmc.irqEnable)
		{
			Nes::GetCpu()->ClearApuDmcIRQ();
		}

		dmc.loopSet = ((value & 0x40) != 0);

		u8 periodTableIndex = value & 0xF;
		dmc.period = DmcPeriodTable[periodTableIndex];

		break;
	}
	case 0x4011:
	{
		u8 outputValue = value & 0x7F;
		dmc.currentOutput = outputValue;

		if(outputValue != dmc.lastOutput)
		{
			s16 delta = outputValue - dmc.lastOutput;
			dmc.lastOutput = outputValue;

			dmc.outputDeltas[dmc.lastCycle] = delta;
			AddDeltaCycleNum(dmc.lastCycle);
		}

		break;
	}
	case 0x4012:
	{
		dmc.sampleAddress = 0xC000 | ((u32)value << 6);
		break;
	}
	case 0x4013:
	{
		dmc.sampleLength = (value << 4) | 0x1;
		break;
	}
	case 0x4015:
	{
		square1.enabled = ((value & 0x1) != 0);
		square2.enabled = ((value & 0x2) != 0);
		triangle.enabled = ((value & 0x4) != 0);
		noise.enabled = ((value & 0x8) != 0);
		dmc.enabled = ((value & 0x10) != 0);

		if(!square1.enabled) { square1.lengthCounter = 0; }
		if(!square2.enabled) { square2.lengthCounter = 0; }
		if(!triangle.enabled) { triangle.lengthCounter = 0; }
		if(!noise.enabled) { noise.lengthCounter = 0; }

		dmc.EnabledUpdated();

		break;
	}
	case 0x4017:
	{
		Cpu *cpu = Nes::GetCpu();

		frameCounterIrqInhibit = ((value & 0x40) != 0);
		if(frameCounterIrqInhibit)
		{
			cpu->ClearApuFrameCounterIRQ();
		}

		// NOTE Writing to $4017 resets the frame counter and the quarter/half frame
		// triggers happen simultaneously, but only on "odd" cycles (and only after the
		// first "even" cycle after the write occurs) - thus, it happens either 2 or 3 cycles
		// after the write (i.e. on the 2nd or 3rd cycle of the next instruction).
		// After 2 or 3 clock cycles (depending on when the write is performed), the timer is reset.
		// Writing to $4017 with bit 7 set ($80) will immediately clock all of its controlled units at
		//the beginning of the 5-step sequence; with bit 7 clear, only the sequence is reset without
		// clocking any of its units.
		frameCounterPendingWriteValue = value;
		frameCounterWriteWaitCycles = cpu->IsOddCycle() ? 4 : 3;

		break;
	}
	}
}

u8 Apu::ReadRegisters(u16 address)
{
	u8 value = 0;
	UpdateCycles();

	switch(address)
	{
	case 0x4015:
	{
		value |= (square1.lengthCounter > 0) ? 0x1 : 0;
		value |= (square2.lengthCounter > 0) ? 0x2 : 0;
		value |= (triangle.lengthCounter > 0) ? 0x4 : 0;
		value |= (noise.lengthCounter > 0) ? 0x08 : 0;
		value |= (dmc.bytesLeft > 0) ? 0x10 : 0;

		value |= Nes::GetCpu()->IsApuFrameCounterIRQSet() ? 0x40 : 0;
		value |= Nes::GetCpu()->IsApuDmcIRQSet() ? 0x80 : 0;

		Nes::GetCpu()->ClearApuFrameCounterIRQ();

		break;
	}
	}
	Nes::SetOpenBus(value);
	return value;
}

void Apu::RunCycle()
{
	++cycle;

	if(cycle == (CyclesEachAudioFrame - 1))
	{
		UpdateCycles();
		
		square1.lastCycle = 0;
		square2.lastCycle = 0;
		triangle.lastCycle = 0;
		noise.lastCycle = 0;
		dmc.lastCycle = 0;

		constexpr float apuVolume = 8.0f;

		u64 lastCycleNumMixed = 0;

		for(u32 deltaIdx = 0; deltaIdx < totalDeltas; ++deltaIdx)
		{
			u64 cycleNum = deltaCycleNums[deltaIdx];

			if(cycleNum == lastCycleNumMixed)
			{
				continue;
			}

			square1Output += square1.outputDeltas[cycleNum];
			square2Output += square2.outputDeltas[cycleNum];
			triangleOutput += triangle.outputDeltas[cycleNum];
			noiseOutput += noise.outputDeltas[cycleNum];
			dmcOutput += dmc.outputDeltas[cycleNum];

			s32 squaresOutput = square1Output + square2Output;
			s32 squaresVolume = 477600 / ((8128.0 / squaresOutput) + 100.0);

			s32 tndOuput = 3.0 * triangleOutput + 2 * noiseOutput + 1 * dmcOutput;
			s32 tndVolume = 818350 / ((24329.0 / tndOuput) + 100.0);

			s16 mixedOuput = squaresVolume + tndVolume;

			s16 mixedDelta = mixedOuput - lastMixedOutput;

			blip_add_delta(sampleBuffer, cycleNum, apuVolume * mixedDelta);

			
			lastMixedOutput = mixedOuput;

			//TraceLog(LOG_INFO, "square1Output=%d", square1Output);

			lastCycleNumMixed = cycleNum;
		}

		lastCycleNumMixed = 0;

		blip_end_frame(sampleBuffer, cycle);

		MemorySet(deltaCycleNums, 0, CyclesEachAudioFrame * sizeof(u64));
		totalDeltas = 0;
		
		MemorySet(square1.outputDeltas, 0, CyclesEachAudioFrame * sizeof(s16));
		MemorySet(square2.outputDeltas, 0, CyclesEachAudioFrame * sizeof(s16));
		MemorySet(triangle.outputDeltas, 0, CyclesEachAudioFrame * sizeof(s16));
		MemorySet(noise.outputDeltas, 0, CyclesEachAudioFrame * sizeof(s16));
		MemorySet(dmc.outputDeltas, 0, CyclesEachAudioFrame * sizeof(s16));

		

		cycle = 0;
		lastCycle = 0;
	}
	else if(dmc.NeedsUpdate() || forceUpdateSet)
	{
		forceUpdateSet = false;
		UpdateCycles();
	}
	else
	{
		u32 cycleDelta = cycle - lastCycle;
		bool frameCounterNeedsUpdate = IsFrameCounterUpdateRequired(cycleDelta);
		bool dmcHasIrq = dmc.IsIrqUpcoming(cycleDelta);

		if(frameCounterNeedsUpdate)
		{
			UpdateCycles();
		}
	}
	
	//FillAudioBuffer();
}


void Apu::UpdateCycles()
{
	s32 cyclesThisUpdate = cycle - lastCycle;

	while(cyclesThisUpdate > 0)
	{
		u32 frameCounterCyclesExecuted = 0;
		u32 newFrameCounterCycleNum = frameCounterLastCycle + cyclesThisUpdate;

		u32 cyclesToNextStep = FrameCounterCycleSteps[frameCounterMode][frameCounterCurrentStep];

		if(newFrameCounterCycleNum >= cyclesToNextStep)
		{
			// IRQ from Framecounter
			bool modeHasIrq = (frameCounterMode == 0);
			bool inIrqStep = (frameCounterCurrentStep >= 3);
			if(modeHasIrq && inIrqStep && !frameCounterIrqInhibit)
			{
				Nes::GetCpu()->SetApuFrameCounterIRQ();
			}

			if(!frameCounterSkipCycles)
			{
				FrameCounterStepType type = FrameCounterStepTypes[frameCounterMode][frameCounterCurrentStep];
				if(type != FrameCounterStepType::NONE)
				{
					bool isHalfFrame = (type == FrameCounterStepType::HALF);
					OnFrameCounterClock(isHalfFrame);
					frameCounterSkipCycles = 2;
				}
			}

			frameCounterCyclesExecuted = cyclesToNextStep - frameCounterLastCycle;

			cyclesThisUpdate -= frameCounterCyclesExecuted;

			frameCounterCurrentStep++;
			if(frameCounterCurrentStep == 6) // Reset step and cycles
			{
				frameCounterCurrentStep = 0;
				frameCounterLastCycle = 0;
			}
			else
			{
				frameCounterLastCycle += frameCounterCyclesExecuted;
			}
		}
		else
		{
			// Not at the next framecounter clock. So move forward and set cyclesThisUpdate to zero to exit loop
			frameCounterCyclesExecuted = cyclesThisUpdate;
			frameCounterLastCycle += frameCounterCyclesExecuted;
			cyclesThisUpdate = 0; 
		}

		// Write takes 'frameCounterWriteWaitCycles' time to actually change the Framecounter mode. Here we wait for that to happen 
		if(frameCounterPendingWriteValue >= 0)
		{
			frameCounterWriteWaitCycles--;
			if(frameCounterWriteWaitCycles == 0)
			{
				// Reset all counters/steps ect
				frameCounterWriteWaitCycles = -1;
				frameCounterLastCycle = 0;
				frameCounterPendingWriteValue = -1;
				frameCounterCurrentStep = 0;

				// Change mode
				frameCounterMode = ((frameCounterPendingWriteValue & 0x80) == 0x80) ? 1 : 0;
				
				// mode set to 1 runs a half frame clock instantly
				if(frameCounterMode && !frameCounterSkipCycles)
				{
					bool isHalfFrame = true;
					OnFrameCounterClock(isHalfFrame);
					frameCounterSkipCycles = 2;
				}
			}
		}
		
		if(frameCounterSkipCycles > 0) { frameCounterSkipCycles--; }

		lastCycle += frameCounterCyclesExecuted;

		square1.ClockUpdate(lastCycle, this);
		square2.ClockUpdate(lastCycle, this);
		triangle.ClockUpdate(lastCycle, this);
		noise.ClockUpdate(lastCycle, this);
		dmc.ClockUpdate(lastCycle, this);
	}
}

bool Apu::IsFrameCounterUpdateRequired(u32 cycleDelta)
{
	bool hasSkipCycles = frameCounterSkipCycles > 0;

	u64 frameCounterCycle = lastCycle + cycleDelta;
	u64 currentStepClockCycle = FrameCounterCycleSteps[frameCounterMode][frameCounterCurrentStep];
	bool hitNextStep = frameCounterCycle >= (currentStepClockCycle - 1);

	bool hasNewValue = frameCounterPendingWriteValue;

	return hasSkipCycles || hitNextStep || hasNewValue;
}

void Apu::AddDeltaCycleNum(u64 cycleNum)
{
	deltaCycleNums[totalDeltas] = cycleNum;
	++totalDeltas;
}


void Apu::OnFrameCounterClock(bool isHalfFrame)
{
	if(isHalfFrame) // On Half Frames
	{
		square1.UpdateSweep();
		square2.UpdateSweep();

		// Update length counters
		if(square1.lengthCounter > 0 && !square1.haltLengthCounter) { square1.lengthCounter -= 1; }
		if(square2.lengthCounter > 0 && !square2.haltLengthCounter) { square2.lengthCounter -= 1; }
		if(triangle.lengthCounter > 0 && !triangle.haltLengthCounter) { triangle.lengthCounter -= 1; }
		if(noise.lengthCounter > 0 && !noise.haltLengthCounter) { noise.lengthCounter -= 1; }
	}

	square1.envelope.UpdateEnvelope();
	square2.envelope.UpdateEnvelope();
	triangle.UpdateLinearCounter();
	noise.envelope.UpdateEnvelope();
}



void SquareChannel::ClockUpdate(u64 currentCycle, Apu *apu)
{
	s64 cycleDelta = currentCycle - lastCycle;
	if(cycleDelta > timer)
	{
		cycleDelta -= timer + 1;
		lastCycle += timer + 1;

		currentDutyPos = (currentDutyPos - 1) & 0x07;

		s16 output = 0;

		if(lengthCounter > 0 && IsUnmuted())
		{
			u8 volume = envelope.GetEnvelopeVolume();
			u8 seqValue = SquareDutySequences[dutyCycle][currentDutyPos];
			output = volume * seqValue;	
		}
	
		if(output != lastOutput)
		{
			s16 delta = output - lastOutput;
			lastOutput = output;

			outputDeltas[lastCycle] = delta;
			apu->AddDeltaCycleNum(lastCycle);
		}

		timer = doublePeriod;
	}

	timer -= cycleDelta;
	lastCycle = currentCycle;
}

void TriangleChannel::ClockUpdate(u64 currentCycle, Apu *apu)
{
	s64 cycleDelta = currentCycle - lastCycle;
	if(cycleDelta > timer)
	{
		cycleDelta -= timer + 1;
		lastCycle += timer + 1;

		bool isActive = currentLinearCounter > 0 && lengthCounter > 0;
		if(isActive)
		{
			triangleSequencePos = (triangleSequencePos + 1) & 0x1F;

			s16 output = (lengthCounter > 0 && IsUnmuted()) ? TriangleSequence[triangleSequencePos] : 0;
			if(output != lastOutput)
			{
				s16 delta = output - lastOutput;
				lastOutput = output;

				outputDeltas[lastCycle] = delta;
				apu->AddDeltaCycleNum(lastCycle);
			}
		}
		timer = period;
	}

	timer -= cycleDelta;
	lastCycle = currentCycle;
}

void NoiseChannel::ClockUpdate(u64 currentCycle, Apu *apu)
{
	s64 cycleDelta = currentCycle - lastCycle;
	if(cycleDelta > timer)
	{
		cycleDelta -= timer + 1;
		lastCycle += timer + 1;

		u8 shiftBy = modeFlag ? 6 : 1; // Mode changes the shift amount, making a more metalic sound

		u16 feedback = (shiftRegister & 0x01) ^ ((shiftRegister >> shiftBy) & 0x01);
		shiftRegister >>= 1;
		shiftRegister |= (feedback << 14); // Move shifted value into MSB

		s16 output = (lengthCounter > 0 && IsUnmuted()) ? envelope.GetEnvelopeVolume() : 0;
		if(output != lastOutput)
		{
			s16 delta = output - lastOutput;
			lastOutput = output;

			outputDeltas[lastCycle] = delta;
			apu->AddDeltaCycleNum(lastCycle);
		}

		timer = period;
	}

	timer -= cycleDelta;
	lastCycle = currentCycle;
}

void DmcChannel::ClockUpdate(u64 currentCycle, Apu *apu)
{

	s64 cycleDelta = currentCycle - lastCycle;
	if(cycleDelta > timer)
	{
		cycleDelta -= timer + 1;
		lastCycle += timer + 1;

		if(!silenceSet)
		{
			if(shiftRegister & 0x01)
			{
				if(currentOutput <= 125) { currentOutput += 2; }
			}
			else
			{
				if(currentOutput >= 2) { currentOutput -= 2; }
			}
			shiftRegister >>= 1;
		}

		bitsToRead--;

		if(bitsToRead == 0)
		{
			bitsToRead = 8;
			if(hasBufferData)
			{
				silenceSet = false;
				shiftRegister = readByte;
				hasBufferData = false;
				StartDMCWrite();
			}
			else
			{
				silenceSet = true;
			}
		}

		if(currentOutput != lastOutput)
		{
			s16 delta = currentOutput - lastOutput;
			lastOutput = currentOutput;

			outputDeltas[lastCycle] = delta;
			apu->AddDeltaCycleNum(lastCycle);
		}

		timer = period;
	}

	timer -= cycleDelta;
	lastCycle = currentCycle;

}

void DmcChannel::StartSample()
{
	activeAddress = sampleAddress;
	bytesLeft = sampleLength;
	if(bytesLeft > 0)
	{
		updateRequired = true;
	}
}

void DmcChannel::StartDMCWrite()
{
	if(!hasBufferData && bytesLeft > 0)
	{
		Nes::GetCpu()->StartApuDMCWrite();
	}
}


void DmcChannel::EnabledUpdated()
{
	if(!enabled)
	{
		updateRequired = false;
		bytesLeft = 0;
	}
	else if(bytesLeft == 0)
	{
		StartSample();

		enabledCycleDelay = Nes::GetCpu()->IsOddCycle() ? 3 : 2;
	}
}

bool DmcChannel::NeedsUpdate()
{
	if(enabledCycleDelay > 0)
	{
		enabledCycleDelay -= 1;
		if(enabledCycleDelay == 0)
		{
			StartDMCWrite();
		}
	}
	
	return updateRequired;
}

void DmcChannel::SetDmcReadBuffer(u8 value)
{
	if(bytesLeft > 0)
	{
		activeAddress++;
		if(activeAddress == 0) { activeAddress = 0x8000; } // Address wraps to 0x8000

		readByte = value;
		bytesLeft--;
		hasBufferData = true;

		if(bytesLeft == 0)
		{
			updateRequired = false;
			if(loopSet) { StartSample(); }
			else if(irqEnable) { Nes::GetCpu()->SetApuDmcIRQ(); }
		}
	}
}

bool DmcChannel::IsIrqUpcoming(u32 cycleDelta)
{
	if(!irqEnable && bytesLeft == 0)
	{
		return false;
	}

	u32 bitsRemaining = (bitsToRead + (bytesLeft - 1) * 8);
	u32 cyclesLeftUntilEmptyBuffer = bitsRemaining * period;
	return (cycleDelta >= cyclesLeftUntilEmptyBuffer);
}