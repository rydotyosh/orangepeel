
#pragma once

#include <math.h>
#include "matrix.h"
#include "matrixutil.h"



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
		int coedgeIndex;
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

	// returns adjacent faces from faces[fi]
	std::vector<int> FindAdjacentFaces(int fi)const
	{
		if(fi < 0 || fi >= faces.size())return std::vector<int>();

		const Face &f=faces[fi];
		std::vector<int> r;
		for(size_t i = 0; i < f.coedgeIndices.size(); ++i)
		{
			int k = coedges[f.coedgeIndices[i]].coedgeIndex;
			if(k >= 0)
			{
				r.push_back(coedges[k].faceIndex);
			}
		}
		return r;
	}

	// returns connected faces from faces[fi] where belongs to vertices[pi]
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
			for(int i = 0; i < working.size(); ++i)
			{
				std::vector<int> adj=FindAdjacentFaces(working[i]);
				for(int j = 0; j < adj.size(); ++j)
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
		for(int i = 0; i < mesh.size(); ++i)
		{
			const std::vector<int> &m = mesh[i];
			if(m.size() <= 2)continue;
			for(int j = 0; j + 2 < m.size(); ++j)
			{
				std::vector<int> v;
				v.push_back(m[0]);
				for(int k = 1; k < 3; ++k)
				{
					v.push_back(m[j + k]);
				}
				this->mesh.push_back(v);
			}
		}
		return true;
	}

	bool CreateFaces()
	{
		faces.clear();
		coedges.clear();

		faces.resize(mesh.size());
		coedges.resize(mesh.size() * 3);
		for(size_t i = 0; i < mesh.size(); ++i)
		{
			for(int k = 0; k < 3; ++k)
			{
				faces[i].coedgeIndices.push_back(i * 3 + k);
				Coedge &c = coedges[i * 3 + k];
				c.faceIndex = i;
				c.pointIndices.push_back(mesh[i][k]);
				c.pointIndices.push_back(mesh[i][(k + 1) % 3]);
			}
			for(int k = 0; k < 3; ++k)
			{
				faces[i].pointIndices.push_back(mesh[i][k]);
			}
		}

		// find sibling
		for(size_t i = 0; i < coedges.size(); ++i)
		{
			for(size_t k = i + 1; k < coedges.size(); ++k)
			{
				Coedge &c1 = coedges[i];
				Coedge &c2 = coedges[k];
				if(!c1.IsSibling(c2)) continue;

				c1.coedgeIndex = k;
				c2.coedgeIndex = i;
			}
		}

		return true;
	}

	bool CreateEdges()
	{
		edges.clear();
		edges.reserve(coedges.size());
		for(size_t i = 0; i < coedges.size(); ++i)
		{
			Coedge &c1 = coedges[i];
			size_t k = c1.coedgeIndex;

			if(k < 0)
			{
				// has no adjacent
				int eidx = edges.size();
				edges.push_back(Edge());
				Edge &e = edges.back();

				e.coedgeIndices.push_back(i);
				e.split = true;
				e.splPointIndices = c1.pointIndices;
				c1.edgeIndex = eidx;
				c1.sense = true;
				continue;
			}

			if(i > k) continue;

			// has adjacent
			Coedge &c2 = coedges[k];

			int eidx = edges.size();
			edges.push_back(Edge());
			Edge &e = edges.back();
			e.coedgeIndices.push_back(i);
			e.coedgeIndices.push_back(k);
			e.split = false;
			e.splPointIndices = c1.pointIndices;
			c1.edgeIndex = eidx;
			c2.edgeIndex = eidx;
			c1.sense = true;
			c2.sense = false;
		}

		return true;
	}

	// split coedges by cutter
	bool SplitByCutter()
	{
		for(size_t i = 0; i < cutter.size(); ++i)
		{
			const std::vector<int> &cu = cutter[i];
			Coedge tmp;
			tmp.pointIndices.push_back(cu[0]);
			tmp.pointIndices.push_back(cu[1]);

			for(size_t k = 0; k < coedges.size(); ++k)
			{
				Coedge &c1 = coedges[k];
				if(!c1.IsSibling(tmp)) continue;

				// split
				int cidx = coedges[k].coedgeIndex;
				if(cidx < 0) continue;

				Coedge &c2 = coedges[cidx];

				c1.coedgeIndex = -1;
				c2.coedgeIndex = -1;

				break;
			}
		}

		return true;
	}

	bool CreateSplit()
	{
		CreateFaces();
		SplitByCutter();
		CreateEdges();


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

