
// proxy.h
//
// proxy template
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// �\�z�̃^�C�~���O��x�点��Ƃ��Ɏg���B
// �ŏ��ɃA�N�Z�X�����Ƃ���T�^�̃I�u�W�F�N�g�������B
// �r���ŉ�̂ł��A�A�N�Z�X������Ƃ܂������B
// �֗��Ȃ̂��s�ւȂ̂�...

#pragma once

#include <boost/smart_ptr.hpp>
#include "cthread.h"

namespace Rydot
{

template <class T,class A=void>
struct proxy;

template <class T>
struct proxy<T,void>
{
private:
	T *obj;
	criticalsection cs;
public:
	proxy():obj(NULL){}
	~proxy(){release();}
	
	//�A�N�Z�X����
	T &instance()
	{
		{synchronized(cs);
			if(!obj) obj=new T;
		}
		return *obj;
	}
	
	//�|�C���^���ۂ�����������
	T &operator*(){return instance();}
	T *operator->(){return &instance();}
	
	//��̂���
	void release()
	{
		{synchronized(cs);
			if(obj)
			{
				delete obj;
				obj=NULL;
			}
		}
	}
};

template <class T,class A>
struct proxy
{
private:
	T *obj;
	criticalsection cs;
	//const A &arg;
	boost::shared_ptr<A> arg;
public:
	//proxy(const A &_arg):obj(NULL),arg(_arg){}
	proxy(boost::shared_ptr<A> _arg):obj(NULL),arg(_arg){}
	~proxy(){release();}
	
	//�A�N�Z�X����
	T &instance()
	{
		{synchronized sync(cs);
			if(!obj) obj=new T(*arg);
		}
		return *obj;
	}
	
	//�|�C���^���ۂ�����������
	T &operator*(){return instance();}
	T *operator->(){return &instance();}
	
	//��̂���
	void release()
	{
		{synchronized sync(cs);
			if(obj)
			{
				delete obj;
				obj=NULL;
			}
		}
	}
};

}//end of namespace Rydot

