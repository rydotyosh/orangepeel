

#include <windows.h>
#include <iostream>

// 吉村と橋本が作ったテキトーなスレッド。
// たぶん正しく動く。が、保障はしない。

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

class synchronized
{
private:
	criticalsection &cs;
public:
	synchronized(criticalsection &_cs):cs(_cs){cs.enter();}
	~synchronized(){cs.leave();}
};

////////////////////////////////////////
// スレッドクラス

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
// 使ってみる。

// リソース

class resource
{
private:
	int count;
	int count2;
	int termination;
	criticalsection cs; // ここにクリティカルセクションを置く

	// クラスをガードしたいときはこんな感じのやつを使う
	// static criticalsection scs;
public:
	resource():count(0),count2(0),termination(0){}
	int write(int c)
	{
		synchronized s(cs);//シンクロしたい関数の頭に置く
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

// スレッドで動くクラス
// run関数をオーバーライドするべし。

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

	writer w(res);//スレッドで動くクラスのインスタンスを作る
	reader r(res);
	HANDLE hw=w.start();//スレッドを起動する
	HANDLE hr=r.start();
	
	while(1)
	{
		Sleep(1);
		if(GetAsyncKeyState(VK_SPACE)&0x8000)break;
	}
	res.end();//スレッドを終了するんじゃないところが微妙・・・

	HANDLE h[]={hw,hr};
	WaitForMultipleObjects(2,h,TRUE,1000);
}