
// texturebase.h
//
// texture
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

//融通の利かない謎のGL拡張。
//jpeg,png,bmpのロードができる

#pragma once

#ifndef __RYDOT_TEXTURE_BASE_H__
#define __RYDOT_TEXTURE_BASE_H__

#include <stdlib.h>
#include <gl/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "matrixutil.h"

#include <windows.h>

#include <vector>
#include <string>

// signed とunsignedの不一致を無視 
//#pragma warning( disable:4018)

namespace Rydot
{

enum ClippingState
{
	CLIP_REPEAT=GL_REPEAT,
	CLIP_CLAMP=GL_CLAMP
};

enum InterpolatingState
{
	INTERPOLATE_NEAREST=GL_NEAREST,
	INTERPOLATE_LINEAR=GL_LINEAR
};

class Texture
{
protected:
	int *m_image;
	int m_id;
	int m_width,m_height,m_width2,m_height2;
public:
	union TARGB
	{
		unsigned int ARGB;
		struct
		{
			unsigned char R;
			unsigned char G;
			unsigned char B;
			unsigned char A;
		};
	};

	static int powlog2(int v)
	{
		int i=0;
		v--;
		while(v){i++;v>>=1;}
		return 1<<i;
	}

public:
	Texture():m_image(0),m_id(0){}
//	Texture(const Texture &_t):m_image(0){Copy(_t);}
//	Texture &operator=(const Texture &_t){Copy(_t);return *this;}
	virtual ~Texture(){Release();}

/*	virtual void Copy(const Texture &_t)
	{
		if(&_t==this)return;
		if(_t.m_image==0)
		{
			Release();
			return;
		}
		Release();
		m_width=_t.m_width;m_height=_t.m_height;
		m_width2=_t.m_width2;m_height2=_t.m_height2;
		m_id=_t.m_id;
		m_image=new int[m_width2*m_height2];
		memcpy(m_image,_t.m_image,sizeof(int)*m_width2*m_height2);

		glBindTexture(GL_TEXTURE_2D,m_id);
	}*/

	//width,heightは正の整数。ただしイメージマップは2のn乗に変更される。
	virtual int *Create(int width,int height)
	{
		Release();
		m_width2=powlog2(width);
		m_height2=powlog2(height);
		m_image=new int[m_width2*m_height2];
		m_width=width;
		m_height=height;

		//Textureをバインドする
		if(m_id==0)
			glGenTextures(1,(unsigned int*)&m_id);
		glBindTexture(GL_TEXTURE_2D,m_id);

		return m_image;
	}

	virtual void Release()
	{
		if(m_image)
		{delete[]m_image;m_image=0;}
	}

	//datで指定したデータで確保領域全体を埋める
	int Fill(int dat)
	{
		if(!m_image)return -1;

		int sz=m_width2*m_height2;
		for(int i=0;i<sz;i++)
		{
			m_image[i]=dat;
		}
		return 0;
	}

	/*int LoadFile(const char *fn,int clip=CLIP_CLAMP,int interpolate=INTERPOLATE_LINEAR,int inw=0,int inh=0)
	{
		int l=strlen(fn);

		if( (strcmp("jpg",fn+l-3)==0) || (strcmp("jpeg",fn+l-4)==0) )
		{
//			return LoadJpeg(fn,clip,interpolate,inw,inh);
			JpegLoader(*this,fn);
			SetClip(clip);
			SetInterpolate(interpolate);
		}
		if(strcmp("png",fn+l-3)==0)
		{
//			return LoadPng(fn,clip,interpolate,inw,inh);
			PngLoader(*this,fn);
			SetClip(clip);
			SetInterpolate(interpolate);
		}
		if(strcmp("bmp",fn+l-3)==0)
		{
//			return LoadBmp(fn,clip,interpolate,inw,inh);
			BmpLoader(*this,fn);
			SetClip(clip);
			SetInterpolate(interpolate);
		}

		return -1;
	}*/
	
	Quaternionf IntToQ(unsigned int color)
	{
		return Quaternionf((color>>16)&0xff,(color>>8)&0xff,(color)&0xff,color>>24);
	}
	int QToInt(Quaternionf q)
	{
		int r=q.x,g=q.y,b=q.z,a=q.w;
		if(r<0)r=0;if(r>255)r=255;
		if(g<0)g=0;if(g>255)g=255;
		if(b<0)b=0;if(b>255)b=255;
		if(a<0)a=0;if(a>255)a=255;
		return (a<<24)|(r<<16)|(g<<8)|(b);
	}
	
	int FitToPow2()
	{
		if(m_width!=m_width2||m_height!=m_height2)
		{
			std::vector<int> v;
			v.resize(m_width2*m_height2);
			int *nw=&v[0];
			FitToPow2i((unsigned int*)m_image,(unsigned int*)nw);
			memcpy(m_image,nw,m_width2*m_height2*4);
			
			m_width=m_width2;
			m_height=m_height2;
		}
		return 0;
	}
	
	int FitToPow2i(unsigned int *image,unsigned int *out)
	{
		if(m_width==m_width2&&m_height==m_height2)return 0;
		for(int i=0;i<m_height2;i++)
		{
			float y=((float)i*(m_height-1))/(m_height2-1);
			int y0=floor(y);
			int y1=y0+1;
			if(y1>=m_height2)y1=m_height2-1;
			float yr=y-y0;
			for(int j=0;j<m_width2;j++)
			{
				float x=((float)j*(m_width-1))/(m_width2-1);
				int x0=floor(x);
				int x1=x0+1;
				if(x1>=m_width2)x1=m_width2-1;
				float xr=x-x0;
				int p00=image[x0+y0*m_width2],p01=image[x1+y0*m_width2],
					p10=image[x0+y1*m_width2],p11=image[x1+y1*m_width2];
				Quaternionf q00=IntToQ(p00),q01=IntToQ(p01),
					q10=IntToQ(p10),q11=IntToQ(p11);
				Quaternionf r=Bilinear(q00,q01,q10,q11,xr,yr);
				int c=QToInt(r);
				out[j+i*m_width2]=c;
			}
		}
		return 0;
	}

