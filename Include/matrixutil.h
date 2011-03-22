
// matrixutil.h
//
// matrix utilities
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// matrix.h�̃N���X���g�������ɗ��������Ȋ֐������Ă����Ƃ���

#pragma once

#ifndef __RYDOT_MATRIX_UTIL_H__
#define __RYDOT_MATRIX_UTIL_H__

//for�̓W�J
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
// �e���v���[�g�֐�

// b,n,t�̃x�N�g������N�H�[�^�j�I���𐶐�����
template<class T>
Quaternion<T> Vector3ToQuaternion(const Vector3<T> &p0,const Vector3<T> &p1,const Vector3<T> &p2)
{
	Vector3<T> v0,v1,v2;

	v0=p1-p0;v1=p2-p1;
	v0.Normalize();
	v2=v1.ExProd(v0).Norm();
	v1=v0.ExProd(v2).Norm();

	//b,n,t�̏��ɐςށB
	Matrix33<T> M(v2,v1,v0);
	Quaternion<T> q=M.ToQuaternion();
	return q.Conj();
}

//�����Ɠ_�Ƃ̋��������߂�
//����p0-p1�Ɠ_c�Ƃ̋���
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

//�����Ɠ_�Ƃ̋��������߂�
//p0-p1��ʂ钼���Ɠ_c�Ƃ̋���
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

//�_�񂩂�o�E���f�B���O�{�b�N�X�����߂�B
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

//���� p+t*v ���O�p�`�ƌ�_������ (�O�p�`�� d0+a*d1+b*d2 (a,b>0 a+b<1))
//p:�����̌��_ v:�����̕���
//p0:�O�p�`�̒��_ d1,d2:p0�ɂ������Ă���O�p�`��2��
//t>=0�Ȃ�1
//t<0�Ȃ�-1
//��_�������Ȃ����0
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

//�_�Ɛ����̋���
//p:�_ p0:�����̌��_ d:p0����̕�
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

