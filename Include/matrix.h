
// matrix.h
//
// matrix template
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// 行列とかベクトルとかのテンプレートクラス
// 3DCG用っぽく

#pragma once

#ifndef __RYDOT_MATRIX_H__
#define __RYDOT_MATRIX_H__

//forの展開
//#pragma warning( disable : 8027 )

#include <math.h>
#include <algorithm>
#include <ostream>
#include <vector>
#include <list>

const double PI=3.14159265358979;

#define for if(0);else for

namespace Rydot
{

template<class T> struct Vector2;
template<class T> struct Vector3;
template<class T> struct Quaternion;
template<class T> struct Matrix22;
template<class T> struct Matrix33;
template<class T> struct Matrix44;
template<class T> struct Rect2;
template<class T> struct Rect3;

typedef Vector2<float> Vector2f;
typedef Vector3<float> Vector3f;
typedef Quaternion<float> Quaternionf;
typedef Matrix22<float> Matrix22f;
typedef Matrix33<float> Matrix33f;
typedef Matrix44<float> Matrix44f;
typedef Rect2<float> Rect2f;
typedef Rect3<float> Rect3f;

typedef Vector2<int> Vector2i;
typedef Vector3<int> Vector3i;
typedef Rect2<int> Rect2i;
typedef Rect3<int> Rect3i;

typedef Vector2<double> Vector2d;
typedef Vector3<double> Vector3d;
typedef Quaternion<double> Quaterniond;
typedef Matrix22<double> Matrix22d;
typedef Matrix33<double> Matrix33d;
typedef Matrix44<double> Matrix44d;
typedef Rect2<double> Rect2d;
typedef Rect3<double> Rect3d;

typedef std::pair<int,int> iipair;
typedef std::vector<iipair> iipairv;

typedef std::vector<int> intv;
typedef std::vector<float> floatv;
typedef std::vector<Vector3f> Vector3fv;
typedef std::vector<Vector2f> Vector2fv;
typedef std::vector<Quaternionf> Quaternionfv;
typedef std::vector<Vector3i> Vector3iv;
typedef std::vector<Vector2i> Vector2iv;

const int sini[]={0,1,0,-1};

/////////////////////////////
// 2次元ベクトル

//たまに複素数の演算も入っていたりする
template <class T>
struct Vector2
{
public:
	T x,y;
public:
	Vector2(){}
	Vector2(const T _x,const T _y):x(_x),y(_y){}
	Vector2(const Vector2 &_v):x(_v.x),y(_v.y){}
	Vector2(const T *_a):x(_a[0]),y(_a[1]){}
	//無理やり変換
	Vector2(const Vector3<T> &_v);
	Vector2(const Quaternion<T> &_v);

	Vector2 &operator=(const Vector2 &_v){x=_v.x;y=_v.y;return *this;}

	void ToArray(T *_a)const{_a[0]=x;_a[1]=y;}

	Vector2 &Translate(const T _x,const T _y){x+=_x;y+=_y;return *this;}
	Vector2 &Translate(const Vector2 &_v){x+=_v.x;y+=_v.y;return *this;}

	Vector2 &operator+=(const Vector2 &_v){x+=_v.x;y+=_v.y;return *this;}
	Vector2 &operator-=(const Vector2 &_v){x-=_v.x;y-=_v.y;return *this;}
	Vector2 &operator*=(const T _m){x*=_m;y*=_m;return *this;}
	//複素数の掛け算
	Vector2 &operator*=(const Vector2 &_v){Vector2 v(x*_v.x-y*_v.y,x*_v.y+y*_v.x);*this=v;return *this;}

	Vector2 operator+(const Vector2 &_v)const{return Vector2<T>(x+_v.x,y+_v.y);}
	Vector2 operator-(const Vector2 &_v)const{return Vector2<T>(x-_v.x,y-_v.y);}
	Vector2 operator-()const{Vector2 v(-x,-y);return v;}
	Vector2 operator*(const T _m)const{return Vector2<T>(x*_m,y*_m);}
	//複素数の掛け算
	Vector2 operator*(const Vector2 &_v)const{return Vector2<T>(x*_v.x-y*_v.y,x*_v.y+y*_v.x);}

	bool operator==(const Vector2 &_v)const{return (x==_v.x)&&(y==_v.y);}
	bool operator!=(const Vector2 &_v)const{return (x!=_v.x)||(y!=_v.y);}
	
	//無理やり順序づけ
	bool operator<(const Vector2 &_v)const
	{
		if(x<_v.x)return true;
		if(x>_v.x)return false;
		if(y<_v.y)return true;
		return false;
	}

	//絶対値(長さ)
	double Abs()const{return sqrt(x*x+y*y);}
	//絶対値の2乗
	T Abs2()const{return x*x+y*y;}
	
	//距離
	double Dist(const Vector2 &_v)const{Vector2 v=*this-_v;return v.Abs();}
	//距離の2乗
	T Dist2(const Vector2 &_v)const{Vector2 v=*this-_v;return v.Abs2();}

	//正規化
	//not for int
	Vector2 Norm()const{double d=sqrt(x*x+y*y);if(d==0){return Vector2<T>(1,0);}return Vector2<T>(x/d,y/d);}
	Vector2 &Normalize(){double d=sqrt(x*x+y*y);if(d==0){x=1;y=0;return *this;}x/=d;y/=d;return *this;}
	
	//複素数の共役
	Vector2 Conj()const{return Vector2<T>(x,-y);}
	Vector2 &Conjugate(){y=-y;return *this;}

	//各要素を積算する(内積のΣなし)
	Vector2 MultEach(const Vector2 &_v)const{return Vector2<T>(x*_v.x,y*_v.y);}
	Vector2 &MultiplyEach(const Vector2 &_v){x*=_v.x;y*=_v.y;return *this;}

	//_aラジアン左回転させる
	//not for int
	Vector2 &Rotate(const double _a)
	{
		double s=sin(_a),c=cos(_a);
		Vector2 r(x*c-y*s,x*s+y*c);*this=r;return *this;
	}
	
	//90degの_r倍回転させる
	Vector2 &Rotate90(const int _r)
	{
		const int rr=_r&3;
		const int s=sini[rr],c=sini[(rr+1)&3];
		Vector2 r(x*c-y*s,x*s+y*c);*this=r;return *this;
	}
	
	//内積
	T DotProd(const Vector2 &_v)const{return x*_v.x+y*_v.y;}
	//外積
	T ExProd(const Vector2 &_v)const{return x*_v.y-y*_v.x;}
	
	//x=1,y=0の方向を0度、左回りのラジアン角
	double Angle()const{return atan2(y,x);}
	
	//xとyを交換する
	void Swap(){std::swap(x,y);}
};

//int用関数

//intは回転は90度ずつだけにしてほしい。
template <>
Vector2<int> &Vector2<int>::Rotate(const double _a)
{
	int i=((int)floor(_a*2/PI+0.5))&3;
	return Rotate90(i);
}

//出力
template<class T>
std::ostream &operator<<(std::ostream &_o,const Vector2<T> &_v)
{_o<<_v.x<<","<<_v.y;return _o;}

/////////////////////////////
// 3次元ベクトル

template <class T>
struct Vector3
{
public:
	T x,y,z;
public:
	Vector3(){}
	Vector3(const T _x,const T _y,const T _z):x(_x),y(_y),z(_z){}
	Vector3(const Vector3 &_v):x(_v.x),y(_v.y),z(_v.z){}
	Vector3(const T *_a):x(_a[0]),y(_a[1]),z(_a[2]){}
	//無理やり変換
	Vector3(const Vector2<T> &_v);
	Vector3(const Quaternion<T> &_v);

	Vector3 &operator=(const Vector3 &_v){x=_v.x;y=_v.y;z=_v.z;return *this;}

	void ToArray(T *_a)const{_a[0]=x;_a[1]=y;_a[2]=z;}

	Vector3 &Translate(const T _x,const T _y,const T _z){x+=_x;y+=_y;z+=_z;return *this;}
	Vector3 &Translate(const Vector3 &_v){x+=_v.x;y+=_v.y;z+=_v.z;return *this;}

	Vector3 &operator+=(const Vector3 &_v){x+=_v.x;y+=_v.y;z+=_v.z;return *this;}
	Vector3 &operator-=(const Vector3 &_v){x-=_v.x;y-=_v.y;z-=_v.z;return *this;}
	Vector3 &operator*=(const T _m){x*=_m;y*=_m;z*=_m;return *this;}

