
// texturefileloader.h
//
// texture file loader
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

//Texture�t�@�C�������[�h��̃x�[�X�N���X���ۂ����

#pragma once

#include <stdio.h>
#include <string>
#include "texture.h"
#include <fileex.h>

namespace Rydot
{

class ITextureFileLoader
{
public:
	ITextureFileLoader():succeed(false){}
	virtual ~ITextureFileLoader(){}

	bool IsSucceed(){return succeed;}
protected:
	bool succeed;

	void Success(){succeed=true;}

	virtual int Load(Texture &tex,std::string fn)=0;
};

}//end of namespace Rydot


