/*
  262 Scanlines a frame
  341 Clocks Per scanline
  1 Cpu Cycle = 3 ppu Cycles
  Each ppu Cycle produces a pixel output
 */

 /*
 TODO LIST

 - Power up state
 - DMA takes 512 cpu cycles. It stops normal execution of the cpu
 - Secondary OAM Clear and sprite eval do not happen on pre render line. Sprite tile fetches still do

 */

#include "nes.h"
#include "ppu.h"

void Ppu::DrawPixel(u16 x, u16 y, Color color)
{
	Color *pixel = (m_pixelBuffer + (y * ppuPixelWidth)) + x;
	*pixel = color;
}

u8 Ppu::ReadRegisters(u16 address)
{
	u8 openBusVal = Nes::GetOpenBus();
	
	u8 readByte = 0;
	if(address == 0x2002)
	{
		readByte |= spriteOverflow ? 0x20 : 0;
		readByte |= spriteZeroHit ? 0x40 : 0;
		readByte |= verticalBlank ? 0x80 : 0;
		readByte |= (openBusVal & 0x1F); // Low 5 bits is the open bus

		verticalBlank = false;
		Nes::GetCpu()->SetNMI(false);

		latchWrite = false; // VRAM latch reset

		// NOTE: Reading VBL one cycle before it is set, returns clear and suppresses vbl
		if(m_scanline == 241 && (m_cycle == 0))
		{
			supressVbl = true;
		}
		else
		{
			supressVbl = false;
		}
	}
	else if(address == 0x2004)
	{
		readByte = oam[oamAddress];
	}
	else if(address == 0x2007)
	{
		readByte = vRamDataBuffer;
		vRamDataBuffer = ReadPpuMemory(vRamAdrs & 0x3FFF);

		bool onPalette = (vRamAdrs & 0x3FFF) >= 0x3F00;
		if(onPalette)
		{
			readByte = ReadPpuMemory(vRamAdrs);

			readByte &= 0x3F; // Pulled from nes dev forum
			readByte |= openBusVal & 0xC0;
		}

		if(!(renderingEnabled) || (m_scanline >= 240 && m_scanline <= 260))
		{
			vRamAdrs = (vRamAdrs + vRamIncrement) & 0x7FFF;
		} 
		else
		{
			// TODO: Weird update of vRam, check ppu_scrolling on wiki
		}
	}

	Nes::SetOpenBus(readByte);
	return readByte;
}

void Ppu::WriteRegisters(u16 address, u8 value)
{
	Nes::SetOpenBus(value);

	if(address == 0x2000)
	{
		nametableBase = value & 0x03;
		vRamIncrement = ((value & 0x04) != 0) ? 32 : 1;
		sPRTPattenBase = ((value & 0x08) != 0) ? 0x1000 : 0;
		bGPatternBase = ((value & 0x10) != 0) ? 0x1000 : 0;
		spriteSize8x16 = ((value & 0x20) != 0);
		ppuSlave = ((value & 0x40) != 0);
		generateNMI = ((value & 0x80) != 0);

		tempVRamAdrs &= ~0xC00;
		tempVRamAdrs |= (nametableBase) << 10;

		// Nmi On Timing
		if(!generateNMI)
		{
			Nes::GetCpu()->SetNMI(false);
		}
		else if(verticalBlank)
		{
			Nes::GetCpu()->SetNMI(true);
		}
	}
	else if(address == 0x2001)
	{
		greyScale = ((value & 0x01) != 0);
		showBGLeft8Pixels = ((value & 0x02) != 0);
		showSPRTLeft8Pixels = ((value & 0x04) != 0);
		showBackground = ((value & 0x08) != 0);
		showSprites = ((value & 0x10) != 0);
		emphasizeRed = ((value & 0x20) != 0);
		emphasizeGreen = ((value & 0x40) != 0);
		emphasizeBlue = ((value & 0x80) != 0);

		renderingEnabled = (showBackground || showSprites);
	}
	else if(address == 0x2003)
	{
		oamAddress = value;
	}
	else if(address == 0x2004)
	{
		WriteOAMValue(value);
	}
	else if(address == 0x2005)
	{
		if(!latchWrite)
		{
			fineX = value & 0x07; // Bit 0,1, and 2 are fine X
			tempVRamAdrs = tempVRamAdrs & ~0x001F; // Clear Bits
			tempVRamAdrs |= ((u16)value) >> 3;
		} 
		else
		{
			tempVRamAdrs = tempVRamAdrs & ~0x73E0; // Clear Bits
			tempVRamAdrs |= ((u16)(value & 0x07)) << 12; // Set fine scroll Y, bits 0-2 set bit 12-14
			tempVRamAdrs |= ((u16)(value & 0xF8)) << 2; // Set coarse Y, bits 3-7 set bit 5-9
		}
		latchWrite = !latchWrite;
	}
	else if(address == 0x2006)
	{
		if(!latchWrite)
		{
			tempVRamAdrs = tempVRamAdrs & ~0xFF00; // Clear Bits
			tempVRamAdrs |= ((u16)(value & 0x003F)) << 8;
		}
		else
		{
			tempVRamAdrs &= 0x7F00; // Clear low byte
			tempVRamAdrs |= (u16)(value & 0x00FF);
			vRamAdrs = tempVRamAdrs;
		}
		latchWrite = !latchWrite;
	}
	else if(address == 0x2007)
	{
		//TraceLog(LOG_INFO, "0x2007 Write - vRamAdrs %04X, value = %02X", vRamAdrs, value);
		WritePpuMemory(vRamAdrs, value);

		if(!(showBackground || showSprites) ||
			(m_scanline > 240 && m_scanline <= 260))
		{
			vRamAdrs += vRamIncrement;
		}
	}
	else if(address == 0x4014)
	{
		Nes::GetCpu()->StartDMAWrite(value);
	}
}

