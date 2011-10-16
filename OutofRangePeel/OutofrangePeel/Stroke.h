

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

	double dist;


public:
	Stroke()
	:dist(0.01)
	{
	}



public:
	// v must be on the unit sphere
	bool Add(const Vector3 &v)
	{
		record.push_back(v.Norm());
		return true;
	}



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
			if(simplified.size()>2)
			{
				const Vector3 &bk=simplified.back();
				bool crs=false;
				for(size_t k=1;k<simplified.size()-1;++k)
				{
					const Vector3 &a=simplified[k-1];
					const Vector3 &b=simplified[k];
					Vector3 cross;
					if(Rydot::Spherical_Intersection_ArcArc(
						a,b,bk,p, cross))
					{
						crs=true;
						break;
					}
				}
				if(crs)continue;
			}
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
	
	
	bool Finalize()
	{
		// divide
		std::vector<Vector3> tmp;
		for(size_t i=0;i<simplified.size();++i)
		{
			if(i==0)
			{
				tmp.push_back(simplified[0]);
				continue;
			}
			const Vector3 ds=simplified[i]-simplified[i-1];
			double l=ds.Abs2();
			if(l>dist*2.0)
			{
				int n=ceil(l/dist/4.0);
				for(size_t k=0;k<n;++k)
				{
					tmp.push_back((simplified[i-1]+ds*(double(k+1)/(n))).Norm());
				}
			}
			else
			{
				tmp.push_back(simplified[i]);
			}
		}
		simplified=tmp;
		return true;
	}



	const std::vector<Vector3> &Get()const{return simplified;}



};


