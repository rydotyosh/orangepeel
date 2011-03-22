
// proxy.h
//
// proxy template
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// 構築のタイミングを遅らせるときに使う。
// 最初にアクセスしたときにT型のオブジェクトが作られる。
// 途中で解体でき、アクセスがあるとまた作られる。
// 便利なのか不便なのか...

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
	
	//アクセスする
	T &instance()
	{
		{synchronized(cs);
			if(!obj) obj=new T;
		}
		return *obj;
	}
	
	//ポインタっぽく見せかける
	T &operator*(){return instance();}
	T *operator->(){return &instance();}
	
	//解体する
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
	
	//アクセスする
	T &instance()
	{
		{synchronized sync(cs);
			if(!obj) obj=new T(*arg);
		}
		return *obj;
	}
	
	//ポインタっぽく見せかける
	T &operator*(){return instance();}
	T *operator->(){return &instance();}
	
	//解体する
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