static u16 PpuMemoryMirror(u16 address)
{
	// Over half of the memory map is mirrored
	address = address % 0x4000;

	if(0x3F00 <= address && address < 0x4000) // Palette
	{
		address = (address % 0x20) + 0x3F00;

		// Some of palette addresses are identical
		if(address == 0x3F10) { address = 0x3F00; }
		else if(address == 0x3F14) { address = 0x3F04; }
		else if(address == 0x3F18) { address = 0x3F08; }
		else if(address == 0x3F1C) { address = 0x3F0C; }
	}

	if(0x3000 <= address && address < 0x3F00) // Nametable Mirroring
	{
		address -= 0x1000;
	}

	return address;
}

u8 Ppu::ReadPpuMemory(u16 address)
{
	address = PpuMemoryMirror(address);

	if((showBackground || showSprites) &&
		(address == 0x3F04 || address == 0x3F08 || address == 0x3F0C))
	{
		address = 0x3F00;
	}

	if(address < 0x2000)
	{	
		return Nes::GetCartridge()->ReadCHRMemory(address);
	}
	else if(address >= 0x2000 && address < 0x3000)
	{
		u8 *nametableBank = GetNametableBank(address);
		return nametableBank[address % 0x400];
	}
	else if(address >= 0x3F00 && address < 0x3F20)
	{
		return m_paletteMemory[address - 0x3F00];
	}
	else
	{
		Assert(0);
		return 0;
	}
}

void Ppu::WritePpuMemory(u16 address, u8 value)
{
	address = PpuMemoryMirror(address);

	if(address < 0x2000)
	{
		Nes::GetCartridge()->WriteCHRMemory(address, value);
	}
	else if(0x2000 <= address && address < 0x3000)
	{
		u8 *nametableBank = GetNametableBank(address);
		nametableBank[address % 0x400] = value;
	}
	else if(address >= 0x3F00 && address < 0x3F20)
	{
		m_paletteMemory[address - 0x3F00] = value;
	}
	else
	{
		Assert(0);
	}
}



void Ppu::ResetScrollHorz()
{
	vRamAdrs &= ~(0x041F);
	vRamAdrs |= (tempVRamAdrs & 0x041F);
}

void Ppu::ResetScrollVert()
{
	vRamAdrs = tempVRamAdrs;
}

void Ppu::ScrollIncHorz()
{
	//NOTE: Code take from nesdev wiki. Could be quicker??
	if((vRamAdrs & 0x001F) == 31) // if coarse X == 31
	{
		vRamAdrs &= ~0x001F;  // coarse X = 0
		vRamAdrs ^= 0x0400;   // switch horizontal nametable
	}
	else
	{
		vRamAdrs += 1; // increment coarse X
	}
}

