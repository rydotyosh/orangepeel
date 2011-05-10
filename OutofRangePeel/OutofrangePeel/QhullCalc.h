
#pragma once

#include "matrix.h"

class QhullCalc
{
	typedef Rydot::Vector2<double> v2d;
	typedef Rydot::Vector3<double> v3d;

public:
	bool Calc(
		const std::vector<v3d> &pin,
		std::vector<v3d> &pout,
		std::vector<std::vector<int> > &fout)const;

};

