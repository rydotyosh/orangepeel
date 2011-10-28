

#pragma once

#include <math.h>
#include <map>
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

	struct Edge
	{
		int edgeId;
		std::vector<int> tessIndex;
		Edge(){}
		Edge(int eid, const std::vector<int> &ts):edgeId(eid), tessIndex(ts){}
	};
	std::vector<Edge> edges;

	struct Vertex
	{
		int tessIndex;
		std::vector<int> edges;// [edge index]
		Vertex(){}
		Vertex(int ti_, const std::vector<int> &eds_):tessIndex(ti_), edges(eds_){}
	};
	std::vector<Vertex> vertices;

	std::vector< std::vector<Vector3> > baked;

	double dist;
	int scratchNum;
	double scratchThreshold;


public:
	Stroke()
	: dist(0.002)
	, scratchNum(2)
	, scratchThreshold(0.8)
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

public:
	bool IsScratch()
	{
		Simplify();
		std::vector<Vector3> &pts = simplified;
		// has N or more peak edge
		const int N = scratchNum;
		int peaks = 0;
		for(size_t i = 0;i + 2 < pts.size(); ++i)
		{
			const Vector3 &p = pts[i];
			const Vector3 &q = pts[i + 1];
			const Vector3 &r = pts[i + 2];
			Vector3 dp = q - p;
			Vector3 dr = r - q;
			if(dp.Norm().DotProd(dr.Norm()) < -scratchThreshold)
			{
				++peaks;
				if(peaks >= N)
					return true;
			}
		}
		return false;
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

	// cross point info
private:
	struct ijc
	{
		int i;
		int j;
		Vector3 cross;
		double iparam;
		double jparam;
		ijc(){}
		ijc(int i_, int j_, const Vector3 &c_, double _iparam, double _jparam)
			:i(i_)
			, j(j_)
			, cross(c_)
			, iparam(_iparam)
			, jparam(_jparam){}
	};

private:
	struct ijc2
	{
		int ei;
		int i;
		int ej;
		int j;
		Vector3 cross;
		double iparam;
		double jparam;
		ijc2(){}
		ijc2(int ei_, int i_, int ej_, int j_, const Vector3 &c_, double _iparam, double _jparam)
			: ei(ei_)
			, i(i_)
			, ej(ej_)
			, j(j_)
			, cross(c_)
			, iparam(_iparam)
			, jparam(_jparam){}
	};


	// half cross info
private:
	struct kc
	{
		int crossindex;
		double iparam;
		kc(int ci_, double ip_)
			: crossindex(ci_)
			, iparam(ip_)
		{}
		bool operator<(const kc &x)const{return iparam < x.iparam;}
	};

	struct kc2
	{
		int edgeindex;
		int crossindex;
		double iparam;
		kc2(int ei_, int ci_, double ip_)
			: edgeindex(ei_)
			, crossindex(ci_)
			, iparam(ip_)
		{}
		bool operator<(const kc2 &x)const{return iparam < x.iparam;}
	};

	struct eii
	{
		int ei;
		int i;
		eii(){}
		eii(int ei_, int i_):ei(ei_), i(i_){}
		bool operator<(const eii &x)const
		{
			if(ei < x.ei)return true;
			if(ei > x.ei)return false;
			if(i < x.i)return true;
			return false;
		}
	};

public:
	void DecomposeSelfCross(std::vector<Vector3> &ts, std::vector< std::vector<int> > &edg)
	{
		std::vector<ijc> crosses;
		for(int i = 0; i + 2 < (int)simplified.size(); ++i)
		{
			const Vector3 &a = simplified[i];
			const Vector3 &b = simplified[i + 1];
			for(int j = i + 2; j + 1 < (int)simplified.size(); ++j)
			{
				const Vector3 &c = simplified[j];
				const Vector3 &d = simplified[j + 1];
				Vector3 cross;
				if(Rydot::Spherical_Intersection_ArcArc(
					a,b,c,d, cross))
				{
					const Vector3 da = cross - a;
					const Vector3 db = b - a;
					const Vector3 dc = cross - c;
					const Vector3 dd = d - c;

					crosses.push_back(ijc(i, j, cross, da.DotProd(db)/da.Abs2(), dc.DotProd(dd)/dc.Abs2()));
				}
			}
		}

		ts = simplified;
		for(int i = 0; i < (int)crosses.size(); ++i)
		{
			ts.push_back(crosses[i].cross);
		}

		std::vector<std::vector<kc> > halfcross(simplified.size());
		for(int i = 0; i < (int)crosses.size(); ++i)
		{
			const ijc &x = crosses[i];
			halfcross[x.i].push_back(kc((int)simplified.size() + i, x.iparam));
			halfcross[x.j].push_back(kc((int)simplified.size() + i, x.jparam));
		}

		for(int i = 0; i < (int)halfcross.size(); ++i)
		{
			std::vector<kc> &x = halfcross[i];
			std::sort(x.begin(), x.end());
		}

		edg.clear();// [][tess index]
		edg.push_back(std::vector<int>());
		for(int i = 0; i + 1 < (int)simplified.size(); ++i)
		{
			const std::vector<kc> &hci = halfcross[i];
			if(hci.empty())
			{
				edg.back().push_back(i);
				continue;
			}

			edg.back().push_back(i);
			edg.back().push_back(hci.front().crossindex);
			edg.push_back(std::vector<int>());
			for(int k = 0; k + 1 < (int)hci.size(); ++ k)
			{
				edg.back().push_back(hci[k].crossindex);
				edg.back().push_back(hci[k + 1].crossindex);
				edg.push_back(std::vector<int>());
			}
			edg.back().push_back(hci.back().crossindex);
		}
		edg.back().push_back((int)simplified.size() - 1);
	}

	void CalcEdgeCross(
		const std::vector<Vector3> &ts,
		const Edge &e1,
		const Edge &e2,
		std::vector<ijc2> &crosses)
	{
		for(int i = 0; i + 1 < (int)e1.tessIndex.size(); ++i)
		{
			const Vector3 &a = ts[e1.tessIndex[i]];
			const Vector3 &b = ts[e1.tessIndex[i + 1]];
			for(int j = 0; j + 1 < (int)e2.tessIndex.size(); ++j)
			{
				const Vector3 &c = ts[e2.tessIndex[j]];
				const Vector3 &d = ts[e2.tessIndex[j + 1]];
				Vector3 cross;
				if(Rydot::Spherical_Intersection_ArcArc(
					a,b,c,d, cross))
				{
					const Vector3 da = cross - a;
					const Vector3 db = b - a;
					const Vector3 dc = cross - c;
					const Vector3 dd = d - c;

					double iparam = da.DotProd(db)/db.Abs2();
					double jparam = dc.DotProd(dd)/dd.Abs2();

					if(iparam < 0 || jparam < 0)continue;
					if(iparam > 1.0 || jparam > 1.0)continue;

					crosses.push_back(ijc2(e1.edgeId, i, e2.edgeId, j, cross, iparam, jparam));
				}
			}
		}
	}

	void AddCross(std::vector<Vector3> &ts, std::vector< std::vector<int> > &edg)
	{
		// offset edg index
		std::vector<Edge> es(edg.size());
		for(int i = 0; i < (int)edg.size(); ++i)
		{
			std::vector<int> &ei = edg[i];
			es[i].edgeId = i + (int)edges.size();
			es[i].tessIndex = ei;
			for(int j = 0; j < (int)ei.size(); ++j)
			{
				es[i].tessIndex[j] += (int)tess.size();
			}
		}
		tess.insert(tess.end(), ts.begin(), ts.end());

		std::vector<ijc2> crs;
		for(int i = 0; i < (int)edges.size(); ++i)
		{
			for(int j = 0; j < (int)es.size(); ++j)
			{
				CalcEdgeCross(tess, edges[i], es[j], crs);
			}
		}

		std::map<eii, std::vector<kc> > halfcross;
		for(int i = 0; i < (int)crs.size(); ++i)
		{
			const ijc2 &x = crs[i];
			halfcross[eii(x.ei, x.i)].push_back(kc((int)tess.size() + i, x.iparam));
			halfcross[eii(x.ej, x.j)].push_back(kc((int)tess.size() + i, x.jparam));
		}

		for(std::map<eii, std::vector<kc> >::iterator i = halfcross.begin(); i != halfcross.end(); ++i)
		{
			std::sort(i->second.begin(), i->second.end());
		}

		std::vector<std::vector<int> > e; // [][tess index]
		for(int i = 0; i < edges.size(); ++i)
		{
			e.push_back(std::vector<int>());
			for(int j = 0; j + 1 < edges[i].tessIndex.size(); ++j)
			{
				int x = edges[i].tessIndex[j];
				std::map<eii, std::vector<kc> >::iterator f = halfcross.find(eii(edges[i].edgeId, j));
				if(f == halfcross.end())
				{
					e.back().push_back(x);
					continue;
				}
				std::vector<kc> &hci = f->second;

				e.back().push_back(x);
				e.back().push_back(hci.front().crossindex);
				e.push_back(std::vector<int>());
				for(int k = 0; k + 1 < (int)hci.size(); ++ k)
				{
					e.back().push_back(hci[k].crossindex);
					e.back().push_back(hci[k + 1].crossindex);
					e.push_back(std::vector<int>());
				}
				e.back().push_back(hci.back().crossindex);
			}
			e.back().push_back(edges[i].tessIndex.back());
		}
		for(int i = 0; i < es.size(); ++i)
		{
			e.push_back(std::vector<int>());
			for(int j = 0; j + 1 < es[i].tessIndex.size(); ++j)
			{
				int x = es[i].tessIndex[j];
				std::map<eii, std::vector<kc> >::iterator f = halfcross.find(eii(es[i].edgeId, j));
				if(f == halfcross.end())
				{
					e.back().push_back(x);
					continue;
				}
				std::vector<kc> &hci = f->second;

				e.back().push_back(x);
				e.back().push_back(hci.front().crossindex);
				e.push_back(std::vector<int>());
				for(int k = 0; k + 1 < (int)hci.size(); ++ k)
				{
					e.back().push_back(hci[k].crossindex);
					e.back().push_back(hci[k + 1].crossindex);
					e.push_back(std::vector<int>());
				}
				e.back().push_back(hci.back().crossindex);
			}
			e.back().push_back(es[i].tessIndex.back());
		}

		edges.clear();
		edges.resize(e.size());
		for(int i = 0; i < e.size(); ++i)
		{
			edges[i] = Edge(i, e[i]);
		}

		for(int i = 0; i < (int)crs.size(); ++i)
		{
			tess.push_back(crs[i].cross);
		}
	}

	void Bake()
	{
		baked.clear();
		baked.resize(edges.size());
		for(int i = 0; i < (int)edges.size(); ++i)
		{
			const Edge &ei = edges[i];
			for(int j = 0; j < (int)ei.tessIndex.size(); ++j)
				baked[i].push_back(tess[ei.tessIndex[j]]);
		}
	}


	// number of segments
public:
	const int Size()const
	{
		return (int)edges.size() + 1;
		//return 1;
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
		Simplify();
		record.clear();
	}

public:
	void Optimize()
	{
		//Simplify();
		Finalize();

		if(simplified.size() > 2)
		{
			std::vector<Vector3> ts;
			std::vector<std::vector<int> > edg;
			DecomposeSelfCross(ts, edg);

			AddCross(ts, edg);
		}

		Bake();
	}

public:
	void Erase()
	{
		//Simplify();

		std::vector<Edge> remains;

		int N = scratchNum;

		for(int n = 0; n < edges.size(); ++n)
		{
			const Edge &e1 = edges[n];

			int numcross = 0;

			for(int i = 0; i + 1 < (int)e1.tessIndex.size(); ++i)
			{
				const Vector3 &a = tess[e1.tessIndex[i]];
				const Vector3 &b = tess[e1.tessIndex[i + 1]];
				for(int j = 0; j + 1 < (int)simplified.size(); ++j)
				{
					const Vector3 &c = simplified[j];
					const Vector3 &d = simplified[j + 1];
					Vector3 cross;
					if(Rydot::Spherical_Intersection_ArcArc(
						a,b,c,d, cross))
					{
						++numcross;
						if(numcross >= N)break;
					}
				}
				if(numcross >= N)break;
			}

			if(numcross >= N)continue;

			remains.push_back(e1);
		}

		edges = remains;

		Bake();
	}

	// get segment #i
public:
	const std::vector<Vector3> &GetSeg(int i)const
	{
		if (baked.size() == i)return simplified;
		return baked[i];
	}



};


