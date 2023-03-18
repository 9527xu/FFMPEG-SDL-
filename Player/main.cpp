#include"Media.h"
#include "VideoDisplay.h"
#define URL "01.mp4"
#define URL3 "sweet.mp4"
#define URL2 "C:\\Users\\xu\\Desktop\\VideoPlayer\\VideoPlayer\\1.mp3"
int main(int argv, char* argc[])
{
	Media media(URL);
	media.play();
	SDL_Event event;
	while (true) // SDL event loop
	{
		SDL_WaitEvent(&event);
		switch (event.type)
		{
		case FF_QUIT_EVENT:
		
			return 0;
			break;

		case FF_REFRESH_EVENT:
			video_refresh_timer(&media);
			break;

		default:
			break;
		}
	}
	getchar();
	return 0;
}