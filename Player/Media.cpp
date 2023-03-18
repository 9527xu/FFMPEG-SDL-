#include "Media.h"
static const double NOSYNC_THRESHOLD = 0.003;
#define error_null(p,info)\
do{\
	if (!p){\
    printf("File Fame: %s\n Present Line: %d\n", __FILE__,__LINE__);\
		cout << info << " error\n";\
		return 0;\
	}}while(0)

#define error_less_zero(p)\
do{\
	if (p < 0){\
    printf("File Fame: %s\n Present Line: %d\n", __FILE__,__LINE__);\
		char info[521];\
		av_strerror(p, info, 521);\
		cout << info << "\n";\
		return 0;\
	}}while(0)
Media::Media(const char* url)
	:url(url)
{
}

Media::~Media()
{
	if (read_packet_thr.joinable())
	{
		read_packet_thr.join();
	}
	if (format_ctx)
	{
		avformat_free_context(format_ctx);
	}
}

void Media::play()
{
	bool flag=openInput();
	if (flag)
	{
		read_packet_thr = thread(&Media::read_packet, this);
	}
	//初始化SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
		printf("There is something wrong with your SDL Libs. Couldn't run");
	//打开音频驱动
#ifdef _WIN32
	SDL_AudioInit("directsound");
#endif
	audio.play();
	video.setWidth(videoDisplay.getWidth());
	video.setHeight(videoDisplay.getHeight());
	video.play();
	
	videoDisplay.display();
	schedule_refresh(this, 0.1);
}

bool Media::openInput()
{
	avformat_network_init();
	
	//打开文件
	int ret = avformat_open_input(&format_ctx, url, nullptr, nullptr);
	error_less_zero(ret);
	//寻找文件流信息
	ret = avformat_find_stream_info(format_ctx, nullptr);
	error_less_zero(ret);

	//视频

	//寻找对应流中的索引
	int video_index= av_find_best_stream(format_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	error_less_zero(video_index);
	//寻找对应流中的编解码器
	const AVCodec*video_codec= avcodec_find_decoder(format_ctx->streams[video_index]->codecpar->codec_id);
	error_null(video_codec, "avcodec_find_decoder video");
	//初始化编解码器上下文
	AVCodecContext* video_ctx = avcodec_alloc_context3(video_codec);
	error_null(video_ctx, "avcodec_alloc_context3 video");

	//配置编解码器上下文参数
	ret = avcodec_parameters_to_context(video_ctx, format_ctx->streams[video_index]->codecpar);
	error_less_zero(ret);
	//打开编解码器
	ret = avcodec_open2(video_ctx, video_codec, nullptr);

	//音频

//寻找对应流中的索引
	int audio_index = av_find_best_stream(format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	error_less_zero(audio_index);
	
	//寻找对应流中的编解码器
	const AVCodec* audio_codec = avcodec_find_decoder(format_ctx->streams[audio_index]->codecpar->codec_id);
	error_null(audio_codec, "avcodec_find_decoder audio");
	//初始化编解码器上下文
	AVCodecContext* audio_ctx = avcodec_alloc_context3(audio_codec);
	error_null(audio_ctx, "avcodec_alloc_context3 audio");

	//配置编解码器上下文参数
	ret = avcodec_parameters_to_context(audio_ctx, format_ctx->streams[audio_index]->codecpar);
	error_less_zero(ret);
	//打开编解码器
	ret = avcodec_open2(audio_ctx, audio_codec, nullptr);

	video.setVideoCodecctx(video_ctx);
	video.setVideoindex(video_index);
	video.setVideostream(format_ctx->streams[video_index]);

	audio.setAudioindex(audio_index);
	audio.setAudiostream(format_ctx->streams[audio_index]);
	audio.setAudiocodecctx(audio_ctx);


	return true;
}

void Media::read_packet()
{
	while (true)
	{
		AVPacket* packet = av_packet_alloc();
		int ret = av_read_frame(format_ctx, packet);
		if (ret < 0)
		{
			if (ret == AVERROR_EOF)
				break;
			if (format_ctx->pb->error == 0)
				continue;
		}
		if (packet->stream_index == audio.getAudioindex())
		{
			AVRational time=format_ctx->streams[packet->stream_index]->time_base;
		//	cout << "audio time=" << packet->pts * av_q2d(time) << "\n";
			audio.packet_que.push(packet);
		}
		else if(packet->stream_index == video.getVideoindex())
		{
			AVRational time = format_ctx->streams[packet->stream_index]->time_base;
			//cout << "video time=" << packet->pts * av_q2d(time) << "\n";
			video.packet_que.push(packet);
		}
		else
		{
			av_packet_free(&packet);
		}
	}
}

uint32_t sdl_refresh_timer_cb(uint32_t interval, void* opaque)
{
	SDL_Event event;
	event.type = FF_REFRESH_EVENT;
	event.user.data1 = opaque;
	SDL_PushEvent(&event);
	return 0; /* 0 means stop timer */
}

void video_refresh_timer(void* userdata)
{
	Media* media = (Media*)userdata;
	Video* video = &media->video;
	while(video->getVideoindex() >= 0)
	{
			FrameInfo frameInfo;
			video->frame_set.pop_begin(frameInfo);
			AVFrame* frame = frameInfo.frame;
			double audio_time = media->audio.get_play_clock();//当前音频播放到的时刻
			double video_time = frameInfo.pts;
			cout <<"video_time=" << video_time << "\n";
			cout <<"audio_time=" << audio_time << "\n";
			double video_show_time = frame->pkt_duration* av_q2d(video->getVideostream()->time_base);
			
			double diff = audio_time - video_time;
			double actual_delay = 0;
			
			if (abs(diff) > NOSYNC_THRESHOLD)
			{
				
				if (diff > 0)//音频快，缩短展示时间
				{
					
					actual_delay = video_show_time *0.5;
				}
				else//视频快，延长展示时间
				{
					actual_delay = video_show_time+ video_show_time * 0.5;
				}
			}
			
			AVFrame* resFrame = video->convert(frame);
			
			

			SDL_UpdateYUVTexture(media->videoDisplay.texture, nullptr,
				resFrame->data[0], resFrame->linesize[0],
				resFrame->data[1], resFrame->linesize[1],
				resFrame->data[2], resFrame->linesize[2]);
			SDL_RenderClear(media->videoDisplay.render);
			SDL_RenderCopy(media->videoDisplay.render, media->videoDisplay.texture, nullptr, nullptr);
			SDL_RenderPresent(media->videoDisplay.render);
			
			//schedule_refresh(media, static_cast<int>(actual_delay * 1000 + 0.5));
			av_usleep();
			av_frame_free(&frame);
	}
	
	
}
// 延迟delay ms后刷新video帧
void schedule_refresh(Media* media, int delay)
{
	//SDL_TimerID SDL_AddTimer(Uint32 interval, SDL_TimerCallback callback, void *param):
	//向系统请求增加一个定时器.milliseconds毫秒
	//sdl_refresh_timer_cb: 该函数返回值为下次唤醒的时长, 若返回0, 则不会再唤醒.
	SDL_AddTimer(delay, sdl_refresh_timer_cb, media);
}