#include "Audio.h"
#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000


int Audio::getAudioindex() const
{
    return audio_index;
}

void Audio::setAudioindex(int audioindex)
{
    audio_index = audioindex;
}


AVCodecContext* Audio::getAudiocodecctx() const
{
    return audio_codec_ctx;
}

void Audio::setAudiocodecctx(AVCodecContext* audiocodecctx)
{
    audio_codec_ctx = audiocodecctx;
}

Audio::Audio()
{
 
}

Audio::~Audio()
{
  if (audio_codec_ctx)
  {
    avcodec_free_context(&audio_codec_ctx);
  }
}
void Audio::play()
{
	//初始化SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
		printf("There is something wrong with your SDL Libs. Couldn't run");
	//打开音频驱动
#ifdef _WIN32
	SDL_AudioInit("directsound");
#endif

	static SDL_AudioSpec wantedSpec = { 0 }, audioSpec = { 0 };
	memset(&wantedSpec, 0, sizeof wantedSpec);
	wantedSpec.channels = audio_codec_ctx->channels;
	wantedSpec.freq = audio_codec_ctx->sample_rate;
	wantedSpec.format = AUDIO_S16SYS;
	wantedSpec.silence = 0;
	wantedSpec.samples = SDL_AUDIO_BUFFER_SIZE;
	wantedSpec.userdata = this;
	wantedSpec.callback = audio_callback;
	if (SDL_OpenAudio(&wantedSpec, &audioSpec) < 0)
	{
		printf("Failed to open audio");
		throw;
	}
	wanted_frame.format = AV_SAMPLE_FMT_S16;
	wanted_frame.sample_rate = audioSpec.freq;
	wanted_frame.channel_layout = av_get_default_channel_layout(audioSpec.channels);
	wanted_frame.channels = audioSpec.channels;

	SDL_PauseAudio(0);
}
int Audio::audio_decode_packet(uint8_t* audio_buf, int buf_size)
{
	SwrContext* swr_ctx = nullptr;
	AVPacket* pkt = nullptr;
	AVFrame *frame=av_frame_alloc();
	while (true)
	{
		//从队列里面取数据
		packet_que.pop(pkt);
		
		if (pkt->pts != AV_NOPTS_VALUE)
		{
			//av_q2d(audio_state->stream->time_base)尺子的刻度
		//pkt.pts物体的长度
		//上面两个相乘，就可以得到当前帧的显示时间
		//av_frame_get_best_effort_timestamp（AVFrame*frame）返回值和pkt_pts相同
			audio_clock = av_q2d(audio_stream->time_base) * pkt->pts;
		}
		int audio_pkt_size = pkt->size;
		//开始解压packet
		while (audio_pkt_size>0)
		{
			int ret=avcodec_send_packet(audio_codec_ctx, pkt);
			if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
			{
				break;
			}
			avcodec_receive_frame(audio_codec_ctx, frame);
			int len = frame->pkt_size;
			if (len < 0)
			{
				break;
			}
			audio_pkt_size -= len;

			//获取通道信息
			if (frame->channels > 0 && frame->channel_layout == 0)
				frame->channel_layout = av_get_default_channel_layout(frame->channels);
			else if (frame->channels == 0 && frame->channel_layout > 0)
				frame->channels = av_get_channel_layout_nb_channels(frame->channel_layout);

			if (swr_ctx)
			{
				swr_free(&swr_ctx);
				swr_ctx = NULL;
			}
			//对音频格式进行转换,重采样
			swr_ctx = swr_alloc_set_opts(NULL, wanted_frame.channel_layout, (AVSampleFormat)wanted_frame.format, wanted_frame.sample_rate,
				frame->channel_layout, (AVSampleFormat)frame->format, frame->sample_rate, 0, NULL);

			if (!swr_ctx || swr_init(swr_ctx) < 0)
			{
				printf("swr_init failed\n");
			}
			//计算理论重采样后的数据量
				 /* compute the number of converted samples: buffering is avoided
				 * ensuring that the output buffer will contain at least all the
				 * converted input samples */
				 //e=av_rescale_rnd(a,b,c,d) => e=a*b/c
				 //这是按比例来求取的目标通道的采样数据:src_num/src_rate = dst_num/dst_rate => dst_num = src_num * dst_rate/src_rate
				 // AV_ROUND_UP向上取整
			//max_dst_nb_samples = dst_nb_samples =
			//av_rescale_rnd(src_nb_samples, outSamplerate, inSamplerate, AV_ROUND_UP);

			int dst_nb_samples = av_rescale_rnd(frame->nb_samples, wanted_frame.sample_rate, frame->sample_rate, AV_ROUND_UP);

			int len2 = swr_convert(swr_ctx, //返回值每个通道输出的样本数
				//转化后的数据
				&audio_buf, dst_nb_samples,
				//转化前的数据
				(const uint8_t**)frame->data, frame->nb_samples);
			if (len2 < 0)
			{
				printf("swr_convert failed\n");
			}
			if (swr_ctx)
			{
				swr_free(&swr_ctx);
				swr_ctx = NULL;
			}
			AVSampleFormat dst_format= av_get_packed_sample_fmt((AVSampleFormat)wanted_frame.format);
			int data_size= wanted_frame.channels * len2 * av_get_bytes_per_sample(dst_format);

			// 每秒钟音频播放的字节数 sample_rate * channels * sample_format(一个sample占用的字节数)
			//audio_clock += static_cast<double>(data_size) / (2 * audio_stream->codecpar->channels * audio_stream->codecpar->sample_rate);
			av_frame_free(&frame);
			return data_size;//返回数据长度
		}
	}
	return 0;
}

void audio_callback(void* userdata, Uint8* stream, int len)
{
	Audio* audio = (Audio*)userdata;

	

	SDL_memset(stream, 0, len);
	while (len > 0)
	{
		if (audio->audio_buf_index >= audio->audio_buf_size)
		{
			int audio_size = audio->audio_decode_packet(audio->audio_buff, sizeof(audio->audio_buff));
			if (audio_size < 0)
			{
				audio->audio_buf_size = 1024;
				memset(audio->audio_buff, 0, audio->audio_buf_size);
			}
			else
				audio->audio_buf_size = audio_size;

			audio->audio_buf_index = 0;
		}

		//剩余未用的数据长度
		int len1 = audio->audio_buf_size - audio->audio_buf_index;
		if (len1 > len)
			len1 = len;
		//播放取到的音频数据
		SDL_MixAudio(stream, audio->audio_buff + audio->audio_buf_index, len, SDL_MIX_MAXVOLUME);
		len -= len1;
		stream += len1;
		audio->audio_buf_index +=len1;
	}
}

AVStream* Audio::getAudiostream() const
{
    return audio_stream;
}

void Audio::setAudiostream(AVStream* audiostream)
{
    audio_stream = audiostream;
}

double Audio::get_play_clock()
{
	//剩余的数据量的长度
	int len = audio_buf_size - audio_buf_index;
	//每秒钟音频播放的字节数=采样率*通道数*采样格式(sample format是16位的无符号整型，占用2个字节)
	int bytes_per_sec = audio_stream->codecpar->sample_rate * audio_codec_ctx->channels * 2;
	//当前的audio的播放时长
	double pts = audio_clock - static_cast<double>(len) / bytes_per_sec;
	return pts;
}




