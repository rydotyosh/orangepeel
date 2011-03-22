
// texturemgr.h
//
// texture manager
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

//テクスチャの管理もどきができる
//開放をサポートしていないとか...

#pragma once

#include <stdlib.h>
#include <gl/glut.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <matrixutil.h>
#include <singleton.h>

#include "texturebase.h"

#include "bmploader.h"
#include "jpegloader.h"
#include "pngloader.h"

// TextureManagerはシングルトンとして使う。
// 全てのスコープでTextureManagerはユニークなはず。

namespace Rydot
{

class cTextureManager
{
public:
	typedef std::vector<Texture*> Texturepv;
	typedef std::vector<std::string> stringv;
protected:
	Texturepv m_tex;
	stringv m_texname;

protected:
	int LoadFile(Texture &tex,const std::string &fn,int clip=CLIP_CLAMP,int interpolate=INTERPOLATE_LINEAR,int inw=0,int inh=0)
	{
		int l=fn.length();

		if( (fn.compare(l-3,3,"jpg")==0) || (fn.compare(l-4,4,"jpeg")==0) )
		{
//			return LoadJpeg(fn,clip,interpolate,inw,inh);
			if(!JpegLoader(tex,fn).IsSucceed())return -1;
			tex.SetClip(clip);
			tex.SetInterpolate(interpolate);
			return 0;
		}
		if(fn.compare(l-3,3,"png")==0)
		{
//			return LoadPng(fn,clip,interpolate,inw,inh);
			if(!PngLoader(tex,fn).IsSucceed())return -1;
			tex.SetClip(clip);
			tex.SetInterpolate(interpolate);
			return 0;
		}
		if(fn.compare(l-3,3,"bmp")==0)
		{
//			return LoadBmp(fn,clip,interpolate,inw,inh);
			if(!BmpLoader(tex,fn).IsSucceed())return -1;
			tex.SetClip(clip);
			tex.SetInterpolate(interpolate);
			return 0;
		}

		return -1;
	}
public:
	cTextureManager()
	{
	}
	~cTextureManager()
	{
		Release();
	}
	int Release()
	{
		for(int i=0;i<m_tex.size();++i)
		{
			delete m_tex[i];
		}
		return 0;
	}
	int Load(std::string &fn,int clip=CLIP_REPEAT,int interpolate=INTERPOLATE_LINEAR,bool overwrite=false)
	{
		return Load(fn.c_str(),clip,interpolate,overwrite);
	}
	int Load(const char *fn,int clip=CLIP_REPEAT,int interpolate=INTERPOLATE_LINEAR,bool overwrite=false)
	{
		if(!overwrite)
		{
			for(int i=0;i<m_texname.size();i++)
			{
				if(strcmp(fn,m_texname[i].c_str())==0)
				{
					return i;
				}
			}
		}
		Texture *t=new Texture;
		if(LoadFile(*t,fn,clip,interpolate)!=0)
		{
			delete t;
			return -1;
		}
		return Append(fn,t,overwrite);
		/*m_tex.push_back(t);
		m_texname.push_back(std::string(fn));
		return m_texname.size()-1;*/
	}
	int Append(std::string &name,Texture *texture,bool overwrite=false)
	{
		return Append(name.c_str(),texture,overwrite);
	}
	int Append(const char *name,Texture *texture,bool overwrite=false)
	{
		for(int i=0;i<m_texname.size();i++)
		{
			if(strcmp(name,m_texname[i].c_str())==0)
			{
				if(overwrite)
				{
					if(m_tex[i]!=texture)
					{
						delete m_tex[i];
						m_texname[i]=name;
						m_tex[i]=texture;
					}
				}
				return i;
			}
		}
		m_tex.push_back(texture);
		m_texname.push_back(std::string(name));
		return m_texname.size()-1;
	}
	Texture *GetTexture(int i)const
	{
		return m_tex[i];
	}
	std::string GetTextureName(int i)const
	{
		return m_texname[i];
	}
	void Activate(int i)const
	{
		if(i<0)return;
		if(i>=m_tex.size())return;
		m_tex[i]->Activate();
	}
};

typedef singleton<cTextureManager> TextureManager;

}//end of namespace Rydot

