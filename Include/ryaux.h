
// ryaux.h
//
// rydot auxiliary templates
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// なんとなく使えそうな気がするテンプレート関数群

#pragma once

#include <list>
#include <vector>

namespace Rydot
{

//列データをストリームに出力
//std::vector版
template<class T>
std::ostream &operator<<(std::ostream &_o,std::vector<T> &_v)
{
	bool c=false;
	for(std::vector<T>::iterator i=_v.begin();i!=_v.end();++i)
	{
		if(c)_o<<",";
		_o<<"("<<*i<<")";
		c=true;
	}
	return _o;
}

//列データをストリームに出力
//std::list版
template<class T>
std::ostream &operator<<(std::ostream &_o,std::list<T> &_v)
{
	bool c=false;
	for(std::list<T>::iterator i=_v.begin();i!=_v.end();++i)
	{
		if(c)_o<<",";
		_o<<"("<<*i<<")";
		c=true;
	}
	return _o;
}

//リストをベクタに変換
template <class T>
std::vector<T> list2vector(const std::list<T> &l)
{
	std::vector<T> v;
	v.reserve(l.size());
	for(std::list<T>::const_iterator i=l.begin();i!=l.end();++i)
		v.push_back(*i);
	return v;
}

template <class T>
std::vector<T> &list2vector(const std::list<T> &l,std::vector<T> &v)
{
	v.reserve(l.size());
	for(std::list<T>::const_iterator i=l.begin();i!=l.end();++i)
		v.push_back(*i);
	return v;
}

//ベクタをリストに変換
template <class T>
std::list<T> vector2list(const std::vector<T> &v)
{
	std::list<T> l;
	for(std::vector<T>::const_iterator i;i=v.begin();i!=v.end();++i)
		l.push_back(*i);
	return l;
}

template <class T>
std::list<T> &vector2list(const std::vector<T> &v,std::list<T> &l)
{
	for(std::vector<T>::const_iterator i;i=v.begin();i!=v.end();++i)
		l.push_back(*i);
	return l;
}

//2乗
template <class T>
T pow2(const T v)
{
	return v*v;
}

};

