#pragma once

#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"

enum class GameState
{
	PLAYING,
	PAUSED,
	PLAYER_WON,
	EXIT
};

struct Vector2
{
	float x;
	float y;
};

class Game
{
private:
	int screenWidth;
	int screenHeight;

	SDL_Window* mGameWindow;
	SDL_Renderer* mRenderer;

	GameState mGameState;

	int mPlayer1PaddleDir;
	int mPlayer2PaddleDir;

	Vector2 mPlayer1PaddlePos;
	Vector2 mPlayer2PaddlePos;

	Vector2 mBallPos;
	Vector2 mBallVelocity;

	Uint32 mTicksCount;

	int mPlayer1Score;
	int mPlayer2Score;

	float mPausedTime;

	// Text display
	TTF_Font* mGameFont;
	
	SDL_Texture* mScore1Texture;
	SDL_Rect mScore1Rect;

	SDL_Texture* mScore2Texture;
	SDL_Rect mScore2Rect;

	SDL_Texture* mWinTexture;
	SDL_Rect mWinTextRect;

	float mWinCountdown;


	void ProcessInput();
	void Update();
	void Render();

	void DisplayText();

	void ResetBallAndPaddles();

	void ShowWinDisplay();

public:
	Game();
	~Game();

	bool Init();
	void Shutdown();
	void GameLoop();
};

