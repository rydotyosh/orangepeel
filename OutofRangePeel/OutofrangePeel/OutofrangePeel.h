
#pragma once

#include <math.h>
#include "Events.h"
#include "matrix.h"
#include "matrixutil.h"
#include "dashpod.h"

#include "View.h"
#include "mesh.h"

#include "QhullCalc.h"

#include <boost/unordered_set.hpp>

#include <fstream>



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



class CutMesh
{
public:

	typedef Rydot::Vector3f	Vector3;

	struct Face
	{
		std::vector<int> coedgeIndices;

		// sub
		std::vector<int> pointIndices;
		std::vector<int> splpointIndices;

		bool HasPoint(int pi)const
		{
			for(size_t i=0;i<pointIndices.size();++i)
			{
				if(pointIndices[i]==pi)return true;
			}
			return false;
		}
	};

	struct Coedge
	{
		int faceIndex;
		int edgeIndex;
		std::vector<int> pointIndices;
		int sense;

		bool IsSibling(const Coedge &c)const
		{
			if(pointIndices[0]==c.pointIndices[0] && pointIndices[1]==c.pointIndices[1] )return true;
			if(pointIndices[0]==c.pointIndices[1] && pointIndices[1]==c.pointIndices[0] )return true;
			return false;
		}
	};

	struct Edge
	{
		std::vector<int> coedgeIndices;
		std::vector<int> splPointIndices;
		bool split;
	};

	struct Vertex
	{
		Vector3 point;

		// sub
		std::vector<int> faceIndices;
		std::vector<int> edgeIndices;
	};

private:

	//std::vector<Vector3> points;
	std::vector<Vertex> vertices;
	std::vector<std::vector<int> > cutter;
	std::vector<std::vector<int> > mesh;

	std::vector<Vertex> splvertices;

	std::vector<Vector3> splpoints;

	std::vector<Face> faces;
	std::vector<Coedge> coedges;
	std::vector<Edge> edges;

	// returns sibling coedge of coedges[ce]
	int FindSiblingCoedge(int ce)const
	{
		if(ce<0 || ce>=coedges.size())return -1;
		const Coedge &c=coedges[ce];
		if(c.edgeIndex<0)return -1;
		const Edge &e=edges[c.edgeIndex];
		if(e.coedgeIndices.size()<2)return -1;
		return (e.coedgeIndices[0]==ce)?e.coedgeIndices[1]:e.coedgeIndices[0];
	}

	std::vector<int> FindAdjacentFaces(int fi)const
	{
		if(fi<0 || fi>=faces.size())return std::vector<int>();

		const Face &f=faces[fi];
		std::vector<int> r;
		for(size_t i=0;i<f.coedgeIndices.size();++i)
		{
			int sib=FindSiblingCoedge(f.coedgeIndices[i]);
			if(sib>=0)
			{
				r.push_back(coedges[sib].faceIndex);
			}
		}
		return r;
	}

	// returns adjacent faces from faces[fi] where belongs to vertices[pi]
	std::vector<int> PointFacePie(int pi, int fi)const
	{
		if(pi<0 || pi>=vertices.size())return std::vector<int>();
		if(fi<0 || fi>=faces.size())return std::vector<int>();

		std::vector<int> working;
		boost::unordered_set<int> used;
		working.push_back(fi);
		used.insert(fi);
		while(!working.empty())
		{
			std::vector<int> newwork;
			for(int i=0;i<working.size();++i)
			{
				std::vector<int> adj=FindAdjacentFaces(working[i]);
				for(int j=0;j<adj.size();++j)
				{
					if((used.find(adj[j])==used.end()) && faces[adj[j]].HasPoint(pi))
					{
						newwork.push_back(adj[j]);
						used.insert(adj[j]);
					}
				}
			}
			working=newwork;
		}
		std::vector<int> res;
		std::copy(used.begin(),used.end(),back_inserter(res));
		return res;
	}

	bool ReassignEdges(int pi, int fi, int pi2)
	{
		if(pi<0 || pi>=vertices.size())return false;
		if(fi<0 || fi>=faces.size())return false;

		const Face &f=faces[fi];
		for(size_t i=0;i<f.coedgeIndices.size();++i)
		{
			int eidx=coedges[f.coedgeIndices[i]].edgeIndex;
			Edge &e=edges[eidx];
			if(e.splPointIndices[0]==pi)e.splPointIndices[0]=pi2;
			if(e.splPointIndices[1]==pi)e.splPointIndices[1]=pi2;
		}
		return true;
	}

public:
	CutMesh()
	{
	}