	Vector3 operator+(const Vector3 &_v)const{return Vector3<T>(x+_v.x,y+_v.y,z+_v.z);}
	Vector3 operator-(const Vector3 &_v)const{return Vector3<T>(x-_v.x,y-_v.y,z-_v.z);}
	Vector3 operator-()const{return Vector3<T>(-x,-y,-z);}
	Vector3 operator*(const T _m)const{return Vector3<T>(x*_m,y*_m,z*_m);}

	bool operator==(const Vector3 &_v)const{return (x==_v.x)&&(y==_v.y)&&(z==_v.z);}
	bool operator!=(const Vector3 &_v)const{return (x!=_v.x)||(y!=_v.y)||(z!=_v.z);}

	//無理やり順序づけ
	bool operator<(const Vector3 &_v)const
	{
		if(x<_v.x)return true;
		if(x>_v.x)return false;
		if(y<_v.y)return true;
		if(y>_v.y)return false;
		if(z<_v.z)return true;
		return false;
	}

	//絶対値(長さ)
	double Abs()const{return sqrt(x*x+y*y+z*z);}
	T Abs2()const{return x*x+y*y+z*z;}

	//距離
	double Dist(const Vector3 &_v)const{Vector3 v=*this-_v;return v.Abs();}
	T Dist2(const Vector3 &_v)const{Vector3 v=*this-_v;return v.Abs2();}

	//各要素をそれぞれ積算する
	Vector3 MultEach(const Vector3 &_v)const{return Vector3<T>(x*_v.x,y*_v.y,z*_v.z);}
	Vector3 &MultiplyEach(const Vector3 &_v){x*=_v.x;y*=_v.y;z*=_v.z;return *this;}

	//正規化
	//not for int
	Vector3 Norm()const
	{
		const double d=sqrt(x*x+y*y+z*z);
		if(d==0)return Vector3<T>(1,0,0);
		return Vector3<T>(T(x/d),T(y/d),T(z/d));
	}
	Vector3 &Normalize()
	{
		const T d=sqrt(x*x+y*y+z*z);
		if(d==0){x=1;y=0;z=0;return *this;}
		x/=d;y/=d;z/=d;
		return *this;
	}

	//内積
	T DotProd(const Vector3 &_v)const{return x*_v.x+y*_v.y+z*_v.z;}
	//外積
	Vector3 ExProd(const Vector3 &_v)const
	{
		Vector3<T> v(y*_v.z-z*_v.y,z*_v.x-x*_v.z,x*_v.y-y*_v.x);
		return v;
	}
	
	//外積の左側のベクトルの行列表現
	Matrix33<T> Skew()const{return Matrix33<T>(
		 0,-z, y,
		 z, 0,-x,
		-y, x, 0
		);
	}
	
	//回転
	//not for int
	Vector3 &RotateZ(const double r)
	{
		const double s=sin(r),c=cos(r);
		const double _x=c*x-s*y,_y=s*x+c*y;x=_x;y=_y;
		return *this;
	}
	Vector3 &RotateX(const double r)
	{
		const double s=sin(r),c=cos(r);
		const double _y=c*y-s*z,_z=s*y+c*z;y=_y;z=_z;
		return *this;
	}
	Vector3 &RotateY(const double r)
	{
		const double s=sin(r),c=cos(r);
		const double _z=c*z-s*x,_x=s*z+c*x;z=_z;x=_x;
		return *this;
	}
	
	Vector3 &RotateZ90(const int r)
	{
		const int i=r&3;const int s=sini[i],c=sini[(i+1)&3];
		const T _x=c*x-s*y,_y=s*x+c*y;x=_x;y=_y;
		return *this;
	}
	Vector3 &RotateX90(const int r)
	{
		const int i=r&3;const int s=sini[i],c=sini[(i+1)&3];
		const T _y=c*y-s*z,_z=s*y+c*z;y=_y;z=_z;
		return *this;
	}
	Vector3 &RotateY90(const int r)
	{
		const int i=r&3;const int s=sini[i],c=sini[(i+1)&3];
		const T _z=c*z-s*x,_x=s*z+c*x;z=_z;x=_x;
		return *this;
	}
	
	//軸_vを中心に_thラジアン回転
	Vector3 &RotateAxis(const Vector3 &_v,const double _th)
	{
		Quaternion<T> q;
		q.RotationAxis(_v,_th);
		*this=q.Rotate(*this);
		return *this;
	}

};

//とりあえずint用関数

//90度ずつまわしてください。
template<> Vector3<int> &Vector3<int>::RotateZ(const double r)
{const int i=((int)floor(r*2/PI+0.5))&3;return RotateZ90(i);}
template<> Vector3<int> &Vector3<int>::RotateX(const double r)
{const int i=((int)floor(r*2/PI+0.5))&3;return RotateX90(i);}
template<> Vector3<int> &Vector3<int>::RotateY(const double r)
{const int i=((int)floor(r*2/PI+0.5))&3;return RotateY90(i);}

//出力
template<class T>
std::ostream &operator<<(std::ostream &_o,const Vector3<T> &_v)
{_o<<_v.x<<","<<_v.y<<","<<_v.z;return _o;}


/////////////////////////////
// クォータニオン

template<class T>
struct Quaternion
{
public:
	T x,y,z,w;
	typedef const T cT;
public:
	Quaternion(){}
	Quaternion(cT _x,cT _y,cT _z,cT _w):w(_w),x(_x),y(_y),z(_z){}
	Quaternion(const Vector3<T> &_v,cT _w):w(_w),x(_v.x),y(_v.y),z(_v.z){}
	Quaternion(const Quaternion &_q):w(_q.w),x(_q.x),y(_q.y),z(_q.z){}
	Quaternion(cT *_a):w(_a[3]),x(_a[0]),y(_a[1]),z(_a[2]){}
	//無理やり変換
	Quaternion(const Vector2<T> &_v);
	Quaternion(const Vector3<T> &_v);

	Quaternion &operator=(const Quaternion &_q){w=_q.w;x=_q.x;y=_q.y;z=_q.z;return *this;}

	void ToArray(cT *_a)const{_a[0]=x;_a[1]=y;_a[2]=z;_a[3]=w;}

	Quaternion &operator*=(const Quaternion &_q)
	{
		Vector3<double> v0(x,y,z),v1(_q.x,_q.y,_q.z),v;
		v=v0.ExProd(v1)+v1*w+v0*_q.w;x=v.x;y=v.y;z=v.z;
		w=w*_q.w-v0.DotProd(v1);
		return *this;
	}
	Quaternion operator*(const Quaternion &_q)const
	{
		Quaternion q;
		Vector3<double> v0(x,y,z),v1(_q.x,_q.y,_q.z),v;
		v=v0.ExProd(v1)+v1*w+v0*_q.w;q.x=v.x;q.y=v.y;q.z=v.z;
		q.w=w*_q.w-v0.DotProd(v1);
		return q;
	}

	Quaternion &operator+=(const Quaternion &_q){w+=_q.w;x+=_q.x;y+=_q.y;z+=_q.z;return *this;}
	Quaternion operator+(const Quaternion &_q)const{return Quaternion<T>(x+_q.x,y+_q.y,z+_q.z,w+_q.w);}
	Quaternion &operator-=(const Quaternion &_q){w-=_q.w;x-=_q.x;y-=_q.y;z-=_q.z;return *this;}
	Quaternion operator-(const Quaternion &_q)const{return Quaternion<T>(x-_q.x,y-_q.y,z-_q.z,w-_q.w);}
	Quaternion &operator*=(cT _m){w*=_m;x*=_m;y*=_m;z*=_m;return *this;}
	Quaternion operator*(cT _m)const{return Quaternion<T>(x*_m,y*_m,z*_m,w*_m);}
	Quaternion operator-()const{return Quaternion<T>(-x,-y,-z,-w);}

	bool operator==(const Quaternion &_v)const{return (x==_v.x)&&(y==_v.y)&&(z==_v.z)&&(w==_v.w);}
	bool operator!=(const Quaternion &_v)const{return (x!=_v.x)||(y!=_v.y)||(z!=_v.z)||(w!=_v.w);}

