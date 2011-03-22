
// key.h
//
// key
//
// BSD license

// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// キー入力とかを保管しておくクラス

#pragma once

#include <list>
#include "singleton.h"

namespace Rydot
{

class Key;

class cKeyManager
{
public:
	int keyrepeatstart;
	int keyrepeatinterval;
private:
	std::list<Key*> keys;
	public:
	void regist(Key* _k){keys.push_back(_k);}
	void unregist(Key* _k){keys.remove(_k);}
	void update(int v=1);

	friend class Key;
};

typedef singleton<cKeyManager> KeyManager;

class Key
{
public:
	KeyManager mgr;

public:
	bool push;
	bool down;
	bool repeat;
	int count;//down中のカウント
	int dcount;//最後にpushしたときからのカウント
public:
	Key():count(0),dcount(0){mgr->regist(this);}
	~Key(){mgr->unregist(this);}
	void set(bool k)
	{
		if(k)
		{
			if(!down)push=true;
		}
		down=k;
	}
	void set()
	{
		if(!down)push=true;
		down=true;
	}
	void reset()
	{
		down=false;
	}
	void update(int v=1)
	{
		repeat=false;
		dcount+=v;
		if(push)dcount=0;
		push=false;
		if(down)count+=v;
		else count=0;
		
		if(count>mgr->keyrepeatstart)
		{
			if((count-mgr->keyrepeatstart)%mgr->keyrepeatinterval==0)repeat=true;
		}
	}
};

inline void cKeyManager::update(int v)
{
	for(std::list<Key*>::iterator i=keys.begin();i!=keys.end();i++)
	{
		(*i)->update(v);
	}
}

}


