
// bmploader.h
//
// windows bitmap loader
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

//bmpをTextureにロードする

#pragma once

#include "texturefileloader.h"

#ifdef _DEBUG
#pragma comment(lib,"libjpegd.lib")
#else
#pragma comment(lib,"libjpeg.lib")
#endif

namespace Rydot
{

class BmpLoader:public ITextureFileLoader
{
public:
	BmpLoader(Texture &tex,std::string fn){Load(tex,fn);}
private:

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

	int Load(Texture &tex,std::string fn)
	{
		{
		file f(fn,"rb");
		if(!f.get())return -1;

		BITMAPFILEHEADER fh;
		BITMAPINFOHEADER ih;
		f.read(&fh,sizeof(fh),1);

		if(fh.bfType!=0x4d42)//BM
			return -1;
		
		f.read(&ih,sizeof(ih),1);

		int *image=tex.Create(ih.biWidth,ih.biHeight);
		tex.Fill(0xff000000);

		int index=ih.biClrUsed;
		int bit=ih.biBitCount;
		if(bit==8)
		{
			std::vector<int> idx;
			if(index==0)//グレイスケールにしちゃう。
			{
				idx.resize(256);
				for(int i=0;i<256;i++)
				{
					idx[i]=i|(i<<8)|(i<<16);
				}
			}
			else
			{
				idx.resize(index+1);
				f.read(&idx[0],index*4,1);
				for(int i=0;i<index;i++)
				{
					TARGB px;
					px.R=(idx[i]>>16)&0xff;px.G=(idx[i]>>8)&0xff;px.B=(idx[i])&0xff;
					px.A=255;
					idx[i]=px.ARGB;
				}
			}

			{
				std::vector<unsigned char> tmp;
				int sz=fh.bfOffBits-sizeof(fh)-sizeof(ih)-index*4;
				tmp.resize(sz+1);
				f.read(&tmp[0],sz,1);
			}
			//f.seek(fh.bfOffBits,SEEK_SET);
			
			std::vector<unsigned char> bmp;
			int pitch=((ih.biWidth+3)>>2)<<2;
			bmp.resize(pitch+1);
			
			int w2=Texture::powlog2(ih.biWidth);

			for(int i=0;i<ih.biHeight;i++)
			{
				int iw=(ih.biHeight-i-1)*w2;
				f.read(&bmp[0],pitch,1);
				for(int j=0;j<ih.biWidth;j++)
				{
					image[iw+j]=idx[bmp[j]];
				}
			}	
		}
		else if(bit==24)//fullcolor
		{
			//f.seek(fh.bfOffBits,SEEK_SET);
			{
				std::vector<unsigned char> tmp;
				int sz=fh.bfOffBits-sizeof(fh)-sizeof(ih);
				tmp.resize(sz+1);
				f.read(&tmp[0],sz,1);
			}
			
			std::vector<unsigned char> bmp;
			int pitch=((ih.biWidth*3+3)>>2)<<2;
			bmp.resize(pitch+1);
			
			int w2=Texture::powlog2(ih.biWidth);

			for(int i=0;i<ih.biHeight;i++)
			{
				int iw=(ih.biHeight-i-1)*w2;
				f.read(&bmp[0],pitch,1);
				for(int j=0;j<ih.biWidth;j++)
				{
					TARGB px;
					px.R=bmp[j*3+2];px.G=bmp[j*3+1];px.B=bmp[j*3+0];
					px.A=255;
					image[iw+j]=px.ARGB;
				}
			}
		}
		else
		{
			return -1;//not support
		}
		}
		
		
		tex.FitToPow2();
		
		//InnerResize(inw,inh);
		
		//tex.Set(clip,interpolate);
		
		tex.SetImage();

		Success();
		return 0;
	}
};



}//end of namespace Rydot

