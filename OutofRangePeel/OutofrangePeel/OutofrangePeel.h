
#pragma once

#include <math.h>
#include "Events.h"
#include "matrix.h"
#include "matrixutil.h"
#include "dashpod.h"

#include "View.h"
#include "mesh.h"



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

	std::vector<Vector3> record;


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
			Vector3(0,0,dist.get()),
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

		// ラティス変形してみる

		// 制御点を設定
		std::vector<Vector3> control(64);
		for(int a=0;a<64;a++)
		{
			int i=a%4,j=(a/4)%4,k=a/16;

			Vector3 displacement(0,0,0);
			if(j==3)
			{
				displacement=Vector3(0,-0.05,0);
				//if( (i==1||i==2)&&(k==1||k==2) )
				//{
				//	displacement=Vector3(0,0,0);
				//}
			}
			if(j==0)
			{
				displacement=Vector3(0,0.05,0);
				if( (i==1||i==2)&&(k==1||k==2) )
				{
					displacement=Vector3(0,0.25,0);
				}
			}
			control[a]=Vector3( i/3.0, (j-1.5)/3.0*0.95+0.5, k/3.0 )+displacement;
		}

		Rydot::MeshDeformer::LatticeDeform(m, control);
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

		glEnable(GL_DEPTH_TEST);

		glEnable(GL_NORMALIZE);

		glShadeModel(GL_SMOOTH);

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

		int n=6;
		for(int i=0;i<n*n;i++)
		{
			glPushMatrix();

			glScalef(0.2f,0.2f,0.2f);
			glTranslatef((i%n-(n-1)/2.0)*2,(i/n-(n-1)/2.0)*2,0);

			glBegin(GL_TRIANGLES);
				/* 1つ目の三角形 */
				glTexCoord2f(0,0);
				glNormal3f(-1,-1,3);
				glVertex3f(-1,-1,0);

				glTexCoord2f(1,0);
				glNormal3f(1,-1,3);
				glVertex3f(1,-1,0);

				glTexCoord2f(1,1);
				glNormal3f(1,1,3);
				glVertex3f(1,1,0);

				/* 2つ目の三角形 */
				glTexCoord2f(1,1);
				glNormal3f(1,1,3);
				glVertex3f(1,1,0);

				glTexCoord2f(0,1);
				glNormal3f(-1,1,3);
				glVertex3f(-1,1,0);

				glTexCoord2f(0,0);
				glNormal3f(-1,-1,3);
				glVertex3f(-1,-1,0);
			glEnd();

			glPopMatrix();
		}

		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);

		glColor3f(1,1,1);
		glPointSize(5);
		glBegin(GL_POINTS);
			glVertex3f(light_position[0],light_position[1],light_position[2]);
			glVertex3f(light_position2[0],light_position2[1],light_position2[2]);
		glEnd();

		glColor3f(0.95f,0.6f,0.1f);
		glBegin(GL_TRIANGLES);
		for(size_t i=0;i<m.faces.size();++i)
		{
			for(size_t v=0;v<3;++v)
			{
				glVertex3fv(&(m.vertex[m.faces[i].vertex[v]].x));
			}
		}
		glEnd();

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
					glPointSize(10);
					glColor3f(1,0,0);
					glBegin(GL_POINTS);
					glVertex3f(coll.x, coll.y, coll.z);
					glEnd();
					if(pdown)
					{
						record.push_back(coll);
					}
					break;
				}
			}

			glPointSize(3);
			glColor3f(0,0,1);
			glBegin(GL_POINTS);
			for(size_t i=0;i<record.size();++i)
			{
				const Vector3 &r=record[i];
				glVertex3f(r.x, r.y, r.z);

			}
			glEnd();
		}

		this->theta.update();
		this->phi.update();
		this->dist.update();
	}



	void Animate()
	{
		//glutPostRedisplay();
		
		t++;
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
		v.SetView(Rect2(0,0,width,height));
		v.Apply();
	}
};


