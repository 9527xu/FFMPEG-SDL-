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
#include"SharedQueue.h"
#include"SharedSet.h"
using namespace std;
struct FrameInfo
{
  double pts;
  AVFrame* frame;
  bool operator<(const FrameInfo &other)const 
  {
    return this->pts < other.pts;
  }
};
class Video
{
private:
	AVCodecContext* video_codec_ctx=nullptr;
	int video_index = -1;
  //保留流
  AVStream* video_stream = nullptr;
  double defalut_delay_time=0;

  AVFrame* yuv420Frame = av_frame_alloc();
  
  uint8_t* outBuffer = nullptr;//数据转换缓冲区
  SwsContext* sws_ctx = nullptr;//转换器

  //解压到帧时的时长，用于给每个frame指定对应的pts
  double video_clock=0;
  thread decode_thr;
  void video_decode_packet();

  //pts+解码所花费的时间
  double synchronize(AVFrame*srcFrame,double pts);

  //配置转换参数
  void config_convert_par();

  int width;
  int height;

public:
    SharedQueue<AVPacket*>packet_que;
    //因为视频的pts与dts不同，而队列是以dts为顺序的不妥，frame用哈希表存,实现以pts为顺序从小到大排序
    ///SharedQueue<AVFrame*>frame_que;
    //注：哈希表不能用指针，否则是根据指针的地址排序的
    SharedSet<FrameInfo> frame_set;

    Video();
    ~Video();
    void play();

    AVCodecContext* getVideoCodecctx() const;
    void setVideoCodecctx(AVCodecContext* videoCodecctx);

    int getVideoindex() const;
    void setVideoindex(int videoindex);

    AVStream* getVideostream() const;
    void setVideostream(AVStream* videostream);


    int getWidth() const;
    void setWidth(int width);

    int getHeight() const;
    void setHeight(int height);
    //转换成对应帧输出
    AVFrame* convert(AVFrame*& frame);
};

