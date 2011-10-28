
#if defined(__cplusplus)
extern "C"
{
#endif
#include <stdio.h>
#include <stdlib.h>
#include <libqhull/libqhull.h>
#include <libqhull/mem.h>
#include <libqhull/qset.h>
#include <libqhull/geom.h>
#include <libqhull/merge.h>
#include <libqhull/poly.h>
#include <libqhull/io.h>
#include <libqhull/stat.h>
#if defined(__cplusplus)
}
#endif

#ifdef _WIN32
#pragma comment(lib,"../../../qhull/lib/qhullstatic.lib")
#endif


#include "QhullCalc.h"

typedef Rydot::Vector2<double> v2d;
typedef Rydot::Vector3<double> v3d;

bool QhullCalc::Calc(
	const std::vector<v3d> &pin,
	std::vector<v3d> &pout,
	std::vector<std::vector<int> > &fout)const
{
	if(pin.size() < 4)
		return false;

	try
	{
		char cmd[]="qhull ";
		if(qh_new_qhull(3,int(pin.size()),const_cast<double*>(&pin[0].x),false,cmd,NULL,NULL))
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
			qh_setsize(vertices);
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
	catch(...)
	{
		return false;
	}
}


