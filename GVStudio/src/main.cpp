#include "GVStudio/GVStudio.h"
#include "SDL3/SDL_init.h"
#include <iostream>
int main(int argc, char** argv)
{
	GV_STUDIO gv_studio;
	return gv_studio.RUN();

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
		std::cin.get();
		return -1;


	}

	std::cout << "SDL_Init successsssss" << std::endl;

	SDL_Quit();
	std::cin.get();
	return 0;

}