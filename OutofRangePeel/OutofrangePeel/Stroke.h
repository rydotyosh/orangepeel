

#pragma once

#include <math.h>
#include "Events.h"
#include "matrix.h"
#include "matrixutil.h"
#include "dashpod.h"



class Stroke
{
	typedef float Float;
	typedef Rydot::Vector2f	Vector2;
	typedef Rydot::Vector3f	Vector3;

	std::vector<Vector3> record;
	std::vector<Vector3> simplified;

	std::vector<Vector3> tess;
	std::vector<std::vector<int> > edges;// [][tess index]

	struct Vertex
	{
		int tessIndex;
		std::vector<int> eges;// [edge index]
	};
	std::vector<Vertex> vertices;


	double dist;


public:
	Stroke()
	:dist(0.005)
	{
	}



public:
	// v must be on the unit sphere
	bool Add(const Vector3 &v)
	{
		record.push_back(v.Norm());
		Simplify();
		return true;
	}


private:
	bool Simplify()
	{
		simplified.clear();

		for(size_t i=0;i<record.size();++i)
		{
			if(i==0)
			{
				simplified.push_back(record[0]);
				continue;
			}
			Vector3 p = (record[i]+record[i-1])*0.5;
			if(simplified.back().Dist2(p)<dist)continue;
			//check cross
			//if(simplified.size()>2)
			//{
			//	const Vector3 &bk=simplified.back();
			//	bool crs=false;
			//	for(size_t k=1;k<simplified.size()-1;++k)
			//	{
			//		const Vector3 &a=simplified[k-1];
			//		const Vector3 &b=simplified[k];
			//		Vector3 cross;
			//		if(Rydot::Spherical_Intersection_ArcArc(
			//			a,b,bk,p, cross))
			//		{
			//			crs=true;
			//			break;
			//		}
			//	}
			//	if(crs)continue;
			//}
			simplified.push_back(p);
		}
		if(!record.empty())
		{
			if(simplified.size()>=2 && simplified.back().Dist2(record.back())<dist)
				simplified.back()=record.back();
			else
				simplified.push_back(record.back());
		}
		return true;
	}


private:
	std::vector<Vector3> Divide(const std::vector<Vector3> &poly)
	{
		std::vector<Vector3> tmp;
		for(size_t i = 0; i < poly.size(); ++i)
		{
			if(i == 0)
			{
				tmp.push_back(poly[0]);
				continue;
			}
			const Vector3 ds = poly[i] - poly[i - 1];
			double l = ds.Abs2();
			if(l > dist * 2.0)
			{
				int n=(int)ceil(l / (dist * 4.0));
				for(int k = 0; k < n; ++k)
				{
					tmp.push_back((poly[i - 1] + ds * (float(k + 1) / n)).Norm());
				}
			}
			else
			{
				tmp.push_back(poly[i]);
			}
		}
		return tmp;
	}

private:
	bool Finalize()
	{
		simplified = Divide(simplified);
		return true;
	}

public:
	void ExtractCrossPoints()
	{
		struct ijc
		{
			int i;
			int j;
			Vector3 cross;
			ijc(){}
			ijc(int i_, int j_, const Vector3 &c_):i(i_), j(j_), cross(c_){}
		};
		std::vector<ijc> crosses;
		for(int i = 0; i + 2 < simplified.size(); ++i)
		{
			const Vector3 &a = simplified[i];
			const Vector3 &b = simplified[i + 1];
			for(int j = i + 2; j + 1 < simplified.size(); ++j)
			{
				const Vector3 &c = simplified[j];
				const Vector3 &d = simplified[j + 1];
				Vector3 cross;
				if(Rydot::Spherical_Intersection_ArcArc(
					a,b,c,d, cross))
				{
					crosses.push_back(ijc(i, j, cross));
				}
			}
		}
		
		std::vector<Vector3> ts;
		std::vector<std::vector<int> > ed;// [][tess index]
		std::vector<Vertex> vx;

		

	}


	// number of segments
public:
	const int Size()const
	{
		return 1;
	}

public:
	void Open()
	{
		record.clear();
		simplified.clear();
	}

public:
	void Close()
	{

	}

public:
	void Optimize()
	{
		Simplify();
		Finalize();
	}

	// get segment #i
public:
	const std::vector<Vector3> &GetSeg(int i)const{return simplified;}



};


