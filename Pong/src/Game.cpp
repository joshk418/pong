#include "Game.h"

#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>

#include <direct.h>


const int thickness = 15;
const float paddleH = 100.0f;

Game::Game() : 
	mGameWindow(nullptr), 
	mRenderer(nullptr), 
	mTicksCount(0), 
	mPlayer1PaddleDir(0), 
	mPlayer2PaddleDir(0),
	screenWidth(1024),
	screenHeight(768),
	mPlayer1Score(0),
	mPlayer2Score(0),
	mGameFont(nullptr)
{
	mGameState = GameState::PLAYING;
}

Game::~Game()
{

}

bool Game::Init()
{
	int sdlResult = SDL_Init(SDL_INIT_VIDEO);
	if (sdlResult != 0)
	{
		SDL_Log("SDL_Init: %s", SDL_GetError());
		return false;
	}

	SDL_Log("SDL initialized");

	mGameWindow = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, 0);
	if (!mGameWindow)
	{
		SDL_Log("SDL_CreateWindow: %s", SDL_GetError());
		return false;
	}

	SDL_Log("Window created");

	mRenderer = SDL_CreateRenderer(mGameWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!mRenderer)
	{
		SDL_Log("SDL_CreateRenderer: %s", SDL_GetError());
		return false;
	}

	SDL_Log("Renderer created");

	if (TTF_Init() != 0)
	{
		SDL_Log("TTF_Init: %s", TTF_GetError());
		return false;
	}

	mGameFont = TTF_OpenFont("fonts/OpenSans-Regular.ttf", 24);
	if (!mGameFont)
	{
		SDL_Log("TTF_OpenFont: %s", TTF_GetError());
		return false;
	}

	SDL_Log("TTF initialized and font opened");

	ResetBallAndPaddles();

	return true;
}

void Game::GameLoop()
{
	while (mGameState != GameState::EXIT)
	{
		ProcessInput();
		Update();
		Render();
	}
}

void Game::ProcessInput()
{
	SDL_Event evt;
	while (SDL_PollEvent(&evt))
	{
		switch (evt.type)
		{
		case SDL_QUIT: 
			mGameState = GameState::EXIT;
			break;
		}
	}

	const Uint8* state = SDL_GetKeyboardState(nullptr);

	mPlayer1PaddleDir = 0;
	if (state[SDL_SCANCODE_W])
	{
		mPlayer1PaddleDir -= 1;
	}
	if (state[SDL_SCANCODE_S])
	{
		mPlayer1PaddleDir += 1;
	}

	mPlayer2PaddleDir = 0;
	if (state[SDL_SCANCODE_I])
	{
		mPlayer2PaddleDir -= 1;
	}
	if (state[SDL_SCANCODE_K])
	{
		mPlayer2PaddleDir += 1;
	}
}

void Game::Update()
{
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16));

	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.f;

	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	mTicksCount = SDL_GetTicks();

	if (mGameState == GameState::PAUSED)
	{
		mPausedTime += 1.f * deltaTime;

		if (mPausedTime >= 3.f)
		{
			mGameState = GameState::PLAYING;
		}
		else 
		{
			return;
		}
	}

	if (mGameState == GameState::PLAYER_WON)
	{
		mWinCountdown += 1.f * deltaTime;

		if (mWinCountdown >= 5.f)
		{
			mGameState = GameState::EXIT;
		}

		return;
	}
	

	if (mPlayer1PaddleDir != 0)
	{
		mPlayer1PaddlePos.y += mPlayer1PaddleDir * 300.f * deltaTime;

		if (mPlayer1PaddlePos.y < (paddleH / 2.f + thickness))
		{
			mPlayer1PaddlePos.y = paddleH / 2.f + thickness;
		}
		else if (mPlayer1PaddlePos.y > (screenHeight - paddleH / 2.f - thickness))
		{
			mPlayer1PaddlePos.y = screenHeight - paddleH / 2.f - thickness;
		}
	}

	if (mPlayer2PaddleDir != 0)
	{
		mPlayer2PaddlePos.y += mPlayer2PaddleDir * 300.f * deltaTime;

		if (mPlayer2PaddlePos.y < (paddleH / 2.f + thickness))
		{
			mPlayer2PaddlePos.y = paddleH / 2.f + thickness;
		}
		else if (mPlayer2PaddlePos.y > (screenHeight - paddleH / 2.f - thickness))
		{
			mPlayer2PaddlePos.y = screenHeight - paddleH / 2.f - thickness;
		}
	}

	mBallPos.x += mBallVelocity.x * deltaTime;
	mBallPos.y += mBallVelocity.y * deltaTime;

	// ball diff to paddle 1
	float diff1 = mPlayer1PaddlePos.y - mBallPos.y;
	diff1 = (diff1 > 0.f) ? diff1 : -diff1;

	// ball diff to paddle 2
	float diff2 = mPlayer2PaddlePos.y - mBallPos.y;
	diff2 = (diff2 > 0.f) ? diff2 : -diff2;

	if (diff1 <= paddleH / 2.0f && mBallPos.x <= 25.0f && mBallPos.x >= 20.0f && mBallVelocity.x < 0.0f)
	{
		mBallVelocity.x *= -1.0f;
	}
	else if (diff2 <= paddleH / 2.f && mBallPos.x >= screenWidth - (thickness + 20.f) && mBallPos.x <= screenWidth - (thickness + 10.f) && mBallVelocity.x > 0.f)
	{
		mBallVelocity.x *= -1.0f;
	}
	else if (mBallPos.x <= 0.f)
	{
		mPlayer2Score++;
		SDL_Log("Player 2 scored: %d", mPlayer2Score);
		ResetBallAndPaddles();

		mPausedTime = 0.f;
		mGameState = GameState::PAUSED;
	}
	else if (mBallPos.x >= screenWidth)
	{
		mPlayer1Score++;
		SDL_Log("Player 1 scored: %d", mPlayer1Score);
		ResetBallAndPaddles();
		
		mPausedTime = 0.f;
		mGameState = GameState::PAUSED;
	}

	if (mBallPos.y <= thickness && mBallPos.y < 0.f)
	{
		mBallVelocity.y *= -1;
	}
	else if (mBallPos.y >= (screenHeight - thickness) && mBallVelocity.y > 0.f)
	{
		mBallVelocity.y *= -1;
	}

	if (mPlayer1Score >= 1 || mPlayer2Score >= 1)
	{
		mWinCountdown = 0;
		mGameState = GameState::PLAYER_WON;
	}
}

