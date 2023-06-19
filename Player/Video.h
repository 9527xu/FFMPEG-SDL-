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

  
  //返回pts对应的时间
  double get_pts_time(double pts);

  //配置转换器参数
  void config_convert_par();

  int width;
  int height;

public:
  //在
  //packet->pts不是，所以存packet->pts要用set，frame->pts随意
 // 读取内容的packetframe->pts是递增的
  
  //队列可存packet->pts、frame->pts
  //set只可存frame->pts

    SharedQueue<AVPacket*>packet_que;
    //因为视频的pts与dts不同，而队列是以dts为顺序的不妥，frame用哈希表存,实现以pts为顺序从小到大排序
    SharedQueue<FrameInfo*>frame_que;
    //注：哈希表不能用指针，否则是根据指针的地址排序的
    //改为set会出现抖动，不知道原因是啥
    //解决：使用set存的是而不是packet->pts不然会抖。队列存的是packet->pts
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

