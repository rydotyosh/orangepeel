
// csocket.h
//
// socket class
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// Linux と Windows 共用のソケットクラスのはず?
// スレッドセーフではないかもしれない(だめじゃん)

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
	#include <windows.h>
	#pragma comment(lib,"ws2_32.lib")
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <unistd.h>
	#include <arpa/inet.h>
#endif

#include <cthread.h>
#include <singleton.h>

namespace Rydot
{

#ifdef _WIN32
class WSA
{
private:
	WSADATA wsaData;
public:
	WSA()
	{ 
		WSAStartup(MAKEWORD(1,1), &wsaData);
		
		//printf("maxsocket:%d\n",wsaData.iMaxSockets);
	}
	~WSA()
	{
		//printf("wsacleanup\n");
		WSACleanup();
	}
};
#endif

class Socket
{
protected:
	#ifdef _WIN32
		SOCKET sock;
		//static WSA wsa;
		singleton<WSA> env;
	#else
		int sock;
		singleton<int> env;
	#endif
	int *count;
	
	criticalsection cs;
	
public:

	// default constructor
	Socket():sock(0),count(0)
	{
		synchronized sync(cs);
		env.instance();
		
		sock = socket(AF_INET, SOCK_STREAM, 0);
		count=new int;
		*count=1;
		//printf("socket default constructor count:%d\n",*count);
	}

	// copy constructor
	Socket(Socket &s):sock(s.sock),count(s.count)
	{
		synchronized sync(cs);
		env.instance();
		
		(*s.count)++;
		
		//printf("socket copy count:%d,%d\n",*count,*s.count);
	}
	
	// copy operator
	Socket &operator=(Socket &s)
	{
		synchronized sync(cs);
		
		sock=s.sock;
		count=s.count;
		(*s.count)++;
		
		//printf("socket copy count:%d,%d\n",*count,*s.count);
		
		return *this;
	}

public:
	// constructor with socket handle
	#ifdef _WIN32
	Socket(SOCKET s):sock(s),count(0)
	#else
	Socket(int s):sock(s),count(0)
	#endif
	{
		synchronized sync(cs);
		env.instance();
		
		count=new int;
		*count=1;
		
		//printf("socket handle constructor count:%d\n",*count);
	}
	
public:
	// destructor
	~Socket()
	{
		synchronized sync(cs);
		
		(*count)--;
		//printf("socket destructor count:%d\n",*count);
		if(*count==0&&sock)
		{
			#ifdef _WIN32
				closesocket(sock);
			#else
				close(sock);
			#endif
			sock=0;
			delete count;
			
			//printf("delcount\n");
		}
	}
	
	// 有効かどうか
	bool IsValid()
	{
		return sock;
	}
	
	// send
	// buf: sending data
	// len: bytes of data
	int Send(const char *buf,int len)
	{
		#ifdef _WIN32
			return send(sock,buf,len,0);
		#else
			return write(sock,buf,len);
		#endif
	}
	
	// recieve
	// buf: area for recieving data
	// len: bytes of area of buf
	// returns: bytes of data
	int Recv(char *buf,int len)
	{
		#ifdef _WIN32
			return recv(sock,buf,len,0);
		#else
			return read(sock,buf,len);
		#endif
	}
	
	int Connect(int port,const char *addr)
	{
		hostent *he;
		int ad;
		sockaddr_in server;
		server.sin_family = AF_INET;
		server.sin_port = htons(port);
		#ifdef _WIN32
			he=gethostbyname(addr);
			if(!he)
			{
				return -1;
			}
			//server.sin_addr.S_un.S_addr = inet_addr(addr);

			unsigned int **addr_p=(unsigned int **)he->h_addr_list;
			
			while(*addr_p!=NULL)
			{
				server.sin_addr=*((in_addr*)*addr_p);
				if(connect(sock,(sockaddr *)&server, sizeof(server))==0)
				{
					return 0;
				}
				++addr_p;
			}
			return -1;

		#else
			server.sin_addr.s_addr = inet_addr(addr);
		#endif
		return connect(sock,(sockaddr *)&server, sizeof(server));
	}

public:

	//テキトーなサーバ名取得
	static int GetServerNameFromURL(char *sv,const char *url)
	{
		int n=strlen(url);
		int j=0;
		int sct=0;
		for(int i=0;i<n;i++)
		{
			if(url[i]=='/')
			{
				if(sct==0)
					sct=1;
				if(sct==2)
				{
					sv[j]=0;
					return j;
				}
			}
			else
			{
				if(sct==1||sct==2)
				{
					sct=2;
					sv[j]=url[i];
					j++;
				}
			}
		}
		sv[j]=0;
		return j;
	}
};

class Server:public Socket
{
protected:
	sockaddr_in client;
	
	enum{BACK_LOG=5};

public:
	int Listen(int port,char *addr)
	{
		client.sin_family = AF_INET;
		client.sin_port = htons(port);
		#ifdef _WIN32
			if(addr==0)
				client.sin_addr.S_un.S_addr = INADDR_ANY;
			else
				client.sin_addr.S_un.S_addr = inet_addr(addr);
		#else
			if(addr==0)
				client.sin_addr.s_addr = INADDR_ANY;
			else
				client.sin_addr.s_addr = inet_addr(addr);
		#endif
		if(bind(sock,(sockaddr *)&client, sizeof(client)))return -1;
		if(listen(sock,5))return -1;
		
		return 0;
	}
	
	Socket Accept()
	{
		#ifdef _WIN32
			int l=sizeof(client);
			SOCKET sock2=accept(sock,(sockaddr *)&client,&l);
			if(sock2<0)
			{
				Socket s(0);
				return s;
			}
			else
			{
				Socket s(sock2);
				return s;
			}
		#else
			int l=sizeof(client);
			sock2=accept(sock,(sockaddr *)&client,(socklen_t *)&l);
			if(sock2<0)return Socket(0);
			return Socket(sock2);
		#endif
	}
};

};

/*
うざげ
usage:

// Client.cpp

Client cl;
cl.Connect(12345,"127.0.0.1");
cl.Send("aiueo",5);


// Server.cpp

Server sv;
sv.Listen(12345,NULL);
Socket sc=sv.Accept();
if(sc.IsValid())
{
	char buf[256];
	sc.Recv(buf,sizeof(buf));
}


*/


