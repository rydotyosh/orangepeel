
// matrixutil.h
//
// matrix utilities
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// matrix.hのクラスを使った役に立ちそうな関数を入れておくところ

#pragma once

#ifndef __RYDOT_MATRIX_UTIL_H__
#define __RYDOT_MATRIX_UTIL_H__

//forの展開
//#pragma warning( disable : 8027 )

#include "matrix.h"

#include <math.h>
#include <algorithm>
#include <ostream>
#include <vector>
#include <list>

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

/////////////////////////////
// テンプレート関数

// b,n,tのベクトルからクォータニオンを生成する
template<class T>
Quaternion<T> Vector3ToQuaternion(const Vector3<T> &p0,const Vector3<T> &p1,const Vector3<T> &p2)
{
	Vector3<T> v0,v1,v2;

	v0=p1-p0;v1=p2-p1;
	v0.Normalize();
	v2=v1.ExProd(v0).Norm();
	v1=v0.ExProd(v2).Norm();

	//b,n,tの順に積む。
	Matrix33<T> M(v2,v1,v0);
	Quaternion<T> q=M.ToQuaternion();
	return q.Conj();
}

//線分と点との距離を求める
//線分p0-p1と点cとの距離
template<class T>
double DistLineToPoint3(const Vector3<T> &p0,const Vector3<T> &p1,const Vector3<T> &c)
{
	Vector3<T> d1=p1-p0;
	if(d1==Vector3<T>(0,0,0))
		return p0.Dist(c);
	Vector3<T> d2=c-p0;
	double ab2=d1.Abs2();
	double npm=d1.DotProd(d2)/ab2;
	if(npm<0)return p0.Dist(c);
	else if(npm>1)return p1.Dist(c);
	else return c.Dist(p0+d1*npm);
}
template<class T>
double DistLineToPoint2(const Vector2<T> &p0,const Vector2<T> &p1,const Vector2<T> &c)
{
	Vector2<T> d1=p1-p0;
	if(d1==Vector2<T>(0,0))
		return p0.Dist(c);
	Vector2<T> d2=c-p0;
	double ab2=d1.Abs2();
	double npm=d1.DotProd(d2)/ab2;
	if(npm<0)return p0.Dist(c);
	else if(npm>1)return p1.Dist(c);
	else return c.Dist(p0+d1*npm);
}

//直線と点との距離を求める
//p0-p1を通る直線と点cとの距離
template<class T>
double DistLineToPointPrim3(const Vector3<T> &p0,const Vector3<T> &p1,const Vector3<T> &c)
{
	Vector3<T> d1=p1-p0;
	if(d1==Vector3<T>(0,0,0))
		return p0.Dist(c);
	Vector3<T> d2=c-p0;
	double ab2=d1.Abs2();
	double npm=d1.DotProd(d2)/ab2;
	return c.Dist(p0+d1*npm);
}
template<class T>
double DistLineToPointPrim2(const Vector2<T> &p0,const Vector2<T> &p1,const Vector2<T> &c)
{
	Vector2<T> d1=p1-p0;
	if(d1==Vector2<T>(0,0))
		return p0.Dist(c);
	Vector2<T> d2=c-p0;
	double ab2=d1.Abs2();
	double npm=d1.DotProd(d2)/ab2;
	return c.Dist(p0+d1*npm);
}

//点列からバウンディングボックスを求める。
template<class T>
int BoundingBox(const std::vector<Vector3<T> > &p,Rect3<T> &out)
{
	if(p.empty())return -1;
	out.p1=p[0];out.p2=p[0];
	int sz=p.size();
	for(int i=1;i<sz;++i)
	{
		out.Insert(p[i]);
	}
	return 0;
}

