#pragma once
extern "C" {

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include"libavutil/imgutils.h"
}
#include<iostream>
#include<bits/stdc++.h>
using namespace std;
#include <mutex>
template<class T>
class SharedQueue
{
public:
	SharedQueue();
	~SharedQueue();
	void push(T item);
	void pop(T& data);
	int size();
	bool empty();

private:
	queue<T>que;
	mutex mtx;
	std::condition_variable m_cond;
	int maxSize = 1024;
};

template<class T>
inline SharedQueue<T>::SharedQueue()
{
}

template<class T>
inline SharedQueue<T>::~SharedQueue()
{
}

template<class T>
inline void SharedQueue<T>::push(T item)
{
	lock_guard<mutex>lock(mtx);
	que.push(item);
	if (que.size() < maxSize)
	{
		m_cond.notify_all();
	}
	
	
}

template<class T>
inline void SharedQueue<T>::pop(T& data)
{
	unique_lock<mutex>lock(mtx);
	m_cond.wait(lock, [this]
		{
			return que.size();
		});
	data = move(que.front());
	que.pop();
}

template<class T>
inline int SharedQueue<T>::size()
{
	lock_guard<mutex>lock(mtx);
	int size = que.size();

	return size;
}

template<class T>
inline bool SharedQueue<T>::empty()
{
	lock_guard<mutex>lock(mtx);
	return que.empty();
}
