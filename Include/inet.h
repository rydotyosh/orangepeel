
// inet.h
//
// inet
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

//テキトーな
//inetを使うやつ
//微妙にマルチスレッドかもしれない

#pragma once

#include <windows.h>
#include <wininet.h>

#include "cthread.h"
#include "messagethread.h"

#pragma comment(lib,"wininet.lib")

namespace Rydot
{

class httpRequest;

class internetConnection
{
private:
	HINTERNET hInet,hConn;
public:
	internetConnection():hInet(0),hConn(0){}
	
	internetConnection(const std::string &host,const std::string &proxy="")
	{
		connect(host,proxy);
	}
	
	bool connect(const std::string &host,const std::string &proxy="")
	{
		if(proxy.empty())
			hInet = InternetOpenA("Rydot::InternetConnection", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
		else
			hInet = InternetOpenA("Rydot::InternetConnection", INTERNET_OPEN_TYPE_PROXY,proxy.c_str(), NULL, 0);
		
		if(!hInet)
		{
			//throw std::exception("cannot open internet");
			return false;
		}
		
		hConn=InternetConnectA(hInet, host.c_str(), INTERNET_DEFAULT_HTTP_PORT,
				NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
		
		if(!hConn)
		{
			//throw std::exception("cannot connect internet");
			InternetCloseHandle(hInet);
			hInet=0;
			return false;
		}
		
		return true;
	}
	
	void release()
	{
		if(hConn){InternetCloseHandle(hConn);hConn=0;}
		if(hInet){InternetCloseHandle(hInet);hInet=0;}
	}
	
	~internetConnection()
	{
		release();
	}

	friend class httpRequest;
};

class httpRequest:public thread
{
private:
	HINTERNET hRequest;
	int contentlength;
	
	int receivelength;
	bool isfinished;
	int error;
	
	typedef std::vector<BYTE> bv;
	typedef std::list<bv> bvl;
	
	//inner strage
	bool store;
	criticalsection cs;
	bvl buffer;
	int outputlength;
	
	//message
	typedef std::list<messagethread<bv>*> mtl;
	mtl handlers;
	
public:
	enum MethodType
	{
		HTTP_GET,
		HTTP_POST
	};
	
	enum ErrorType
	{
		ERROR_ILLEGAL_PARAM=1,
		ERROR_OPEN_REQUEST=2,
		ERROR_SEND_RESUEST=3,
		ERROR_QUERY_AVAILABLE=11,
		ERROR_READ_FILE=12,
		FORCE_CLOSE=21
	};
	
	static std::string getErrorString(int err)
	{
		std::string e;
		switch(err)
		{
		case ERROR_ILLEGAL_PARAM:	e="Illegal parameter";break;
		case ERROR_OPEN_REQUEST:	e="Cannot open request";break;
		case ERROR_SEND_RESUEST:	e="Cannot send request";break;
		case ERROR_QUERY_AVAILABLE:e="Not available data";break;
		case ERROR_READ_FILE:		e="Cannot read file";break;
		case FORCE_CLOSE:				e="Force quit";break;
		
		default:e="Unknown error";break;
		};
		return e;
	}
	
	httpRequest(internetConnection &connect,MethodType method,
		const std::string &path,const std::string& address,const std::string& postdata="")
	:hRequest(0),contentlength(-1),receivelength(0),isfinished(false),outputlength(0),store(true)
	{
		request(connect,method,path,address,postdata);
	}
	
	~httpRequest()
	{
		if(hRequest)
		{
			InternetCloseHandle(hRequest);
		}
	}
	
	void appendhandler(messagethread<bv> *h)
	{
		handlers.push_back(h);
		store=false;
	}

private:
	void request(internetConnection &connect,MethodType method,
		const std::string &path,const std::string& address,const std::string& postdata)
	{
		//method
		char meth[8]="";
		if(method==HTTP_GET)
		{
			strcpy(meth,"GET");
		}
		else if(method==HTTP_POST)
		{
			strcpy(meth,"POST");
		}
		else
		{
			error=ERROR_ILLEGAL_PARAM;
			return;
		}
		
		LPCSTR accept[]={"*/*",NULL};
		
		//open
		hRequest=HttpOpenRequestA(
			connect.hConn,meth,path.c_str(),NULL,address.c_str(),accept,0,0);
		if(!hRequest)
		{
			//printf("cannotopen http\n");
			error=ERROR_OPEN_REQUEST;
			return;
		}
		
		//send
		BOOL bResult;

		if(postdata.size())
		{
			bResult=HttpSendRequest(
				hRequest,NULL,0,(void*)postdata.c_str(),postdata.size());
		}
		else
		{
			bResult=HttpSendRequest(
				hRequest,NULL,0,NULL,0);
		}
		if (!bResult)
		{
			//printf("cannotsend http\n");
			error=ERROR_SEND_RESUEST;
			return;
		}
		
		//check contentlength
		BYTE buf[10];
		DWORD buflen=10;
		memset(buf,0,10);
		bResult=HttpQueryInfoA(hRequest,HTTP_QUERY_CONTENT_LENGTH,(LPVOID)buf,&buflen,NULL);
		if(!bResult)
		{
			contentlength=-1;
		}
		else
		{
			contentlength=atoi((char*)buf);
		}
		error=0;
	}
	
	//内部バッファにデータを追加
	void pushbuffer(const bv &buf)
	{
		synchronized sync(cs);
		
		buffer.push_back(buf);
	}
	
	//outにデータを追加
	void getbuffer(bv &out)
	{
		synchronized sync(cs);
		
		for(bvl::iterator i=buffer.begin();i!=buffer.end();i++)
		{
			out.insert(out.end(),i->begin(),i->end());
		}
	}
	//内部バッファをフラッシュ
	void flushbuffer()
	{
		synchronized sync(cs);
		
		if(contentlength>0)
		{
			if(contentlength<=receivelength)
			{
				isfinished=true;
			}
		}
		buffer.clear();
	}
	
	//ハンドラに送信
	void pushhandler(const bv &buf)
	{
		for(mtl::iterator i=handlers.begin();i!=handlers.end();i++)
		{
			(*i)->push(buf);
		}
	}
	
	int run()
	{
		bv buf;
		for (;;)
		{
			DWORD dwNOBA;
			BOOL bResult;

			sleep(0);
			bResult = InternetQueryDataAvailable(hRequest, &dwNOBA, 0, 0);
			if (!bResult) { error=ERROR_QUERY_AVAILABLE;return -1; }
			
			if(dwNOBA==0){sleep(1);continue;}
			
			if(dwNOBA>10240)dwNOBA=10240;
			
			buf.resize(dwNOBA);

			DWORD sizeRead;
			
			bResult = InternetReadFile(hRequest, &buf[0], dwNOBA, &sizeRead);
			if (!bResult) { error=ERROR_READ_FILE;return -1; }
			if (sizeRead == 0) break;
			
			buf.resize(sizeRead);
			
			receivelength+=buf.size();
			
			if(store)
				pushbuffer(buf);
			
			pushhandler(buf);
			
			//printf("pushed:%d\n",buf.size());
			
			if(!isvalid()){ error=FORCE_CLOSE;return -1;}
		}
		isfinished=true;
		
		buf.resize(0);
		
		return 0;
	}
	
protected:
	virtual int handle()
	{
		return 0;
	}

public:
	int getContentLength()
	{
		return contentlength;
	}
	bool get(bv &out)
	{
		getbuffer(out);
		flushbuffer();
		sleep(0);
		return isfinished||error;
	}
	bool pop()
	{
		flushbuffer();
		return isfinished||error;
	}
	int getError()
	{
		return error;
	}
};



};//endof namespace Rydot