	//無理やり順序づけ
	bool operator<(const Quaternion &_v)const
	{
		if(x<_v.x)return true;
		if(x>_v.x)return false;
		if(y<_v.y)return true;
		if(y>_v.y)return false;
		if(z<_v.z)return true;
		if(z>_v.z)return false;
		if(w<_v.w)return true;
		return false;
	}

	Quaternion &Identity(){w=1;x=0;y=0;z=0;return *this;}

	//_vを軸に_thラジアン回転するクォータニオン
	//not for int
	Quaternion &RotationAxis(const Vector3<T> &_v,double _th)
	{
		Vector3<T> v=_v.Norm()*sin(_th/2);
		w=cos(_th/2);x=v.x;y=v.y;z=v.z;
		return *this;
	}

	Vector3<T> Rotate(const Vector3<T> &_v)const
	{
		Quaternion cj=Conj();
		Quaternion v(_v,0);
		Quaternion r=(*this)*v*cj;
		return Vector3<T>(r.x,r.y,r.z);
	}

	//共役
	Quaternion Conj()const{return Quaternion<T>(-x,-y,-z,w);}
	Quaternion &Conjugate(){x=-x;y=-y;z=-z;return *this;}

	//長さ
	T Length2()const{return w*w+x*x+y*y+z*z;}
	double Length()const{return sqrt(w*w+x*x+y*y+z*z);}

	//逆数
	//not for int
	Quaternion operator~()const{return Inv();}
	Quaternion Inv()const{double l=Length2();return Quaternion<T>(-x/l,-y/l,-z/l,w/l);}
	Quaternion &Invert(){double l=Length2();w=w/l;x=-x/l;y=-y/l;z=-z/l;return *this;}

	//正規化
	//not for int
	Quaternion Norm()const{double l=Length();if(l==0){return Quaternion<T>(1,0,0,0);}return Quaternion<T>(x/l,y/l,z/l,w/l);}
	Quaternion &Normalize(){double l=Length();if(l==0){x=1;y=0;z=0;w=0;return *this;}w=w/l;x=x/l;y=y/l;z=z/l;return *this;}

	//内積
	T DotProd(const Quaternion &_q)const{return w*_q.w+x*_q.x+y*_q.y+z*_q.z;}

	//補間関数
	//not for int
	
	//線形補間
	static Quaternion Lerp(const Quaternion &_a,const Quaternion &_b,const double _t)
	{
		return _a+(_b-_a)*_t;
	}
	Quaternion Lerp(const Quaternion &_q,const double _t)const
	{
		return *this+(_q-*this)*_t;
	}
	//球面線形補間
	static Quaternion Slerp(const Quaternion &_a,const Quaternion &_b,const double _t)
	{
		Quaternion tmpq;
		T dot=_a.DotProd(_b);
		if(dot<0){dot=-dot;tmpq=_b*-1;}
		else{tmpq=_b;}
		if(dot<0.95)
		{
		double th=acos(dot);
		return (  _a*sin(th*(1-_t)) + tmpq*sin(th*_t)  )*(1/sin(th));
		}
		else
		{
			return Lerp(_a,tmpq,_t);
		}
	}
	Quaternion Slerp(const Quaternion &_q,const double _t)const
	{
		Quaternion tmpq;
		T dot=DotProd(_q);
		if(dot<0){dot=-dot;tmpq=_q*-1;}
		else{tmpq=_q;}
		if(dot<0.95)
		{
			double th=acos(dot);
			return (  (*this)*sin(th*(1-_t)) + tmpq*sin(th*_t)  )*(1/sin(th));
		}
		else
		{
			return Lerp(tmpq,_t);
		}
	}
	//反転なし球面線形補間
	static Quaternion SlerpNoInv(const Quaternion &_a,const Quaternion &_b,const double _t)
	{
		T dot=_a.DotProd(_b);
		if(dot>-0.95&&dot<0.95)
		{
		double th=acos(dot);
		return (  _a*sin(th*(1-_t)) + _b*sin(th*_t)  )*(1/sin(th));
		}
		else
		{
			return Lerp(_a,_b,_t);
		}
	}
	Quaternion SlerpNoInv(const Quaternion &_q,const double _t)const
	{
		T dot=DotProd(_q);
		if(dot>-0.95&&dot<0.95)
		{
			double th=acos(dot);
			return (  (*this)*sin(th*(1-_t)) + _q*sin(th*_t)  )*(1/sin(th));
		}
		else
		{
			return Lerp(_q,_t);
		}
	}
	//球面2次補間
	static Quaternion Squad
	(const Quaternion &_a,const Quaternion &_b,const Quaternion &_c,const Quaternion &_d,const double _t)
	{
		return SlerpNoInv(SlerpNoInv(_a,_d,_t),SlerpNoInv(_b,_c,_t),2*_t*(1-_t));
	}
	Quaternion Squad
	(const Quaternion &_b,const Quaternion &_c,const Quaternion &_d,const double _t)const
	{
		return SlerpNoInv(*this,_d,_t).SlerpNoInv(SlerpNoInv(_b,_c,_t),2*_t*(1-_t));
	}

	//指数(純粋クォータニオン w=0 に限る)
	Quaternion Exp()const
	{
		const double th=sqrt(x*x+y*y+z*z),c=cos(th);
		if(th>0)
		{
			double sc=sin(th)/th;
			return Quaternion<T>(x*sc,y*sc,z*sc,c);
		}
		else
		{
			return Quaternion<T>(0,0,0,c);
		}
	}
	static Quaternion Exp(const Quaternion &_q)
	{
		const double th=sqrt(_q.x*_q.x+_q.y*_q.y+_q.z*_q.z),c=cos(th);
		if(th>0){const double sc=sin(th)/th;return Quaternion<T>(_q.x*sc,_q.y*sc,_q.z*sc,c);}
		else{return Quaternion<T>(0,0,0,c);}
	}
	//対数(単位クォータニオン l=0 に限る)
	Quaternion Ln()const
	{
		const double th=acos(w);
		const double s=sin(th);
		if(s>0)
		{
			double sc=th/s;
			return Quaternion<T>(x*sc,y*sc,z*sc,0);
		}
		else
			return Quaternion<T>(0,0,0,0);
	}
	static Quaternion Ln(const Quaternion &_q)
	{
		const double th=acos(_q.w),s=sin(th);
		//const double th2=atan2(sqrt(_q.x*_q.x+_q.y*_q.y+_q.z*_q.z),_q.w);
		//const double l=_q.Length();
		if(s>0){double sc=th/s;return Quaternion<T>(_q.x*sc,_q.y*sc,_q.z*sc,0);}
		else{return Quaternion<T>(0,0,0,0);}
	}
	//スプライン補間用
	// q0 q1 q2 q3があって、
	// a=Spline(q0,q1,q2)
	// b=Spline(q1,q2,q3)
	// result=Squad(q0,a,b,q3,t)
	//というふうに使う
	static Quaternion Spline(const Quaternion &_m,const Quaternion &_0,const Quaternion &_p)
	{
		const Quaternion inv=_0.Conj();
		return _0*Exp(  (Ln(inv*_m)+Ln(inv*_p)) * -0.25  );
	}

	//回転行列に変換
	Matrix33<T> ToRotMatrix33()const
	{
		return Matrix33<T>(
			1-2*y*y-2*z*z,	2*x*y+2*w*z,	2*x*z-2*w*y,
			2*x*y-2*w*z,	1-2*x*x-2*z*z,	2*y*z+2*w*x,
			2*x*z+2*w*y,	2*y*z-2*w*x,	1-2*x*x-2*y*y
			);
	}
	