	const std::vector<Face> &GetFaces(){return faces;}
	const std::vector<Vector3> &GetSplPoints(){return splpoints;}



	bool SetupPoints(const std::vector<Vector3> &points)
	{
		vertices.resize(points.size());
		for(size_t i=0;i<points.size();++i)vertices[i].point=points[i];
		return true;
	}

	bool SetupCutter(const std::vector<std::vector<int> > &cutter)
	{
		this->cutter=cutter;
		return true;
	}

	bool SetupMesh(const std::vector<std::vector<int> > &mesh)
	{
		this->mesh.clear();
		for(int i=0;i<mesh.size();++i)
		{
			const std::vector<int> &m=mesh[i];
			if(m.size()<=2)continue;
			for(int j=0;j+2<m.size();++j)
			{
				std::vector<int> v;
				v.push_back(m[0]);
				for(int k=1;k<3;++k)
				{
					v.push_back(m[j+k]);
				}
				this->mesh.push_back(v);
			}
		}
		return true;
	}

	bool CreateSplit()
	{
		faces.clear();
		coedges.clear();

		faces.resize(mesh.size());
		coedges.resize(mesh.size()*3);
		for(size_t i=0;i<mesh.size();++i)
		{
			for(int k=0;k<3;++k)
			{
				faces[i].coedgeIndices.push_back(i*3+k);
				Coedge &c=coedges[i*3+k];
				c.faceIndex=i;
				c.pointIndices.push_back(mesh[i][k]);
				c.pointIndices.push_back(mesh[i][(k+1)%3]);
			}
			for(int k=0;k<3;++k)
			{
				faces[i].pointIndices.push_back(mesh[i][k]);
			}
		}

		// find sibling
		edges.clear();
		edges.reserve(coedges.size());
		for(size_t i=0;i<coedges.size();++i)
		{
			for(size_t k=i+1;k<coedges.size();++k)
			{
				if(coedges[i].IsSibling(coedges[k]))
				{
					int eidx=edges.size();
					edges.push_back(Edge());
					Edge &e=edges.back();
					e.coedgeIndices.push_back(std::min(i,k));
					e.coedgeIndices.push_back(std::max(i,k));
					e.split=false;
					e.splPointIndices=coedges[i].pointIndices;
					coedges[i].edgeIndex=eidx;
					coedges[k].edgeIndex=eidx;
					coedges[i].sense=true;
					coedges[k].sense=false;
				}
			}
		}

		// split by cutter
		for(size_t i=0;i<cutter.size();++i)
		{
			Coedge tmp;
			tmp.pointIndices.push_back(cutter[i][0]);
			tmp.pointIndices.push_back(cutter[i][1]);

			for(size_t k=0;k<coedges.size();++k)
			{
				if(coedges[k].IsSibling(tmp))
				{
					// split
					int eidx=coedges[k].edgeIndex;
					if(eidx>=0)
					{
						Edge &e0=edges[eidx];
						if(e0.split)continue;
						coedges[e0.coedgeIndices[1]].edgeIndex=edges.size();
						Edge e1=e0;
						e1.coedgeIndices.clear();
						e1.coedgeIndices.push_back(e0.coedgeIndices[1]);
						e0.coedgeIndices.pop_back();
						e0.split=true;
						e1.split=true;
						edges.push_back(e1);
					}
					break;
				}
			}
		}

		// setup vertex invref to face index
		for(size_t i=0;i<faces.size();++i)
		{
			const Face &f=faces[i];
			for(size_t j=0;j<f.pointIndices.size();++j)
			{
				vertices[f.pointIndices[j]].faceIndices.push_back(i);
			}
		}

		// cluster
		// [point index][][]=>face index
		std::vector<std::vector<std::vector<int> > > picluster(vertices.size());
		for(size_t i=0;i<vertices.size();++i)
		{
			const std::vector<int> &fcs=vertices[i].faceIndices;
			boost::unordered_set<int> used;
			for(size_t j=0;j<fcs.size();++j)
			{
				if(used.find(fcs[j])!=used.end())continue;
				std::vector<int> pie=PointFacePie(i, fcs[j]);
				picluster[i].push_back(pie);
				for(size_t k=0;k<pie.size();++k)
				{
					used.insert(pie[k]);
				}
			}
		}

		// assign split points
		splvertices.resize(vertices.size());
		for(size_t i=0;i<vertices.size();++i)
		{
			splvertices[i].point=vertices[i].point;
		}
		for(size_t i=0;i<picluster.size();++i)
		{
			if(picluster[i].size()>1)
			{
				for(size_t k=1;k<picluster[i].size();++k)
				{
					int splidx=splvertices.size();
					splvertices.push_back(vertices[i]);
					splvertices.back().faceIndices.clear();
					for(size_t a=0;a<picluster[i][k].size();++a)
					{
						ReassignEdges(i, picluster[i][k][a], splidx);
					}
				}
			}
		}

		// assign face point index
		for(size_t i=0;i<faces.size();++i)
		{
			Face &f=faces[i];
			for(size_t j=0;j<f.coedgeIndices.size();++j)
			{
				const Coedge &ce=coedges[f.coedgeIndices[j]];
				int eidx=ce.edgeIndex;
				if(ce.sense)
				{
					f.splpointIndices.push_back(edges[eidx].splPointIndices[0]);
				}
				else
				{
					f.splpointIndices.push_back(edges[eidx].splPointIndices[1]);
				}
			}
		}

		// convert splpoint
		splpoints.resize(splvertices.size());
		for(size_t i=0;i<splvertices.size();++i)
		{
			splpoints[i]=splvertices[i].point;
		}

		return false;
	}


