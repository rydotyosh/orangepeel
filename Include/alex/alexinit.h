
// alexinit.h
//
// al initializer
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// OpenAL�������������̃��b�p
// AlexInit�̃I�u�W�F�N�g��main�ɂł������Ă����΂����͂�

#pragma once

#include <AL/alut.h>
#include "singleton.h"

namespace Rydot
{

class cAlexInit
{
public:
	cAlexInit(){alutInit(NULL,NULL);}
	~cAlexInit(){alutExit();}
};

typedef singleton<cAlexInit> AlexInit;

}


