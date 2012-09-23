/*
 *  DefaultEvent.h
 *  OutofrangePeel
 *
 *  Created by ryogo yoshimura on 12/09/23.
 *  Copyright 2012 c and g systems inc. All rights reserved.
 *
 */

#include "Events.h"
#include <vector>
#include <algorithm>

class LargeTextureTest
{
	typedef std::vector<unsigned char> vchar;
private:
	unsigned int imageid;
	vchar image;
	int tt;

public:
	LargeTextureTest():tt(0)
	{
	}
	void Initialize()
	{
		glGenTextures(1,(unsigned int*)&imageid);
		glBindTexture(GL_TEXTURE_2D,imageid);
		
		int N = 4096;
		image.resize(N*N*4);
		
		unsigned char *p=&image[0];
		for(int i=0;i<N;i++)
		{
			for(int j=0;j<N;j++,p+=4)
			{
				if( std::max(abs(int(i-2047.5)),abs(int(j-2047.5)))&1 )
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
		//	(GL_TEXTURE_2D,0,4,N,N,
		//	0,GL_RGBA,GL_UNSIGNED_BYTE,&image[0]);
		gluBuild2DMipmaps(
						  GL_TEXTURE_2D, GL_RGB, N, N,
						  GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
		
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		//glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		
		image.clear();
		vchar().swap(image);
	}
	
	void Display(float x)
	{
		glEnable(GL_TEXTURE_2D);
		
		glBindTexture(GL_TEXTURE_2D, imageid);
		
		tt++;
		
		double sz = sin(tt*0.01);
		sz*=sz;
		sz+=0.001;
		
		glBegin(GL_TRIANGLES);
		glTexCoord2f(0.0,0.0);
		glVertex3f(x,0,0);
		glTexCoord2f(sz,0.0);
		glVertex3f(x+1,0,0);
		glTexCoord2f(0.0,sz);
		glVertex3f(x,1,0);
		glEnd();
	}
};

class DefaultEvent : public IGLEvents
{
private:
	std::vector<LargeTextureTest> largeTextures;
public:
	virtual void Initialize()
	{
		largeTextures.resize(3);
		glClearColor(0.5,0.8,0.5,1.0);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		
		largeTextures[0].Initialize();
		largeTextures[1].Initialize();
		largeTextures[2].Initialize();
		
	}
	
	virtual void Display()
	{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		largeTextures[0].Display(0.0);
		largeTextures[1].Display(-0.5);
		largeTextures[2].Display(-1.0);
		
		
	}
	
	virtual void Animate()
	{
	}
	
	virtual void Mouse(const MouseEvent &me)
	{
	}
	
	virtual void Resize(int width, int height)
	{
	}
};