void Game::Render()
{
	SDL_SetRenderDrawColor(mRenderer, 0, 0, 255, 255);

	SDL_RenderClear(mRenderer);

	SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);

	SDL_Rect paddle1{ static_cast<int>(mPlayer1PaddlePos.x), static_cast<int>(mPlayer1PaddlePos.y - paddleH / 2), thickness, static_cast<int>(paddleH) };
	SDL_RenderFillRect(mRenderer, &paddle1);

	SDL_Rect paddle2{ static_cast<int>(mPlayer2PaddlePos.x), static_cast<int>(mPlayer2PaddlePos.y - paddleH / 2), thickness, static_cast<int>(paddleH) };
	SDL_RenderFillRect(mRenderer, &paddle2);

	if (mGameState == GameState::PLAYER_WON)
	{
		ShowWinDisplay();
	}
	else 
	{
		// ball will block text otherwise
		SDL_Rect ball{ static_cast<int>(mBallPos.x - thickness / 2), static_cast<int>(mBallPos.y - thickness / 2), thickness, thickness };
		SDL_RenderFillRect(mRenderer, &ball);
	}

	DisplayText();

	SDL_RenderPresent(mRenderer);
}

void Game::DisplayText()
{
	SDL_Surface* score1TextSurface = TTF_RenderText_Solid(mGameFont, std::to_string(mPlayer1Score).c_str(), SDL_Color{255, 255, 255, 255});
	SDL_Surface* score2TextSurface = TTF_RenderText_Solid(mGameFont, std::to_string(mPlayer2Score).c_str(), SDL_Color{ 255, 255, 255, 255 });

	mScore1Texture = SDL_CreateTextureFromSurface(mRenderer, score1TextSurface);
	mScore2Texture = SDL_CreateTextureFromSurface(mRenderer, score2TextSurface);

	mScore1Rect.x = screenWidth / 2.f - 150.f;
	mScore1Rect.y = 15.f;
	mScore1Rect.w = score1TextSurface->w;
	mScore1Rect.h = score1TextSurface->h;

	mScore2Rect.x = screenWidth / 2.f + 125.f;
	mScore2Rect.y = 15.f;
	mScore2Rect.w = score2TextSurface->w;
	mScore2Rect.h = score2TextSurface->h;

	SDL_FreeSurface(score1TextSurface);
	SDL_FreeSurface(score2TextSurface);
	SDL_RenderCopy(mRenderer, mScore1Texture, nullptr, &mScore1Rect);
	SDL_RenderCopy(mRenderer, mScore2Texture, nullptr, &mScore2Rect);
}

void Game::ShowWinDisplay()
{
	std::string winText = mPlayer1Score > mPlayer2Score ? "Player 1 wins!" : "Player 2 wins!";
	SDL_Surface* winTextSurface = TTF_RenderText_Solid(mGameFont, winText.c_str(), SDL_Color{ 255, 255, 255, 255 });

	mWinTexture = SDL_CreateTextureFromSurface(mRenderer, winTextSurface);

	mWinTextRect.x = (screenWidth / 2.f) - (winTextSurface->w / 2.f);
	mWinTextRect.y = (screenHeight / 2.f) - (winTextSurface->h / 2.f);
	mWinTextRect.w = winTextSurface->w;
	mWinTextRect.h = winTextSurface->h;
	
	SDL_FreeSurface(winTextSurface);
	SDL_RenderCopy(mRenderer, mWinTexture, nullptr, &mWinTextRect);
}

void Game::Shutdown()
{
	SDL_DestroyRenderer(mRenderer);
	SDL_DestroyWindow(mGameWindow);

	SDL_Quit();
}

void Game::ResetBallAndPaddles()
{
	mPlayer1PaddlePos.x = 10.0f;
	mPlayer1PaddlePos.y = 768.0f / 2.0f;

	mPlayer2PaddlePos.x = screenWidth - (thickness + 10.f);
	mPlayer2PaddlePos.y = 768.0f / 2.0f;

	mBallPos.x = 1024.0f / 2.0f;
	mBallPos.y = 768.0f / 2.0f;
	mBallVelocity.x = -200.0f * 1.3f;
	mBallVelocity.y = 235.0f * 1.3f;
}