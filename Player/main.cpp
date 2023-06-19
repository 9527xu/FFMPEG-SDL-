#include"Media.h"
#include "VideoDisplay.h"
#define URL "01.mp4"
#define URL3 "sweet.mp4"
#define URL2 "C:\\Users\\xu\\Desktop\\VideoPlayer\\VideoPlayer\\1.mp3"
#define YUV "out.yuv"
#define NO_MUSIC_URL "out.mp4"
int main(int argv, char* argc[])
{
	printf("ffmpeg version:%s\n", av_version_info());
	//const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	Media media(URL3);
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