//直線 p+t*v が三角形と交点を持つか (三角形は d0+a*d1+b*d2 (a,b>0 a+b<1))
//p:直線の原点 v:直線の方向
//p0:三角形の頂点 d1,d2:p0にくっついている三角形の2辺
//t>=0なら1
//t<0なら-1
//交点を持たなければ0
template<class T>
int Intersection_LineTriangle(const Vector3<T> &p,const Vector3<T> &v,const Vector3<T> &p0,const Vector3<T> &d1,const Vector3<T> &d2)
{
	const Vector3<T> dp=p-p0;
	float dA;
	{
		const Matrix33<T> A(d1,d2,v);
		dA=A.Determinant();
		if(dA==0)return 0;
	}
	float a;
	{
		const Matrix33<T> M(dp,d2,v);
		a=M.Determinant()/dA;
		if(a<0)return 0;
	}
	float b;
	{
		const Matrix33<T> M(d1,dp,v);
		b=M.Determinant()/dA;
		if(b<0)return 0;
	}
	if(a+b>1)return 0;
	
	{
		const Matrix33<T> M(d1,d2,dp);
		double t=-M.Determinant()/dA;

		if(t>=0)return 1;
		return -1;
	}
}

//点と線分の距離
//p:点 p0:線分の原点 d:p0からの辺
template<class T>
double Distance_PointLine(const Vector3<T> &p,const Vector3<T> &p0,const Vector3<T> &d)
{
	double dd=d.Abs2();
	if(dd==0)
	{
		return p.Dist(p0);
	}
	Vector3<T> q=p-p0;
	double t=q.DotProd(d); t/=dd;
	if(t<0)
	{
		return p.Dist(p0);
	}
	if(t>1)
	{
		return p.Dist(p0+d);
	}
	Vector3<T> a=d; a*=t; a+=p0; a-=p;
	return a.Abs();
}

//点と三角形の距離
//p:点 p0:三角形の頂点 d1,d2:p0にくっついている三角形の2辺
//裏向きの場合は距離はマイナス
template<class T>
double Distance_PointTriangle(const Vector3<T> &p,const Vector3<T> &p0,const Vector3<T> &d1,const Vector3<T> &d2)
{
	const Vector3<T> n=d1.ExProd(d2);//三角形の法線
	const Vector3<T> nn=n.Norm();//法線の正規化
	const Vector3<T> dp=p-p0;

	double dA;
	{
		dA=n.Abs();
		if(dA==0)
		{
			//三角形じゃない感じ。
			//とりあえずテキトーになんか返しておく。
			return dp.Abs();
		}
	}
	float a;
	{
		const Matrix33<T> M(dp,d2,nn);
		a=M.Determinant()/dA;
		if(a<0)
		{
			return Distance_PointLine(p,p0,d2);
		}
	}
	float b;
	{
		const Matrix33<T> M(d1,dp,nn);
		b=M.Determinant()/dA;
		if(b<0)
		{
			return Distance_PointLine(p,p0,d1);
		}
	}
	if(a+b>1)
	{
		return Distance_PointLine(p,p0+d1,d2-d1);
	}
	
	{
		const Matrix33<T> M(d1,d2,dp);
		double t=-M.Determinant()/dA;
		return t;
	}
}

//CatmullRom補間を求める。
//Hermite補間の亜種。
//p2とp3間をsの比で補間する。
// Q(s)= [(-s3 + 2s2 - s)p1 + (3s3 - 5s2 + 2)p2 + 
//         (-3s3 + 4s2 + s)p3 + (s3 - s2)p4] / 2
template<class T>
Vector3<T> CatmullRom(const Vector3<T> &p1,const Vector3<T> &p2,const Vector3<T> &p3,const Vector3<T> &p4,const double s)
{
	const double t=s*.5;
	const double t2=t*s,t3=t2*s;
	return p1*(-t3+2*t2-t)+p2*(3*t3-5*t2+1)+p3*(-3*t3+4*t2+t)+p4*(t3-t2);
}

//符号を求める。
template<class T>
int sign(T x)
{
	if(x<0)return -1;
	else if(x>0)return 1;
	return 0;
}

//三角形の法線を求める
template<class T>
Vector3<T> TriangleNormal(const Vector3<T> &p1,const Vector3<T> &p2,const Vector3<T> &p3)
{
	Vector3<T> v1=p2-p1;
	Vector3<T> v2=p3-p1;
	return v1.ExProd(v2).Norm();
}

//project vector to plane 平面にベクトルを正射影する
//v(in):vector to project 射影するベクトル
//n(in):normal unity vector of palne 平面の法線(単位ベクトル)
//returns:vector projected 射影したベクトル
template<class T>
Vector3<T> ProjectVector(const Vector3<T> &v,const Vector3<T> &n)
{
	return -v.ExProd(n).ExProd(n);
}

