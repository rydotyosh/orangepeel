
// messagethread.h
//
// message thread
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// ���b�Z�[�W���󂯎��X���b�h
// �O������push�Ń��b�Z�[�W��ǉ�
// handler��idle���������Ďg���B

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
	//����Ń��b�Z�[�W��ǉ�
	void push(const MessageType &m)
	{
		msg.push(m);
	}

protected:
	//������������Ă���ƁB
	virtual void handler(const MessageType &m)
	{
		return;
	}

	//�Ȃ�ɂ����b�Z�[�W���Ȃ��Ƃ��͂������ˁB
	virtual void idle()
	{
		sleep(1);//�e�L�g�[
		return;
	}
};

};//end of namespace Rydot

