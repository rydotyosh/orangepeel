
// modelloader.h
//
// model loader
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// 3Dモデルを読み込んだりするクラスのインタフェースクラス

#pragma once

#include <string>
#include <fstream>

#include "mesh.h"
#include "motion.h"

namespace Rydot
{

class Model;
class Mesh;
class Face;
class Material;
class Pose;
class Motion;

class IModelLoader
{
protected:
	std::istream *i;
	bool correct;
public:
	IModelLoader():i(NULL),correct(false){}
	virtual ~IModelLoader(){if(i)delete i;}
	bool IsCorrect(){return correct;}
protected:
	//トークンを読み出す。
	//動作としては、バイオレーションをスペース、タブ、改行、カッコとして、
	//バイオレーション以外のものから、次のバイオレーションまでを読み出し、
	//バイオレーション位置にポインタを置く
	//ex. \t1\t2AAA(3 ptr:(3
	int ReadToken(std::string &s)
	{
		int n=0;
		while(1)
		{
			char c;
			c=i->peek();
			if(c==' '||c=='\t'||c=='\n'||c=='\r'||c=='('||c=='{')
			{if(n)return 0;}
			else{n++;s+=c;}
			i->get(c);	
		}
	}
	//文字列を読み出す。
	//動作としては、ダブルクォーテーションに挟まれた領域を読み出し、
	//後のダブルクォーテーションの1文字後にポインタを置く
	//ex.  123"AAA"456 ptr:4
	int ReadLit(std::string &s)
	{
		int n=-1;
		while(1)
		{
			char c;
			c=i->peek();
			if(c=='\x22')// "
			{
				if(n>=0){i->get(c);return 0;}
				else n=0;
			}
			else if(n>=0){n++;s+=c;}
			i->get(c);
		}
	}
	//endをこえるまでスキップ
	int SkipPreChar(char end)
	{
		while(1)
		{
			char c;
			i->get(c);
			if(c==end){return 0;}
		}
	}
	//スペース・タブ・改行を飛ばす
	int SkipSpace()
	{
		int n=0;
		while(1)
		{
			char c;
			c=i->peek();
			if(c==' '||c=='\t'||c=='\n'||c=='\r');
			else{return 0;}
			i->get(c);	
		}
	}
	//カッコ{}に囲まれた領域を読み込む
	int ReadBlock(std::string &s)
	{
		int n=-1;
		while(1)
		{
			char c;
			i->get(c);
			if(c=='}'){return 0;}
			else if(c=='{'){n=0;}
			else if(n>=0){n++;s+=c;}	
		}
	}
};

}//end of namespace Rydot