//円にレイを反射させる
//円の内側からレイが発生していたら衝突しない。
// emit:レイの発生点
// reach:レイの衝突しない場合の到達点
// circle:円。x,yは中心座標、zは半径を表す
// collide:衝突点
// reflect:レイの反射後の到達点
//戻り値
// 0:no reflection
// 1:reflected
template<class T>
int ReflectCircleRay(const Vector2<T> &emit,const Vector2<T> &reach,
	const Vector3<T> &circle,
	Vector2<T> &collide,Vector2<T> &reflect)
{
	Vector2<T> center(circle.x,circle.y);
	const double radiusinv=1/circle.z;
	Vector2<T> A=(emit-center)*radiusinv,B=(reach-center)*radiusinv;

	//Ar-Br, Ai-Bi
	const Vector2<T> AB=A-B;
	//|R'|
	const double R=AB.Abs();
	const double Rinv=1/R;
	//Distance of CV
	const double d=AB.ExProd(A)*Rinv;
	
	if(d<-1 || 1<d) return 0;
	
	const double e=sqrt(1-d*d);
	const double f=B.DotProd(AB)*Rinv;
	if(f>e || f+R<-e)return 0;
	const double g=A.DotProd(AB)*Rinv;
	if(fabs(g)<e)return 0;
	
	// collide
	collide=Vector2<T>(e,d)*AB*(Rinv*circle.z)+center;
	
	// reflect
	Vector2<T> s=Vector2<T>(e*e-d*d,2*e*d)*AB;
	s.Normalize();
	s*=circle.z;//radius
	
	reflect=s*(e-f)+collide;
	
	return 1;
}
//Reflectのないバージョン
template<class T>
int ReflectCircleRay(const Vector2<T> &emit,const Vector2<T> &reach,
	const Vector3<T> &circle,
	Vector2<T> &collide)
{
	Vector2<T> center(circle.x,circle.y);
	double radiusinv=1/circle.z;
	const Vector2<T> A=(emit-center)*radiusinv,B=(reach-center)*radiusinv;
	//Ar-Br, Ai-Bi
	const Vector2<T> AB=A-B;
	//|R'|
	const double R=AB.Abs();
	const double Rinv=1/R;
	//Distance of CV
	const double d=AB.ExProd(A)*Rinv;
	
	if(d<-1 || 1<d) return 0;
	
	const double e=sqrt(1-d*d);
	const double f=B.DotProd(AB)*Rinv;
	if(f>e || f+R<-e)return 0;
	
	// collide
	collide=Vector2<T>(e,d)*AB*(Rinv*circle.z)+center;
	return 1;
}

//線分にレイを反射させる。
//線分はp1からp2に伸びており、線分のベクトルの左側しか反射しない。
// emit:レイの発生点
// reach:レイの衝突しない場合の到達点
// line:p1が始点、p2が終点を表す
// collide:衝突点
// reflect:レイの反射後の到達点
//戻り値
// 0:no reflection
// 1:reflected
template<class T>
int ReflectLineRay(const Vector2<T> &emit,const Vector2<T> &reach,
	const Rect2<T> &line,
	Vector2<T> &collide,Vector2<T> &reflect)
{
	const Vector2<T> A=emit-line.p1,B=reach-line.p1,Q=line.p2-line.p1;
	//Ai", Bi"
	const T Aidd=Q.ExProd(A);
	const T Bidd=Q.ExProd(B);
	
	if(Aidd<0 || Bidd>0) return 0;
	
	//Cr'
	const Vector2<T> Cd(B.ExProd(A)/Q.ExProd(A-B),0);
	
	if(Cd.x<0 || 1<Cd.x) return 0;
	
	collide=Q*Cd+line.p1;
	
	const T Q2mQ2=Q.x*Q.x-Q.y*Q.y,QQ=2*Q.x*Q.y,Q2pQ2=Q.Abs2();
	reflect.x=(B.x*Q2mQ2+B.y*QQ)/Q2pQ2;
	reflect.y=(-B.y*Q2mQ2+B.x*QQ)/Q2pQ2;
	reflect+=line.p1;
	
	return 1;
}
//Reflectのないバージョン
template<class T>
int ReflectLineRay(const Vector2<T> &emit,const Vector2<T> &reach,
	const Rect2<T> &line,
	Vector2<T> &collide)
{
	const Vector2<T> A=emit-line.p1,B=reach-line.p1,Q=line.p2-line.p1;
	//Ai", Bi"
	const T Aidd=Q.ExProd(A);
	const T Bidd=Q.ExProd(B);
	
	if(Aidd<0 || Bidd>0) return 0;
	
	//Cr'
	const Vector2<T> Cd(B.ExProd(A)/Q.ExProd(A-B),0);
	
	if(Cd.x<0 || 1<Cd.x) return 0;
	
	collide=Q*Cd+line.p1;
	return 1;
}

