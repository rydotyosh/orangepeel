
// messagequeue.h
//
// message queue
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// �e�L�g�[�ȃ��b�Z�[�W�L���[

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

	//���b�Z�[�W��ǉ�
	void push(const T &mes)
	{
		{
			synchronized s(cs);
			q.push(mes);
		}
	}
	
	//���b�Z�[�W���݂邾��
	const T &top()const
	{
		synchronized s(cs);
		return q.top();
	}
	const T &peek()const// top�̃G�C���A�X(����Ȃ�����)
	{
		return top();
	}
	
	//���b�Z�[�W���󂯎��
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

