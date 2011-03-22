
// messagequeue.h
//
// message queue
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// テキトーなメッセージキュー

#pragma once

#include "cthread.h"
#include <queue>

namespace Rydot
{

template <class T>
class messagequeue
{
protected:
	Rydot::criticalsection cs;
	std::queue<T> q;
public:

	//messagequeue(){while(!q.empty())q.pop();}

	//メッセージを追加
	void push(const T &mes)
	{
		{
			synchronized s(cs);
			q.push(mes);
		}
	}
	
	//メッセージをみるだけ
	const T &top()const
	{
		synchronized s(cs);
		return q.top();
	}
	const T &peek()const// topのエイリアス(おんなじもの)
	{
		return top();
	}
	
	//メッセージを受け取る
	bool get(T &m)
	{
		bool r=false;
		{
			synchronized s(cs);
			if(!q.empty())
			{
				m=q.front();
				q.pop();
				r=true;
			}
		}
		thread::sleep();
		return r;
	}
	
};

}//end of namespace Rydot

