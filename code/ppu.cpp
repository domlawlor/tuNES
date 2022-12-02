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
#include "palette.cpp"

void Ppu::DrawPixel(u16 x, u16 y, Color color)
{
	Color *pixel = (m_pixelBuffer + (y * ppuPixelWidth)) + x;
	*pixel = color;
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
	if((1 <= scanlineCycle && scanlineCycle <= 255) ||
		(321 <= scanlineCycle && scanlineCycle <= 336))
	{
		u8 Cycle = (scanlineCycle) % 8;
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
			nextNametableAdrs = ReadPpu8(nametableAddress, ppu) << 4;
			nextNametableAdrs += bGPatternBase;
		}
		else if(Cycle == 4)
		{
			u16 AtrbAddress = 0x23C0 | (vRamAdrs & 0x0C00) |
				((vRamAdrs >> 4) & 0x38) | ((vRamAdrs >> 2) & 0x07);
			u8 Atrb = ReadPpu8(AtrbAddress, ppu);
			int QuadrantSelect = ((vRamAdrs & 2) >> 1) | ((vRamAdrs & 0x40) >> 5);

			nextAtrbByte = ((0xFF & Atrb) >> (QuadrantSelect * 2)) & 3;
		}
		else if(Cycle == 6)
		{
			nextNametableAdrs = nextNametableAdrs + ((vRamAdrs & 0x7000) >> 12);
			nextLowPattern = ReadPpu8(nextNametableAdrs, ppu);
		}
		else if(Cycle == 0)
		{
			nextHighPattern = ReadPpu8(nextNametableAdrs + 8, ppu);
			ScrollIncHorz();
		}
	}
	else if(scanlineCycle == 256)
	{
		ScrollIncVert();
	}
	else if(scanlineCycle == 257)
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
	u8 *oam = oam;
	Sprite *secondaryOam = secondaryOam;
	secondarySpriteCount = 0;

	u8 spriteHeight = (spriteSize8x16 != 0) ? 16 : 8;

	for(u8 oamSpriteCount = 0;
		oamSpriteCount < oamSpriteTotal && secondarySpriteCount != secondaryOamSpriteMax;
		++oamSpriteCount)
	{
		OamSprite *oamSprite = (OamSprite *)oam + oamSpriteCount;

		if(oamSprite->Y <= scanline && scanline < (oamSprite->Y + spriteHeight))
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
			u8 tileRelY = (u8)(scanline - sprite.oamData.Y) % spriteHeight;
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

			sprite.patternLow = ReadPpu8(lowAddress, ppu);
			sprite.patternHigh = ReadPpu8(highAddress, ppu);

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
	u16 scanline = scanline;
	u16 cycle = scanlineCycle;

	if(renderingEnabled)
	{
		LoadFutureData();

		if(cycle == 64) // NOTE: Clearing takes 64 cycles. Running on very last one
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
		else if(cycle == 256) // Sprite Evaluation happens from cycle 65 to 256
		{
			EvaluateSecondaryOam();
		}
		else if(cycle == 257)
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
	}

	if(1 <= cycle && cycle <= 256)
	{
		u16 pixelX = cycle - 1;
		u16 pixelY = scanline;

		// Get the default colour
		u8 blankPaletteIndex = ReadPpu8(backgroundPaletteAddress, ppu);
		Color pixelColor = ColorPalette[blankPaletteIndex];

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

			u8 bgrdPaletteIndex = ReadPpu8(backgroundPaletteAddress + backgroundResult, ppu);
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
						u8 spriteColour = (sprite->paletteValue << 2) | patternValue;

						if(sprite->spriteZero && !spriteZeroHit &&
							showBackground && backgroundResult != 0 &&
							scanline <= 239 /*&& Sprite->OamData.Y != 255*/ && scanlineCycle != 256)
						{
							spriteZeroHit = true;
						}

						if(sprite->priority || backgroundResult == 0)
						{
							u8 sprtPaletteIndex = ReadPpu8(spritePaletteAddress + spriteColour, ppu);
							pixelColor = ColorPalette[sprtPaletteIndex];
						}
					}
				}
			}
		}

		// PIXEL OUTPUT - using the resulting colour
		DrawPixel(pixelX, pixelY, pixelColor);
	}

}