	//半分のサイズにする
	int ScaleQ()
	{
		gluScaleImage(GL_RGBA,
			m_width2,m_height2,GL_UNSIGNED_BYTE,m_image,
			m_width2>>1,m_height2>>1,GL_UNSIGNED_BYTE,m_image);

		m_width2>>=1;m_height2>>=1;
		m_width>>=1;m_height>>=1;

		return 0;
	}
	
	//inw*inhに入るようなサイズにする
	int InnerResize(int inw,int inh)
	{
		if(inw>0)
		{
			while(m_width>inw)
			{
				ScaleQ();
			}
		}
		if(inh>0)
		{
			while(m_height>inh)
			{
				ScaleQ();
			}
		}
		return 0;
	}

	void SetImage()
	{
		//イメージを設定
		glTexImage2D
			(GL_TEXTURE_2D,0,GL_RGBA8,m_width2,m_height2,
			0,GL_RGBA,GL_UNSIGNED_BYTE,m_image);
	}
	
	void SetClip(int clip)
	{
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,clip);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,clip);
	}
	
	void SetInterpolate(int interpolate)
	{
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,interpolate);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,interpolate);
	}

	void Set(int clip,int interpolate)
	{
		//イメージを設定
		glTexImage2D
			(GL_TEXTURE_2D,0,GL_RGBA8,m_width2,m_height2,
			0,GL_RGBA,GL_UNSIGNED_BYTE,m_image);

		//クランプ
//		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
//		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		//繰り返し
//		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
//		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,clip);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,clip);

		//リニア補完
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,interpolate);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,interpolate);

	}

	//このテクスチャをアクティブにする
	void Activate()const{glBindTexture(GL_TEXTURE_2D,m_id);}

	int* GetImage()const{return m_image;}

	int GetWidth()const{return m_width;}
	int GetHeight()const{return m_height;}
	void SetWidth(int w){m_width=w;}
	void SetHeight(int h){m_height=h;}
	void SetSize(Vector2i &wh){m_width=wh.x;m_height=wh.y;}
	int GetID()const{return m_id;}
	Vector2f GetRatio()const
	{return Vector2f((float)m_width/m_width2,(float)m_height/m_height2);}
};


typedef Texture* pTex;



//DIBを用いたGL用テクスチャ。
//Setに時間がかかるのでリアルタイム性は微妙。
//DIBとか言っているがむしろText専用に近い。
class TextureDIB:public Texture
{
protected:
	int m_dc;
	int m_bmp;
	
public:

	TextureDIB():m_dc(0){}
	TextureDIB(const Texture &_t):Texture(_t),m_dc(0){}
	
	virtual ~TextureDIB()
	{if(m_dc){::DeleteDC((HDC)m_dc);m_dc=0;}Release();}
	
	//width,heightは正の整数。ただしイメージマップは2のn乗に変更される。
	virtual int *Create(int width,int height)
	{
		Release();

		if(!m_dc)
		{
			HDC hscreen;
			hscreen=::CreateDCA("DISPLAY",0,0,0);
			m_dc=(int)::CreateCompatibleDC(hscreen);
			::DeleteDC(hscreen);
		}
	
		m_width2=powlog2(width);
		m_height2=powlog2(height);

		BITMAPINFO bi;
		bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth=m_width2;  //横幅
		bi.bmiHeader.biHeight=m_height2;//高さ
		bi.bmiHeader.biPlanes=1;        //プレーンの数
		bi.bmiHeader.biBitCount=32;     //プレーンの色数
		bi.bmiHeader.biCompression=BI_RGB;
		bi.bmiHeader.biSizeImage=0;
		bi.bmiHeader.biXPelsPerMeter=0;
		bi.bmiHeader.biYPelsPerMeter=0;
		bi.bmiHeader.biClrUsed=0;
		bi.bmiHeader.biClrImportant=0;

		m_bmp=(int)::CreateDIBSection((HDC)m_dc,&bi,DIB_RGB_COLORS,(void**)&m_image,0,0);

		::SelectObject((HDC)m_dc,(HBITMAP)m_bmp);

		::SetBkMode((HDC)m_dc,TRANSPARENT);

		m_width=width;
		m_height=height;

		//Textureをバインドする
		if(!m_id)
			glGenTextures(1,(unsigned int*)&m_id);
		glBindTexture(GL_TEXTURE_2D,m_id);

		return m_image;
	}

	virtual void Release()
	{
		if(m_image)
		{
			::DeleteObject((HBITMAP)m_bmp);
			m_bmp=0;
			m_image=0;
		}
	}
	
	int GetDC()const{return m_dc;}

	int TextOut(int x,int y,const char *str,Vector2i *ext=0,int len=-1)
	{
		if(len<0)
		{
			len=strlen(str);
		}
		::TextOutA((HDC)m_dc,x,y,str,len);
		if(ext)
		{
			SIZE sz;
			::GetTextExtentPoint32A((HDC)m_dc,str,len,&sz);
			ext->x=sz.cx;
			ext->y=sz.cy;
		}
		return 0;
	}

	int SetTextColor(int col)
	{
		return ::SetTextColor((HDC)m_dc,col);
	}

	HGDIOBJ SelectObject(void *obj)
	{
		return ::SelectObject((HDC)m_dc,(HGDIOBJ)obj);
	}

};

}//end of namespace Rydot


#endif

