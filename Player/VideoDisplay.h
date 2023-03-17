#pragma once
extern "C" {

#include "libavcodec/avcodec.h"
#include"libswscale/swscale.h"
#include "libavformat/avformat.h"
#include"libavformat/avio.h"
#include"libavutil/file.h"
#include"libavutil/imgutils.h"
#include"libavutil/parseutils.h"
#include <libavutil/channel_layout.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>

#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include<SDL.h>

}
#define FF_REFRESH_EVENT (SDL_USEREVENT)
#define FF_QUIT_EVENT (SDL_USEREVENT + 1)
#define WIDTH 864
#define HEIGHT 468

class VideoDisplay
{
public:
  SDL_Window* window = nullptr;
  SDL_Renderer* render = nullptr;
  SDL_Texture* texture = nullptr;

  void display();
  int getWidth()const;
  int getHeight()const;
 


 
  
};
