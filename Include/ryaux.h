
// ryaux.h
//
// rydot auxiliary templates
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// �Ȃ�ƂȂ��g�������ȋC������e���v���[�g�֐��Q

#pragma once

#include <list>
#include <vector>

namespace Rydot
{

//��f�[�^���X�g���[���ɏo��
//std::vector��
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

//��f�[�^���X�g���[���ɏo��
//std::list��
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

//���X�g���x�N�^�ɕϊ�
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

//�x�N�^�����X�g�ɕϊ�
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

//2��
template <class T>
T pow2(const T v)
{
	return v*v;
}

};

