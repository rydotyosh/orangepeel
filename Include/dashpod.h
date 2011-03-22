
// dashpod.h
//
// dashpod
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// ���͂��Ȃ܂���B���͂�1��lpf�������ďo�͂���B

#pragma once

namespace Rydot
{

template<class T>
class Dashpod
{
protected:
	T value;
	T reference;
	T k;
public:
	Dashpod(){k=1.5;}
	Dashpod(T ini){setvalue(ini);k=1.5;}
	void setk(T _k){k=_k;}
	void setvalue(T v){value=v;reference=v;}
	void set(T v){reference=v;}
	T get()const{return value;}
	T getrefevernce()const{return reference;}
	void update(){value=(value*k+reference)/(k+1);}
};

};

