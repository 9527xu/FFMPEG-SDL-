#pragma once

#include <iostream>

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
using namespace std;
#include"SharedQueue.h"
class Audio
{
  
private:
	AVCodecContext* audio_codec_ctx=nullptr;
	int audio_index=-1;
	
  AVFrame wanted_frame;
  double audio_clock=0; //audio_clock=pts+每秒音频播放的字节数

  //保留只要是用于
  AVStream* audio_stream=nullptr;


public:
	SharedQueue<AVPacket*>packet_que;
  uint8_t audio_buff[192000 * 3 / 2];
  unsigned int audio_buf_size = 0;
  unsigned int audio_buf_index = 0;
    
    int getAudioindex() const;
    void setAudioindex(int audioindex);

    AVCodecContext* getAudiocodecctx() const;
    void setAudiocodecctx(AVCodecContext* audiocodecctx);

    Audio();
    ~Audio();
    void play();
    //解码数据，并进行转化放入audio_buf装数据的容器，buf_size容器容量
    int audio_decode_packet(uint8_t* audio_buf, int buf_size);

    AVStream* getAudiostream() const;
    void setAudiostream(AVStream* audiostream);

    // 返回当前音频的播放时刻
    double get_play_clock();


};

//音频数据包的回调函数
void audio_callback(void* userdata, Uint8* stream, int len);
