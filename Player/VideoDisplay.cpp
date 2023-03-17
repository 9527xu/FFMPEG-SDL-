#include "VideoDisplay.h"
extern "C"
{
#include<libavutil/time.h>
}


void VideoDisplay::display()
{
  window = SDL_CreateWindow("Player", 100, 100, WIDTH, HEIGHT,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  render = SDL_CreateRenderer(window, -1, 0);
  texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_TARGET, WIDTH, HEIGHT);

 
}
int VideoDisplay::getHeight() const
{
  return  HEIGHT;
}
int VideoDisplay::getWidth() const
{
  return WIDTH;
}