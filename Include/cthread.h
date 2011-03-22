
// cthread.h
//
// thread class
//
// Autohr:
// Ryogo Yoshimura (ry@jyoken.net)
// Takuya Hashimoto
//
// BSD license

// �g���Ƌ��{��������e�L�g�[�ȃX���b�h�B
// ���Ԃ񐳂��������B���A�ۏ�͂��Ȃ��B
// WinAPI��C-Runtime�ŌĂяo���K�񂪈Ⴄ����
// CreateThread��beginthread���g�������Ȃ��Ƃ����Ȃ��炵������
// �߂�ǂ�����_beginthreadex�����ɂ��Ă�B
// ����܂�悭�Ȃ����Ƃ͊m���B
// WinAPI��stdcall
// C-Runtime��cdecl
// �Ȃ񂾂�ȁ[�B

// todo:linux�p��posix������������

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
// �N���e�B�J���Z�N�V���������b�v

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
// �V���N���N���X

// �u���b�N�ň͂�Ŏ��̂����ƁA
// �u���b�N���甲����܂ŃV���O���ɂȂ�͂��B
// �������֐��u���b�N�ł�ok

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
// �e�L�g�[�ȃX���b�h�N���X

// �e�L�g�[�ɔh��������run����������
// new�ō����
// start���Ăяo���΂����͂��B

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
	
	//������I�[�o�[���C�h����B
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
// �e�L�g�[�ȃX���b�h�Ǘ�

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

