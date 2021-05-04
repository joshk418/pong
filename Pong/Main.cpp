#include "src/Game.h"

int main(int argc, char** argv)
{
	Game pong;
	
	if (!pong.Init())
	{
		return 1;
	}

	pong.GameLoop();
	pong.Shutdown();

	return 0;
}