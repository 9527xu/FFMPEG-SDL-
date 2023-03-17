#pragma once
extern "C" {

#include <libavformat\avformat.h>
#include<libavutil/time.h>
}
#include<iostream>
#include<thread>
#include"Audio.h"
#include"Video.h"
#include"VideoDisplay.h"

using namespace std;
class Media
{
public:
	Media(const char*url);
	~Media();
	void play();
	Audio audio;
	Video video;
	VideoDisplay videoDisplay;
private:
	//为解压做准备,获取多媒体、解码器上下文
	bool openInput();
	//读取多媒体文件中的压缩数据
	void read_packet();
	AVFormatContext* format_ctx=nullptr;

	const char* url;
	thread read_packet_thr;




};

//发送视频显示事件
uint32_t sdl_refresh_timer_cb(uint32_t interval, void* opaque);
// 启用定时器，刷新视频显示
void schedule_refresh(Media* media, int delay);
//实现音视频同步
void video_refresh_timer(void* userdata);