//三角形にレイを反射させる。
//レイの発生点が三角形の裏側だったらno reflectionになる。
// emit:レイの発生点
// reach:レイの衝突しない場合の到達点
// p1,p2,p3:三角形の頂点
// collide:衝突点
// reflect:レイの反射後の到達点
//戻り値
// 0:no reflection
// 1:reflected
template<class T>
int ReflectTriangleRay(const Vector3<T> &emit,const Vector3<T> &reach,
	const Vector3<T> &p1,const Vector3<T> &p2,const Vector3<T> &p3,
	Vector3<T> &collide,Vector3<T> &reflect)
{
	const Matrix33<T> M(p2-p1,p3-p1,emit-reach);
	
	//M.Invert();
	//Vector3<T> v=M*(emit-p1);
	if(M.Determinant()==0)return 0;
	const Vector3<T> v=M.Solve(emit-p1);
	
	if(v.x<0 || v.y<0 || v.x+v.y>1)return 0;
	if(v.z<0 || v.z>1)return 0;
	collide=emit+(reach-emit)*v.z;
	
	const Vector3<T> i=collide-reach;
	const Vector3<T> n=TriangleNormal<T>(p1,p2,p3);

	//三角形が裏向きなら衝突しない
	const T n_dot_i=n.DotProd(i);
	if(n_dot_i>0)return 0;

	// n dot r = n dot i
	//また、n,r,iは同一平面上にある。
	//r=2n*(n dot i)-i
	// n dot iが0に近いときは不安定だが、割り算がないから不定にはならない。

	reflect=n*(2*n_dot_i)-i+collide;

	return 1;
}
//反射なしバージョン
template<class T>
int ReflectTriangleRay(const Vector3<T> &emit,const Vector3<T> &reach,
	const Vector3<T> &p1,const Vector3<T> &p2,const Vector3<T> &p3,
	Vector3<T> &collide)
{
	const Matrix33<T> M(p2-p1,p3-p1,emit-reach);
	//M.Invert();
	//Vector3<T> v=M*(emit-p1);
	if(M.Determinant()==0)return 0;
	const Vector3<T> v=M.Solve(emit-p1);

	if(v.x<0 || v.y<0 || v.x+v.y>1)return 0;
	if(v.z<0 || v.z>1)return 0;
	collide=emit+(reach-emit)*v.z;
	
	const Vector3<T> i=collide-reach;
	const Vector3<T> n=TriangleNormal<T>(p1,p2,p3);

	//三角形が裏向きなら衝突しない
	if(n.DotProd(i)>0)return 0;

	return 1;
}
//衝突の計算の位相の係数だけを求める
//coef.x:衝突点のp1-p2方向の位置
//coef.y:衝突点のp1-p3方向の位置
//coef.z:衝突点のemit-reach方向の位置
template<class T>
int ReflectTriangleRayTopology(const Vector3<T> &emit,const Vector3<T> &reach,
	const Vector3<T> &p1,const Vector3<T> &p2,const Vector3<T> &p3,
	Vector3<T> &coef)
{
	const Matrix33<T> M(p2-p1,p3-p1,emit-reach);
	//M.Invert();
	//coef=M*(emit-p1);
	
	if(M.Determinant()==0)return 0;
	coef=M.Solve(emit-p1);
	
	return 1;
}

