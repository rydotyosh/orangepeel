
// polynomial.h
//
// polynomial solver
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// 2次式から4次式までをテキトーに解くやつ
// 精度は知らん。

#pragma once

#include <complex>

namespace Rydot
{

//x^2+ax+b=0
template <class T>
void Solve2ndEq(const std::complex<T> &a,const std::complex<T> &b,
std::complex<T> &x1,std::complex<T> &x2)
{
	typedef std::complex<T> ct;
	
	ct r=sqrt(a*a*((T)(1.0/4.0))-b);
	ct h=-a*((T)(1.0/2.0));
	x1=h+r;
	x2=h-r;
}

//x^3+ax^2+bx+c=0
template <class T>
void Solve3rdEq(const std::complex<T> &a,const std::complex<T> &b,const std::complex<T> &c,
std::complex<T> &x1,std::complex<T> &x2,std::complex<T> &x3)
{
	typedef std::complex<T> ct;
	
	ct m=-a*a*((T)(1.0/3.0))+b,n=a*a*a*((T)(2.0/27.0))-a*b*((T)(1.0/3.0))+c;
	ct u3,v3;
	Solve2ndEq<T>(n,-m*m*m*((T)(1.0/27.0)),u3,v3);
	ct u,v;
	u=pow(u3,(T)(1.0/3.0));
	v=-m/u*((T)(1.0/3.0));
	
	ct omega(-0.5,sqrt(3.0)/2.0);
	ct omega2=omega*omega;
	ct ap3=a*((T)(1.0/3.0));
	x1=u+v-ap3;
	x2=omega*u+omega2*v-ap3;
	x3=omega2*u+omega*v-ap3;
}

template <class T>
void Solve3rdEqPart(const std::complex<T> &a,const std::complex<T> &b,const std::complex<T> &c,
std::complex<T> &x1)
{
	typedef std::complex<T> ct;
	
	ct m=-a*a*((T)(1.0/3.0))+b,n=a*a*a*((T)(2.0/27.0))-a*b*((T)(1.0/3.0))+c;
	ct u3,v3;
	Solve2ndEq<T>(n,-m*m*m*((T)(1.0/27.0)),u3,v3);
	ct u,v;
	u=pow(u3,(T)(1.0/3.0));
	v=-m/u*((T)(1.0/3.0));
	
	ct ap3=a*((T)(1.0/3.0));
	x1=u+v-ap3;
}

//x^4+ax^3+bx^2+cx+d=0
template <class T>
void Solve4thEq(const std::complex<T> &a,const std::complex<T> &b,
const std::complex<T> &c,const std::complex<T> &d,
std::complex<T> &x1,std::complex<T> &x2,std::complex<T> &x3,std::complex<T> &x4)
{
	typedef std::complex<T> ct;
	
	if(abs(a)==0&&abs(c)==0)//biquadratic eq
	{
		ct y1,y2;
		Solve2ndEq<T>(b,d,y1,y2);
		
		ct ry1=sqrt(y1),ry2=sqrt(y2);
		x1=ry1;
		x2=-ry1;
		x3=ry2;
		x4=-ry2;
		return;
	}
	
	//Ferari method
	ct b3=a*((T)(1.0/4.0));
	ct b32=b3*b3;
	ct p=b-((T)6.0)*b32;
	ct q=c+(-((T)2.0)*b+((T)8.0)*b32)*b3;
	ct r=d-c*b3+(+b -((T)3.0)*b32)*b32;

	ct u1;
	//u^3 +2*p*u^2 +(p^2-4r)u -q^2 = 0
	Solve3rdEqPart<T>(((T)2.0)*p,p*p-((T)4.0)*r,-q*q,u1);
	
	ct ru1=sqrt(u1);
	ct y1,y2,y3,y4;
	//y^2 +- ru*y + (p+u)/2 -+ q/(2*u)*ru = 0
	Solve2ndEq<T>(ru1,(p+u1)*((T)(1.0/2.0))-q/(((T)2.0)*u1)*ru1,y1,y2);
	Solve2ndEq<T>(-ru1,(p+u1)*((T)(1.0/2.0))+q/(((T)2.0)*u1)*ru1,y3,y4);
	
	x1=y1-b3;
	x2=y2-b3;
	x3=y3-b3;
	x4=y4-b3;
}


};//end of namespace Rydot
