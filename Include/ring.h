
// ring.h
//
// ring
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// �L������ Z/nZ ���ۂ����
// �v�͎��R��n�Ń��[�v���Ă���+��*�ŕ��Ă�����
//
// �� F �ɂȂ��Ăق����Ƃ���n�͑f������Ȃ��ƍ����B
// ����܂�l���傫���ƍ����B
//
// �����O�o�b�t�@�Ƃ��U�E�����O�Ƃ����[�h�I�u�U�����O�Ƃ�����Ȃ���B
//
// ����܂藝�����Ă��Ȃ������Ɏg���ǂ��낪�Ȃ����B

#pragma once

struct Ring
{
public:
	int v;
	int n;
public:
	Ring();
	Ring(Ring &_r):v(_r.v),n(_r.n){}
	Ring(int _v,int _n):v(_v),n(_n){}
	Ring &operator=(const Ring &_r){v=_r.v;n=_r.n;return *this;}
	bool operator==(const Ring &_r){if(v==_r.v&&n==_r.n)return true;return false;}
	bool operator!=(const Ring &_r){if(v==_r.v&&n==_r.n)return false;return true;}
	
	//n���قȂ�ꍇ�͔퉉�Z���ɍ��킳���B
	Ring operator+(const Ring &_r){return Ring((v+_r.v)%n,n);}
	Ring operator*(const Ring &_r){return Ring((v*_r.v)%n,n);}
	Ring &operator+=(const Ring &_r){v=(v+_r.v)%n;return *this;}
	Ring &operator*=(const Ring &_r){v=(v*_r.v)%n;return *this;}
	
	//�������̂܂ܓ����Ɖ��ŕԂ��Ă���
	Ring operator+(const int _v){return Ring((v+_v)%n,n);}
	Ring operator*(const int _v){return Ring((v*_v)%n,n);}
	Ring &operator+=(const int _v){v=(v+_v)%n;return *this;}
	Ring &operator*=(const int _v){v=(v*_v)%n;return *this;}
};

