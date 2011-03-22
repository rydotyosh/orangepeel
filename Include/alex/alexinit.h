
// alexinit.h
//
// al initializer
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// OpenALを初期化するやつのラッパ
// AlexInitのオブジェクトをmainにでもおいておけばいいはず

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


