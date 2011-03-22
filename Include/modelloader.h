
// modelloader.h
//
// model loader
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// 3D���f����ǂݍ��񂾂肷��N���X�̃C���^�t�F�[�X�N���X

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
	//�g�[�N����ǂݏo���B
	//����Ƃ��ẮA�o�C�I���[�V�������X�y�[�X�A�^�u�A���s�A�J�b�R�Ƃ��āA
	//�o�C�I���[�V�����ȊO�̂��̂���A���̃o�C�I���[�V�����܂ł�ǂݏo���A
	//�o�C�I���[�V�����ʒu�Ƀ|�C���^��u��
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
	//�������ǂݏo���B
	//����Ƃ��ẮA�_�u���N�H�[�e�[�V�����ɋ��܂ꂽ�̈��ǂݏo���A
	//��̃_�u���N�H�[�e�[�V������1������Ƀ|�C���^��u��
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
	//end��������܂ŃX�L�b�v
	int SkipPreChar(char end)
	{
		while(1)
		{
			char c;
			i->get(c);
			if(c==end){return 0;}
		}
	}
	//�X�y�[�X�E�^�u�E���s���΂�
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
	//�J�b�R{}�Ɉ͂܂ꂽ�̈��ǂݍ���
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

