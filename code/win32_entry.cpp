/*
  Entry Point for window platform
*/

#include "nes.h"
#include <stdio.h>

#include "raylib.h"

r64 lowCpuClocks = 21470;
r64 highCpuClocks = 21470;
u64 lowPpuClocks = 89341;
u64 highPpuClocks = 89341;


struct ScreenBuffer
{
	// NOTE: Memory Order BB GG RR XX
	//BITMAPINFO info;
	void *memory;
	s16 width;
	s16 height;
	s16 pitch;
	s16 bytesPerPixel;
};


// TODO: Investigate if all these need to be global
global ScreenBuffer globalScreenBackBuffer;

global Input globalInput = {};

global bool globalRunning;
global bool globalDrawScreen = false;

global s64 globalPerfCountFrequency;

global const u16 romNameMaxSize = 256;
global u8 romFileName[romNameMaxSize];

#include "nes.cpp"

// NOTE: Entry Point

/*int CALLBACK
WinMain(HINSTANCE WindowInstance, HINSTANCE PrevWindowInstance,
		LPSTR CommandLine, int CommandShow)
{
*/
int main()
{
	r32 frameHz = 60.0988f; // aka fps. // TODO: Will be different for PAL
	r32 frameTargetSeconds = 1.0f / frameHz;

	
	/**************************************/
	/* NOTE : Screen back buffer creation */

	u16 renderScaleWidth = 256, renderScaleHeight = 240;
	u8 resScale = 2;
	u16 windowWidth = renderScaleWidth * resScale, windowHeight = renderScaleHeight * resScale;
	globalScreenBackBuffer = {};
	//CreateBackBuffer(&globalScreenBackBuffer, renderScaleWidth, renderScaleHeight);

	/**************************/
	/* NOTE : Window creation */

	//WNDCLASSA windowClass = {};
	//windowClass.style = CS_HREDRAW | CS_VREDRAW;
	//windowClass.lpfnWndProc = WinInputCallback;
	//windowClass.hInstance = GetModuleHandle(NULL);//WindowInstance;
	//windowClass.lpszClassName = "tuNES";

	//u16 initialWindowPosX = 700;
	//u16 initialWindowPosY = 400;

	//if(!RegisterClassA(&windowClass))
	//{
	//	// NOTE: Failed to register window
	//	// TODO: Handle this in a better way
	//	Assert(0);
	//	return(-1);
	//}

	//HWND window = CreateWindowExA(0, windowClass.lpszClassName, "tuNES", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
	//	initialWindowPosX, initialWindowPosY, windowWidth, windowHeight,
	//	0, 0, windowClass.hInstance, 0);

	//if(!window) // If window was created successfully
	//{
	//	// NOTE: Window failed to create
	//	// TODO: Handle this in a better way
	//	Assert(0);
	//	return(-1);
	//}

	/********************************/
	/* NOTE : Window Menu Creation  */

	//HMENU windowMenu = CreateMenu();
	//HMENU subMenu = CreatePopupMenu();

	//AppendMenu(subMenu, MF_STRING, ID_OPEN_ROM_ITEM, "&Open Rom");
	//AppendMenu(subMenu, MF_STRING, ID_CLOSE_ROM_ITEM, "&Close Rom");
	//AppendMenu(subMenu, MF_STRING, ID_QUIT_ITEM, "&Quit");
	//AppendMenu(windowMenu, MF_STRING | MF_POPUP, (u64)subMenu, "&File");

	//SetMenu(window, windowMenu);

	/********************************/
	/* NOTE : Sound Buffer Creation */

	//WinSound winSound = {};
	//winSound.samplesPerSecond = 48000;
	//winSound.channels = 2; // Sterio
	//winSound.bitsPerSample = 16; // Bit depth
	//winSound.bytesPerSample = sizeof(s16) * winSound.channels;
	//winSound.bufferSize = winSound.samplesPerSecond * winSound.bytesPerSample;
	//winSound.safetyBytes = (int)(((r32)winSound.samplesPerSecond * (r32)winSound.bytesPerSample
	//	/ frameHz) / 3.0f);

	//winSound.samples = (s16 *)VirtualAlloc(0, (size_t)winSound.bufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	//LPDIRECTSOUNDBUFFER soundBuffer = CreateSoundBuffer(window, winSound.bufferSize, winSound.channels,
	//	winSound.samplesPerSecond, winSound.bitsPerSample);

	//ClearSoundBuffer(soundBuffer, winSound.bufferSize);
	//soundBuffer->Play(0, 0, DSBPLAY_LOOPING);

	/********************************************************/
	/* NOTE : Timing */
	// Using the vertical scan rate of ~60Hz or 16ms a frame.
	// We run cpu and ppu 16ms worth of ticks. After 16ms
	// elapsed, we then reset the counters that tracks how
	// ticks have been. Repeat

	u8 *initalRom = (u8 *)"../roms/Zelda.nes";

	InitCpu(&globalNes.cpu);
	InitPpu(&globalNes.ppu);
	InitApu(&globalNes.apu);

	// TODO: Check ref?
	LoadCartridge(&globalNes, initalRom);

	// NOTE: Load the program counter with the reset vector
	globalNes.cpu.prgCounter = ReadCpu16(RESET_VEC, &globalNes.cpu);

	globalNes.cpuHz = 1789772.727272728f;

	globalNes.frameClockTotal = globalNes.cpuHz * frameTargetSeconds; // TODO: Put in create nes?
	////

	r32 elapsedTime = 0.0;
	u32 clocksRun = 0;
	r32 frameTime = 0.0;

	u64 lastTime = GetTime();
	u64 frameFlippedTime = GetTime();
	u64 frameLastFlippedTime = GetTime();

	/********************/
	/* NOTE : Main Loop */

	globalRunning = true;
	while(globalRunning)
	{
		/*MSG msg = {};
		while(PeekMessage(&msg, window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}*/

		if(globalNes.frameClocksElapsed < globalNes.frameClockTotal)
		{
			RunNes(&globalNes, &globalInput);
		}
		else
		{
			//Sleep(2);
		}

		/*
		LARGE_INTEGER FrameSoundClock = getClock();
		r32 FrameTimeElapsed = getSecondsElapsed(FrameFlippedClock, FrameSoundClock);
		UpdateAudio(SoundBuffer, &WinSound, FrameHz, FrameTargetSeconds, FrameTimeElapsed);
		*/

		// NOTE: TODO: The cpu might run several more cycles before we get to drawn screen.
		// This could lead to pixels from the next frame overwriting other pixels we haven't displayed
		// To fix, I could implement a double buffer. Two buffers, one is drawn on buy the ppu while the other is the displayed.
		// They are then swapped. This will let the complete frame to be displayed while the new one can created.

		//if(globalDrawScreen)
		//{
		//	globalDrawScreen = false;

		//	GetWindowSize(window, &windowWidth, &windowHeight);

		//	// NOTE: Drawing the backbuffer to the window 
		//	HDC deviceContext = GetDC(window);
		//	DrawScreenBuffer(&globalScreenBackBuffer, deviceContext,
		//		windowWidth, windowHeight);
		//	ReleaseDC(window, deviceContext);

		//	frameFlippedClock = GetClock();

		//	frameTime = GetSecondsElapsed(frameLastFlippedClock, frameFlippedClock);
		//	frameLastFlippedClock = GetClock();
		//}

		r64 endTime = GetTime();

		r64 loopTime = endTime - lastTime;
		elapsedTime += loopTime;

		if(elapsedTime >= frameTargetSeconds)
		{
			// Get how many cpu and ppu clocks have happened since
			//printf("cpu clocks %f, ppu clocks %ld\n", nes.frameClocksElapsed, (long)nes.ppu.clocksHit);

			if(globalNes.frameClocksElapsed > 20.0f)
			{
				if(globalNes.frameClocksElapsed < lowCpuClocks)
				{
					lowCpuClocks = globalNes.frameClocksElapsed;
				}
				if(globalNes.frameClocksElapsed > highCpuClocks)
				{
					highCpuClocks = globalNes.frameClocksElapsed;
				}
			}

			if(globalNes.ppu.clocksHit > 20)
			{
				if(globalNes.ppu.clocksHit < lowPpuClocks)
				{
					lowPpuClocks = globalNes.ppu.clocksHit;
				}
				if(globalNes.ppu.clocksHit > highPpuClocks)
				{
					highPpuClocks = globalNes.ppu.clocksHit;
				}
			}

			elapsedTime -= frameTargetSeconds;

			// The extra clocks we need to add to the next frame. Is 0 or more
			if(globalNes.frameClocksElapsed >= globalNes.frameClockTotal)
			{
				globalNes.frameClocksElapsed = globalNes.frameClocksElapsed - globalNes.frameClockTotal;
			}
			else
			{
				globalNes.frameClocksElapsed = 0;
			}



			globalNes.ppu.clocksHit = 0;
		}

		lastTime = endTime;
	}

	return(0);
}
