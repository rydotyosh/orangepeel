
#pragma once

#include <math.h>
#include "Events.h"
#include "matrix.h"
#include "matrixutil.h"
#include "dashpod.h"

#include "View.h"
#include "mesh.h"

#include "QhullCalc.h"



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



class Peel
{
private:

	typedef Rydot::Vector3f	Vector3;

	std::vector<Vector3> points;
	std::vector<std::vector<int>> cutter;
	std::vector<std::vector<int>> mesh;

	std::vector<Vector3> splpoints;

	struct Face
	{
		std::vector<int> coedgeIndices;
	};

	struct Coedge
	{
		int faceIndex;
		int edgeIndex;
		std::vector<int> pointIndices;

		bool IsSibling(const Coedge &c)
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

	std::vector<Face> faces;
	std::vector<Coedge> coedges;
	std::vector<Edge> edges;

public:
	Peel()
	{
	}



	bool SetupPoints(const std::vector<Vector3> &points)
	{
		this->points=points;
		return true;
	}

	bool SetupCutter(const std::vector<std::vector<int>> &cutter)
	{
		this->cutter=cutter;
		return true;
	}

	bool SetupMesh(const std::vector<std::vector<int>> &mesh)
	{
		this->mesh=mesh;
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
					coedges[i].edgeIndex=eidx;
					coedges[k].edgeIndex=eidx;
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
						Edge &e=edges[eidx];
						coedges[e.coedgeIndices[0]].edgeIndex=-1;
						coedges[e.coedgeIndices[1]].edgeIndex=-1;
						e.split=true;
					}
					break;
				}
			}
		}

		// assign split points
		splpoints.clear();
		size_t ne=edges.size();
		for(size_t i=0;i<ne;++i)
		{
			if(edges[i].split)
			{
				edges.push_back(Edge());
				Edge &ei=edges[i];
				


			}


		}

		
		return false;
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
		double R=10;
		Stroke s;
		for(int i=0;i<400;++i)
		{
			double t=(double)i/(400.0-1.0);
			double z=cos(pi*t);
			double ss=sin(pi*t);
			double x=cos(2.0*pi*R*t)*ss;
			double y=sin(2.0*pi*R*t)*ss;
			s.Add(Vector3(x,z,y));
		}
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
		for(size_t i=0;i<strokes.size();++i)
		{
			const std::vector<Vector3> &s=strokes[i].Get();
			for(size_t k=0;k<s.size();++k)
			{
				vtx.push_back(Rydot::Vector3d(s[k]));
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
		std::vector<int> match(qhp.size());
		for(size_t i=0;i<qhp.size();++i)
		{
			double nearest=1e10;
			int idx=0;
			for(size_t k=0;k<vtx.size();++k)
			{
				double d2=qhp[i].Dist2(vtx[k]);
				if(d2<nearest)
				{
					nearest=d2;
					idx=k;
				}
			}
			match[i]=idx;
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


