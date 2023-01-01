
#include "nes.cpp"
#include <cstdio> // for printf
#include <stdlib.h>

#if !defined(PLATFORM_DESKTOP)
#define CUSTOM_MODAL_DIALOGS 1
#endif
#define GUI_FILE_DIALOGS_IMPLEMENTATION
#include "gui_file_dialogs.h"


#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

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
		Texture2D texture = LoadTextureFromImage(image);
		SetTextureFilter(texture, TEXTURE_FILTER_POINT);
		backBuffers[i] = texture;
	}
}

enum class GameState
{
	NES_MENU = 0,
	NES_GAME,
};
GameState gameState = GameState::NES_MENU;

void MenuUpdate();

int main()
{
	const char *workDir = GetWorkingDirectory();
	TraceLog(LOG_INFO, "workDir = %s", workDir);

	InitWindow(gWindowWidth, gWindowHeight, "tuNES");

	InitialiseBackBuffers(gNesWidth, gNesHeight);

	Vector2 originVec = {0,0};
	Rectangle nesDrawRect = {0,0, gNesWidth, gNesHeight};
	Rectangle windowDrawRect = {0,0, gWindowWidth, gWindowHeight};

	Nes::GetInstance().Init();

	while(!WindowShouldClose())
	{
		if(IsKeyPressed(KEY_TAB))
		{
			gameState = (gameState == GameState::NES_GAME) ? GameState::NES_MENU : GameState::NES_GAME;
		}

		if(IsFileDropped())
		{
			s32 droppedFileCount = 0;
			FilePathList droppedFiles = LoadDroppedFiles();

			TraceLog(LOG_INFO, "-- Dropped Files -- count = %u", droppedFiles.count);
			for(u32 fileNum = 0; fileNum < droppedFiles.count; ++fileNum)
			{
				TraceLog(LOG_INFO, "%u - %s", fileNum, droppedFiles.paths[0]);
			}

			if(IsFileExtension(droppedFiles.paths[0], ".nes"))
			{
				Nes::QueueRomLoad(droppedFiles.paths[0]);
				gameState = GameState::NES_GAME;
			}
			UnloadDroppedFiles(droppedFiles);
		}

		if(gameState == GameState::NES_GAME)
		{
			Nes::GetInstance().Update();
		}

		Texture2D drawTexture = backBuffers[currentDrawBackBufferIndex];
		currentDrawBackBufferIndex = (currentDrawBackBufferIndex + 1) % backBufferCount;

		Color *pixelBuffer = Nes::GetPpu()->GetPixelBuffer();
		UpdateTexture(drawTexture, pixelBuffer);

		BeginDrawing();
		ClearBackground(BLACK);

		Color colorTint = (gameState == GameState::NES_MENU) ? GRAY : WHITE;
		DrawTexturePro(drawTexture, nesDrawRect, windowDrawRect, originVec, 0, colorTint);

		if(gameState == GameState::NES_MENU)
		{
			MenuUpdate();
		}

		//DrawFPS(10, 10);

		EndDrawing();
	}

	Nes::GetInstance().Deinit();

	CloseAudioDevice();
	CloseWindow();

	return 0;
}

constexpr float buttonStartX = 20;
constexpr float buttonStartY = 20;

constexpr float buttonWidth = 120;
constexpr float buttonHeight = 40;
constexpr float buttonSpacing = 10;

static bool showLoadRomUI = false;

void MenuUpdate()
{
	float buttonPosX = buttonStartX;
	float buttonPosY = buttonStartY;

	Rectangle fileDialogRect = {buttonPosX, buttonPosY, buttonWidth, buttonHeight};

	bool loadRomPressed = GuiButton(fileDialogRect, "Load Rom");


	if(loadRomPressed)
	{
		showLoadRomUI = true;
	}

	if(showLoadRomUI)
	{
		char loadFilename[1024] = {};

#if defined(CUSTOM_MODAL_DIALOGS)
		int result = GuiFileDialog(DIALOG_MESSAGE, "Load File", loadFilename, "OK", "Drag and drop a .nes file to load Rom");

		if(result > 0)
		{
			showLoadRomUI = false;
		}
#else
		int result = GuiFileDialog(DIALOG_OPEN_FILE, "Load File", loadFilename, "*.nes;*.nsf", "Files (*.nes, *.nsf)");

		if(result > 0)
		{
			Nes::QueueRomLoad(loadFilename);
			gameState = GameState::NES_GAME;
		}

		showLoadRomUI = false;
#endif	
	}
	buttonPosY += (buttonHeight + buttonSpacing);

	if(Nes::HasRomLoaded())
	{
		Rectangle powerButtonRect = {buttonPosX, buttonPosY, buttonWidth, buttonHeight};
		if(GuiButton(powerButtonRect, "Power"))
		{
			Nes::PowerButton();
			gameState = GameState::NES_GAME;
		}
		buttonPosY += (buttonHeight + buttonSpacing);

		Rectangle resetButtonRect = {buttonPosX, buttonPosY, buttonWidth, buttonHeight};
		if(GuiButton(resetButtonRect, "Reset"))
		{
			Nes::ResetButton();
			gameState = GameState::NES_GAME;
		}
	}
}