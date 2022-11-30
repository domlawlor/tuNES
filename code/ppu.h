#pragma once 

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
	u8 tile;
	u8 atrb;
	u8 X;
};

struct Sprite
{
	OamSprite oamData;

	bool priority;
	bool spriteZero;

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

//constexpr u64 PpuMemorySize = Kilobytes(64);
constexpr u64 PpuMemorySize = Kilobytes(16);
constexpr u64 NametableBankSize = Kilobytes(1);

struct Ppu
{
	Color *pixelBuffer;
	u8 memory[PpuMemorySize];

	u8 nametableBankA[NametableBankSize];
	u8 nametableBankB[NametableBankSize];
	u8 nametableBankC[NametableBankSize];
	u8 nametableBankD[NametableBankSize];

	bool hitEndFrame;
//};
//
//struct Ppu
//{
	u64 clocksHit;

	bool renderingEnabled;

	bool oddFrame;

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

	// Control Reg
	u8 nametableBase;
	u8 vRamIncrement;
	u16 sPRTPattenBase;
	u16 bGPatternBase;
	bool spriteSize8x16;
	bool ppuSlave;
	bool generateNMI;

	// Mask Reg
	bool greyScale;
	bool showBGLeft8Pixels;
	bool showSPRTLeft8Pixels;
	bool showBackground;
	bool showSprites;
	bool emphasizeRed;
	bool emphasizeGreen;
	bool emphasizeBlue;

	// Status Reg
	bool spriteOverflow;
	bool spriteZeroHit;
	bool verticalBlank;

	bool supressVbl;
	bool supressNmiSet;

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