void Ppu::ScrollIncVert()
{
	// NOTE: Code take from nesdev wiki. Could be quicker??
	if((vRamAdrs & 0x7000) != 0x7000) // if fine Y < 7
	{
		vRamAdrs += 0x1000; // increment fine Y
	}
	else
	{
		vRamAdrs &= ~0x7000; // fine Y = 0        
		u16 y = (vRamAdrs & 0x03E0) >> 5; // let y = coarse Y

		if(y == 29)
		{
			y = 0; // coarse Y = 0
			vRamAdrs ^= 0x0800; // switch vertical nametable
		}
		else if(y == 31)
		{
			y = 0; // coarse Y = 0, nametable not switched
		}
		else
		{
			y += 1; // increment coarse Y
		}
		vRamAdrs = (vRamAdrs & ~0x03E0) | (y << 5); // put coarse Y back into v
	}
}

void Ppu::LoadFutureData()
{
	if((1 <= m_cycle && m_cycle <= 255) ||
		(321 <= m_cycle && m_cycle <= 336))
	{
		u8 Cycle = (m_cycle) % 8;
		if(Cycle == 1)
		{
			lowPatternShiftReg = (lowPatternShiftReg << 8) | nextLowPattern;
			highPatternShiftReg = (highPatternShiftReg << 8) | nextHighPattern;
			paletteLatchOld = paletteLatchNew;
			paletteLatchNew = nextAtrbByte << 2;
		}
		else if(Cycle == 2)
		{
			u16 nametableAddress = 0x2000 | (vRamAdrs & 0x0FFF);
			nextNametableAdrs = ReadPpuMemory(nametableAddress) << 4;
			nextNametableAdrs += bGPatternBase;
		}
		else if(Cycle == 4)
		{
			u16 AtrbAddress = 0x23C0 | (vRamAdrs & 0x0C00) |
				((vRamAdrs >> 4) & 0x38) | ((vRamAdrs >> 2) & 0x07);
			u8 Atrb = ReadPpuMemory(AtrbAddress);
			int QuadrantSelect = ((vRamAdrs & 2) >> 1) | ((vRamAdrs & 0x40) >> 5);

			nextAtrbByte = ((0xFF & Atrb) >> (QuadrantSelect * 2)) & 3;
		}
		else if(Cycle == 6)
		{
			nextNametableAdrs = nextNametableAdrs + ((vRamAdrs & 0x7000) >> 12);
			nextLowPattern = ReadPpuMemory(nextNametableAdrs);
		}
		else if(Cycle == 0)
		{
			nextHighPattern = ReadPpuMemory(nextNametableAdrs + 8);
			ScrollIncHorz();
		}
	}
	else if(m_cycle == 256)
	{
		ScrollIncVert();
	}
	else if(m_cycle == 257)
	{
		ResetScrollHorz();
	}
}

// NOTE: Reverse the bits within a byte, eg. 0011 0101 becomes 1010 1100
//       Works by diving into halves and swaping the two. 
inline u8 byteReverse(u8 byte)
{
	byte = (byte & 0xF0) >> 4 | (byte & 0x0F) << 4; // 1111 0000 and 0000 1111 swap
	byte = (byte & 0xCC) >> 2 | (byte & 0x33) << 2; // 1100 and 0011 swap
	byte = (byte & 0xAA) >> 1 | (byte & 0x55) << 1; // 10 and 01 swap

	return byte;
}