	//xジョイントとx回転に分割
	//q==joint*RotateAxis(Vector3(1,0,0),rot)
	//手を回す回転はこういう感じなはず。
	void SplitJointRot(Quaternion<T> &joint,T &rot)const
	{
		Vector3<T> xe(1,0,0);
		Vector3<T> xdash=Rotate(xe);
		Vector3<T> n=xe.ExProd(xdash);
		n.Normalize();
		T cs=xe.DotProd(xdash);
		double ss=sqrt((1-cs)*0.5);
		double cc=sqrt((1+cs)*0.5);

		joint=Quaternion<T>(n.x*ss,n.y*ss,n.z*ss,cc);
		
		Quaternion<T> xrot=joint.Inv()*(*this);
		rot=atan2(xrot.x,xrot.w)*2;
	}
	//中心軸をaxisにした場合
	//q==joint*RotateAxis(axis,rot)
	void SplitJointRot(const Vector3<T> &axis,Quaternion<T> &joint,T &rot)const
	{
		Vector3<T> xe=axis;
		xe.Normalize();
		Vector3<T> xdash=Rotate(xe);
		Vector3<T> n=xe.ExProd(xdash);
		n.Normalize();
		T cs=xe.DotProd(xdash);
		double ss=sqrt((1-cs)*0.5);
		double cc=sqrt((1+cs)*0.5);

		joint=Quaternion<T>(n.x*ss,n.y*ss,n.z*ss,cc);
		
		Quaternion<T> xrot=joint.Inv()*(*this);
		rot=atan2(xrot.x,xrot.w)*2;
	}
};

//出力
template<class T>
std::ostream &operator<<(std::ostream &_o,const Quaternion<T> &_q)
{_o<<_q.x<<","<<_q.y<<","<<_q.z<<","<<_q.w;return _o;}


//////////////////////////////
// 無理やり変換
//

//ベクトルとかを無理やり他のベクトルとかに変換する。
template<class T> Vector2<T>::Vector2(const Vector3<T> &_v):x(_v.x),y(_v.y){}
template<class T> Vector2<T>::Vector2(const Quaternion<T> &_v):x(_v.x),y(_v.y){}
template<class T> Vector3<T>::Vector3(const Vector2<T> &_v):x(_v.x),y(_v.y),z(0){}
template<class T> Vector3<T>::Vector3(const Quaternion<T> &_v):x(_v.x),y(_v.y),z(_v.z){}
template<class T> Quaternion<T>::Quaternion(const Vector2<T> &_v):x(_v.x),y(_v.y),z(0),w(0){}
template<class T> Quaternion<T>::Quaternion(const Vector3<T> &_v):x(_v.x),y(_v.y),z(_v.z),w(0){}


/////////////////////////////
// 2x2行列
//
template<class T>
struct Matrix22
{
public:
	T m[2][2];
	typedef const T cT;
public:
	Matrix22(){}
	Matrix22(cT _00,cT _01,cT _10,cT _11)
	{
		m[0][0]=_00;m[0][1]=_01;
		m[1][0]=_10;m[1][1]=_11;
	}
	Matrix22(const Vector2<T> &_v0,const Vector2<T> &_v1)
	{
		m[0][0]=_v0.x;m[0][1]=_v1.x;
		m[1][0]=_v0.y;m[1][1]=_v1.y;
	}
	Matrix22(const Matrix22 &_M)
	{
		*this=_M;
	}
	//単位行列
	Matrix22 &Identity()
	{
		m[0][0]=1;m[0][1]=0;
		m[1][0]=0;m[1][1]=1;
		return *this;
	}
	//ゼロ行列
	Matrix22 &Zero()
	{
		m[0][0]=0;m[0][1]=0;
		m[1][0]=0;m[1][1]=0;
		return *this;
	}
	
	Matrix22 &operator+=(const Matrix22 &_M)
	{
		for(int i=0;i<4;i++){int a=i/2,b=i%2;m[a][b]+=_M.m[a][b];}
		return *this;
	}
	Matrix22 &operator-=(const Matrix22 &_M)
	{
		for(int i=0;i<4;i++){int a=i/2,b=i%2;m[a][b]-=_M.m[a][b];}
		return *this;
	}
	Matrix22 &operator*=(const Matrix22 &_M)
	{
		Matrix22 t;
		for(int i=0;i<2;i++)
		{
			for(int j=0;j<2;j++)
			{
				t.m[i][j]=m[i][0]*_M.m[0][j]+m[i][1]*_M.m[1][j];
			}
		}
		*this=t;
		return *this;
	}

	Matrix22 operator+(const Matrix22 &_M)const
	{
		Matrix22 t;
		for(int i=0;i<4;i++){int a=i/2,b=i%2;t.m[a][b]=m[a][b]+_M.m[a][b];}
		return t;
	}
	Matrix22 operator-(const Matrix22 &_M)const
	{
		Matrix22 t;
		for(int i=0;i<4;i++){int a=i/2,b=i%2;t.m[a][b]=m[a][b]-_M.m[a][b];}
		return t;
	}
	Matrix22 operator-()const
	{
		Matrix22 t;
		for(int i=0;i<4;i++){int a=i/2,b=i%2;t.m[a][b]=-m[a][b];}
		return t;
	}
	Matrix22 operator*(const Matrix22 &_M)const
	{
		Matrix22 t;
		for(int i=0;i<2;i++)
		{
			for(int j=0;j<2;j++)
			{
				t.m[i][j]=m[i][0]*_M.m[0][j]+m[i][1]*_M.m[1][j];
			}
		}
		return t;
	}
	Vector2<T> operator*(const Vector2<T> &_v)const
	{
		return Vector2<T>(
			m[0][0]*_v.x+m[0][1]*_v.y,
			m[1][0]*_v.x+m[1][1]*_v.y
		);
	}
	Matrix22 operator*(cT _m)const
	{
		Matrix22 t;
		for(int i=0;i<4;i++){int a=i/2,b=i%2;t.m[a][b]=m[a][b]*_m;}
		return t;
	}
	
	bool operator==(const Matrix22 &_M)const
	{
		for(int i=0;i<4;++i){int a=i/2,b=i%2;if(m[a][b]!=_M.m[a][b])return false;}
		return true;
	}
	bool operator!=(const Matrix22 &_M)const
	{
		return !((*this)==_M);
	}
	
	//無理矢理順序づけ
	bool operator<(const Matrix22 &_M)const
	{
		for(int i=0;i<4;++i)
		{
			int a=i/2,b=i%2;
			if(m[a][b]<_M.m[a][b])return true;
			if(m[a][b]>_M.m[a][b])return false;
		}
		return false;
	}
	
	void ToArray44(T *_a)const
	{
		int n=0;
		for(int i=0;i<2;i++)
		{
			for(int j=0;j<2;j++,n++)
			{
				_a[n]=m[i][j];
			}
			_a[n]=0;n++;
			_a[n]=0;n++;
		}	
		_a[n]=0;n++;
		_a[n]=0;n++;
		_a[n]=1;n++;
		_a[n]=0;n++;
		
		_a[n]=0;n++;
		_a[n]=0;n++;
		_a[n]=0;n++;
		_a[n]=1;n++;
	}
	
	//転置
	Matrix22 Transp()const
	{
		Matrix22 M(*this);
		std::swap(M.m[1][0],M.m[0][1]);
		return M;
	}
	Matrix22 &Transpose()
	{
		std::swap<T>(m[1][0],m[0][1]);
		return *this;
	}
	
	//トレース
	T Trace()const
	{
		return m[0][0]+m[1][1];
	}
	
	//行列式
	T Determinant()const
	{
		return
			+m[0][0]*m[1][1]
			-m[0][1]*m[1][0];
	}
	
	//余因子
	T Cofactor(const int _i,const int _j)const
	{
		const int sig=((_i+_j)&1)?-1:1;
		return sig*m[1-_i][1-_j];
	}
	
	//逆行列
	//not for int
	Matrix22 operator~()const{return Inv();}
	Matrix22 Inv()const
	{
		Matrix22 t;
		cT d=Determinant();
		for(int i=0;i<2;i++)
		{
			for(int j=0;j<2;j++)
			{
				t.m[i][j]=Cofactor(j,i)/d;
			}
		}
		return t;
	}
	Matrix22 &Invert()
	{
		Matrix22 t;
		cT d=Determinant();
		for(int i=0;i<2;i++)
		{
			for(int j=0;j<2;j++)
			{
				t.m[i][j]=Cofactor(j,i)/d;
			}
		}
		*this=t;
		return *this;
	}
	
	// Ax=bを解く
	//not for int
	Vector2<T> Solve(const Vector2<T> &_b)const
	{
		cT d=Determinant();
		if(d==0)return Vector2<T>(0,0);
		Vector2<T> r;
		r.x=_b.x*m[1][1]-m[0][1]*_b.y;
		r.y=m[0][0]*_b.y-_b.x*m[1][0];
		return r*(1/d);
	}
	
