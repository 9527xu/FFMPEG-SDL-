#include"Media.h"
#include "VideoDisplay.h"
#define URL "01.mp4"
#define URL3 "sweet.mp4"
#define URL2 "C:\\Users\\xu\\Desktop\\VideoPlayer\\VideoPlayer\\1.mp3"
int main(int argv, char* argc[])
{
	Media media(URL3);
	media.play();
	SDL_Event event;
	getchar();
	return 0;
}