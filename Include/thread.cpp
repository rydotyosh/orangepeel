

#include <windows.h>
#include <iostream>

// �g���Ƌ��{��������e�L�g�[�ȃX���b�h�B
// ���Ԃ񐳂��������B���A�ۏ�͂��Ȃ��B

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

class synchronized
{
private:
	criticalsection &cs;
public:
	synchronized(criticalsection &_cs):cs(_cs){cs.enter();}
	~synchronized(){cs.leave();}
};

////////////////////////////////////////
// �X���b�h�N���X

class thread
{
private:
	DWORD id;
public:
	thread(){}
	virtual ~thread(){}
	static DWORD WINAPI _run(LPVOID param)
	{
		return ((thread*)param)->run();
	}
	HANDLE start()
	{
		SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES),NULL,FALSE};
		return CreateThread(&sa,0,thread::_run,this,0,&id);
	}
	virtual int run()
	{
		return 0;
	}
};



////////////////////////////////////////
// �g���Ă݂�B

// ���\�[�X

class resource
{
private:
	int count;
	int count2;
	int termination;
	criticalsection cs; // �����ɃN���e�B�J���Z�N�V������u��

	// �N���X���K�[�h�������Ƃ��͂���Ȋ����̂���g��
	// static criticalsection scs;
public:
	resource():count(0),count2(0),termination(0){}
	int write(int c)
	{
		synchronized s(cs);//�V���N���������֐��̓��ɒu��
		count=c;
		DWORD t=GetCurrentThreadId();
		count2++;
		Sleep(10);
		std::cout<<"w:"<<t<<"/"<<count<<","<<count2<<std::endl;
		return termination;
	}
	int read(int &c,int &c2)
	{
		synchronized s(cs);
		DWORD t=GetCurrentThreadId();
		c=count;
		c2=count2;
		Sleep(10);
		std::cout<<"r:"<<t<<"/"<<count<<","<<count2<<std::endl;
		return termination;
	}
	void end()
	{
		synchronized s(cs);
		termination=1;
	}
};

// �X���b�h�œ����N���X
// run�֐����I�[�o�[���C�h����ׂ��B

class writer:public thread
{
private:
	resource &res;
public:
	writer(resource &r):res(r){}
	int run()
	{
		std::cout<<"writer start"<<std::endl;
		while(1)
		{
			int r=res.write(rand());
			if(r)break;
			Sleep(750);
		}
		std::cout<<"writer end"<<std::endl;
		return 0;
	}
};

class reader:public thread
{
private:
	resource &res;
public:
	reader(resource &r):res(r){}
	int run()
	{
		std::cout<<"reader start"<<std::endl;
		while(1)
		{
			int a,b;
			int r;
			r=res.read(a,b);
			if(r)break;
			Sleep(500);
		}
		std::cout<<"reader end"<<std::endl;
	}
};

int main()
{
	resource res;

	writer w(res);//�X���b�h�œ����N���X�̃C���X�^���X�����
	reader r(res);
	HANDLE hw=w.start();//�X���b�h���N������
	HANDLE hr=r.start();
	
	while(1)
	{
		Sleep(1);
		if(GetAsyncKeyState(VK_SPACE)&0x8000)break;
	}
	res.end();//�X���b�h���I������񂶂�Ȃ��Ƃ��낪�����E�E�E

	HANDLE h[]={hw,hr};
	WaitForMultipleObjects(2,h,TRUE,1000);
}