
#include "nes.cpp"
#include <cstdio> // for printf
#include <stdlib.h>


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

// Cycles per second (hz)
float frequency = 440.0f;

// Audio frequency, for smoothing
bool updateAudioFrequency = false;
float audioFrequency = 440.0f;

// Index for audio rendering
float sineIdx = 0.0f;

// Audio input processing callback
void AudioInputCallback(void *buffer, unsigned int frames)
{
	audioFrequency = frequency + (audioFrequency - frequency) * 0.95f;
	audioFrequency += 1.0f;
	audioFrequency -= 1.0f;
	float incr = audioFrequency / 44100.0f;
	short *d = (short *)buffer;

	for(int i = 0; i < frames; i++)
	{
		d[i] = (short)(32000.0f * sinf(2 * PI * sineIdx));
		sineIdx += incr;
		if(sineIdx > 1.0f) sineIdx -= 1.0f;
	}
}

int main()
{
	const char *workDir = GetWorkingDirectory();
	TraceLog(LOG_INFO, "workDir = %s", workDir);

	InitWindow(gWindowWidth, gWindowHeight, "tuNES");

	//////

	int waveLength = 1;
	updateAudioFrequency = false;

	InitAudioDevice();

	constexpr u32 samplesPerFrame = 4096;
	SetAudioStreamBufferSizeDefault(samplesPerFrame);
	
	// Init raw audio stream (sample rate: 44100, sample size: 16bit-short, channels: 1-mono)
	AudioStream stream = LoadAudioStream(44100, 16, 1);
	
	SetAudioStreamCallback(stream, AudioInputCallback);

	PlayAudioStream(stream);        // Start processing stream buffer (no data loaded currently)

	//////

	u8 targetFPS = 60;
	SetTargetFPS(targetFPS);

	InitialiseBackBuffers(gNesWidth, gNesHeight);

	Vector2 originVec = {0,0};
	Rectangle nesDrawRect = {0,0, gNesWidth, gNesHeight};
	Rectangle windowDrawRect = {0,0, gWindowWidth, gWindowHeight};

	Nes::GetInstance().Init();

	float lastTime = GetTime();

	u32 framesPerSec = 0;
	float perSecondCountDown = 1.0f;

	while(!WindowShouldClose())
	{ 
		if(IsKeyPressed(KEY_F1))
		{
			gameState = (gameState == GameState::NES_GAME) ? GameState::NES_MENU : GameState::NES_GAME;
		}

		if(gameState == GameState::NES_GAME)
		{
			Nes::GetInstance().Update();
		}


		Color *pixelBuffer = Nes::GetPpu()->GetPixelBuffer();
		
		Texture2D drawTexture = backBuffers[currentDrawBackBufferIndex];
		currentDrawBackBufferIndex = (currentDrawBackBufferIndex + 1) % backBufferCount;
		UpdateTexture(drawTexture, pixelBuffer);

		BeginDrawing();
		ClearBackground(BLACK);

		Color colorTint = (gameState == GameState::NES_MENU) ? GRAY : WHITE;
		DrawTexturePro(drawTexture, nesDrawRect, windowDrawRect, originVec, 0, colorTint);

		if(gameState == GameState::NES_MENU)
		{
			MenuUpdate();
		}

		EndDrawing();

#if 0 // FPS Log
		{
			framesPerSec++;

			float currentTime = GetTime();
			float deltaTime = currentTime - lastTime;
			lastTime = currentTime;

			perSecondCountDown -= deltaTime;
			if(perSecondCountDown <= 0.0f)
			{
				TraceLog(LOG_INFO, "FPS - %u", framesPerSec);
				framesPerSec = 0;
				perSecondCountDown = 1.0f;
			}
		}
#endif
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

void MenuUpdate()
{
	float buttonPosX = buttonStartX;
	float buttonPosY = buttonStartY;

	Rectangle fileDialogRect = {buttonPosX, buttonPosY, buttonWidth, buttonHeight};
	if(GuiButton(fileDialogRect, "Load Rom"))
	{
		char loadFilename[1024] = {};
		bool success = GuiFileDialog(DIALOG_OPEN_FILE, "Load ROM", loadFilename, "*.nes", "ROM Files (*.nes)");
		if(success)
		{
			Nes::QueueRomLoad(loadFilename);
			gameState = GameState::NES_GAME;
		}
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