	//debug
	bool OutDot()
	{
		std::ofstream f("c:\\a.dot");
		f<<"graph sample{\n";
		for(size_t i=0;i<edges.size();++i)
		{
			f<<"  p"<<edges[i].splPointIndices[0]<<" -- p"<<edges[i].splPointIndices[1]<<"\n";
		}
		f<<"}\n";
		return true;
	}




};


class Peel
{
public:
	typedef Rydot::Vector3f	Vector3;

	struct Face
	{
		std::vector<int> vertexIndices;

		// sub
		Vector3 normalCache;
		Vector3 centerCache;
	};

	struct Vertex
	{
		Vector3 orgPoint;
		Vector3 velocity;
		Vector3 moved;

		// sub
		std::vector<int> faceIndices;
	};

	struct Edge
	{
		std::vector<int> vertexIndices;
		std::vector<int> faceIndices;

		double orgLength;
	};

	std::vector<Vertex> vertices;
	std::vector<Face> faces;
	std::vector<Edge> edges;

public:
	bool SetupVertices(const std::vector<Vector3> &points)
	{
		this->vertices.resize(points.size());
		for(size_t i=0;i<points.size();++i)
		{
			this->vertices[i].orgPoint=points[i];
			this->vertices[i].moved=points[i];
			this->vertices[i].velocity=Vector3(0,0,0);
		}
		return true;
	}

	bool SetupFaces(const std::vector<CutMesh::Face> &faces)
	{
		this->faces.resize(faces.size());
		for(size_t i=0;i<faces.size();++i)
		{
			const std::vector<int> &fv=faces[i].splpointIndices;
			this->faces[i].vertexIndices=fv;
		}
		return true;
	}

	int FindSameEdge(int v0,int v1)
	{
		for(int k=0;k<edges.size();++k)
		{
			if(edges[k].vertexIndices[0]==v0 && edges[k].vertexIndices[1]==v1)
				return k;
		}
		Edge e;
		e.vertexIndices.push_back(v0);
		e.vertexIndices.push_back(v1);
		e.orgLength=vertices[v0].orgPoint.Dist(vertices[v1].orgPoint);
		edges.push_back(e);
		return edges.size()-1;
	}

	bool SetupInvRef()
	{
		edges.clear();

		for(size_t i=0;i<faces.size();++i)
		{
			const std::vector<int> &vis=faces[i].vertexIndices;
			for(size_t j=0;j<vis.size();++j)
			{
				vertices[vis[j]].faceIndices.push_back(i);
			}
			for(size_t j=0;j<vis.size();++j)
			{
				size_t jn=(j+1)%vis.size();
				int v0=std::min(vis[j],vis[jn]);
				int v1=std::max(vis[j],vis[jn]);
				int idx=FindSameEdge(v0,v1);
				edges[idx].faceIndices.push_back(i);
			}
		}
		return true;
	}

private:
	void FaceCenter(int fi)
	{
		const std::vector<int> &fv=faces[fi].vertexIndices;
		Vector3 v(0,0,0);
		for(size_t i=1;i<fv.size();++i)
		{
			v+=vertices[fv[i]].moved;
		}
		v*=1.0/(fv.size());
		faces[fi].centerCache=v;
	}

	void FaceNormal(int fi)
	{
		const std::vector<int> &fv=faces[fi].vertexIndices;
		Vector3 a=vertices[fv[0]].moved-vertices[fv[1]].moved;
		Vector3 b=vertices[fv[2]].moved-vertices[fv[1]].moved;
		faces[fi].normalCache=a.ExProd(b).Norm();
	}

