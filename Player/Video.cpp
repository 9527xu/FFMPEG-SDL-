#include "Video.h"
#define CAPACITY 30

AVCodecContext* Video::getVideoCodecctx() const
{
    return video_codec_ctx;
}

void Video::setVideoCodecctx(AVCodecContext* videoCodecctx)
{
    video_codec_ctx = videoCodecctx;
}

int Video::getVideoindex() const
{
    return video_index;
}

void Video::setVideoindex(int videoindex)
{
    video_index = videoindex;
}

void Video::video_decode_packet()
{
  while (true)
  {
    AVPacket* packet;
    packet_que.pop(packet);
    
    AVFrame* frame = av_frame_alloc();
    int ret = avcodec_send_packet(video_codec_ctx, packet);
    if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
    {
      av_frame_free(&frame);
      av_packet_free(&packet);
      continue;
    }
  
    ret = avcodec_receive_frame(video_codec_ctx, frame);
    if (ret < 0 && ret != AVERROR_EOF)
    {
      av_frame_free(&frame);
      av_packet_free(&packet);

      continue;
    }
    
   
    
    ////避免解码过快
    //if (frame_que.size() >= CAPACITY)
    //  this_thread::sleep_for(std::chrono::milliseconds(50));
    
    double pts = packet->pts;
    if (pts == AV_NOPTS_VALUE)
    {
      pts = 0;
    }

    pts *= av_q2d(video_stream->time_base);//当前帧的时长
   // pts = synchronize(frame, pts);
    
    /*FrameInfo frameInfo;
    frameInfo.frame = frame;
    frameInfo.pts = pts;
    frame_set.insert(frameInfo);*/
    FrameInfo *frameInfo=new FrameInfo;
   frameInfo->frame = frame;
   frameInfo->pts = pts;
    frame_que.push(frameInfo);
    av_packet_free(&packet);

  }
}

double Video::synchronize(AVFrame* srcFrame, double pts)
{
  //pts = av_frame_get_best_effort_timestamp(frame)) == AV_NOPTS_VALUE，
  //获取pts无效，采取之前的播放长度
  if (pts != 0)
  {
    video_clock = pts;
  }
  else
  {
    pts = video_clock;
  }
  //解码时，这表示图片必须延迟多少时间。extra_delay = repeat_pict / (2*fps)
  //fps：每秒的帧数=刻度=av_q2d(stream->codecpar->time_base)
  //解码所需时间
  double frame_delay = srcFrame->repeat_pict * (av_q2d(video_stream->time_base) * 0.5);

  video_clock += frame_delay;
  return pts;
}

void Video::config_convert_par()
{
//#define WIDTH 864
//#define HEIGHT 468
  //为了显示正常，要进行转换
  // 通过指定像素的宽高来计算所需内存的大小。align：1字节对齐
  //不知道为什么纯填上面的数字会发生错误
  int numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, video_codec_ctx->width, video_codec_ctx->height, 1);
//int numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, 964, 568, 1);
  if (outBuffer)av_free(outBuffer);//清除上一次文件的数据

  outBuffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));//实例化缓冲区

  //把缓冲区设置给yuv420Frame
  av_image_fill_arrays(yuv420Frame->data, yuv420Frame->linesize, outBuffer, AV_PIX_FMT_YUV420P, video_codec_ctx->width, video_codec_ctx->height, 1);

  sws_ctx = sws_getContext(
    //输入
    video_codec_ctx->width,
    video_codec_ctx->height,
    video_codec_ctx->pix_fmt,
    //输出
    width,
    height,
    AV_PIX_FMT_YUV420P,

    SWS_BICUBIC, nullptr, nullptr, nullptr

  );
}

Video::Video()
{

}

Video::~Video()
{
  if (video_codec_ctx)
  {
    avcodec_free_context(&video_codec_ctx);
  }
  if (decode_thr.joinable())
  {
    decode_thr.join();
  }
}

void Video::play()
{

  config_convert_par();

  decode_thr = thread(&Video::video_decode_packet, this);

  
}



AVStream* Video::getVideostream() const
{
    return video_stream;
}

void Video::setVideostream(AVStream* videostream)
{
    video_stream = videostream;
}



int Video::getWidth() const
{
    return width;
}

void Video::setWidth(int width)
{
    this->width = width;
}

int Video::getHeight() const
{
    return height;
}

void Video::setHeight(int height)
{
    this->height = height;
}

AVFrame* Video::convert(AVFrame*& frame)
{
  sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height, yuv420Frame->data, yuv420Frame->linesize);
  return yuv420Frame;
}

