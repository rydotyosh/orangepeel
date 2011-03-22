
// metasequoialoader.h
//
// metasequoia model loader
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// メタセコイアで作った3Dモデルを読み込んだりするクラス

#pragma once

#include <string>
#include <fstream>

#include "matrix.h"

#include "mesh.h"
#include "motion.h"
#include "render.h"

#include "modelloader.h"

namespace Rydot
{

/////////////////////////////////////////////////
// Metasequoia Loader
//
class MetasequoiaLoader : public IModelLoader
{
public:
	MetasequoiaLoader(const std::string &filename,IRender &render,Model &model)
	{
		//mqoファイルを読み込む
		{
			i=new std::ifstream(filename.c_str(),std::ios::in|std::ios::binary);
			if(!LoadMqo(render,model))return;
			correct=true;
		}
	}
protected:
	bool LoadMqo(IRender &render,Model &model);
	bool LoadObject(Mesh &mesh);
	bool LoadFace(Face &face);
	bool LoadMaterial(Material &material);
};

/////////////////////////////////////////////////
// Mikoto Loader
//
class MikotoLoader : public IModelLoader
{
public:
	MikotoLoader(const std::string &filename,Pose &pose)
	{
		//mkmファイルを読み込む
		{
			i=new std::ifstream(filename.c_str(),std::ios::in|std::ios::binary);
			if(!LoadMkm(pose))return;
			correct=true;
		}
	}
protected:
	bool LoadMkm(Pose &pose);
	bool LoadMotion(Motion &motion);
};

};