/* Loops through the OAM and find the sprites one the next scanline */
void Ppu::EvaluateSecondaryOam()
{
	secondarySpriteCount = 0;

	u8 spriteHeight = (spriteSize8x16 != 0) ? 16 : 8;

	for(u8 oamSpriteCount = 0;
		oamSpriteCount < oamSpriteTotal && secondarySpriteCount != secondaryOamSpriteMax;
		++oamSpriteCount)
	{
		OamSprite *oamSprite = (OamSprite *)oam + oamSpriteCount;

		if(oamSprite->Y <= m_scanline && m_scanline < (oamSprite->Y + spriteHeight))
		{
			Sprite sprite = {};
			sprite.oamData = *oamSprite;
			sprite.priority = !(sprite.oamData.atrb & (1 << 5));

			if(oamSpriteCount == 0)
			{
				sprite.spriteZero = true;
			}

			// Palette
			sprite.paletteValue = sprite.oamData.atrb & 3;

			// Chr Pattern Select
			u8 tileRelY = (u8)(m_scanline - sprite.oamData.Y) % spriteHeight;
			u8 yOffset = (sprite.oamData.atrb & (1 << 7)) ? ((spriteHeight - 1) - tileRelY) : tileRelY;

			u8 tileIndex;
			u16 spritePatternBase;

			if(!spriteSize8x16)
			{
				tileIndex = sprite.oamData.tile;
				spritePatternBase = sPRTPattenBase;
			}
			else
			{
				tileIndex = sprite.oamData.tile & ~0x1; // Bit 0 is ignored in 8x16 mode 
				spritePatternBase = (sprite.oamData.tile & 1) ? 0x1000 : 0;

				if(yOffset >= 8)
				{
					yOffset -= 8;
					tileIndex += 1;
				}
			}

			u16 lowAddress = (spritePatternBase + (tileIndex * 16)) + yOffset;
			u16 highAddress = (spritePatternBase + (tileIndex * 16) + 8) + yOffset;

			sprite.patternLow = ReadPpuMemory(lowAddress);
			sprite.patternHigh = ReadPpuMemory(highAddress);

			if(sprite.oamData.atrb & (1 << 6))
			{
				sprite.patternLow = byteReverse(sprite.patternLow);
				sprite.patternHigh = byteReverse(sprite.patternHigh);
			}

			secondaryOam[secondarySpriteCount++] = sprite;
		}
	}
}

void Ppu::VisibleLine()
{
	if(renderingEnabled)
	{
		LoadFutureData();

		if(m_cycle == 64) // NOTE: Clearing takes 64 cycles. Running on very last one
		{
			// Secondary Oam Clear
			secondarySpriteCount = 0;
			for(u8 spriteCount = 0; spriteCount < secondaryOamSpriteMax; ++spriteCount)
			{
				Sprite *sprite = secondaryOam + spriteCount;
				sprite->oamData.Y = 0xFF;
				sprite->oamData.tile = 0xFF;
				sprite->oamData.atrb = 0xFF;
				sprite->oamData.X = 0xFF;

				sprite->priority = false;
				sprite->spriteZero = false;

				sprite->paletteValue = 0;
				sprite->patternLow = 0;
				sprite->patternHigh = 0;
			}
			// TODO: attempting to read $2004 will return $FF
		}
		else if(m_cycle == 256) // Sprite Evaluation happens from cycle 65 to 256
		{
			EvaluateSecondaryOam();
		}
		else if(m_cycle == 257)
		{
			// Clear prepared sprites
			preparedSpriteCount = 0;
			for(u8 spriteCount = 0; spriteCount < secondaryOamSpriteMax; ++spriteCount)
			{
				Sprite *sprite = preparedSprites + spriteCount;
				sprite->oamData.Y = 0xFF;
				sprite->oamData.tile = 0xFF;
				sprite->oamData.atrb = 0xFF;
				sprite->oamData.X = 0xFF;

				sprite->priority = false;
				sprite->spriteZero = false;

				sprite->paletteValue = 0;
				sprite->patternLow = 0;
				sprite->patternHigh = 0;
			}

			// Copy to prepared sprites to secondary buffer can evaluate the next scanline
			preparedSpriteCount = secondarySpriteCount;

			for(u8 spriteIdx = 0; spriteIdx < secondarySpriteCount; ++spriteIdx)
			{
				preparedSprites[spriteIdx] = secondaryOam[spriteIdx];
			}
			// TODO: First empty slot has sprite 64s y coord followed by 3 0xFF bytes. Other empty slots are all 0xFF
		}

		// Reset on sprite loading interval
		if(m_cycle >= 257 && m_cycle <= 320)
		{
			oamAddress = 0;
		}
	}

	if(1 <= m_cycle && m_cycle <= 256)
	{
		u16 pixelX = m_cycle - 1;
		u16 pixelY = m_scanline;

		// Get the default colour
		constexpr u8 DefaultColorIndex = 0;
		Color pixelColor = ColorPalette[DefaultColorIndex];

		constexpr u8 spritePaletteOffset = 0x10;

		/* *********************** */
		/* Background Calculations */

		u8 backgroundResult = 0;

		if(showBackground && !(pixelX < 8 && !showBGLeft8Pixels))
		{
			u8 xOffset = 15 - (fineX + ((pixelX) % 8));

			u8 patternPixelValue = (((highPatternShiftReg >> (xOffset - 1)) & 2) |
				(lowPatternShiftReg >> xOffset) & 1);

			if(patternPixelValue != 0) // NOTE: If Value is zero, then it is a background/transparent
			{
				u8 atrbPixelValue = (xOffset >= 8) ? paletteLatchOld : paletteLatchNew;
				backgroundResult = atrbPixelValue | patternPixelValue;
			}

			Assert(backgroundResult < spritePaletteOffset);
			u8 bgrdPaletteIndex = m_paletteMemory[backgroundResult];
			pixelColor = ColorPalette[bgrdPaletteIndex];
		}

		/* ******************* */
		/* Sprite Calculations */

		if(showSprites && !(pixelX < 8 && !showSPRTLeft8Pixels))
		{
			for(s16 spriteIdx = preparedSpriteCount - 1; spriteIdx >= 0; --spriteIdx)
			{
				Sprite *sprite = preparedSprites + spriteIdx;
				u8 spriteX = sprite->oamData.X;

				if(pixelX != 0xFF && spriteX <= pixelX && pixelX < (spriteX + pixelsPerTile))
				{
					u8 relX = ((pixelX - (spriteX)) % 8);
					u8 patternValue = (((sprite->patternHigh >> (7 - relX)) & 1) << 1) |
						((sprite->patternLow >> (7 - relX)) & 1);

					if(patternValue != 0) // If bottom two bits is 0, then is multiple of 4.
					{
						u8 spriteColor = (sprite->paletteValue << 2) | patternValue;

						if(sprite->spriteZero && !spriteZeroHit &&
							showBackground && backgroundResult != 0 &&
							m_scanline <= 239 /*&& Sprite->OamData.Y != 255*/ && m_cycle != 256)
						{
							spriteZeroHit = true;
						}

						if(sprite->priority || backgroundResult == 0)
						{
							u8 sprtPaletteIndex = m_paletteMemory[spritePaletteOffset + spriteColor];
							pixelColor = ColorPalette[sprtPaletteIndex];
						}
					}
				}
			}
		}

		DrawPixel(pixelX, pixelY, pixelColor); // PIXEL OUTPUT - using the resulting colour
	}
}