	//複素数に変換(回転行列に限る)
	Vector2<T> ToComplex()const
	{
		Vector2<T> v(m[0][0],m[0][1]);
		return v;
	}
};

//出力
template<class T>
std::ostream &operator<<(std::ostream &_o,const Matrix22<T> &_m)
{
	_o<<_m.m[0][0]<<","<<_m.m[0][1]<<std::endl
	  <<_m.m[1][0]<<","<<_m.m[1][1]<<std::endl;
	return _o;
}



/////////////////////////////
// 3x3行列

template<class T>
struct Matrix33
{
public:
	T m[3][3];
	typedef const T cT;
public:
	Matrix33(){}
	Matrix33(cT _00,cT _01,cT _02,cT _10,cT _11,cT _12,cT _20,cT _21,cT _22)
	{
		m[0][0]=_00;m[0][1]=_01;m[0][2]=_02;
		m[1][0]=_10;m[1][1]=_11;m[1][2]=_12;
		m[2][0]=_20;m[2][1]=_21;m[2][2]=_22;
	}
	Matrix33(const Vector3<T> &_v0,const Vector3<T> &_v1,const Vector3<T> &_v2)
	{
		//m[0][0]=_v0.x;m[0][1]=_v0.y;m[0][2]=_v0.z;
		//m[1][0]=_v1.x;m[1][1]=_v1.y;m[1][2]=_v1.z;
		//m[2][0]=_v2.x;m[2][1]=_v2.y;m[2][2]=_v2.z;
		m[0][0]=_v0.x;m[0][1]=_v1.x;m[0][2]=_v2.x;
		m[1][0]=_v0.y;m[1][1]=_v1.y;m[1][2]=_v2.y;
		m[2][0]=_v0.z;m[2][1]=_v1.z;m[2][2]=_v2.z;
	}
	Matrix33(const Matrix33 &_M)
	{
		*this=_M;
	}
	//単位行列
	Matrix33 &Identity()
	{
		m[0][0]=1;m[0][1]=0;m[0][2]=0;
		m[1][0]=0;m[1][1]=1;m[1][2]=0;
		m[2][0]=0;m[2][1]=0;m[2][2]=1;
		return *this;
	}
	//ゼロ行列
	Matrix33 &Zero()
	{
		m[0][0]=0;m[0][1]=0;m[0][2]=0;
		m[1][0]=0;m[1][1]=0;m[1][2]=0;
		m[2][0]=0;m[2][1]=0;m[2][2]=0;
		return *this;
	}
	
	Matrix33 &operator+=(const Matrix33 &_M)
	{
		for(int i=0;i<9;i++){int a=i/3,b=i%3;m[a][b]+=_M.m[a][b];}
		return *this;
	}
	Matrix33 &operator-=(const Matrix33 &_M)
	{
		for(int i=0;i<9;i++){int a=i/3,b=i%3;m[a][b]-=_M.m[a][b];}
		return *this;
	}
	Matrix33 &operator*=(const Matrix33 &_M)
	{
		Matrix33 t;
		for(int i=0;i<3;i++)
		{
			for(int j=0;j<3;j++)
			{
				t.m[i][j]=m[i][0]*_M.m[0][j]+m[i][1]*_M.m[1][j]+m[i][2]*_M.m[2][j];
			}
		}
		*this=t;
		return *this;
	}

	Matrix33 operator+(const Matrix33 &_M)const
	{
		Matrix33 t;
		for(int i=0;i<9;i++){int a=i/3,b=i%3;t.m[a][b]=m[a][b]+_M.m[a][b];}
		return t;
	}
	Matrix33 operator-(const Matrix33 &_M)const
	{
		Matrix33 t;
		for(int i=0;i<9;i++){int a=i/3,b=i%3;t.m[a][b]=m[a][b]-_M.m[a][b];}
		return t;
	}
	Matrix33 operator-()const
	{
		Matrix33 t;
		for(int i=0;i<9;i++){int a=i/3,b=i%3;t.m[a][b]=-m[a][b];}
		return t;
	}
	Matrix33 operator*(const Matrix33 &_M)const
	{
		Matrix33 t;
		for(int i=0;i<3;i++)
		{
			for(int j=0;j<3;j++)
			{
				t.m[i][j]=m[i][0]*_M.m[0][j]+m[i][1]*_M.m[1][j]+m[i][2]*_M.m[2][j];
			}
		}
		return t;
	}
	Vector3<T> operator*(const Vector3<T> &_v)const
	{
		return Vector3<T>(
			m[0][0]*_v.x+m[0][1]*_v.y+m[0][2]*_v.z,
			m[1][0]*_v.x+m[1][1]*_v.y+m[1][2]*_v.z,
			m[2][0]*_v.x+m[2][1]*_v.y+m[2][2]*_v.z
		);
	}
	Matrix33 operator*(cT _m)const
	{
		Matrix33 t;
		for(int i=0;i<9;i++){int a=i/3,b=i%3;t.m[a][b]=m[a][b]*_m;}
		return t;
	}

	bool operator==(const Matrix33 &_M)const
	{
		for(int i=0;i<9;++i){int a=i/3,b=i%3;if(m[a][b]!=_M.m[a][b])return false;}
		return true;
	}
	bool operator!=(const Matrix33 &_M)const
	{
		return !((*this)==_M);
	}
	
	//無理矢理順序づけ
	bool operator<(const Matrix33 &_M)const
	{
		for(int i=0;i<9;++i)
		{
			int a=i/3,b=i%3;
			if(m[a][b]<_M.m[a][b])return true;
			if(m[a][b]>_M.m[a][b])return false;
		}
		return false;
	}

	void ToArray44(T *_a)const
	{
		int n=0;
		for(int i=0;i<3;i++)
		{
			for(int j=0;j<3;j++,n++)
			{
				_a[n]=m[i][j];
			}
			_a[n]=0;n++;
		}
		_a[n]=0;n++;
		_a[n]=0;n++;
		_a[n]=0;n++;
		_a[n]=1;n++;
	}
	
	//転置
	Matrix33 Transp()const
	{
		Matrix33 M(*this);
		std::swap(M.m[1][0],M.m[0][1]);
		std::swap(M.m[1][2],M.m[2][1]);
		std::swap(M.m[2][0],M.m[0][2]);
		return M;
	}
	Matrix33 &Transpose()
	{
		std::swap(m[1][0],m[0][1]);
		std::swap(m[1][2],m[2][1]);
		std::swap(m[2][0],m[0][2]);
		return *this;
	}
	
	//トレース
	T Trace()const
	{
		return m[0][0]+m[1][1]+m[2][2];
	}
	
	//行列式
	T Determinant()const
	{
		return
			+m[0][0]*m[1][1]*m[2][2]
			+m[0][1]*m[1][2]*m[2][0]
			+m[0][2]*m[1][0]*m[2][1]
			-m[0][2]*m[1][1]*m[2][0]
			-m[0][1]*m[1][0]*m[2][2]
			-m[0][0]*m[1][2]*m[2][1];
	}
	
	//余因子
	T Cofactor(int _i,int _j)const
	{
		int a[2],b[2];
		for(int i=0,j=0;i<3;i++){if(_i==i)continue;a[j]=i;j++;}
		for(int i=0,j=0;i<3;i++){if(_j==i)continue;b[j]=i;j++;}
		const int sig=((_i+_j)&1)?-1:1;
		
		return (m[a[0]][b[0]]*m[a[1]][b[1]]-m[a[0]][b[1]]*m[a[1]][b[0]])*sig;
	}
	
	//逆行列
	//not for int
	Matrix33 operator~()const{return Inv();}
	Matrix33 Inv()const
	{
		Matrix33 t;
		cT d=Determinant();
		for(int i=0;i<3;i++)
		{
			for(int j=0;j<3;j++)
			{
				t.m[i][j]=Cofactor(j,i)/d;
			}
		}
		return t;
	}
	Matrix33 &Invert()
	{
		Matrix33 t;
		cT d=Determinant();
		for(int i=0;i<3;i++)
		{
			for(int j=0;j<3;j++)
			{
				t.m[i][j]=Cofactor(j,i)/d;
			}
		}
		*this=t;
		return *this;
	}
	