//面のなす角度(ラジアン)を求める
template<class T>
T AngleFace(const Vector3<T>& n1,const Vector3<T>& n2,const Vector3<T> &v)
{
	T alpha=n1.DotProd(n2);
	Vector3<T> beta=n1.ExProd(n2);
	T b=beta.Abs();
	if(beta.DotProd(v)<0)b=-b;
	return atan2((T)b,(T)alpha);
}

//角度保存曲げ
//辺を共有した2つの三角形を通るときに、
//2つの三角形が平面上にあるときに直線となる線が
//今の角度に曲がったときの線の方向を求める。
// n1: 入ってくる三角形の法線
// n2: 出て行く三角形の法線
// v:  共有辺のベクトル
// a:  入ってくるベクトル
// returns: 出て行くベクトル
// n1,n2,vは既に正規化されているものとする。aは正規化じゃなくていい。
//vはn1Xn2(の正規化)で求まるが、
//2つの三角形が同一平面に近いと値が安定しないので必要。
//n1Xn2とvの方向が著しくずれている場合、解は保障できない。
template<class T>
Vector3<T> AngleSavedBend
 (const Vector3<T>& n1,const Vector3<T>& n2,
  const Vector3<T>& v,const Vector3<T>& a)
{
	T alpha=n1.DotProd(n2);
	Vector3<T> beta=n1.ExProd(n2);
	T fai=atan2((T)(beta.Abs()),(T)alpha);

	// alpha=n1.n2=cos fai
	// beta =n1Xn2=sin fai * v   (|v|=1)
	// fai=arctan( |beta| / alpha )
	// q'v=sin fai/2 * v
	// q'w=cos fai/2
	// b=q' a q'~

	if(beta.DotProd(v)<0)beta=-v;else beta=v;
	fai*=0.5;
	alpha=cos(fai);
	beta*=sin(fai);
	Quaternion<T> q(beta,alpha);
	return q.Rotate(a);
}

