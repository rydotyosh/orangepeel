
// singleton.h
//
// singleton template
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// T型のオブジェクトをインスタンスに1個だけ欲しいときに使う。
// まあグローバルの代用。
// 最初にアクセスしたときにT型のオブジェクトが作られ、インスタンス解体時に解体される。
// 途中で解体できないので、非常にでかいオブジェクトだとちょっと困るかも。
// Tクラスはsingletonをかぶせて使う。
//	singleton<xxx> a;
//	singleton<xxx> b;
// とかやっても&a.instance()==&b.instance()になる。
// 便利なのか不便なのか...

#pragma once

#ifndef __RYDOT_SINGLETON_H__
#define __RYDOT_SINGLETON_H__

namespace Rydot
{

template <class T>
struct singleton
{
	T &instance()
	{
		static T inst;
		return inst;
	}
	
	//ポインタっぽく見せかける
	T &operator*(){return instance();}
	T *operator->(){return &instance();}
};

/*
template <class T>
struct const_singleton
{
	const T &instance()const
	{
		static const T inst;
		return inst;
	}

	//ポインタっぽく見せかける
	const T &operator*()const{return instance();}
	const T *operator->()const{return &instance();}
};*/


}//end of namespace Rydot

#endif

