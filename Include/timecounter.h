
// timecounter.h
//
// time counter
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// windowsとlinuxで使えるはず。
//
// TimeConter:
//  コンストラクタが実行されてからの時間を測る
//
// TimeInterval:
//  updateの時間間隔に達したかどうかを調べる。テキトーなfps調整。

#pragma once

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")// timeGetTime
#else
#include <sys/time.h>           // gettimeofday
#endif

#include "singleton.h"

namespace Rydot
{

class TimeCounter
{
private:
	long sec_init;

public:
	TimeCounter()
	{
		init();
	}

	void init()
	{
	#ifdef _WIN32
		int t=timeGetTime();
		sec_init=t/1000;
	#else
		struct timeval timeval;
		gettimeofday(&timeval, NULL);
		sec_init = timeval.tv_sec;
	#endif
	}

public:
	
	double get()const
	{
	#ifdef _WIN32
		int t=timeGetTime();
		return (t/1000-sec_init)+(t%1000)*0.001;
	#else
		struct timeval timeval;
		gettimeofday(&timeval, NULL);
		return (timeval.tv_sec - sec_init) + timeval.tv_usec * 0.000001;
	#endif
	}
	
	static void sleep(double s)
	{
	#ifdef _WIN32
		Sleep(s*1000);
	#else
		usleep(s*1000000);
	#endif
	}
};

class TimeInterval
{
public:
	singleton<TimeCounter> tc;
	
private:
	double last;
	double lastdt;
	double interval;
	double accum;

public:
	TimeInterval(double iv)
	{
		last=tc.instance().get();
		interval=iv;
		accum=0;
	}
	
	//return
	//true:suggest to skip
	bool update()
	{
		double t=tc.instance().get();
		double dt=t-last;
		double sataccum=1.0;//誤差蓄積の飽和
		last=t;
		lastdt=dt;
		if(interval)
		{
			accum+=dt-interval;
			//std::cout<<"accum:"<<accum<<std::endl;
			if(accum>0)
			{
				if(accum>sataccum)accum=sataccum;
				return true;
			}
			if(accum<0)
			{
				tc.instance().sleep(-accum);
			}
		}
		return false;
	}
	double getlast()
	{
		return last;
	}
	double getlastdt()
	{
		return lastdt;
	}
};

};//end of namespace Rydot