	Vector3 PseudoNormalOfVertex(int vi)
	{
		const std::vector<int> &vf=vertices[vi].faceIndices;
		Vector3 v(0,0,0);
		for(size_t i=0;i<vf.size();++i)
		{
			v+=faces[vf[i]].normalCache;
		}
		return v.Norm();
	}
	Vector3 PseudoCenterOfVertex(int vi)
	{
		const std::vector<int> &vf=vertices[vi].faceIndices;
		Vector3 v(0,0,0);
		for(size_t i=0;i<vf.size();++i)
		{
			v+=faces[vf[i]].centerCache;
		}
		return v*(1.0/vf.size());
	}

	Vector3 PseudoNormalOfEdge(int ei)
	{
		const std::vector<int> &ef=edges[ei].faceIndices;
		Vector3 v(0,0,0);
		for(size_t i=0;i<ef.size();++i)
		{
			v+=faces[ef[i]].centerCache;
		}
		return v*(1.0/ef.size());
	}

	int DiagVertex(int fi, int ei)
	{
		std::vector<int> res=faces[fi].vertexIndices;
		const std::vector<int> &ve=edges[ei].vertexIndices;
		for(int i=0;i<ve.size();++i)
		{
			std::remove(res.begin(),res.end(),ve[i]);
		}
		return res[0];
	}

	std::vector<int> DiagVerticesOfEdge(int ei)
	{
		const std::vector<int> &ef=edges[ei].faceIndices;
		if(ef.size()!=2)return std::vector<int>();

		std::vector<int> res;
		res.push_back(DiagVertex(ef[0],ei));
		res.push_back(DiagVertex(ef[1],ei));
		return res;
	}

	double rnd()
	{
		return rand()/32767.0;
	}

	double satu(double x)
	{
		if(x<-1)return -1;
		if(x>1)return 1;
		return x;
	}

public:
	bool Proceed()
	{
		for(int i=0;i<faces.size();++i)FaceNormal(i);
		for(int i=0;i<faces.size();++i)FaceCenter(i);

		for(int i=0;i<vertices.size();++i)
		{
			vertices[i].velocity*=0.0;
		}

		double r=0.3;
		double r2=0.05;
		//double r3=0.05;
		double dhsum=0;
		for(int i=0;i<edges.size();++i)
		{
			std::vector<int> dv=DiagVerticesOfEdge(i);

			if(dv.size()!=2)continue;

			Vector3 n1=faces[edges[i].faceIndices[0]].normalCache;
			Vector3 n0=faces[edges[i].faceIndices[1]].normalCache;

			Vector3 e0=vertices[edges[i].vertexIndices[0]].moved;
			Vector3 e1=vertices[edges[i].vertexIndices[1]].moved;

			Vector3 de=e1-e0;
			de.Normalize();

			double h0=vertices[dv[1]].moved.DotProd(n0);
			double h1=vertices[dv[0]].moved.DotProd(n1);

			double hv0=vertices[dv[0]].moved.DotProd(n0);
			double hv1=vertices[dv[1]].moved.DotProd(n1);

			double dh0=(h0-hv0);
			double dh1=(h1-hv1);

			Vertex &v0=vertices[dv[0]];
			Vertex &v1=vertices[dv[1]];

			v0.velocity+=n0*(dh0*r);
			v1.velocity+=n1*(dh1*r);

			Vector3 d=v1.moved-v0.moved;

			Vector3 dp=d-de*d.DotProd(de);
			Vector3 dn=dp.Norm();

			v0.velocity-=dn*r2;
			v1.velocity+=dn*r2;

			//Vector3 dq=d-dn*d.DotProd(dn);
			//v0.velocity+=dq*r3;
			//v1.velocity-=dq*r3;


			dhsum+=dh0*dh0+dh1*dh1;

			
		}
		
			{
				char s[256];
				sprintf(s,"%f\n",dhsum);
				OutputDebugStringA(s);

			}

		//double rr=0.1;
		//for(int i=0;i<faces.size();++i)
		//{
		//	Vector3 n=faces[i].normalCache;
		//	for(int j=0;j<faces[i].vertexIndices.size();++j)
		//	{
		//		vertices[faces[i].vertexIndices[j]].velocity+=n*rr;
		//	}
		//}

		double k=0.1;
		for(int i=0;i<edges.size();++i)
		{
			const Edge &e=edges[i];
			Vertex &v0=vertices[e.vertexIndices[0]];
			Vertex &v1=vertices[e.vertexIndices[1]];
			Vector3 dv=v1.moved-v0.moved;
			double dl=dv.Abs()-e.orgLength;
			Vector3 dvn=dv.Norm();
			v0.velocity+=dvn*(dl*k);
			v1.velocity-=dvn*(dl*k);
		}

		//double ratio=satu( dhsum)*0.1;
		double ratio=0.1;
		for(int i=0;i<vertices.size();++i)
		{
			vertices[i].moved+=vertices[i].velocity*ratio;
		}

		Vector3 v(0,0,0);
		for(int i=0;i<vertices.size();++i)
		{
			v+=vertices[i].moved;
		}
		v*=(1.0/vertices.size());//g
		for(int i=0;i<vertices.size();++i)
		{
			vertices[i].moved-=v;
		}




		return true;
	}

};