//�_�ƎO�p�`�̋���
//p:�_ p0:�O�p�`�̒��_ d1,d2:p0�ɂ������Ă���O�p�`��2��
//�������̏ꍇ�͋����̓}�C�i�X
template<class T>
double Distance_PointTriangle(const Vector3<T> &p,const Vector3<T> &p0,const Vector3<T> &d1,const Vector3<T> &d2)
{
	const Vector3<T> n=d1.ExProd(d2);//�O�p�`�̖@��
	const Vector3<T> nn=n.Norm();//�@���̐��K��
	const Vector3<T> dp=p-p0;

	double dA;
	{
		dA=n.Abs();
		if(dA==0)
		{
			//�O�p�`����Ȃ������B
			//�Ƃ肠�����e�L�g�[�ɂȂ񂩕Ԃ��Ă����B
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

//CatmullRom��Ԃ����߂�B
//Hermite��Ԃ̈���B
//p2��p3�Ԃ�s�̔�ŕ�Ԃ���B
// Q(s)= [(-s3 + 2s2 - s)p1 + (3s3 - 5s2 + 2)p2 + 
//         (-3s3 + 4s2 + s)p3 + (s3 - s2)p4] / 2
template<class T>
Vector3<T> CatmullRom(const Vector3<T> &p1,const Vector3<T> &p2,const Vector3<T> &p3,const Vector3<T> &p4,const double s)
{
	const double t=s*.5;
	const double t2=t*s,t3=t2*s;
	return p1*(-t3+2*t2-t)+p2*(3*t3-5*t2+1)+p3*(-3*t3+4*t2+t)+p4*(t3-t2);
}

//���������߂�B
template<class T>
int sign(T x)
{
	if(x<0)return -1;
	else if(x>0)return 1;
	return 0;
}

//�O�p�`�̖@�������߂�
template<class T>
Vector3<T> TriangleNormal(const Vector3<T> &p1,const Vector3<T> &p2,const Vector3<T> &p3)
{
	Vector3<T> v1=p2-p1;
	Vector3<T> v2=p3-p1;
	return v1.ExProd(v2).Norm();
}

//project vector to plane ���ʂɃx�N�g���𐳎ˉe����
//v(in):vector to project �ˉe����x�N�g��
//n(in):normal unity vector of palne ���ʂ̖@��(�P�ʃx�N�g��)
//returns:vector projected �ˉe�����x�N�g��
template<class T>
Vector3<T> ProjectVector(const Vector3<T> &v,const Vector3<T> &n)
{
	return -v.ExProd(n).ExProd(n);
}

//�~�Ƀ��C�𔽎˂�����
//�~�̓������烌�C���������Ă�����Փ˂��Ȃ��B
// emit:���C�̔����_
// reach:���C�̏Փ˂��Ȃ��ꍇ�̓��B�_
// circle:�~�Bx,y�͒��S���W�Az�͔��a��\��
// collide:�Փ˓_
// reflect:���C�̔��ˌ�̓��B�_
//�߂�l
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
//Reflect�̂Ȃ��o�[�W����
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

//�����Ƀ��C�𔽎˂�����B
//������p1����p2�ɐL�тĂ���A�����̃x�N�g���̍����������˂��Ȃ��B
// emit:���C�̔����_
// reach:���C�̏Փ˂��Ȃ��ꍇ�̓��B�_
// line:p1���n�_�Ap2���I�_��\��
// collide:�Փ˓_
// reflect:���C�̔��ˌ�̓��B�_
//�߂�l
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
//Reflect�̂Ȃ��o�[�W����
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

//�O�p�`�Ƀ��C�𔽎˂�����B
//���C�̔����_���O�p�`�̗�����������no reflection�ɂȂ�B
// emit:���C�̔����_
// reach:���C�̏Փ˂��Ȃ��ꍇ�̓��B�_
// p1,p2,p3:�O�p�`�̒��_
// collide:�Փ˓_
// reflect:���C�̔��ˌ�̓��B�_
//�߂�l
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

	//�O�p�`���������Ȃ�Փ˂��Ȃ�
	const T n_dot_i=n.DotProd(i);
	if(n_dot_i>0)return 0;

	// n dot r = n dot i
	//�܂��An,r,i�͓��ꕽ�ʏ�ɂ���B
	//r=2n*(n dot i)-i
	// n dot i��0�ɋ߂��Ƃ��͕s���肾���A����Z���Ȃ�����s��ɂ͂Ȃ�Ȃ��B

	reflect=n*(2*n_dot_i)-i+collide;

	return 1;
}
//���˂Ȃ��o�[�W����
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

	//�O�p�`���������Ȃ�Փ˂��Ȃ�
	if(n.DotProd(i)>0)return 0;

	return 1;
}
//�Փ˂̌v�Z�̈ʑ��̌W�����������߂�
//coef.x:�Փ˓_��p1-p2�����̈ʒu
//coef.y:�Փ˓_��p1-p3�����̈ʒu
//coef.z:�Փ˓_��emit-reach�����̈ʒu
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

//�ʂ̂Ȃ��p�x(���W�A��)�����߂�
template<class T>
T AngleFace(const Vector3<T>& n1,const Vector3<T>& n2,const Vector3<T> &v)
{
	T alpha=n1.DotProd(n2);
	Vector3<T> beta=n1.ExProd(n2);
	T b=beta.Abs();
	if(beta.DotProd(v)<0)b=-b;
	return atan2((T)b,(T)alpha);
}

//�p�x�ۑ��Ȃ�
//�ӂ����L����2�̎O�p�`��ʂ�Ƃ��ɁA
//2�̎O�p�`�����ʏ�ɂ���Ƃ��ɒ����ƂȂ����
//���̊p�x�ɋȂ������Ƃ��̐��̕��������߂�B
// n1: �����Ă���O�p�`�̖@��
// n2: �o�čs���O�p�`�̖@��
// v:  ���L�ӂ̃x�N�g��
// a:  �����Ă���x�N�g��
// returns: �o�čs���x�N�g��
// n1,n2,v�͊��ɐ��K������Ă�����̂Ƃ���Ba�͐��K������Ȃ��Ă����B
//v��n1Xn2(�̐��K��)�ŋ��܂邪�A
//2�̎O�p�`�����ꕽ�ʂɋ߂��ƒl�����肵�Ȃ��̂ŕK�v�B
//n1Xn2��v�̕���������������Ă���ꍇ�A���͕ۏ�ł��Ȃ��B
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

//�O�p�`�̌������������߂�B
//�O�p�`�̗��\�͋C�ɂ��Ȃ��B
//p:��O�p�`�̒��_�Q
//q:��r�O�p�`�̒��_�Q
//a,b:�o�͐���
//�߂�l:�����Ȃ�true
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
	//��O�p�`�̈ʑ�����������B�L���̈��
	//x>0,y>0,x+y<1
	//    1
	//    |�_
	// y  |  �_
	// �� |    �_
	//    0�P�P�P1
	//       �� x
	
	//u,v is transformed a,b
	Vector3<T> u,v;

	//cross not found
	if(!foundcross)return false;
	//not in base-triangle
	if(cright.x<0)return false;
	if(cleft.x>1)return false;
	if(cleft.y<0&&cright.y<0)return false;
	if(cleft.x+cleft.y>1&&cright.x+cright.y>1)return false;
	
	//���̎��_�ŁAcright.x>=0 , cleft.x<=1 �ł���B

	const T dx=cright.x-cleft.x;//dx>=0
	const T dy=cright.y-cleft.y;
	
	//cleft-cright���c����
	if(dx==0)
	{
		u.x=v.x=cleft.x;
		if(dy>0)//cright����
		{
			if(cright.y<0)return false;
			if(cleft.x+cleft.y>1)return false;
			
			if(cleft.y<0)u.y=0;
			else u.y=cleft.y;
			
			if(cright.x+cright.y>1)v.y=1-v.x;
			else v.y=cright.y;
		}
		else//cleft����
		{
			if(cleft.y<0)return false;
			if(cright.x+cright.y>1)return false;
			
			if(cright.y<0)v.y=0;
			else v.y=cright.y;
			
			if(cleft.x+cleft.y>1)u.y=1-u.x;
			else u.y=cleft.y;
		}
	}
	//cleft-cright��������
	else if(dy==0)
	{
		u.y=v.y=cleft.y;
		if(cleft.x+cleft.y>1)return false;
		
		if(cleft.x<0)u.x=0;
		else u.x=cleft.x;
		
		if(cright.x+cright.y>1)v.x=1-v.y;
		else v.x=cright.x;
	}
	//�΂�
	else
	{
		//a�̈ʑ�u�ɂ���
		
		//cleft is in base-triangle
		if(cleft.x>=0&&cleft.y>=0&&cleft.x+cleft.y<=1)
		{
			u=cleft;
		}
		else
		{
			//���̕ӂ𒲂ׂ�
			T uy=-cleft.x/(dx)*(dy)+cleft.y;
			if(uy>=0&&uy<=1)
			{
				u.x=0;
				u.y=uy;
			}
			//���̕ӂ𒲂ׂ�
			else if(uy<0&&cleft.y<0)
			{
				T ux=-cleft.y/(cright.y-cleft.y)*(cright.x-cleft.x)+cleft.x;
				if(ux<0||ux>1)return false;
				u.x=ux;
				u.y=0;
			}
			//�Εӂ𒲂ׂ�
			else//uy>1
			{
				T D=-1/dy-1/dx;
				if(D==0)return false;//�Εӂƕ��s
	
				//Cramer
				T k=cleft.x/dx-cleft.y/dy;
				T x=(-1/dy-k)/D;
				T y=(k-1/dx)/D;
	
				if(x<0||x>1||y<0||y>1)return false;
				u.x=x;
				u.y=y;
			}
		}//end of a
		
		//b�̈ʑ�v�ɂ���
		
		//cright is in base-triangle
		if(cright.x>=0&&cright.y>=0&&cright.x+cright.y<=1)
		{
			v=cright;
		}
		else
		{
			//���̕ӂ𒲂ׂ�
			T vx=-cleft.y/(cright.y-cleft.y)*(cright.x-cleft.x)+cleft.x;
			if(vx>=0&&vx<=1&&cright.y<0)
			{
				v.x=vx;
				v.y=0;
			}
			//�Εӂ𒲂ׂ�
			else
			{
				T D=-1/dy-1/dx;
				if(D==0)return false;//�Εӂƕ��s
				
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

//�o�C���j�A���
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

