/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

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

#include "palette.cpp"

static void DrawPixel(Ppu *ppu, u16 X, u16 Y, Colour colour)
{
    u32 *CurrentPixel = (ppu->basePixel + (Y * ppuPixelWidth)) + X;
    *CurrentPixel  = ((colour.R << 16) | (colour.G << 8) | colour.B);
}

static void ResetScrollHorz(Ppu *ppu)
{   
    ppu->vRamAdrs &= ~(0x041F);
    ppu->vRamAdrs |= (ppu->tempVRamAdrs & 0x041F);
}

static void ResetScrollVert(Ppu *ppu)
{
    ppu->vRamAdrs = ppu->tempVRamAdrs;
}

static void ScrollIncHorz(Ppu *ppu)
{
    //NOTE: Code take from nesdev wiki. Could be quicker??
    if ((ppu->vRamAdrs & 0x001F) == 31) // if coarse X == 31
    {
        ppu->vRamAdrs &= ~0x001F;  // coarse X = 0
        ppu->vRamAdrs ^= 0x0400;   // switch horizontal nametable
    }
    else
    {
        ppu->vRamAdrs += 1; // increment coarse X
    }
}

static void ScrollIncVert(Ppu *ppu)
{
    // NOTE: Code take from nesdev wiki. Could be quicker??
    if ((ppu->vRamAdrs & 0x7000) != 0x7000) // if fine Y < 7
    {
        ppu->vRamAdrs += 0x1000; // increment fine Y
    }
    else
    {
        ppu->vRamAdrs &= ~0x7000; // fine Y = 0        
        u16 y = (ppu->vRamAdrs & 0x03E0) >> 5 ; // let y = coarse Y
    
        if (y == 29)
        {
            y = 0; // coarse Y = 0
            ppu->vRamAdrs ^= 0x0800; // switch vertical nametable
        }
        else if (y == 31)
        {
            y = 0; // coarse Y = 0, nametable not switched
        }
        else
        {
            y += 1; // increment coarse Y
        }
        ppu->vRamAdrs = (ppu->vRamAdrs & ~0x03E0) | (y << 5); // put coarse Y back into v
    }
}