void Ppu::VblankLine()
{
	u16 scanline = scanline;
	u16 cycle = scanlineCycle;

	if(scanline == 241 && cycle == 1)
	{
		// TODO TODO Check over
		if(!supressVbl)
		{
			verticalBlank = true;
		}

		nmiFlag = generateNMI && verticalBlank && !supressNmiSet;

		if(supressNmiSet)
		{
			supressNmiSet = false;
		}

		globalDrawScreen = true;
	}
}

void Ppu::PreRenderLine()
{
	u16 cycle = scanlineCycle;

	if(cycle == 1)
	{
		spriteOverflow = false;
		spriteZeroHit = false;
		verticalBlank = false;
		SetNmi(false);
	}

	if(renderingEnabled)
	{
		if(cycle == 64) // NOTE: Clearing takes 64 cycles. Running on very last one
		{
			// Secondary Oam Clear
			u8 *data = (u8 *)secondaryOam;

			for(u16 byte = 0; byte < (secondaryOamSpriteMax * sizeof(Sprite)); ++byte)
			{
				data[byte] = 0xFF;
			}
		}
		else if(cycle == 257)
		{
			// Clear prepared sprites
			u8 *data = (u8 *)preparedSprites;

			for(u16 byte = 0; byte < (secondaryOamSpriteMax * sizeof(Sprite)); ++byte)
			{
				data[byte] = 0xFF;
			}
		}

		LoadFutureData();

		if(280 <= cycle && cycle <= 304)
		{
			ResetScrollVert();
		}
	}
}

void Ppu::RunCycle()
{
	if(scanlineType == ScanlineType::VISIBLE) { VisibleLine(); }
	else if(scanlineType == ScanlineType::POST_RENDER) { PostRenderLine(); }
	else if(scanlineType == ScanlineType::VBLANK) { VblankLine(); }
	else if(scanlineType == ScanlineType::PRE_RENDER) { PreRenderLine(); }
	else { Assert(0); }

	++scanlineCycle; // Once finished, update the cycle and scanline if on new one
	Assert(scanlineCycle <= 341);

	if(scanlineCycle == 341) // Moving to new Scanline
	{
		scanlineCycle = 0;
		++scanline;

		if(scanline == 262) // If reached new Frame
		{
			scanline = 0;
			oddFrame = !oddFrame;

			scanlineType = ScanlineType::VISIBLE;

			// On oddframe, when rendering is enabled, the first cycle of frame is skipped
			if(oddFrame && renderingEnabled)
			{
				++scanlineCycle;
			}
		}
		else if(scanline == 261) { scanlineType = ScanlineType::PRE_RENDER; }
		else if(241 <= scanline) { scanlineType = ScanlineType::VBLANK; }
		else if(scanline == 240) { scanlineType = ScanlineType::POST_RENDER; }
		else { Assert(0); }
	}
}


void Ppu::Init()
{
	MemorySet(memory, 0, PpuMemorySize);

	/// Ppu pixel buffer
	u32 pixelCount = gNesWidth * gNesHeight;
	m_pixelBuffer = (Color *)MemAlloc(pixelCount * sizeof(Color)); // in R8G8B8A8 format


	// TODO: This is to test if oam is initialized differently.


	// TODO: Check over OAM code



	// Palette at startup according to Blargg
	u8 paletteSize = 32;
	u8 paletteStartup[] = {0x09, 0x01, 0x00, 0x01,
							  0x00, 0x02, 0x02, 0x0D,
							  0x08, 0x10, 0x08, 0x24,
							  0x00, 0x00, 0x04, 0x2C,
							  0x09, 0x01, 0x34, 0x03,
							  0x00, 0x04, 0x00, 0x14,
							  0x08, 0x3A, 0x00, 0x02,
							  0x00, 0x20, 0x2C, 0x08};

	for(u8 idx = 0; idx < paletteSize; ++idx)
	{
		u16 address = backgroundPaletteAddress + idx;
		m_memory[address] = paletteStartup[idx];
	}
}

void Ppu::RunCatchup(u64 masterClock)
{
	RunCycle();
}