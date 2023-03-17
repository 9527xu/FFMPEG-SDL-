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
class SharedSet
{
private:
	set<T>h;
	mutex mtx;
	std::condition_variable m_cond;
public:
	void insert(T item);
	void pop_begin(T& data);

};

template<class T>
inline void SharedSet<T>::insert(T item)
{
	{
		lock_guard<mutex>lock(mtx);
		h.insert(item);
	}
	m_cond.notify_all();
}

template<class T>
inline void SharedSet<T>::pop_begin(T& data)
{
	unique_lock<mutex>lock(mtx);
	m_cond.wait(lock, [this]
		{
			return h.size();
		});
	data = *(h.begin());
	h.erase(h.begin());
}
