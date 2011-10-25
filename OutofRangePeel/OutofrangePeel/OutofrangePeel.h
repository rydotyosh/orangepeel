
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

#include "Stroke.h"
#include "CutMesh.h"



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
				//OutputDebugStringA(s);

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
	typedef Rydot::Face Face;

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

	// collision of mouse ray to mesh
	int collideFace;
	Vector3 collidePoint;

	//std::vector<Vector3> record;
	Stroke stroke;
	
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

		//strokes.push_back(Stroke());

		{
		stroke.Open();
		double pi=4.0*atan(1.0);
		double R=2;
		//Stroke s;
		int N=40;
		for(int i=0;i<N;++i)
		{
			double t=(double)i/(N-1.0);
			double z=cos(pi*t);
			double ss=sin(pi*t);
			double x=cos(2.0*pi*R*t)*ss;
			double y=sin(2.0*pi*R*t)*ss;
			stroke.Add(Vector3(x,z,y));
		}
		//Stroke s;
		//s.Add(Vector3(0,1,0));
		//s.Add(Vector3(1,0,0));
		//s.Add(Vector3(0,0,1));
		//s.Add(Vector3(-1,0,0));
		//s.Add(Vector3(0,0,-1));
		//s.Add(Vector3(0,-1,0));
		//s.Simplify();
		//s.Finalize();
		//strokes[0]=s;
		//strokes.push_back(Stroke());
		stroke.Close();
		stroke.Optimize();
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
		for(size_t i=0;i<stroke.Size();++i)
		{
			const std::vector<Vector3> &s=stroke.GetSeg(i);
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



	void Update()
	{

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
		glDepthMask(false);
		//glColor3f(0.95f,0.6f,0.1f);
		glBegin(GL_TRIANGLES);
		float orange[]={0.95f,0.6f,0.1f,0.7f};
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, orange);
		float amborange[]={0.95f,0.6f,0.1f,1};
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amborange);
		float emiorange[]={0.3f,0.2f,0.1f,1};
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emiorange);

		for(size_t i=0;i<m.faces.size();++i)
		{
			for(size_t v=0;v<3;++v)
			{
				size_t p=m.faces[i].vertex[v];
				glNormal3fv(&(m.normals[p].x));
				glVertex3fv(&(m.vertex[p].x));
			}
		}
		glEnd();
		glDisable(GL_LIGHTING);
		glDepthMask(true);

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


		// peeling motion

		//const std::vector<Peel::Face> &fcs=pe.faces;
		//const std::vector<Peel::Vertex> &vts=pe.vertices;
		//glBegin(GL_TRIANGLES);
		//for(size_t i=0;i<fcs.size();++i)
		//{
		//	for(size_t v=0;v<3;++v)
		//	{
		//		glColor3f((i%10)/10.0,((i/10)%10)/10.0,0);
		//		int idx=fcs[i].vertexIndices[2-v];
		//		Vector3 p=vts[idx].moved+Vector3(0.0,0.0,0.0);
		//		glVertex3fv(&(p.x));
		//	}
		//}
		//glEnd();

		//
		//pe.Proceed();

		{
			CalcCollision();

			glDisable(GL_DEPTH_TEST);

			if(collideFace >= 0)
			{
				glColor3f(0,1,0);
				if(pdown) glColor3f(1,0,0);

				glPointSize(10);
				glBegin(GL_POINTS);
				glVertex3f(collidePoint.x, collidePoint.y, collidePoint.z);
				glEnd();
			}

			bool scratch = stroke.IsScratch();

			glColor3f(0,0,1);
			Rydot::Rect3f bb(-1,-1,-1,1,1,1);
			for(size_t i=0;i<stroke.Size();++i)
			{
				if(i + 1 == stroke.Size() && scratch)
					glColor3f(1,0,0);
				glBegin(GL_LINE_STRIP);
				const std::vector<Vector3> &s=stroke.GetSeg(i);
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
			glColor4f(1,0,0,0.2);
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



	// calc collision of mouse ray to mesh
	void CalcCollision()
	{
		std::pair<Vector3, Vector3> r = v.Ray(Vector2(mousex + 1, mousey + 1));

		collideFace = -1;
		collidePoint = Vector3();

		//cross face
		for(size_t i = 0; i < m.faces.size(); ++i)
		{
			Vector3 coll;
			const Face &fc = m.faces[i];
			const std::vector<int> &fcv = fc.vertex;
			if(!Rydot::ReflectTriangleRay(
				r.first,
				r.first + r.second * 100,
				m.vertex[fcv[0]],
				m.vertex[fcv[2]],
				m.vertex[fcv[1]],
				coll)) continue;

			collideFace = i;
			collidePoint = coll;
			return;
		}
	}



	void Animate()
	{
		//glutPostRedisplay();
		
		//t++;
		t=20;
	}



	void Mouse(const MouseEvent &me)
	{
		mousex = me.X();
		mousey = me.Y();

		if(me.IsClicked())
		{
			if(me.Button() == GLUT_MIDDLE_BUTTON)
			{
				rdown = (me.State() == GLUT_DOWN);
			}
			if(me.Button() == GLUT_LEFT_BUTTON)
			{
				pdown = (me.State() == GLUT_DOWN);

				if(pdown)
				{
					//if(strokes.empty())
					//{
					//}
					//if(!stroke.back().Get().empty())
					//{
					//	strokes.push_back(Stroke());
					//}
					stroke.Open();
				}
				else
				{
					//if(!strokes.empty() && !strokes.back().Get().empty())
					//	strokes.back().Finalize();
					bool scratch = stroke.IsScratch();
					stroke.Close();
					if(scratch)
					{
						stroke.Erase();
					}
					else
					{
						stroke.Optimize();
					}
					CalcQH();
				}
			}
		}

		if(rdown)
		{
			float dx = mousex - tmpmousex;
			float dy = mousey - tmpmousey;

			Rect2 rt = v.GetView();
			float yy = rt.Diagonal().y;

			theta.set(theta.getrefevernce() - dx / yy * 3.0);
			phi.set(phi.getrefevernce() + dy / yy * 3.0);

			if(phi.getrefevernce() > 1.5) phi.set(1.5);
			if(phi.getrefevernce() < -1.5) phi.set(-1.5);

		}

		if(pdown)
		{
			CalcCollision();
			if(collideFace >= 0)
			{
				Vector3 coef;
				std::pair<Vector3, Vector3> r = v.Ray(Vector2(mousex + 1, mousey + 1));

				const std::vector<int> &fcv = m.faces[collideFace].vertex;
				size_t v0 = fcv[0];
				size_t v1 = fcv[1];
				size_t v2 = fcv[2];

				Rydot::ReflectTriangleRayTopology(
						r.first,
						r.first + r.second * 100,
						m.vertex[v0],
						m.vertex[v2],
						m.vertex[v1],
						coef);

				stroke.Add(
						originalVertices[v0] * (1 - coef.x - coef.y) 
						+ originalVertices[v2] * coef.x 
						+ originalVertices[v1] * coef.y);
				//strokes.back().Simplify();
			}
		}
		
		if(me.IsWheel())
		{
			dist.set(dist.get() * (1.0 - me.Dir() * 0.2));
		}

		tmpmousex = mousex;
		tmpmousey = mousey;
		
		std::cout << mousex << "\t" << mousey << std::endl;

		//glutPostRedisplay();
	}


	void Resize(int width, int height)
	{
		v.SetView(Rect2(0,0,float(width),float(height)));
		v.Apply();
	}
};


