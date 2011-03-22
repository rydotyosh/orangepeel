
// cthread.h
//
// thread class
//
// Autohr:
// Ryogo Yoshimura (ry@jyoken.net)
// Takuya Hashimoto
//
// BSD license

// 吉村と橋本が作ったテキトーなスレッド。
// たぶん正しく動く。が、保障はしない。
// WinAPIとC-Runtimeで呼び出し規約が違うから
// CreateThreadとbeginthreadを使い分けないといけないらしいけど
// めんどいから_beginthreadexだけにしてる。
// あんまりよくないことは確か。
// WinAPIはstdcall
// C-Runtimeはcdecl
// なんだよなー。

// todo:linux用にposixも導入したい

#pragma once

#include <windows.h>
#include <process.h>
#include <iostream>

#include <list>
#include <vector>

#include "singleton.h"

#define for if(0);else for

namespace Rydot
{

////////////////////////////////////////
// クリティカルセクションをラップ

class criticalsection
{
private:
	CRITICAL_SECTION cs;
public:
	criticalsection(){InitializeCriticalSection(&cs);}
	~criticalsection(){DeleteCriticalSection(&cs);}
	void enter(){EnterCriticalSection(&cs);}
	void leave(){LeaveCriticalSection(&cs);}
};

////////////////////////////////////////
// シンクロクラス

// ブロックで囲んで実体を作ると、
// ブロックから抜けるまでシングルになるはず。
// もちろん関数ブロックでもok

class synchronized
{
private:
	criticalsection &cs;
public:
	synchronized(criticalsection &_cs):cs(_cs){cs.enter();}
	~synchronized(){cs.leave();}
};

class thread;
class threadmanager;

////////////////////////////////////////
// テキトーなスレッドクラス

// テキトーに派生させてrunを実装して
// newで作って
// startを呼び出せばいいはず。

class thread
{
private:
	unsigned int id;
	HANDLE hd;
	bool valid;
	
	//static threadmanager tm;
	singleton<threadmanager> tm;
	
	static unsigned int _stdcall _run(LPVOID param);
/*	{
		//while(!((thread*)param)->active){Sleep(0);}
		int r=((thread*)param)->run();
		//ExitThread(r);
		_endthreadex(r);
		((thread*)param)->tm.instance().endthread( *((thread*)param) );
		return r;
	}*/
	
	//これをオーバーライドする。
	virtual int run()
	{
		return 0;
	}

public:
	thread():id(0),hd(0),valid(false){}
	void start();
	virtual ~thread(){}

	void invalidate(){valid=false;}
	bool isvalid(){return valid;}

	static void sleep(int t=0)
	{
		::Sleep(t);
	}
	
friend threadmanager;
};

////////////////////////////////////////
// テキトーなスレッド管理

class threadmanager
{
private:
	std::vector<thread*> threads;
	std::vector<HANDLE> handles;
	criticalsection cs;

private:
	void append(thread &th)
	{
		synchronized sync(cs);
		threads.push_back(&th);
		handles.push_back(th.hd);
	}

public:
	void startthread(thread &th)
	{
		synchronized sync(cs);
		if(th.hd)return;
		SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES),NULL,FALSE};
		th.valid=true;
		//th.hd=CreateThread(&sa,0,thread::_run,&th,0,&(th.id));
		th.hd=(void*)_beginthreadex(&sa,0,thread::_run,&th,0,&(th.id));
		//if(!hd)throw std::string("cant create thread");
		if(th.hd==0)
		{
			//printf("cannot create thread\n");
			std::cerr<<"cannot create thread"<<std::endl;
			return;
		}
		//printf("thhd:%d id:%d\n",hd,id);
		append(th);
	}
	
	void endthread(thread &th)
	{
		synchronized sync(cs);
		
		//find same thread
		for(int i=0;i<threads.size();++i)
		{
			if(&th==threads[i])
			{
				//release
				
				CloseHandle(threads[i]->hd);
				try
				{
					delete threads[i];
				}
				catch(...)
				{
					std::cerr<<"threadmanager: except delete"<<std::endl;
				}
				
				//erase
				
				threads.erase(threads.begin()+i);
				handles.erase(handles.begin()+i);

				break;
			}
		}
	}

	~threadmanager()
	{
		//printf("~tm\n");
		//printf("nthread:%d\n",threads.size());
		
		if(threads.empty())return;
		
		for(std::vector<thread*>::iterator i=threads.begin();i!=threads.end();i++)
		{
			(*i)->invalidate();
		}

		//printf("wait for all thread...\n");
		
		//printf("%d\n",handles[0]);
		
		WaitForMultipleObjects(handles.size(),&(handles[0]),TRUE,10000);

		//printf("del\n");
		
		for(std::vector<thread*>::iterator i=threads.begin();i!=threads.end();i++)
		{
			CloseHandle((*i)->hd);
			try
			{
				delete (*i);
			}
			catch(...)
			{
				std::cerr<<"threadmanager: except delete"<<std::endl;
			}
		}
	}
};

void thread::start()
{
/*	if(hd)return hd;
	SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES),NULL,FALSE};
	active=true;
	hd=CreateThread(&sa,0,thread::_run,this,0,&id);
	//if(!hd)throw std::string("cant create thread");
	if(hd==0)printf("cant create thread\n");
	//printf("thhd:%d id:%d\n",hd,id);
	tm.append(this);
	return hd;*/
	
	//tm.startthread(*this);
	tm.instance().startthread(*this);
}

unsigned int _stdcall thread::_run(LPVOID param)
{
	//while(!((thread*)param)->active){Sleep(0);}
	int r=((thread*)param)->run();
	//ExitThread(r);
	//((thread*)param)->tm.instance().endthread( *((thread*)param) );
	_endthreadex(r);
	return r;
}

}//end of namespace Rydot