//三角形の交差線分を求める。
//三角形の裏表は気にしない。
//p:基準三角形の頂点群
//q:比較三角形の頂点群
//a,b:出力線分
//戻り値:成功ならtrue
template<class T>
bool TriangleCrossTriangle
(const Vector3<T>& p1,const Vector3<T>& p2,const Vector3<T>& p3,
 const Vector3<T>& q1,const Vector3<T>& q2,const Vector3<T>& q3,
 Vector3<T>& a,Vector3<T> &b)
{	
	Vector3<T> c[3];
	if(!ReflectTriangleRayTopology(q1,q2,p1,p2,p3,c[0])){c[0].z=-1;}
	if(!ReflectTriangleRayTopology(q2,q3,p1,p2,p3,c[1])){c[1].z=-1;}
	if(!ReflectTriangleRayTopology(q3,q1,p1,p2,p3,c[2])){c[2].z=-1;}
	
	//check c.z and get min/max of c.x -> cleft,cright
	Vector3<T> cleft,cright;

	bool foundcross=false;
	for(int i=0;i<3;i++)
	{
		if(c[i].z<0||c[i].z>1)continue;
		if(!foundcross)
		{
			cleft=cright=c[i];
			foundcross=true;
		}
		else
		{
			if(c[i].x<cleft.x)cleft=c[i];
			if(c[i].x>cright.x)cright=c[i];
		}
	}
	//基準三角形の位相成分を見る。有効領域は
	//x>0,y>0,x+y<1
	//    1
	//    |＼
	// y  |  ＼
	// ↑ |    ＼
	//    0￣￣￣1
	//       → x
	
	//u,v is transformed a,b
	Vector3<T> u,v;

	//cross not found
	if(!foundcross)return false;
	//not in base-triangle
	if(cright.x<0)return false;
	if(cleft.x>1)return false;
	if(cleft.y<0&&cright.y<0)return false;
	if(cleft.x+cleft.y>1&&cright.x+cright.y>1)return false;
	
	//この時点で、cright.x>=0 , cleft.x<=1 である。

	const T dx=cright.x-cleft.x;//dx>=0
	const T dy=cright.y-cleft.y;
	
	//cleft-crightが縦向き
	if(dx==0)
	{
		u.x=v.x=cleft.x;
		if(dy>0)//crightが上
		{
			if(cright.y<0)return false;
			if(cleft.x+cleft.y>1)return false;
			
			if(cleft.y<0)u.y=0;
			else u.y=cleft.y;
			
			if(cright.x+cright.y>1)v.y=1-v.x;
			else v.y=cright.y;
		}
		else//cleftが上
		{
			if(cleft.y<0)return false;
			if(cright.x+cright.y>1)return false;
			
			if(cright.y<0)v.y=0;
			else v.y=cright.y;
			
			if(cleft.x+cleft.y>1)u.y=1-u.x;
			else u.y=cleft.y;
		}
	}
	//cleft-crightが横向き
	else if(dy==0)
	{
		u.y=v.y=cleft.y;
		if(cleft.x+cleft.y>1)return false;
		
		if(cleft.x<0)u.x=0;
		else u.x=cleft.x;
		
		if(cright.x+cright.y>1)v.x=1-v.y;
		else v.x=cright.x;
	}
	//斜め
	else
	{
		//aの位相uについて
		
		//cleft is in base-triangle
		if(cleft.x>=0&&cleft.y>=0&&cleft.x+cleft.y<=1)
		{
			u=cleft;
		}
		else
		{
			//左の辺を調べる
			T uy=-cleft.x/(dx)*(dy)+cleft.y;
			if(uy>=0&&uy<=1)
			{
				u.x=0;
				u.y=uy;
			}
			//下の辺を調べる
			else if(uy<0&&cleft.y<0)
			{
				T ux=-cleft.y/(cright.y-cleft.y)*(cright.x-cleft.x)+cleft.x;
				if(ux<0||ux>1)return false;
				u.x=ux;
				u.y=0;
			}
			//斜辺を調べる
			else//uy>1
			{
				T D=-1/dy-1/dx;
				if(D==0)return false;//斜辺と平行
	
				//Cramer
				T k=cleft.x/dx-cleft.y/dy;
				T x=(-1/dy-k)/D;
				T y=(k-1/dx)/D;
	
				if(x<0||x>1||y<0||y>1)return false;
				u.x=x;
				u.y=y;
			}
		}//end of a
		
		//bの位相vについて
		
		//cright is in base-triangle
		if(cright.x>=0&&cright.y>=0&&cright.x+cright.y<=1)
		{
			v=cright;
		}
		else
		{
			//下の辺を調べる
			T vx=-cleft.y/(cright.y-cleft.y)*(cright.x-cleft.x)+cleft.x;
			if(vx>=0&&vx<=1&&cright.y<0)
			{
				v.x=vx;
				v.y=0;
			}
			//斜辺を調べる
			else
			{
				T D=-1/dy-1/dx;
				if(D==0)return false;//斜辺と平行
				
				//Cramer
				T k=cleft.x/dx-cleft.y/dy;
				T x=(-1/dy-k)/D;
				T y=(k-1/dx)/D;
				
				if(x<0||x>1||y<0||y>1)return false;
				v.x=x;
				v.y=y;
			}
		}//end of b
		
	}
	
	a=(p2-p1)*u.x+(p3-p1)*u.y+p1;
	b=(p2-p1)*v.x+(p3-p1)*v.y+p1;
	
	return true;
}

//バイリニア補間
// p1 a p2
//    b
// p3 a p4
template<class T>
Vector2<T> Bilinear
(const Vector2<T>& p1,const Vector2<T>& p2,const Vector2<T>& p3,const Vector2<T>& p4,
 const double a,const double b)
{
	return (p1*(1-a)+p2*a)*(1-b)+(p3*(1-a)+p4*a)*b;
}
template<class T>
Vector3<T> Bilinear
(const Vector3<T>& p1,const Vector3<T>& p2,const Vector3<T>& p3,const Vector3<T>& p4,
 const double a,const double b)
{
	return (p1*(1-a)+p2*a)*(1-b)+(p3*(1-a)+p4*a)*b;
}
template<class T>
Quaternion<T> Bilinear
(const Quaternion<T>& p1,const Quaternion<T>& p2,const Quaternion<T>& p3,const Quaternion<T>& p4,
 const double a,const double b)
{
	return (p1*(1-a)+p2*a)*(1-b)+(p3*(1-a)+p4*a)*b;
}

}//end of namespace Rydot

#endif