	// Ax=bを解く
	//not for int
	Vector3<T> Solve(const Vector3<T> &_b)const
	{
		cT d=Determinant();
		if(d==0)return Vector3<T>(0,0,0);
		Vector3<T> r;
		r.x=Matrix33<T>
		(   _b.x,m[0][1],m[0][2],
		    _b.y,m[1][1],m[1][2],
		    _b.z,m[2][1],m[2][2]).Determinant();
		r.y=Matrix33<T>
		(m[0][0],   _b.x,m[0][2],
		 m[1][0],   _b.y,m[1][2],
		 m[2][0],   _b.z,m[2][2]).Determinant();
		r.z=Matrix33<T>
		(m[0][0],m[0][1],   _b.x,
		 m[1][0],m[1][1],   _b.y,
		 m[2][0],m[2][1],   _b.z).Determinant();
		return r*(1/d);
	}
	
	//クォータニオンに変換。(回転行列に限る)
	//not for int
	Quaternion<T> ToQuaternion()const
	{
		cT tr=Trace();
		if(tr>0)
		{
			Quaternion<T> q;
			q.w=sqrt(tr+1)/2;
			q.x=(m[1][2]-m[2][1])/(4*q.w);
			q.y=(m[2][0]-m[0][2])/(4*q.w);
			q.z=(m[0][1]-m[1][0])/(4*q.w);
			return q;
		}
		if(m[0][0]>=m[1][1]&&m[0][0]>=m[2][2])
		{
			Quaternion<T> q;
			q.x=sqrt(m[0][0]-m[1][1]-m[2][2]+1)/2;
			q.w=(m[1][2]-m[2][1])/(4*q.x);
			q.y=(m[0][1]+m[1][0])/(4*q.x);
			q.z=(m[0][2]+m[2][0])/(4*q.x);
			return q;
		}
		if(m[1][1]>=m[0][0]&&m[1][1]>=m[2][2])
		{
			Quaternion<T> q;
			q.y=sqrt(m[1][1]-m[0][0]-m[2][2]+1)/2;
			q.w=(m[2][0]-m[0][2])/(4*q.y);
			q.x=(m[0][1]+m[1][0])/(4*q.y);
			q.z=(m[1][2]+m[2][1])/(4*q.y);
			return q;
		}
		{
			Quaternion<T> q;
			q.z=sqrt(m[2][2]-m[0][0]-m[1][1]+1)/2;
			q.w=(m[0][1]-m[1][0])/(4*q.z);
			q.x=(m[2][0]+m[0][2])/(4*q.z);
			q.y=(m[2][1]+m[1][2])/(4*q.z);
			return q;
		}
	}
};

//出力
template<class T>
std::ostream &operator<<(std::ostream &_o,const Matrix33<T> &_m)
{
	_o<<_m.m[0][0]<<","<<_m.m[0][1]<<","<<_m.m[0][2]<<std::endl
	  <<_m.m[1][0]<<","<<_m.m[1][1]<<","<<_m.m[1][2]<<std::endl
	  <<_m.m[2][0]<<","<<_m.m[2][1]<<","<<_m.m[2][2];
	return _o;
}



/////////////////////////////
// 4x4行列

template<class T>
struct Matrix44
{
public:
	T m[4][4];
	typedef const T cT;
public:
	Matrix44(){}
	template<class S>
	Matrix44(
		S _00,S _01,S _02,S _03,
		S _10,S _11,S _12,S _13,
		S _20,S _21,S _22,S _23,
		S _30,S _31,S _32,S _33
		)
	{
		m[0][0]=_00;m[0][1]=_01;m[0][2]=_02;m[0][3]=_03;
		m[1][0]=_10;m[1][1]=_11;m[1][2]=_12;m[1][3]=_13;
		m[2][0]=_20;m[2][1]=_21;m[2][2]=_22;m[2][3]=_23;
		m[3][0]=_30;m[3][1]=_31;m[3][2]=_32;m[3][3]=_33;
	}
	template<class S>
	Matrix44(const Vector3<S> &_v0,const Vector3<S> &_v1,const Vector3<S> &_v2,const Vector3<S> &_t)
	{
		m[0][0]=_v0.x;m[0][1]=_v1.x;m[0][2]=_v2.x;m[0][3]=0;
		m[1][0]=_v0.y;m[1][1]=_v1.y;m[1][2]=_v2.y;m[0][3]=0;
		m[2][0]=_v0.z;m[2][1]=_v1.z;m[2][2]=_v2.z;m[0][3]=0;
		m[3][0]= _t.x;m[3][1]= _t.y;m[3][2]= _t.z;m[3][3]=1;
/*		m[0][0]=_v0.x;m[0][1]=_v1.x;m[0][2]=_v2.x;m[0][3]=_t.x;
		m[1][0]=_v0.y;m[1][1]=_v1.y;m[1][2]=_v2.y;m[1][3]=_t.y;
		m[2][0]=_v0.z;m[2][1]=_v1.z;m[2][2]=_v2.z;m[2][3]=_t.z;
		m[3][0]=0;    m[3][1]=0;    m[3][2]=0;    m[3][3]=1;*/
	}
	Matrix44(const Matrix44 &_M)
	{
		*this=_M;
	}
	//単位行列
	Matrix44 &Identity()
	{
		m[0][0]=1;m[0][1]=0;m[0][2]=0;m[0][3]=0;
		m[1][0]=0;m[1][1]=1;m[1][2]=0;m[1][3]=0;
		m[2][0]=0;m[2][1]=0;m[2][2]=1;m[2][3]=0;
		m[3][0]=0;m[3][1]=0;m[3][2]=0;m[3][3]=1;
		return *this;
	}
	//ゼロ行列
	Matrix44 &Zero()
	{
		m[0][0]=0;m[0][1]=0;m[0][2]=0;m[0][3]=0;
		m[1][0]=0;m[1][1]=0;m[1][2]=0;m[1][3]=0;
		m[2][0]=0;m[2][1]=0;m[2][2]=0;m[2][3]=0;
		m[3][0]=0;m[3][1]=0;m[3][2]=0;m[3][3]=0;
		return *this;
	}
	template<class S>
	Matrix44 &Set(S _m[16])
	{
		for(int i=0;i<16;i++){int a=i/4,b=i%4;m[a][b]=_m[i];}
		return *this;
	}
	
	Matrix44 &operator+=(const Matrix44 &_M)
	{
		for(int i=0;i<16;i++){int a=i/4,b=i%4;m[a][b]+=_M.m[a][b];}
		return *this;
	}
	Matrix44 &operator-=(const Matrix44 &_M)
	{
		for(int i=0;i<16;i++){int a=i/4,b=i%4;m[a][b]-=_M.m[a][b];}
		return *this;
	}
	Matrix44 &operator*=(const Matrix44 &_M)
	{
		Matrix44 t;
		for(int i=0;i<4;i++)
		{
			for(int j=0;j<4;j++)
			{
				t.m[i][j]=
					m[i][0]*_M.m[0][j]+
					m[i][1]*_M.m[1][j]+
					m[i][2]*_M.m[2][j]+
					m[i][3]*_M.m[3][j];
			}
		}
		*this=t;
		return *this;
	}
	template<class S>
	Matrix44 &operator*=(S _m)
	{
		for(int i=0;i<16;i++){int a=i/4,b=i%4;m[a][b]*=m;}
		return *this;
	}

	Matrix44 operator+(const Matrix44 &_M)const
	{
		Matrix44 t;
		for(int i=0;i<16;i++){int a=i/4,b=i%4;t.m[a][b]=m[a][b]+_M.m[a][b];}
		return t;
	}
	Matrix44 operator-(const Matrix44 &_M)const
	{
		Matrix44 t;
		for(int i=0;i<16;i++){int a=i/4,b=i%4;t.m[a][b]=m[a][b]-_M.m[a][b];}
		return t;
	}
	Matrix44 operator-()const
	{
		Matrix44 t;
		for(int i=0;i<16;i++){int a=i/4,b=i%4;t.m[a][b]=-m[a][b];}
		return t;
	}
	Matrix44 operator*(const Matrix44 &_M)const
	{
		Matrix44 t;
		for(int i=0;i<4;i++)
		{
			for(int j=0;j<4;j++)
			{
				t.m[i][j]=
					m[i][0]*_M.m[0][j]+
					m[i][1]*_M.m[1][j]+
					m[i][2]*_M.m[2][j]+
					m[i][3]*_M.m[3][j];
			}
		}
		return t;
	}
	template<class S>
	Matrix44 operator*(S _m)const
	{
		Matrix44 t;
		for(int i=0;i<16;i++){int a=i/4,b=i%4;t.m[a][b]=m[a][b]*_m;}
		return t;
	}
	
