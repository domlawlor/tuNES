#if !defined(PPU_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Dom Lawlor $
   ======================================================================== */

const u16 ppuPixelWidth = 256;
const u16 ppuPixelHeight = 240;

const u16 pixelsPerTile = 8;

const u16 backgroundPaletteAddress = 0x3F00;
const u16 spritePaletteAddress = 0x3F10;

const u16 secondaryOamSpriteMax = 8;

const u16 oamSize = 0x100;
const u16 oamSpriteTotal = 64;

enum NameTableMirrorType
{
    SINGLE_SCREEN_BANK_A = 0,
    SINGLE_SCREEN_BANK_B,
    VERTICAL_MIRROR,
    HORIZONTAL_MIRROR,
    FOUR_SCREEN_MIRROR,   
};

struct OamSprite
{
    u8 Y;
    u8 Tile;
    u8 Atrb;
    u8 X;
};

struct Sprite
{
	OamSprite oamData;

    b32 priority;
    b32 spriteZero;
    
    u8 paletteValue;
    u8 patternLow;
    u8 patternHigh;
};

enum ScanlineType
{
    VISIBLE = 0,
    POST_RENDER,
    VBLANK,
    PRE_RENDER
};

struct Ppu
{
    u64 memoryBase;
    u32 *basePixel;

    b32 renderingEnabled;

    b32 oddFrame;
    
    u16 scanline;
    u16 scanlineCycle;

    ScanlineType scanlineType;
    
    // VRAM Address
    u16 vRamAdrs;
    u16 tempVRamAdrs;
    u8 latchWrite;
    u8 fineX;

    // 
    u16 lowPatternShiftReg;
    u16 highPatternShiftReg;
    u8 paletteLatchOld;
    u8 paletteLatchNew;

    u8 nextLowPattern;
    u8 nextHighPattern;
    u8 nextAtrbByte;
    u16 nextNametableAdrs;

    // Name table banks. 
    NameTableMirrorType mirrorType;
    // TODO: Pre allocate?
    u8 nametableBankA[0x400];
    u8 nametableBankB[0x400];
    u8 nametableBankC[0x400];
    u8 nametableBankD[0x400];
    
    // Control Reg
    u8 nametableBase;
    u8 vRamIncrement;
    u16 sPRTPattenBase;
    u16 bGPatternBase;
    b32 spriteSize8x16;
    b32 ppuSlave;
    b32 generateNMI;

    // Mask Reg
    b32 greyScale;
    b32 showBGLeft8Pixels;
    b32 showSPRTLeft8Pixels;
    b32 showBackground;
    b32 showSprites;
    b32 emphasizeRed;
    b32 emphasizeGreen;
    b32 emphasizeBlue;

    // Status Reg
    b32 spriteOverflow;
    b32 spriteZeroHit;
    b32 verticalBlank;

    b32 supressVbl;
    b32 supressNmiSet;
    
    // Oam Address Reg
    u8 oamAddress;

    // VRam Data Read Buffering
    u8 vRamDataBuffer;
    
    //Sprites
    u8 *oamDma; 
    u8 oam[oamSize];

    u8 secondarySpriteCount;
    Sprite secondaryOam[secondaryOamSpriteMax];
    
    u8 preparedSpriteCount;
    Sprite preparedSprites[secondaryOamSpriteMax];
};


#define PPU_H
#endif
