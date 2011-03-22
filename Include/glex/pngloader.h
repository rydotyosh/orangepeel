
// pngloader.h
//
// png loader
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

//pngをtextureにロードする

#pragma once

#include <libpng/png.h>
#include "texturefileloader.h"

#pragma comment(lib,"zlib.lib")
#ifdef _DEBUG
#pragma comment(lib,"libpngd.lib")
#else
#pragma comment(lib,"libpng.lib")
#endif

namespace Rydot
{

class PngLoader:public ITextureFileLoader
{
public:
	PngLoader(Texture &tex,std::string fn){Load(tex,fn);}
private:
	int Load(Texture &tex,std::string fn)
	{
		unsigned long width=0, height=0;
		int bit_depth=0, color_type=0, interlace_type;
		unsigned char **data;

		{
		//ファイルを開く
		file f(fn,"rb");
		if(!f.get())return -1;
		const size_t nSizeSig=8;
		const int nBytesSig=sizeof(unsigned char)*nSizeSig;
		unsigned char sig[nBytesSig];

		//PNGファイルかどうか確かめる	
		size_t nSizeReadSig=f.read(sig,sizeof(unsigned char),nSizeSig);
		if(nSizeReadSig!=nSizeSig)return -1;

		if(!png_check_sig(sig,nBytesSig))return -1;

		//png_struct構造体作成
		png_structp     png_ptr;
		png_ptr = png_create_read_struct(
						PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if(!png_ptr)return -1;

		//png_info構造体作成	
		png_infop       info_ptr;
		info_ptr = png_create_info_struct(png_ptr);
		if(!info_ptr)
		{
			png_destroy_read_struct(&png_ptr,NULL,NULL);
			return -1;
		}

		//エラー発生時用
		if(setjmp(png_ptr->jmpbuf))
		{
			if(data)
			{
				delete[]data;
			}
			png_destroy_read_struct(&png_ptr,NULL,NULL);
			return -1;
		}

		png_init_io(png_ptr, f.get());
		png_set_sig_bytes(png_ptr,nBytesSig);
		png_read_info(png_ptr, info_ptr);

		//IHDRチャンク情報を取得
		png_get_IHDR(png_ptr, info_ptr, &width, &height,
						&bit_depth, &color_type, &interlace_type,
						NULL, NULL);

		//パレット系->RGB系に拡張
		if(color_type==PNG_COLOR_TYPE_PALETTE ||
			(color_type==PNG_COLOR_TYPE_GRAY && bit_depth<8) ||
			png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))
		{
			png_set_expand(png_ptr);
		}

		//16ビット->8ビットに落とす
		if(bit_depth==16)
		{
			png_set_strip_16(png_ptr);
		}

		//グレースケール->RGBに拡張
		if(color_type==PNG_COLOR_TYPE_GRAY ||
			color_type==PNG_COLOR_TYPE_GRAY_ALPHA)
		{
			png_set_gray_to_rgb(png_ptr);
		}

		//αチャンネルが無い場合はα値に255を入れる
		if(color_type!=PNG_COLOR_TYPE_GRAY_ALPHA &&
			color_type!=PNG_COLOR_TYPE_RGB_ALPHA)
		{
			png_set_filler(png_ptr,255,PNG_FILLER_AFTER);
		}

		//γ補正する
		const double gammaLUT=1.0;
		const double gammaCRT=2.2;
		double dispExp=gammaLUT*gammaCRT;
		double gammaFile=0.0;
		if(png_get_gAMA(png_ptr,info_ptr,&gammaFile))
		{
			png_set_gamma(png_ptr,dispExp,gammaFile);
		}

		png_read_update_info(png_ptr,info_ptr);
		
		//画像領域作成
		int *image=tex.Create(width,height);
		tex.Fill(0xff000000);
		//int w2=m_width2,h2=m_height2;
		int w2=Texture::powlog2(width);

		data=new unsigned char*[height];

		for(int i=0;i<height;i++)
		{
			data[i]=(unsigned char*)&(image[i*w2]);
		}

		// 画像データを読み込み
		png_read_image(png_ptr, data);

		//PNGの構造体解放
		png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
		}
		
		tex.FitToPow2();
		
		//InnerResize(inw,inh);
		
		tex.SetImage();
		
		//Set(clip,interpolate);

		Success();
		return 0;
	}
};



}//end of namespace Rydot