	template<class S>
	Vector3<T> operator*(const Vector3<S> _v)const
	{
/*		Vector3<T> r(
			m[0][0]*_v.x+m[0][1]*_v.y+m[0][2]*_v.z,
			m[1][0]*_v.x+m[1][1]*_v.y+m[1][2]*_v.z,
			m[2][0]*_v.x+m[2][1]*_v.y+m[2][2]*_v.z
		);
		r.x+=m[3][0];
		r.y+=m[3][1];
		r.z+=m[3][2];
		return r;*/
		return Vector3<T>(
			m[0][0]*_v.x+m[0][1]*_v.y+m[0][2]*_v.z+m[0][3],
			m[1][0]*_v.x+m[1][1]*_v.y+m[1][2]*_v.z+m[1][3],
			m[2][0]*_v.x+m[2][1]*_v.y+m[2][2]*_v.z+m[2][3]
		);
	}
	template<class S>
	Vector3<T> MultVector(const Vector3<S> _v)const
	{
		return Vector3<T>(
			m[0][0]*_v.x+m[0][1]*_v.y+m[0][2]*_v.z,
			m[1][0]*_v.x+m[1][1]*_v.y+m[1][2]*_v.z,
			m[2][0]*_v.x+m[2][1]*_v.y+m[2][2]*_v.z
		);
	}

	bool operator==(const Matrix44 &_M)const
	{
		for(int i=0;i<16;++i){int a=i/4,b=i%4;if(m[a][b]!=_M.m[a][b])return false;}
		return true;
	}
	bool operator!=(const Matrix44 &_M)const
	{
		return !((*this)==_M);
	}
	
	//無理矢理順序づけ
	bool operator<(const Matrix44 &_M)const
	{
		for(int i=0;i<16;++i)
		{
			int a=i/4,b=i%4;
			if(m[a][b]<_M.m[a][b])return true;
			if(m[a][b]>_M.m[a][b])return false;
		}
		return false;
	}

	template<class S>
	void ToArray44(S *_a)const
	{
		int n=0;
		for(int i=0;i<4;i++)
		{
			for(int j=0;j<4;j++,n++)
			{
				_a[n]=m[i][j];
			}
		}
	}
	
	//転置
	Matrix44 Transp()const
	{
		Matrix44 M(*this);
		std::swap(M.m[1][0],M.m[0][1]);
		std::swap(M.m[2][0],M.m[0][2]);
		std::swap(M.m[3][0],M.m[0][3]);
		std::swap(M.m[2][1],M.m[1][2]);
		std::swap(M.m[3][1],M.m[1][3]);
		std::swap(M.m[3][2],M.m[2][3]);

		return M;
	}
	Matrix44 &Transpose()
	{
		std::swap(m[1][0],m[0][1]);
		std::swap(m[2][0],m[0][2]);
		std::swap(m[3][0],m[0][3]);
		std::swap(m[2][1],m[1][2]);
		std::swap(m[3][1],m[1][3]);
		std::swap(m[3][2],m[2][3]);
		return *this;
	}
	
	//トレース
	T Trace()const
	{
		return m[0][0]+m[1][1]+m[2][2]+m[3][3];
	}

	template<class S>
	Matrix44 &Scale(const Vector3<S> &s)
	{
		m[0][0]=s.x;m[0][1]=0;m[0][2]=0;m[0][3]=0;
		m[1][0]=0;m[1][1]=s.y;m[1][2]=0;m[1][3]=0;
		m[2][0]=0;m[2][1]=0;m[2][2]=s.z;m[2][3]=0;
		m[3][0]=0;m[3][1]=0;m[3][2]=0;m[3][3]=1;
		return *this;
	}
	template<class S>
	Matrix44 &Translate(const Vector3<S> &t)
	{
		m[0][0]=1;m[0][1]=0;m[0][2]=0;m[0][3]=0;
		m[1][0]=0;m[1][1]=1;m[1][2]=0;m[1][3]=0;
		m[2][0]=0;m[2][1]=0;m[2][2]=1;m[2][3]=0;
		m[3][0]=t.x;m[3][1]=t.y;m[3][2]=t.z;m[3][3]=1;
		//m[0][0]=1;m[0][1]=0;m[0][2]=0;m[0][3]=t.x;
		//m[1][0]=0;m[1][1]=1;m[1][2]=0;m[1][3]=t.y;
		//m[2][0]=0;m[2][1]=0;m[2][2]=1;m[2][3]=t.z;
		//m[3][0]=0;m[3][1]=0;m[3][2]=0;m[3][3]=1;
		return *this;
	}
	Vector3<T> Translate()const
	{
		return Vector3<T>(
			m[3][0], m[3][1], m[3][2]);
	}