static void LoadFutureData(Ppu *ppu)
{    
    if((1 <= ppu->scanlineCycle && ppu->scanlineCycle <= 255) ||
       (321 <= ppu->scanlineCycle && ppu->scanlineCycle <= 336))
    {
        u8 Cycle = (ppu->scanlineCycle) % 8;
        if(Cycle == 1) 
        {
            ppu->lowPatternShiftReg = (ppu->lowPatternShiftReg << 8) | ppu->nextLowPattern;
            ppu->highPatternShiftReg = (ppu->highPatternShiftReg << 8) | ppu->nextHighPattern;
            ppu->paletteLatchOld = ppu->paletteLatchNew;
            ppu->paletteLatchNew = ppu->nextAtrbByte << 2;               
        }
        else if(Cycle == 2)
        {
            u16 nametableAddress = 0x2000 | (ppu->vRamAdrs & 0x0FFF);
            ppu->nextNametableAdrs = ReadPpu8(nametableAddress, ppu) << 4;
            ppu->nextNametableAdrs += ppu->bGPatternBase;
        }
        else if(Cycle == 4)
        {
            u16 AtrbAddress = 0x23C0 | (ppu->vRamAdrs & 0x0C00) |
                ((ppu->vRamAdrs >> 4) & 0x38) | ((ppu->vRamAdrs >> 2) & 0x07);   
            u8 Atrb = ReadPpu8(AtrbAddress, ppu);
            int QuadrantSelect = ((ppu->vRamAdrs & 2) >> 1) | ((ppu->vRamAdrs & 0x40) >> 5);
        
            ppu->nextAtrbByte = ((0xFF & Atrb) >> (QuadrantSelect*2)) & 3;       
        }   
        else if(Cycle == 6)
        {        
            ppu->nextNametableAdrs = ppu->nextNametableAdrs + ((ppu->vRamAdrs & 0x7000) >> 12);
            ppu->nextLowPattern = ReadPpu8(ppu->nextNametableAdrs, ppu);
        }
        else if(Cycle == 0)
        {
            ppu->nextHighPattern = ReadPpu8(ppu->nextNametableAdrs+8, ppu);
            ScrollIncHorz(ppu);
        }
    }
    else if(ppu->scanlineCycle == 256)
    {
        ScrollIncVert(ppu);
    }
    else if(ppu->scanlineCycle == 257)
    {
        ResetScrollHorz(ppu);
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
static void EvaluateSecondaryOam(Ppu *ppu)
{
    u8 *oam = ppu->oam;
    Sprite *secondaryOam = ppu->secondaryOam;
    ppu->secondarySpriteCount = 0;
    
    u8 spriteHeight = (ppu->spriteSize8x16 != 0) ? 16: 8;
                
    for(u8 oamSpriteCount = 0;
        oamSpriteCount < oamSpriteTotal && ppu->secondarySpriteCount != secondaryOamSpriteMax;
        ++oamSpriteCount)
    {
        OamSprite *oamSprite = (OamSprite *)oam + oamSpriteCount;
        
        if(oamSprite->Y <= ppu->scanline && ppu->scanline < (oamSprite->Y + spriteHeight))
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
            u8 tileRelY = (u8)(ppu->scanline - sprite.oamData.Y ) % spriteHeight;
            u8 yOffset = (sprite.oamData.atrb & (1 << 7)) ? ((spriteHeight-1) - tileRelY) : tileRelY;

            u8 tileIndex;
            u16 spritePatternBase;
            
            if(!ppu->spriteSize8x16)
            {
                tileIndex = sprite.oamData.tile; 
                spritePatternBase = ppu->sPRTPattenBase;
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

            u16 lowAddress  = (spritePatternBase + (tileIndex * 16)) + yOffset;
            u16 highAddress = (spritePatternBase + (tileIndex * 16) + 8) + yOffset;
            
            sprite.patternLow  = ReadPpu8(lowAddress, ppu);
            sprite.patternHigh = ReadPpu8(highAddress, ppu);
            
            if(sprite.oamData.atrb & (1 << 6))
            {
                sprite.patternLow = byteReverse(sprite.patternLow);
                sprite.patternHigh = byteReverse(sprite.patternHigh);
            }
            
            secondaryOam[ppu->secondarySpriteCount++] = sprite;
        }
    }
}

static void VisibleLine(Ppu *ppu)
{
    u16 scanline = ppu->scanline;
    u16 cycle = ppu->scanlineCycle;

    if(ppu->renderingEnabled)
    {
        LoadFutureData(ppu);

        if(cycle == 64) // NOTE: Clearing takes 64 cycles. Running on very last one
        {
            // Secondary Oam Clear
            ppu->secondarySpriteCount = 0;
            for(u8 spriteCount = 0; spriteCount < secondaryOamSpriteMax; ++spriteCount)
            {
                Sprite *sprite = ppu->secondaryOam + spriteCount;
                sprite->oamData.Y    = 0xFF;
                sprite->oamData.tile = 0xFF;
                sprite->oamData.atrb = 0xFF;
                sprite->oamData.X    = 0xFF;
				
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
            EvaluateSecondaryOam(ppu);
        }
        else if(cycle == 257)
        {
            // Clear prepared sprites
            ppu->preparedSpriteCount = 0;
            for(u8 spriteCount = 0; spriteCount < secondaryOamSpriteMax; ++spriteCount)
            {
                Sprite *sprite = ppu->preparedSprites + spriteCount;
                sprite->oamData.Y    = 0xFF;
                sprite->oamData.tile = 0xFF;
                sprite->oamData.atrb = 0xFF;
                sprite->oamData.X    = 0xFF;
				
                sprite->priority = false;
                sprite->spriteZero = false;
				
                sprite->paletteValue = 0;
                sprite->patternLow = 0;
                sprite->patternHigh = 0;
            }
            
            // Copy to prepared sprites to secondary buffer can evaluate the next scanline
            ppu->preparedSpriteCount = ppu->secondarySpriteCount;
        
            for(u8 spriteIdx = 0; spriteIdx < ppu->secondarySpriteCount; ++spriteIdx)
            {
                ppu->preparedSprites[spriteIdx] = ppu->secondaryOam[spriteIdx];
            }
            // TODO: First empty slot has sprite 64s y coord followed by 3 0xFF bytes. Other empty slots are all 0xFF
        }
    }
    
    if(1 <= cycle && cycle <= 256)
    {
        u16 pixelX = cycle - 1;
        u16 pixelY = scanline;

        Colour pixelColour = {};

        // Get the default colour
        u8 blankPaletteIndex = ReadPpu8(backgroundPaletteAddress, ppu);
        pixelColour = Palette[blankPaletteIndex];
                
        /* *********************** */
        /* Background Calculations */
                
        u8 backgroundResult = 0;
        
        if(ppu->showBackground && !(pixelX < 8 && !ppu->showBGLeft8Pixels) )
        {
            u8 xOffset = 15 - (ppu->fineX + ((pixelX) % 8));
                        
			u8 patternPixelValue = (((ppu->highPatternShiftReg >> (xOffset - 1)) & 2) |
									(ppu->lowPatternShiftReg >> xOffset) & 1);

            if(patternPixelValue != 0) // NOTE: If Value is zero, then it is a background/transparent
            {
                u8 atrbPixelValue = (xOffset >= 8) ? ppu->paletteLatchOld : ppu->paletteLatchNew;
                backgroundResult = atrbPixelValue | patternPixelValue;
            }
                            
            u8 bgrdPaletteIndex = ReadPpu8(backgroundPaletteAddress + backgroundResult, ppu);
            pixelColour = Palette[bgrdPaletteIndex];
        }
                
        /* ******************* */
        /* Sprite Calculations */
                        
        if(ppu->showSprites && !(pixelX < 8 && !ppu->showSPRTLeft8Pixels) )
        {
            for(s16 spriteIdx = ppu->preparedSpriteCount - 1; spriteIdx >= 0; --spriteIdx)
            {
                Sprite *sprite = ppu->preparedSprites + spriteIdx;
                u8 spriteX = sprite->oamData.X;
                            
                if(pixelX != 0xFF && spriteX <= pixelX && pixelX < (spriteX + pixelsPerTile)) 
                {
                    u8 relX = ((pixelX - (spriteX)) % 8);                                    
                    u8 patternValue = (((sprite->patternHigh >> (7 - relX)) & 1) << 1) |
                        ((sprite->patternLow >> (7 - relX)) & 1);
                                    
                    if(patternValue != 0) // If bottom two bits is 0, then is multiple of 4.
                    {
                        u8 spriteColour = (sprite->paletteValue << 2) | patternValue;

                        if(sprite->spriteZero && !ppu->spriteZeroHit &&
                           ppu->showBackground && backgroundResult != 0 &&
                           ppu->scanline <= 239 /*&& Sprite->OamData.Y != 255*/ && ppu->scanlineCycle != 256)
                        {
                            ppu->spriteZeroHit = true;
                        }

                        if( sprite->priority || backgroundResult == 0 )
                        {
                            u8 sprtPaletteIndex = ReadPpu8(spritePaletteAddress + spriteColour, ppu);                       
                            pixelColour = Palette[sprtPaletteIndex];                        
                        }
                    }
                }
            }
        }
        
        // PIXEL OUTPUT - using the resulting colour
        DrawPixel(ppu, pixelX, pixelY, pixelColour);                        
    }

}

static void VblankLine(Ppu *ppu)
{
    u16 scanline = ppu->scanline;
    u16 cycle = ppu->scanlineCycle;

    if(scanline == 241 && cycle == 1)
    {
        // TODO TODO Check over
        if(!ppu->supressVbl)
        {
            ppu->verticalBlank = true;
        }

        nmiFlag = ppu->generateNMI && ppu->verticalBlank && !ppu->supressNmiSet;

        if(ppu->supressNmiSet)
        {
            ppu->supressNmiSet = false;
        }
        
        globalDrawScreen = true;
    }
}

static void PreRenderLine(Ppu *ppu)
{
    u16 cycle = ppu->scanlineCycle;

    if(cycle == 1)
    {
        ppu->spriteOverflow = false;
        ppu->spriteZeroHit = false;
        ppu->verticalBlank = false;
        SetNmi(false);
    }

    if(ppu->renderingEnabled)
    {

        if(cycle == 64) // NOTE: Clearing takes 64 cycles. Running on very last one
        {
            // Secondary Oam Clear
            u8 *data = (u8 *)ppu->secondaryOam;
    
            for(u16 byte = 0; byte < (secondaryOamSpriteMax * sizeof(Sprite)); ++byte)
            {
                data[byte] = 0xFF;
            }
        }
        else if(cycle == 257)
        {
            // Clear prepared sprites
            u8 *data = (u8 *)ppu->preparedSprites;
    
            for(u16 byte = 0; byte < (secondaryOamSpriteMax * sizeof(Sprite)); ++byte)
            {
                data[byte] = 0xFF;
            }
        }
        
        LoadFutureData(ppu);

        if(280 <= cycle && cycle <= 304)
        {
            ResetScrollVert(ppu);
        }
    }
}

static void PostRenderLine(Ppu *ppu)
{
    // Does nothing
}

static void RunPpu(Ppu *ppu, u16 clocksToRun)
{
    while(clocksToRun > 0)
    {
		ppu->clocksHit++;

        --clocksToRun;
        
        switch(ppu->scanlineType)
        {
            case VISIBLE:
            {
                VisibleLine(ppu);
                break;
            }
            case POST_RENDER:
            {
                PostRenderLine(ppu);
                break;
            }
            case VBLANK:
            {
                VblankLine(ppu);
                break;
            }
            case PRE_RENDER:
            {
                PreRenderLine(ppu);
                break;
            }
            default:
            {
                Assert(0);
            }
        }
    
        // Once finished, update the cycle and scanline if on new one
        ++ppu->scanlineCycle;

        Assert(ppu->scanlineCycle <= 341);

        // New Scanline
        if(ppu->scanlineCycle == 341)
        {
            ppu->scanlineCycle = 0;
            ++ppu->scanline;

            Assert(ppu->scanline <= 262);
            
            // If reached new Frame
            if(ppu->scanline == 262)
            {
                ppu->scanline = 0;
                ppu->oddFrame = !ppu->oddFrame;

                ppu->scanlineType = VISIBLE;

                // On oddframe, when rendering is enabled, the first cycle of frame is skipped
                if(ppu->oddFrame && ppu->renderingEnabled)
                {
                    ++ppu->scanlineCycle;
                }
            }
            else if(ppu->scanline == 261)
            {
                ppu->scanlineType = PRE_RENDER;
            }
            else if(241 <= ppu->scanline)
            {
                ppu->scanlineType = VBLANK;
            }            
            else if(ppu->scanline == 240)
            {
                ppu->scanlineType = POST_RENDER;
            }
        }
    }
}


static void InitPpu(Ppu *ppu, u8 * memoryBase, u32 * basePixel)
{
    ZeroMemory(memoryBase, Kilobytes(64));
    
    *ppu = {};

    ppu->memoryBase = memoryBase;
    ppu->basePixel = basePixel;

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
        Write8(paletteStartup[idx], ppu->memoryBase + address);
    }
}
