
//コンパイル通るかどうかテストするやつ

#include <stdio.h>

//test socket and thread

#include "cthread.h"
#include "csocket.h"

using namespace Rydot;

class testsocketClient:public thread
{
public:
	int run()
	{
		Client cl;
		cl.Connect(12345,"127.0.0.1");
		cl.Send("aiueo",5);
	}
};

class testsocketServer:public thread
{
public:
	int run()
	{
		Server sv;
		sv.Listen(12345,NULL);
		Socket sc=sv.Accept();
		if(sc.IsValid())
		{
			char buf[256];
			sc.Recv(buf,sizeof(buf));
		}
	}
}


bool testcsocket()
{
	testsocketClient cl;
	cl.start();

	Server sv;
	sv.Listen(12345,NULL);
	Socket sc=sv.Accept();
	if(sc.IsValid())
	{
		char buf[256];
		sc.Recv(buf,sizeof(buf));
		
		if(memcmp(buf,"aiueo",5)==0)
		{
			printf("PASSED : test csocket\n");
		}
	}
}

int main()
{
	testcsocket();
	
	return 0;
}

