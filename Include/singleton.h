
// singleton.h
//
// singleton template
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// T�^�̃I�u�W�F�N�g���C���X�^���X��1�����~�����Ƃ��Ɏg���B
// �܂��O���[�o���̑�p�B
// �ŏ��ɃA�N�Z�X�����Ƃ���T�^�̃I�u�W�F�N�g������A�C���X�^���X��̎��ɉ�̂����B
// �r���ŉ�̂ł��Ȃ��̂ŁA���ɂł����I�u�W�F�N�g���Ƃ�����ƍ��邩���B
// T�N���X��singleton�����Ԃ��Ďg���B
//	singleton<xxx> a;
//	singleton<xxx> b;
// �Ƃ�����Ă�&a.instance()==&b.instance()�ɂȂ�B
// �֗��Ȃ̂��s�ւȂ̂�...

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
	
	//�|�C���^���ۂ�����������
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

	//�|�C���^���ۂ�����������
	const T &operator*()const{return instance();}
	const T *operator->()const{return &instance();}
};*/


}//end of namespace Rydot

#endif

