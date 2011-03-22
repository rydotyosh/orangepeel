
// file.h
//
// file
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// デストラクタで勝手に閉じる
// FILEのテキトーなラッパ

#pragma once

#include <stdio.h>
#include <string>

namespace Rydot
{

class file
{
protected:
	FILE *fp;
public:
	file(std::string fn,std::string attr)
	{fp=fopen(fn.c_str(),attr.c_str());}
	~file()
	{if(fp){fclose(fp);fp=0;}}
	size_t read(void *buffer, size_t size, size_t count)
	{
		if(fp)
			return fread(buffer,size,count,fp);
		return 0;
	}
	size_t write(const void*buffer, size_t size, size_t count)
	{
		if(fp)
			return fwrite(buffer,size,count,fp);
		return 0;
	}
	int eof()
	{
		if(fp)
			return feof(fp);
		return -1;
	}
	int flush()
	{
		if(fp)
			return fflush(fp);
		return 0;
	}
	int seek(int offset,int origin)
	{
		if(fp)
			return fseek(fp,offset,origin);
		return -1;
	}
	FILE *get(){return fp;}
};

}