class OrangePeel:public IGLEvents
{
	typedef float Float;
	typedef Rydot::Rect2f	Rect2;
	typedef Rydot::Matrix44f	Matrix44;
	typedef Rydot::Vector2f	Vector2;
	typedef Rydot::Vector3f	Vector3;
	typedef Rydot::Dashpod<double> Dashpod;

	typedef Rydot::Mesh Mesh;

	Mesh m;
	std::vector<Vector3> originalVertices;

	std::vector<Vector3> qhpts;
	std::vector<std::vector<int> > qhfcs;

	template <class T>
	T max(const T &a, const T &b){return (a>b)?a:b;}

	unsigned int imageid;
	unsigned char image[16*16*4];
	unsigned int t;

	View v;

	float mousex;
	float mousey;

	float tmpmousex;
	float tmpmousey;
	
	Dashpod theta;
	Dashpod phi;
	Dashpod dist;

	bool rdown;
	bool pdown;

	//std::vector<Vector3> record;
	std::vector<Stroke> strokes;
	
	std::vector<Vector3> control;


	CutMesh pl;
	Peel pe;


public:
	OrangePeel()
		:t(0)
		,mousex(0)
		,mousey(0)
		,tmpmousex(0)
		,tmpmousey(0)
		,theta(0.0)
		,phi(0.0)
		,dist(5.0)
		,rdown(false)
		,pdown(false)
	{
	}



private:
	void Initialize()
	{
		glClearColor(0.5,0.5,0.5,1.0);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		v.SetView(Rect2(0,0,640,480), 45);
		v.SetCamera(
			Vector3(0.0,0.0,dist.get()),
			Vector3(0,0,0),
			Vector3(0,1,0));

		v.Apply();

		glGenTextures(1,(unsigned int*)&imageid);
		glBindTexture(GL_TEXTURE_2D,imageid);

		unsigned char *p=image;
		for(int i=0;i<16;i++)
		{
			for(int j=0;j<16;j++,p+=4)
			{
				if( max(abs(int(i-7.5)),abs(int(j-7.5)))&1 )
				{
					p[0]=128;p[1]=128;p[2]=255;p[3]=255;
				}
				else
				{
					p[0]=255;p[1]=255;p[2]=255;p[3]=255;
				}

			}
		}

		//glTexImage2D
		//	(GL_TEXTURE_2D,0,4,16,16,
		//	0,GL_RGBA,GL_UNSIGNED_BYTE,image);
		gluBuild2DMipmaps(
			GL_TEXTURE_2D, GL_RGB, 16, 16,
			GL_RGBA, GL_UNSIGNED_BYTE, image);

		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);

		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);

		

		float ambient[]={0.6f,0.6f,0.6f,1.0f};
		glMaterialfv(GL_FRONT,GL_AMBIENT,ambient);
		float diffuse[]={0.9f,0.9f,0.9f,1.0f};
		glMaterialfv(GL_FRONT,GL_DIFFUSE,diffuse);
		float shininess[]={0.0f,0.0f,0.0f,1.0f};
		glMaterialfv(GL_FRONT,GL_SHININESS,shininess);

		Rydot::PrimitiveMeshCreator::CreateBox(m);
		m.Subdivision();
		m.Subdivision();
		m.Subdivision();

		for(size_t i=0;i<m.vertex.size();++i)
		{
			m.vertex[i].Normalize();
		}

		originalVertices = m.vertex;

		// ラティス変形してみる

		// 制御点を設定
		control.resize(64);
		for(int a=0;a<64;a++)
		{
			int i=a%4,j=(a/4)%4,k=a/16;

			Vector3 displacement(0,0,0);
			if(j==3)
			{
				displacement=Vector3(0.,-0.05,0.);
				//if( (i==1||i==2)&&(k==1||k==2) )
				//{
				//	displacement=Vector3(0,0,0);
				//}
			}
			if(j==0)
			{
				displacement=Vector3(0.,0.05,0.);
				if( (i==1||i==2)&&(k==1||k==2) )
				{
					displacement=Vector3(0.,0.25,0.);
				}
			}
			control[a]=Vector3( i/3.0, (j-1.5)/3.0*0.95+0.5, k/3.0 )+displacement;
		}

		Rydot::MeshDeformer::LatticeDeform(m, control);

		m.CalcNormal();

		strokes.push_back(Stroke());

		{
		double pi=4.0*atan(1.0);
		double R=2;
		Stroke s;
		int N=20;
		for(int i=0;i<N;++i)
		{
			double t=(double)i/(N-1.0);
			double z=cos(pi*t);
			double ss=sin(pi*t);
			double x=cos(2.0*pi*R*t)*ss;
			double y=sin(2.0*pi*R*t)*ss;
			s.Add(Vector3(x,z,y));
		}
		//Stroke s;
		//s.Add(Vector3(0,1,0));
		//s.Add(Vector3(1,0,0));
		//s.Add(Vector3(0,0,1));
		//s.Add(Vector3(-1,0,0));
		//s.Add(Vector3(0,0,-1));
		//s.Add(Vector3(0,-1,0));
		s.Simplify();
		//s.Finalize();
		strokes[0]=s;
		strokes.push_back(Stroke());
		}

		CalcQH();
	}



	void CalcQH()
	{
		QhullCalc qh;
		qhpts.clear();
		qhfcs.clear();
		std::vector<Rydot::Vector3d> vtx;
		//vtx.push_back(Rydot::Vector3d(1,1,1).Norm());
		//vtx.push_back(Rydot::Vector3d(-1,-1,1).Norm());
		//vtx.push_back(Rydot::Vector3d(1,-1,-1).Norm());
		//vtx.push_back(Rydot::Vector3d(-1,1,-1).Norm());
		std::vector<std::vector<int> > cutter;
		for(size_t i=0;i<strokes.size();++i)
		{
			const std::vector<Vector3> &s=strokes[i].Get();
			int vtxidx=vtx.size();
			for(size_t k=0;k<s.size();++k)
			{
				vtx.push_back(Rydot::Vector3d(s[k]));
			}
			for(size_t k=0;k+1<s.size();++k)
			{
				std::vector<int> cut;
				cut.push_back(vtxidx+k);
				cut.push_back(vtxidx+k+1);
				cutter.push_back(cut);
			}
		}

		std::vector<Rydot::Vector3d> qhp;
		qh.Calc(vtx, qhp, qhfcs);

		Rydot::Rect3f bb(-1,-1,-1,1,1,1);
		qhpts.resize(qhp.size());
		for(size_t i=0;i<qhp.size();++i)
		{
			const Vector3 p(qhp[i]);
			qhpts[i]=bb.TransformFrom(Rydot::MeshDeformer::beziercube(control, bb.TransformTo(p)));
		}

		// find matching
		//std::vector<int> match(qhp.size());
		//for(size_t i=0;i<qhp.size();++i)
		//{
		//	double nearest=1e10;
		//	int idx=0;
		//	for(size_t k=0;k<vtx.size();++k)
		//	{
		//		double d2=qhp[i].Dist2(vtx[k]);
		//		if(d2<nearest)
		//		{
		//			nearest=d2;
		//			idx=k;
		//		}
		//	}
		//	match[i]=idx;
		//}

		//Peel pl;
		pl.SetupMesh(qhfcs);
		pl.SetupCutter(cutter);
		pl.SetupPoints(qhpts);
		pl.CreateSplit();

		//pl.OutDot();

		//
		pe.SetupVertices(pl.GetSplPoints());
		pe.SetupFaces(pl.GetFaces());
		pe.SetupInvRef();

		for(int i=0;i<1;++i)
		{
			pe.Proceed();
		}


#ifdef _WIN32
		double v=0;
		for(size_t i=0;i<qhfcs.size();++i)
		{
			const Rydot::Vector3d &a=qhp[qhfcs[i][0]];
			const Rydot::Vector3d &b=qhp[qhfcs[i][1]];
			const Rydot::Vector3d &c=qhp[qhfcs[i][2]];
			double dv=b.ExProd(a).DotProd(c);
			v+=dv;
		}
		v/=6.0;
		char s[256];
		sprintf(s,"volume:%f\n",v);
		OutputDebugStringA(s);
#endif

	}



	void Display()
	{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		double phi=this->phi.get();
		double theta=this->theta.get();
		double dist=this->dist.get();

		v.SetCamera(
			Vector3(dist*cos(phi)*sin(theta),dist*sin(phi),dist*cos(phi)*cos(theta)),
			Vector3(0,0,0),
			Vector3(0,1,0));

		v.Apply();

		glEnable(GL_BLEND);

		glEnable(GL_DEPTH_TEST);

		glEnable(GL_NORMALIZE);

		glShadeModel(GL_SMOOTH);

		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_LINE_SMOOTH);

		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		float light_position[4]={cos(t*0.03)*1.5,cos(t*0.005)*1,sin(t*0.03)*1.5,1};
		glLightfv(GL_LIGHT0,GL_POSITION,light_position);
		glEnable(GL_LIGHT0);

		float light_position2[4]={cos(t*0.04)*1.5,cos(t*0.004)*1,sin(t*0.04)*1.5,1};
		glLightfv(GL_LIGHT1,GL_POSITION,light_position2);
		glEnable(GL_LIGHT1);
		float col[4] = {0.3f,0.7f,0.5f,1};
		glLightfv(GL_LIGHT1, GL_DIFFUSE, col);

		glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
		
		float white[]={1,1,1,1};
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, white);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, white);
		float black[]={0,0,0,1};
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);

		//int n=6;
		//for(int i=0;i<n*n;i++)
		//{
		//	glPushMatrix();

		//	glScalef(0.2f,0.2f,0.2f);
		//	glTranslatef((i%n-(n-1)/2.0)*2,(i/n-(n-1)/2.0)*2,0);

		//	glBegin(GL_TRIANGLES);
		//		glTexCoord2f(0,0);
		//		glNormal3f(-1,-1,3);
		//		glVertex3f(-1,-1,0);

		//		glTexCoord2f(1,0);
		//		glNormal3f(1,-1,3);
		//		glVertex3f(1,-1,0);

		//		glTexCoord2f(1,1);
		//		glNormal3f(1,1,3);
		//		glVertex3f(1,1,0);

		//		glTexCoord2f(1,1);
		//		glNormal3f(1,1,3);
		//		glVertex3f(1,1,0);

		//		glTexCoord2f(0,1);
		//		glNormal3f(-1,1,3);
		//		glVertex3f(-1,1,0);

		//		glTexCoord2f(0,0);
		//		glNormal3f(-1,-1,3);
		//		glVertex3f(-1,-1,0);
		//	glEnd();

		//	glPopMatrix();
		//}

		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);

		glColor3f(1,1,1);
		glPointSize(5);
		glBegin(GL_POINTS);
			glVertex3f(light_position[0],light_position[1],light_position[2]);
			glVertex3f(light_position2[0],light_position2[1],light_position2[2]);
		glEnd();

		glEnable(GL_LIGHTING);
		//glColor3f(0.95f,0.6f,0.1f);
		glBegin(GL_TRIANGLES);
		float orange[]={0.95f,0.6f,0.1f,1};
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, orange);
		float amborange[]={0.95f,0.6f,0.1f,1};
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amborange);
		float emiorange[]={0.3f,0.2f,0.1f,1};
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emiorange);

		//for(size_t i=0;i<m.faces.size();++i)
		//{
		//	for(size_t v=0;v<3;++v)
		//	{
		//		size_t p=m.faces[i].vertex[v];
		//		glNormal3fv(&(m.normals[p].x));
		//		glVertex3fv(&(m.vertex[p].x));
		//	}
		//}
		glEnd();
		glDisable(GL_LIGHTING);

		//const std::vector<CutMesh::Face> &fcs=pl.GetFaces();
		//const std::vector<Vector3> &pts=pl.GetSplPoints();
		//glBegin(GL_TRIANGLES);
		//for(size_t i=0;i<fcs.size();++i)
		//{
		//	for(size_t v=0;v<3;++v)
		//	{
		//		glColor3f((i%10)/10.0,((i/10)%10)/10.0,0);
		//		int idx=fcs[i].splpointIndices[2-v];
		//		Vector3 p=pts[idx]+Vector3(0.0,0.0,idx*0.001);
		//		glVertex3fv(&(p.x));
		//	}
		//}
		//glEnd();

		const std::vector<Peel::Face> &fcs=pe.faces;
		const std::vector<Peel::Vertex> &vts=pe.vertices;
		glBegin(GL_TRIANGLES);
		for(size_t i=0;i<fcs.size();++i)
		{
			for(size_t v=0;v<3;++v)
			{
				glColor3f((i%10)/10.0,((i/10)%10)/10.0,0);
				int idx=fcs[i].vertexIndices[2-v];
				Vector3 p=vts[idx].moved+Vector3(0.0,0.0,0.0);
				glVertex3fv(&(p.x));
			}
		}
		glEnd();

		//
		pe.Proceed();


		{
			std::pair<Vector3, Vector3> r = v.Ray(Vector2(mousex+1, mousey+1));

			glDisable(GL_DEPTH_TEST);

			//cross face
			for(size_t i=0;i<m.faces.size();++i)
			{
				Vector3 coll;
				if(
					Rydot::ReflectTriangleRay(r.first, r.first+r.second*100,
					m.vertex[m.faces[i].vertex[0]],
					m.vertex[m.faces[i].vertex[2]],
					m.vertex[m.faces[i].vertex[1]],
					coll)
					)
				{
					glColor3f(0,1,0);
					if(pdown)
					{
						glColor3f(1,0,0);
						size_t v0 = m.faces[i].vertex[0];
						size_t v1 = m.faces[i].vertex[1];
						size_t v2 = m.faces[i].vertex[2];
						
						Vector3 coef;
						Rydot::ReflectTriangleRayTopology(r.first, r.first+r.second*100,
														  m.vertex[m.faces[i].vertex[0]],
														  m.vertex[m.faces[i].vertex[2]],
														  m.vertex[m.faces[i].vertex[1]],
														  coef);
						strokes.back().Add(originalVertices[v0]*(1-coef.x-coef.y)+originalVertices[v2]*coef.x+originalVertices[v1]*coef.y);
						strokes.back().Simplify();
					}
					glPointSize(10);
					glBegin(GL_POINTS);
					glVertex3f(coll.x, coll.y, coll.z);
					glEnd();
					break;
				}
			}

			glColor3f(0,0,1);
			Rydot::Rect3f bb(-1,-1,-1,1,1,1);
			for(size_t i=0;i<strokes.size();++i)
			{
				glBegin(GL_LINE_STRIP);
				const std::vector<Vector3> &s=strokes[i].Get();
				for(size_t k=0;k<s.size();++k)
				{
					const Vector3 r=bb.TransformFrom(Rydot::MeshDeformer::beziercube(control, bb.TransformTo(s[k])));
					glVertex3f(r.x, r.y, r.z);
				}
				glEnd();
			}
		}


		// qhshow
		{
			glColor4f(1,0,0,0.5);
			for(size_t i=0;i<qhfcs.size();++i)
			{
				glBegin(GL_LINE_STRIP);
				for(size_t j=0;j<4;++j)
				{
					size_t k=j%3;
					const Vector3 p(qhpts[qhfcs[i][k]]);
					glVertex3fv(&p.x);
				}
				glEnd();
			}
		}

		this->theta.update();
		this->phi.update();
		this->dist.update();
	}



	void Animate()
	{
		//glutPostRedisplay();
		
		//t++;
		t=20;
	}



	void Mouse(const MouseEvent &me)
	{
		mousex=me.X();
		mousey=me.Y();

		if(me.IsClicked())
		{
			if(me.Button()==GLUT_MIDDLE_BUTTON)
			{
				rdown = (me.State()==GLUT_DOWN);
			}
			if(me.Button()==GLUT_LEFT_BUTTON)
			{
				pdown = (me.State()==GLUT_DOWN);

				if(pdown)
				{
					if(strokes.empty())
					{
					}
					if(!strokes.back().Get().empty())
					{
						strokes.push_back(Stroke());
					}
				}
				else
				{
					if(!strokes.empty() && !strokes.back().Get().empty())
						strokes.back().Finalize();
					CalcQH();
				}
			}
		}

		if(rdown)
		{
			float dx=mousex-tmpmousex;
			float dy=mousey-tmpmousey;

			Rect2 rt=v.GetView();
			float yy=rt.Diagonal().y;

			theta.set(theta.getrefevernce()-dx/yy*3.0);
			phi.set(phi.getrefevernce()+dy/yy*3.0);

			if(phi.getrefevernce()>1.5)phi.set(1.5);
			if(phi.getrefevernce()<-1.5)phi.set(-1.5);

		}
		
		if(me.IsWheel())
		{
			dist.set(dist.get()*(1.0-me.Dir()*0.2));
		}

		tmpmousex=mousex;
		tmpmousey=mousey;
		
		std::cout<<mousex<<"	"<<mousey<<std::endl;

		//glutPostRedisplay();
	}


	void Resize(int width, int height)
	{
		v.SetView(Rect2(0,0,float(width),float(height)));
		v.Apply();
	}
};