void Ppu::PostRenderLine()
{
	if(m_cycle == 0)
	{
		m_frameNum++;
	}
}

void Ppu::VblankLine()
{
	if(m_scanline == 241 && m_cycle == 1)
	{
		// TODO TODO Check over
		if(!supressVbl)
		{
			verticalBlank = true;
		}

		if(generateNMI && verticalBlank && !supressNmiSet)
		{
			Nes::GetCpu()->SetNMI(true);
		}

		supressNmiSet = false;
	}
}

void Ppu::PreRenderLine()
{
	if(m_cycle == 1)
	{
		spriteOverflow = false;
		spriteZeroHit = false;
		verticalBlank = false;
		Nes::GetCpu()->SetNMI(false);
	}

	if(renderingEnabled)
	{
		if(m_cycle == 64) // NOTE: Clearing takes 64 cycles. Running on very last one
		{
			// Secondary Oam Clear
			u8 *data = (u8 *)secondaryOam;

			for(u16 byte = 0; byte < (secondaryOamSpriteMax * sizeof(Sprite)); ++byte)
			{
				data[byte] = 0xFF;
			}
		}
		else if(m_cycle == 257)
		{
			// Clear prepared sprites
			u8 *data = (u8 *)preparedSprites;

			for(u16 byte = 0; byte < (secondaryOamSpriteMax * sizeof(Sprite)); ++byte)
			{
				data[byte] = 0xFF;
			}
		}

		LoadFutureData();

		if(280 <= m_cycle && m_cycle <= 304)
		{
			ResetScrollVert();
		}

		// Reset on sprite loading interval
		if(m_cycle >= 257 && m_cycle <= 320)
		{
			oamAddress = 0;
		}

		bool evenFrame = (m_frameNum % 2) == 1;
		if(m_cycle == 339 && evenFrame)
		{
			m_cycle++; // Skip last cycle of prerenderline if odd frame
		}
	}
}