	//not for int
	template<class S>
	Matrix44 &LookAt(const Vector3<S> &eye,const Vector3<S> &look,const Vector3<S> &sky)
	{
		Vector3<S> za=(eye-look).Norm();
		Vector3<S> xa=(sky.ExProd(za)).Norm();
		Vector3<S> ya=za.ExProd(xa);

		S mat[16]={
			xa.x,ya.x,za.x,0,
			xa.y,ya.y,za.y,0,
			xa.z,ya.z,za.z,0,
			-xa.DotProd(eye),-ya.DotProd(eye),-za.DotProd(eye),1
		};
/*		T mat[16]={
			xa.x,ya.x,za.x,-xa.DotProd(eye),
			xa.y,ya.y,za.y,-ya.DotProd(eye),
			xa.z,ya.z,za.z,-za.DotProd(eye),
			0,0,0,1
		};*/
		Set(mat);
		return *this;
	}
	template<class S>
	Matrix44 &Perspective(const S angle,const S aspect,const S zn,const S zf)
	{
		S pi = S(atan(1.0)*4.0);
		S H=2*tan(angle*pi/180/2);
		S W=H*aspect;

		S mat[16]={
			2/W,0,0,0,
			0,2/H,0,0,
			0,0,-2*zf/(zf-zn)+1,-1,
			0,0,-2*zf*zn/(zf-zn),0
		};
		Set(mat);
		return *this;
	}

};

//出力
template<class T>
std::ostream &operator<<(std::ostream &_o,const Matrix44<T> &_m)
{
	_o<<_m.m[0][0]<<","<<_m.m[0][1]<<","<<_m.m[0][2]<<","<<_m.m[0][3]<<std::endl
	  <<_m.m[1][0]<<","<<_m.m[1][1]<<","<<_m.m[1][2]<<","<<_m.m[1][3]<<std::endl
	  <<_m.m[2][0]<<","<<_m.m[2][1]<<","<<_m.m[2][2]<<","<<_m.m[2][3]<<std::endl
	  <<_m.m[3][0]<<","<<_m.m[3][1]<<","<<_m.m[3][2]<<","<<_m.m[3][3]<<std::endl;
	return _o;
}

/////////////////////////////
// 2次元矩形領域

template<class T> struct Rect2
{
public:
	Vector2<T> p1,p2;
	typedef const T cT;
public:
	Rect2<T>(const Vector2<T> &_1,const Vector2<T> &_2):p1(_1),p2(_2){}
	Rect2<T>(const Rect2<T> &_r):p1(_r.p1),p2(_r.p2){}
	Rect2<T>(const T x1,const T y1,const T x2,const T y2):p1(x1,y1),p2(x2,y2){}
	Rect2<T>(){}

	//_vが領域に入っていればtrue、入っていなければfalse
	bool In(const Vector2<T> &_v)const
	{
		/*if(p1.x<=_v.x&&_v.x<=p2.x&&p1.y<=_v.y&&_v.y<=p2.y)
			return true;
		return false;*/
		if(p1.x<=_v.x&&_v.x<p2.x&&p1.y<=_v.y&&_v.y<p2.y)
			return true;
		return false;
	}
	//_rが完全に領域に入っていたらtrue、そうでなければfalse
	bool In(const Rect2<T> &_r)const
	{
		if(In(_r.p1)&&In(_r.p2))return true;
		return false;
	}

	bool operator>=(const Vector2<T> &_v)const{return In(_v);}
	bool operator>=(const Rect2<T> &_r)const{return In(_r);}

	//_rと重なる部分を求める。
	//重なっていればresult.IsRectをとるとtrueになる。
	Rect2<T> Intersect(const Rect2<T> &_r)const
	{
		return Rect2<T>(
			std::max(p1.x,_r.p1.x),std::max(p1.y,_r.p1.y),
			std::min(p2.x,_r.p2.x),std::min(p2.y,_r.p2.y)
		);
	}
	//thisと_rをソートしてからIntersectをとる
	Rect2<T> IntersectSort(const Rect2<T> &_r)const
	{
		Rect2<T> a=*this, b=_r;
		a.AscendingOrder();b.AscendingOrder();
		return a.Intersect(b);
	}
	bool IsIntersect(const Rect2<T> &_r)const
	{
		Rect2<T> a=*this,b=_r;
		a.AscendingOrder();b.AscendingOrder();
		if( (a.p2.x<b.p1.x&&a.p1.x<b.p2.x)||
			(a.p2.x>b.p1.x&&a.p1.x>b.p2.x) )
		{
			return false;
		}
		if( (a.p2.y<b.p1.y&&a.p1.y<b.p2.y)||
			(a.p2.y>b.p1.y&&a.p1.y>b.p2.y) )
		{
			return false;
		}
		return true;
	}
	//_vが領域に入るように広げる。
	void Insert(const Vector2<T> &_v)
	{
		if(p1.x>_v.x){p1.x=_v.x;}
		if(p2.x<_v.x){p2.x=_v.x;}
		if(p1.y>_v.y){p1.y=_v.y;}
		if(p2.y<_v.y){p2.y=_v.y;}
	}
	void Insert(const Rect2<T> &_r)
	{
		Insert(_r.p1);
		Insert(_r.p2);
	}
	Rect2<T> &operator+=(const Vector2<T> &_v)
	{
		Insert(_v);
		return *this;
	}
	Rect2<T> &operator+=(const Rect2<T> &_r)
	{
		Insert(_r);
		return *this;
	}
	Rect2<T> &Offset(const Vector2<T> &_v)
	{
		p1+=_v;p2+=_v;return *this;
	}
	Rect2<T> Ofs(const Vector2<T> &_v)const
	{
		return Rect2<T> (p1+_v,p2+_v);
	}
	//対角線のベクトル
	Vector2<T> Diagonal()const{return p2-p1;}
	//この領域を0-1と仮定したときfで、この領域を分ける
	Rect2<T> Subdiv(const Rect2<T> &_r)const
	{
		Vector2f dthis=Diagonal();
		//Vector2f dsub=_r.Diagonal();

		return Rect2<T>(
			Vector2<T>(p1.x+dthis.x*_r.p1.x,p1.y+dthis.y*_r.p1.y),
			Vector2<T>(p1.x+dthis.x*_r.p2.x,p1.y+dthis.y*_r.p2.y));
	}
	//_v:0-1をこの領域に変換する
	Vector2<T> TransformTo(const Vector2<T> &_v)const
	{
		Vector2<T> dv=_v-p1,dr=Diagonal();
		return Vector2<T>(dv.x/dr.x,dv.y/dr.y);
	}
	//座標値を昇順に並べる
	void AscendingOrder()
	{
		if(p1.x>p2.x)
			std::swap(p1.x,p2.x);
		if(p1.y>p2.y)
			std::swap(p1.y,p2.y);
	}
	//座標値が昇順ならtrue、そうでないならfalse
	bool IsRect()const
	{
		if(p1.x>p2.x)
			return false;
		if(p1.y>p2.y)
			return false;
		return true;
	}
};

//出力
template<class T>
std::ostream &operator<<(std::ostream &_o,const Rect2<T> &_r)
{_o<<"("<<_r.p1<<"),("<<_r.p2<<")";return _o;}


/////////////////////////////
// 3次元矩形領域

template<class T> struct Rect3
{
public:
	Vector3<T> p1,p2;
public:
	Rect3<T>(const Vector3<T> &_1,const Vector3<T> &_2):p1(_1),p2(_2){}
	Rect3<T>(const Rect3<T> &_r):p1(_r.p1),p2(_r.p2){}
	Rect3<T>(const T x1,const T y1,const T z1,const T x2,const T y2,const T z2):p1(x1,y1,z1),p2(x2,y2,z2){}
	Rect3<T>(){}
	//_vが領域に入っていればtrue、入っていなければfalse
	bool In(const Vector3<T> &_v)const
	{
		/*if(p1.x<=_v.x&&_v.x<=p2.x&&
			p1.y<=_v.y&&_v.y<=p2.y&&
			p1.z<=_v.z&&_v.z<=p2.z)
			return true;
		return false;*/
		if(p1.x<=_v.x&&_v.x<p2.x&&
			p1.y<=_v.y&&_v.y<p2.y&&
			p1.z<=_v.z&&_v.z<p2.z)
			return true;
		return false;
	}
	//_rが完全に領域に入っていたらtrue、そうでなければfalse
	bool In(const Rect3<T> &_r)const
	{
		if(In(_r.p1)&&In(_r.p2))return true;
		return false;
	}
	bool operator>=(const Vector3<T> &_v)const{return In(_v);}
	bool operator>=(const Rect3<T> &_r)const{return In(_r);}
	//_rと重なる部分を求める。
	//ただし、thisと_rはIsRectをとるとtrueにならないといけない。
	//重なっていればresult.IsRectをとるとtrueになる。
	Rect3<T> Intersect(const Rect3<T> &_r)const
	{
		return Rect3<T>(
			std::max(p1.x,_r.p1.x),std::max(p1.y,_r.p1.y),std::max(p1.z,_r.p1.z),
			std::min(p2.x,_r.p2.x),std::min(p2.y,_r.p2.y),std::min(p2.z,_r.p2.z)
		);
	}
	//thisと_rをソートしてからIntersectをとる
	Rect3<T> IntersectSort(const Rect3<T> &_r)const
	{
		Rect3<T> a=*this, b=_r;
		a.AscendingOrder();b.AscendingOrder();
		return a.Intersect(b);
	}
	//_vが領域に入るように広げる。
	void Insert(const Vector3<T> &_v)
	{
		if(p1.x>_v.x){p1.x=_v.x;}
		if(p2.x<_v.x){p2.x=_v.x;}
		if(p1.y>_v.y){p1.y=_v.y;}
		if(p2.y<_v.y){p2.y=_v.y;}
		if(p1.z>_v.z){p1.z=_v.z;}
		if(p2.z<_v.z){p2.z=_v.z;}
	}
	//対角線のベクトル
	Vector3<T> Diagonal()const{return p2-p1;}
	//この領域を0-1と仮定したときfで、この領域を分ける
	Rect3<T> Subdiv(const Rect3<T> &_r)const
	{
		Vector3f dthis=Diagonal();

		return Rect3<T>(
			Vector3<T>(p1.x+dthis.x*_r.p1.x,p1.y+dthis.y*_r.p1.y,p1.z+dthis.z*_r.p1.z),
			Vector3<T>(p1.x+dthis.x*_r.p2.x,p1.y+dthis.y*_r.p2.y,p1.z+dthis.z*_r.p2.z));
	}
	//_v:0-1をこの領域に変換する
	Vector3<T> TransformTo(const Vector3<T> &_v)const
	{
		Vector3<T> dv=_v-p1,dr=Diagonal();
		return Vector3<T>(dv.x/dr.x,dv.y/dr.y,dv.z/dr.z);
	}
	Vector3<T> TransformFrom(const Vector3<T> &_v)const
	{
		Vector3<T> dr=Diagonal();
		return Vector3<T>(_v.x*dr.x, _v.y*dr.y, _v.z*dr.z)+p1;
	}
	//座標値を昇順に並べる
	void AscendingOrder()
	{
		if(p1.x>p2.x)
			std::swap<T>(p1.x,p2.x);
		if(p1.y>p2.y)
			std::swap<T>(p1.y,p2.y);
		if(p1.z>p2.z)
			std::swap<T>(p1.z,p2.z);
	}
	//座標値が昇順ならtrue、そうでないならfalse
	bool IsRect()const
	{
		if(p1.x>p2.x)
			return false;
		if(p1.y>p2.y)
			return false;
		if(p1.z>p2.z)
			return false;
		return true;
	}
};

//出力
template<class T>
std::ostream &operator<<(std::ostream &_o,const Rect3<T> &_r)
{_o<<"("<<_r.p1<<"),("<<_r.p2<<")";return _o;}


}//end of namespace Rydot


#endif

