
// messagethread.h
//
// message thread
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// メッセージを受け取るスレッド
// 外部からpushでメッセージを追加
// handlerとidleを実装して使う。

#pragma once

#include "cthread.h"
#include "messagequeue.h"

namespace Rydot
{

template <class MessageType>
class messagethread:public thread
{
private:
	messagequeue<MessageType> msg;

private:
	int run()
	{
		while(isvalid())
		{
			MessageType m;
			if(msg.get(m))
			{
				handler(m);
			}
			else
			{
				idle();
			}
		}
		return 0;
	}

public:	
	//これでメッセージを追加
	void push(const MessageType &m)
	{
		msg.push(m);
	}

protected:
	//これを実装してくれと。
	virtual void handler(const MessageType &m)
	{
		return;
	}

	//なんにもメッセージがないときはこっちね。
	virtual void idle()
	{
		sleep(1);//テキトー
		return;
	}
};

};//end of namespace Rydot

