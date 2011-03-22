
// mesh.cpp
//
// mesh class
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// 3Dモデルのメッシュを扱うやつ
// なぜかCalmull-Clark法だけ入ってる

#include <mesh.h>

namespace Rydot
{

//Catmull-Clarkの方法で細分割する
int Mesh::CatmullClark()
{
	Vector3fv Q;//面の中心位置
	
	int nf=faces.size();
	int nv=vertex.size();
	
	Q.resize(nf);
	
	for(int i=0;i<nf;i++)
	{
		Q[i]=faces[i].M(vertex);
	}
	
	MeshEdgeBased meb;
	meb.Make(*this);
	
	int ne=meb.validedge.size();
	
	//稜線点
	Vector3fv E;//稜線点位置
	Vector3fv R;//辺中点位置
	E.resize(ne);
	R.resize(ne);
	
	for(int i=0;i<ne;i++)
	{
		int *s=meb.edges[meb.validedge[i]].vertex;
		Vector3f m=vertex[s[0]]+vertex[s[1]];
		R[i]=m*0.5;
		
		int q0=meb.edges[meb.validedge[i]].face;
		int q1=meb.edges[meb.edges[meb.validedge[i]].back].face;
		if(q0!=q1)
		{
			m+=Q[q0]+Q[q1];
			m*=(1.0/4.0);
		}
		else
		{
			m+=Q[q0];
			m*=(1.0/3.0);
		}
		E[i]=m;
	}
	
	Vector3fv V;//新頂点
	V.resize(nv);
	
	for(int i=0;i<nv;i++)
	{
		intv f,e;
		
		FindFaceFromVertex(i,f);
		meb.FindEdgeFromVertex(i,e);
		
		int n=e.size();
		int nflocal=f.size();
		
		for(int j=0;j<n;j++)
		{
			e[j]=meb.EdgeToValidEdge(e[j]);
		}
		
		Vector3f v=vertex[i]*((n-3.0)/n);
		
		Vector3f Qbar(0,0,0);

		for(int j=0;j<nflocal;j++)
		{
			Qbar+=Q[f[j]];
		}
		Qbar*=1.0/nflocal;
		
		Vector3f Rbar(0,0,0);
		for(int j=0;j<n;j++)
		{
			Rbar+=R[e[j]];
		}
		Rbar*=1.0/n;
		
		v+=Qbar*(1.0/n)+Rbar*(2.0/n);
		
		V[i]=v;
	}
	
	//新メッシュ構成
	Mesh m;
	
	//新頂点を構成
	//面点、稜線点、新頂点の順で格納
	m.vertex.resize(nf+ne+nv);
	
	int rnf=0;//再構成後の面数=Σ(面の頂点数)
	
	for(int i=0;i<nf;i++)
	{
		m.vertex[i]=Q[i];
		rnf+=faces[i].vertex.size();
	}
	for(int i=0;i<ne;i++)
	{
		m.vertex[i+nf]=E[i];
	}
	for(int i=0;i<nv;i++)
	{
		m.vertex[i+nf+ne]=V[i];
	}
	
	m.faces.resize(rnf);
	
	for(int i=0,p=0;i<nf;i++)
	{
		int n=faces[i].vertex.size();
		intv e;
		meb.FindEdgeFromFace(i,e);
		for(int j=0;j<n;j++)
		{
			e[j]=meb.EdgeToValidEdge(e[j]);
		}
		
		for(int j=0;j<n;j++)
		{
			Face f;
			f.material=faces[i].material;
			f.vertex.resize(4);
			
			f.vertex[0]=i;
			
			f.vertex[1]=e[(j+n-1)%n]+nf;
			f.vertex[2]=faces[i].vertex[j]+nf+ne;
			f.vertex[3]=e[(j+0)%n]+nf;
			
			m.faces[p]=f;
			p++;
		}
	}
	
	this->faces=m.faces;
	this->vertex=m.vertex;
	
	return 0;
}



};//end of namespace Rydot