void Ppu::RunCycle()
{
	if(m_scanline < 240) { VisibleLine(); }
	else if(m_scanline > 240 && m_scanline < 261) { VblankLine(); }
	else if(m_scanline == 240) { PostRenderLine(); }
	else if(m_scanline == 261) { PreRenderLine(); }
	else { Assert(0); }

	++m_cycle; // Once finished, update the cycle and scanline if on new one
	if(m_cycle > 340) // Moving to new Scanline
	{
		m_cycle = 0;
		++m_scanline;

		if(m_scanline > 261) // If reached new Frame
		{
			m_scanline = 0;
		}
	}
}

Ppu::Ppu()
{
	/// Ppu pixel buffer
	u32 pixelCount = gNesWidth * gNesHeight;
	m_pixelBuffer = (Color *)MemAlloc(pixelCount * sizeof(Color)); // in R8G8B8A8 format
}

Ppu::~Ppu()
{
	MemFree(m_pixelBuffer);
	m_pixelBuffer = nullptr;
}

void Ppu::Reset()
{
	MemorySet(m_nametableBankA, 0, NametableBankSize);
	MemorySet(m_nametableBankB, 0, NametableBankSize);
	MemorySet(m_nametableBankC, 0, NametableBankSize);
	MemorySet(m_nametableBankD, 0, NametableBankSize);
	MemorySet(m_paletteMemory, 0, PaletteSize);

	// Palette at startup according to Blargg
	u8 paletteStartup[] = {0x09, 0x01, 0x00, 0x01,
							  0x00, 0x02, 0x02, 0x0D,
							  0x08, 0x10, 0x08, 0x24,
							  0x00, 0x00, 0x04, 0x2C,
							  0x09, 0x01, 0x34, 0x03,
							  0x00, 0x04, 0x00, 0x14,
							  0x08, 0x3A, 0x00, 0x02,
							  0x00, 0x20, 0x2C, 0x08};
	Assert(ArrayCount(paletteStartup) == PaletteSize);

	for(u8 idx = 0; idx < PaletteSize; ++idx)
	{
		m_paletteMemory[idx] = paletteStartup[idx];
	}

	m_frameNum = 0;
}

void Ppu::RunCatchup(u64 actualMasterClock)
{
	constexpr u8 masterClockDivider = 4;
	while(m_masterClock + masterClockDivider <= actualMasterClock)
	{
		RunCycle();
		m_masterClock += masterClockDivider;
	}
}

u8 *Ppu::GetNametableBank(u16 address)
{
	u8 *resultBank = 0;

	NametableMirror mirrorType = Nes::GetCartridge()->GetNametableMirrorType();

	if(mirrorType == NametableMirror::SINGLE_SCREEN_BANK_A)
	{
		resultBank = m_nametableBankA;
	}
	else if(mirrorType == NametableMirror::SINGLE_SCREEN_BANK_B)
	{
		resultBank = m_nametableBankB;
	}
	else if(mirrorType == NametableMirror::VERTICAL_MIRROR)
	{
		if(address < 0x2400 || (0x2800 <= address && address < 0x2C00))
		{
			resultBank = m_nametableBankA;
		}
		else
		{
			resultBank = m_nametableBankB;
		}
	}
	else if(mirrorType == NametableMirror::HORIZONTAL_MIRROR)
	{
		if(address < 0x2800)
		{
			resultBank = m_nametableBankA;
		}
		else
		{
			resultBank = m_nametableBankB;
		}
	}
	else if(mirrorType == NametableMirror::FOUR_SCREEN_MIRROR)
	{
		Assert(0);
		if(address < 0x2400)
		{
			resultBank = m_nametableBankA;
		}
		else if(address < 0x2800)
		{
			resultBank = m_nametableBankB;
		}
		else if(address < 0x2C00)
		{
			resultBank = m_nametableBankC;
		}
		else
		{
			resultBank = m_nametableBankD;
		}
	}

	return resultBank;
}

void Ppu::WriteOAMValue(u8 value)
{
	// If Writing OAM Data while rendering, then a glitch increments it by 4 instead
	if((m_scanline >= 240 && m_scanline != 261) || !renderingEnabled)
	{
		if((oamAddress & 0x03) == 0x02)
		{
			value = value & 0xE3;
		}

		//TraceLog(LOG_INFO, "WriteOAM Address - %04X, Value - %02X", oamAddress, value);

		oam[oamAddress] = value;
		oamAddress++;
	}
	else
	{
		oamAddress += 4;
	}
}