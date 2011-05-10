
#if defined(__cplusplus)
extern "C"
{
#endif
#include <stdio.h>
#include <stdlib.h>
#include <libqhull.h>
#include <mem.h>
#include <qset.h>
#include <geom.h>
#include <merge.h>
#include <poly.h>
#include <io.h>
#include <stat.h>
#if defined(__cplusplus)
}
#endif

#pragma comment(lib,"../../../qhull/lib/qhullstatic.lib")

#include "QhullCalc.h"

typedef Rydot::Vector2<double> v2d;
typedef Rydot::Vector3<double> v3d;

bool QhullCalc::Calc(
	const std::vector<v3d> &pin,
	std::vector<v3d> &pout,
	std::vector<std::vector<int>> &fout)const
{
	if(qh_new_qhull(3,int(pin.size()),const_cast<double*>(&pin[0].x),false,"qhull ",NULL,NULL))
		return false;
	pointT *point, *pointtemp;
	FORALLpoints
	{
		pout.push_back(v3d(point[0],point[1],point[2]));
	}
	facetT *facet;
	vertexT *vertex, **vertexp;
	FORALLfacets
	{
		std::vector<int> idx;
		setT *vertices=qh_facet3vertex(facet);
		int sz=qh_setsize(vertices);
		FOREACHvertex_(vertices)
		{
			idx.push_back(qh_pointid(vertex->point));
		}
		fout.push_back(idx);
		qh_settempfree(&vertices);
	}
	qh_freeqhull(!qh_ALL);
	int curlong, totlong;
	qh_memfreeshort(&curlong, &totlong);
	if(curlong || totlong)
	{
		return false;
	}
	return true;
}


