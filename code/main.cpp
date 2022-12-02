
#include "nes.cpp"
#include <cstdio> // for printf
#include <stdlib.h>

constexpr u8 backBufferCount = 2;
Texture2D backBuffers[backBufferCount] = {};
u8 currentDrawBackBufferIndex = 0;

void InitialiseBackBuffers(u32 bufferWidth, u32 bufferHeight)
{
	// Throw away image for generating the texture2d's
	Image image = {};
	image.data = MemAlloc(bufferWidth * bufferHeight * sizeof(Color));
	image.width = bufferWidth;
	image.height = bufferHeight;
	image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
	image.mipmaps = 1;

	for(int i = 0; i < backBufferCount; ++i)
	{
		backBuffers[i] = LoadTextureFromImage(image);
	}
}

int main()
{
	InitWindow(gWindowWidth, gWindowHeight, "tuNES");

	u8 targetFPS = 60;
	SetTargetFPS(targetFPS);

	InitialiseBackBuffers(gNesWidth, gNesHeight);

	Nes::GetInstance().Init();

	while(!WindowShouldClose())
	{ 
		Nes::GetInstance().Update();

		Texture2D drawTexture = backBuffers[currentDrawBackBufferIndex];
		currentDrawBackBufferIndex = (currentDrawBackBufferIndex + 1) % backBufferCount;
		UpdateTexture(drawTexture, Nes::GetPpu()->GetPixelBuffer());

		BeginDrawing();
		ClearBackground(GREEN);
		DrawTexture(drawTexture, 0, 0, WHITE);
		EndDrawing();
	}

	return 0;
}

