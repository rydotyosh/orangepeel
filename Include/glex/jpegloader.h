
// jpegloader.h
//
// jpeg loader
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

//jpegÇTextureÇ…ÉçÅ[ÉhÇ∑ÇÈ

#pragma once

#include <libjpeg/jpeglib.h>
#include <setjmp.h>
#include "texturefileloader.h"

/*
#ifdef _DEBUG
#pragma comment(lib,"libjpegmdd.lib")
#else
#pragma comment(lib,"libjpegmd.lib")
#endif
*/

namespace Rydot
{

class JpegLoader:public ITextureFileLoader
{
public:
	JpegLoader(Texture &tex,std::string fn){Load(tex,fn);}
private:

	struct Jpeg_ErrorManager
	{
		jpeg_error_mgr m_field;
		jmp_buf m_buffer;
	};

	static void Jpeg_exitByLongJump(j_common_ptr pInfo)
	{
		Jpeg_ErrorManager *pErrManager=
			reinterpret_cast<Jpeg_ErrorManager*>(pInfo->err);
		longjmp(pErrManager->m_buffer,1);
	}
	
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

		jpeg_decompress_struct jpg;
		jpeg_create_decompress(&jpg);

		Jpeg_ErrorManager ErrManager;
		jpg.err=jpeg_std_error(&ErrManager.m_field);
		ErrManager.m_field.error_exit=JpegLoader::Jpeg_exitByLongJump;

		if(setjmp( ErrManager.m_buffer ))
		{
			jpeg_destroy_decompress(&jpg);
			return -1;
		}

		jpeg_stdio_src(&jpg,f.get());
		jpeg_read_header(&jpg,TRUE);
		jpeg_start_decompress(&jpg);

		int row=jpg.output_width*jpg.output_components;

		JSAMPARRAY ppBuf=(*jpg.mem->alloc_sarray)
			(reinterpret_cast<j_common_ptr>(&jpg),
			JPOOL_IMAGE,row,1);

		int *image=tex.Create(jpg.output_width,jpg.output_height);
		tex.Fill(0xff000000);
		//int w2=m_width2,h2=m_height2;
		int w2=Texture::powlog2(jpg.output_width);

		while(jpg.output_scanline<jpg.output_height)
		{
			jpeg_read_scanlines(&jpg,ppBuf,1);
			JSAMPROW pBuf=*ppBuf;

			for(unsigned int x=0;x<jpg.output_width;++x)
			{
				TARGB px;
				if(jpg.out_color_components==3)
				{
					px.R=*pBuf;++pBuf;
					px.G=*pBuf;++pBuf;
					px.B=*pBuf;++pBuf;
				}
				else
				{
					px.R=px.G=px.B=*pBuf;++pBuf;
				}
				px.A=255;
				*image=px.ARGB;++image;
			}
			image+=w2-jpg.output_width;
		}

		jpeg_finish_decompress(&jpg);
		jpeg_destroy_decompress(&jpg);
		}
		
		
		tex.FitToPow2();
		
		//InnerResize(inw,inh);

		//Set(clip,interpolate);
		
		tex.SetImage();

		Success();
		return 0;
	}
};



}//end of namespace Rydot

