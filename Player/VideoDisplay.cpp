#include "VideoDisplay.h"
extern "C"
{
#include<libavutil/time.h>
}


void VideoDisplay::renderer(AVFrame* resFrame)
{
  SDL_UpdateYUVTexture(texture, nullptr,
    resFrame->data[0], resFrame->linesize[0],
    resFrame->data[1], resFrame->linesize[1],
    resFrame->data[2], resFrame->linesize[2]);
  SDL_RenderClear(render);
  SDL_RenderCopy(render, texture, nullptr, nullptr);
  SDL_RenderPresent(render);
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