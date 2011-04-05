
// mesh.h
//
// mesh class
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// 3Dモデルのメッシュを扱うやつ
// かなり混沌

#pragma once

#include "matrix.h"
#include <algorithm>

#include "motion.h"

#include <math.h>

namespace Rydot
{

typedef std::pair<int,int> iipair;
typedef std::vector<iipair> iipairv;

typedef std::vector<int> intv;
typedef std::vector<float> floatv;
typedef std::vector<Vector3f> Vector3fv;
typedef std::vector<Vector2f> Vector2fv;
typedef std::vector<Quaternionf> Quaternionfv;

typedef std::vector<Vector3i> Vector3iv;

class IModelLoader;

class MeshEdgeBased;

//////////////////////////////////////////////////
// マテリアル
//
class Material
{
public:
	std::string name;
	float color[4];
	float diffusecolor[3];
	float specularcolor[3];
	float diffuse,ambient,emission,specular,power;
	std::string texture;
	int itexture;//テクスチャ管理クラスから割り当てられるテクスチャIDが入るはず
public:
	Material():itexture(-1){}
};
typedef std::vector<Material> Materialv;


//////////////////////////////////////////////////
// 辺
//
class Edge
{
public:
	int material;
	int vertex[2];
	Vector2f uv[2];
	float width;//太さ
	bool uvexist;
public:
	Edge():material(-1),uvexist(0){}
	~Edge(){}
	//頂点v1,v2の辺か。
	bool HasEdge(int v1,int v2)const
	{
		return HasEdge(iipair(v1,v2));
	}
	bool HasEdge(const Edge &e)const
	{
		return HasEdge(e.vertex[0],e.vertex[1]);
	}
	bool HasEdge(const iipair &e)const
	{
		if(vertex[0]==e.first&&vertex[1]==e.second)
			return true;
		if(vertex[0]==e.second&&vertex[1]==e.first)
			return true;
		return false;
	}
	//頂点vをもっているか。
	bool HasVertex(int v)const
	{
		if(vertex[0]==v)
				return true;
		if(vertex[1]==v)
				return true;
		return false;
	}
};
typedef std::vector<Edge> Edgev;

//////////////////////////////////////////////////
// 三角形
//
class Triangle
{
public:
	int material;
	int vertex[3];
	Vector2f uv[3];
	bool uvexist;
public:
	Triangle():material(-1),uvexist(0){}

	//頂点v1,v2の辺をもっているか。
	bool HasEdge(int v1,int v2)const
	{
		return HasEdge(iipair(v1,v2));
	}
	bool HasEdge(const Edge &e)const
	{
		return HasEdge(e.vertex[0],e.vertex[1]);
	}
	bool HasEdge(const iipair &e)const
	{
		for(int i=0;i<3;i++)
		{
			if(vertex[i]==e.first&&vertex[(i+1)%3]==e.second)
				return true;
			if(vertex[i]==e.second&&vertex[(i+1)%3]==e.first)
				return true;
		}
		return false;
	}

	//頂点vをもっているか。
	bool HasVertex(int v)const
	{
		for(int i=0;i<3;i++)
		{
			if(vertex[i]==v)
				return true;
		}
		return false;
	}

	//共有頂点をもっているか。
	bool HasLinkedVertex(const Triangle &t)const
	{
		for(int i=0;i<3;i++)
		{
			for(int j=0;j<3;j++)
			{
				if(vertex[i]==t.vertex[j])
					return true;
			}
		}
		return false;
	}

	//交点のzの符号を計算する
	//posからz方向にレイを飛ばして、
	//交点がpos.zよりプラス方向にあれば1、マイナス方向にあれば-1
	int CalcZ(const Vector3f &pos,const Vector3fv &vtx)const
	{
		Vector2f r(pos.x,pos.y);
		Vector2f p[]={
			Vector2f(vtx[vertex[0]].x,vtx[vertex[0]].y),
			Vector2f(vtx[vertex[1]].x,vtx[vertex[1]].y),
			Vector2f(vtx[vertex[2]].x,vtx[vertex[2]].y),
		};
		Vector2f u(p[1]-p[0]);
		Vector2f v(p[2]-p[0]);
		float den=(u).ExProd(v);
		Vector2f a(v.y,-v.x),b(-u.y,u.x);
		Vector2f pp(a.DotProd(r-p[0]),b.DotProd(r-p[0]));
		pp*=1.0/den;
		float z=(vtx[vertex[1]].z-vtx[vertex[0]].z)*pp.x+(vtx[vertex[2]].z-vtx[vertex[0]].z)*pp.y+vtx[vertex[0]].z;
		return ((pos.z-z)>=0)?1:-1;
	}

	//posから発生した+-z方向のレイに三角形面がぶつかるかどうか
	//頂点や辺にぶつかるような場合を考慮して、intv型とiipairv型の2つのテンポラリバッファが必要。
	// intv exceptvertex;
	// iipairv exceptedg;
	// for(探索したいモデルの全部の三角形)
	// {
	// 	int intersect=RayZ(pos,vtx,exceptvertex,exceptedg);
	// 	なんちゃら
	//みたいに使う。
	int RayZ(const Vector3f &pos,const Vector3fv &vtx,intv &exceptvtx,iipairv &exceptedg)const
	{
		//例外頂点があったら無効
		for(int i=0;i<exceptvtx.size();i++)
		{
			if(HasVertex(exceptvtx[i]))return 0;
		}
		//例外エッジがあったら無効
		for(int i=0;i<exceptedg.size();i++)
		{
			if(HasEdge(exceptedg[i]))return 0;
		}

		Vector2f r(pos.x,pos.y);
		
		Vector2f p[]={
			Vector2f(vtx[vertex[0]].x,vtx[vertex[0]].y),
			Vector2f(vtx[vertex[1]].x,vtx[vertex[1]].y),
			Vector2f(vtx[vertex[2]].x,vtx[vertex[2]].y),
		};
		//シグニチャを求める
		int s[]={
			sign((r-p[0]).ExProd(p[1]-p[0])),
			sign((r-p[1]).ExProd(p[2]-p[1])),
			sign((r-p[2]).ExProd(p[0]-p[2])),
		};
		
		//全部同じ側ならレイは三角形にぶつかっている
		if( (s[0]==s[1])&&(s[0]==s[2]) )
			return CalcZ(pos,vtx);
		
		//辺or頂点上にあるとき
		if( (s[0]==0)||(s[1]==0)||(s[2]==0) )
		{
			for(int i=0;i<3;i++)
			{
				//辺
				if((s[i]==0)&&(s[(i+1)%3]==s[(i+2)%3]))
				{
					exceptedg.push_back(iipair(vertex[i],vertex[(i+1)%3]));
					return CalcZ(pos,vtx);
				}
				//頂点
				if((s[i]==0)&&(s[(i+1)%3]==0)&&(s[(i+2)%3]!=0))
				{
					exceptvtx.push_back(vertex[(i+1)%3]);
					return CalcZ(pos,vtx);
				}
			}
		}
		return 0;
	}

	//点pとの三角形の距離
	//三角形のデータには頂点番号しか入っていないので
	//頂点の座標の入ったVector3fv型の頂点バッファvtxが必要。
	float Distance(const Vector3f &p,const Vector3fv &vtx)const
	{
		return Distance_PointTriangle(
			p,
			vtx[vertex[0]],
			vtx[vertex[1]]-vtx[vertex[0]],
			vtx[vertex[2]]-vtx[vertex[0]]);
	}
};
typedef std::vector<Triangle> Trianglev;


//////////////////////////////////////////////////
// 一般的な面。辺と三角形に変換できる。
//
class Face
{
public:
	int material;//マテリアルID
	intv vertex;//頂点ID
	Vector2fv uv;//UV座標
public:
	Face():material(-1){}
	Face(Vector3i &v):material(-1)
	{
		vertex.resize(3);
		vertex[0]=v.x;
		vertex[1]=v.y;
		vertex[2]=v.z;
	}
	Face(int a,int b):material(-1)
	{
		vertex.resize(2);
		vertex[0]=a;
		vertex[1]=b;
	}
	Face(int a,int b,int c):material(-1)
	{
		vertex.resize(3);
		vertex[0]=a;
		vertex[1]=b;
		vertex[2]=c;
	}
	Face(int a,int b,int c,int d):material(-1)
	{
		vertex.resize(4);
		vertex[0]=a;
		vertex[1]=b;
		vertex[2]=c;
		vertex[3]=d;
	}

	//共有頂点があるか
	bool LinkedVertex(const Face &f)const
	{
		for(int i=0;i<vertex.size();i++)
		{
			for(int j=0;j<f.vertex.size();j++)
			{
				if(vertex[i]==f.vertex[j])
					return true;
			}
		}
		return false;
	}
	
	
	bool HasVertex(int v)const
	{
		int nv=vertex.size();
		for(int i=0;i<nv;i++)
			if(vertex[i]==v)return true;
		
		return false;
	}

	//三角形に分解したときの数を返す。
	int GetTriangleNumber()const
	{
		int sz=vertex.size();
		if(sz==3)return 1;
		if(sz==4)return 2;
		return 0;
	}
	//辺形状かどうか。
	int GetEdgeNumber()const
	{
		if(vertex.size()==2)return 1;
		return 0;
	}

	//三角形面に分解する。
	//辺は無視。
	//できた三角形はtrianglesの後ろに追加される。
	//追加した数を返す。
	int MakeTriangles(Trianglev &triangles)const
	{
		int sz=vertex.size();
		//三角形面を作る。
		if(sz==3)
		{
			Triangle t;
			t.material=material;
			for(int i=0;i<3;++i)
			{
				t.vertex[i]=vertex[i];
			}
			if(!uv.empty())
			{
				for(int i=0;i<3;++i)
				{
					t.uv[i]=uv[i];
				}
				t.uvexist=1;
			}
			else
			{
				t.uvexist=0;
			}
			triangles.push_back(t);
			return 1;
		}
		if(sz==4)//四角形面は三角形面に分解。分解方向は特に考えない。
		{
			Triangle t1,t2;
			t1.material=material;
			t2.material=material;
			for(int i=0;i<3;++i)
			{
				t1.vertex[i]=vertex[i];
				t2.vertex[i]=vertex[(i+2)&3];
			}
			if(!uv.empty())
			{
				for(int i=0;i<3;++i)
				{
					t1.uv[i]=uv[i];
					t2.uv[i]=uv[(i+2)&3];
				}
				t1.uvexist=1;
				t2.uvexist=1;
			}
			else
			{
				t1.uvexist=0;
				t2.uvexist=0;
			}
			triangles.push_back(t1);
			triangles.push_back(t2);
			return 2;
		}
		return 0;
	}

	//辺だったらedgesに追加する。
	//追加した数を返す。
	int MakeEdges(Edgev &edges)
	{
		if(vertex.size()==2)
		{
			Edge e;
			e.material=material;
			for(int i=0;i<2;++i)
			{
				e.vertex[i]=vertex[i];
			}
			if(!uv.empty())
			{
				for(int i=0;i<2;++i)
				{
					e.uv[i]=uv[i];
				}
				e.uvexist=1;
			}
			else
			{
				e.uvexist=0;
			}
			e.width=1.0;
			edges.push_back(e);
			return 1;
		}
		return 0;
	}
	
	//頂点の平均位置
	Vector3f M(Vector3fv &_v)
	{
		Vector3f m=_v[vertex[0]];
		int nv=vertex.size();
		for(int i=1;i<nv;i++)
		{
			m+=_v[vertex[i]];
		}
		m*=1.0/nv;
		return m;
	}
};
typedef std::vector<Face> Facev;


//////////////////////////////////////////////////
// スキニングの重み付け
// 1つの頂点に対して
//
class WeightLink
{
public:
	intv link;//リンクしたボーンID
	floatv weight;//重み付け
public:
	//重み付けを正規化する
	void Normalize()
	{
		if(weight.empty())
			return;
		
		float sum=0;
		for(int i=0;i<weight.size();i++)
		{
			sum+=weight[i];
		}
		for(int i=0;i<weight.size();i++)
		{
			weight[i]/=sum;
		}
	}
	void Append(int l,float w)
	{
		link.push_back(l);
		weight.push_back(w);
	}
};
typedef std::vector<WeightLink> WeightLinkv;


/////////////////////////////////////////////////
// 座標割り当て済みの単一マテリアルの三角形リスト
//
class VertexTriangleList
{
public:
	int material;
	Vector3fv vertex;
	Vector2fv uv;
public:
	//三角形と頂点座標から三角形リストを作る。
	//入力される三角形は、すべて同じマテリアルであること。
	void MakeTriangleList(Trianglev &_triangles,Vector3fv &_vertices,Materialv &_materials)
	{
		material=_triangles[0].material;
		
		int fsz=_triangles.size();
		vertex.resize(fsz*3);
		for(int i=0;i<fsz;i++)
		{
			for(int j=0;j<3;j++)
			{
				vertex[i*3+j]=_vertices[_triangles[i].vertex[j]];
			}
		}
		if(material>=0)
		{
			int istexture=_materials[material].itexture>=0;
			if(istexture)
			{
				uv.resize(fsz*3);
				for(int i=0;i<fsz;i++)
				{
					for(int j=0;j<3;j++)
					{
						uv[i*3+j]=_triangles[i].uv[j];
					}
				}
			}
		}
	}

};
typedef std::vector<VertexTriangleList> VertexTriangleListv;

/////////////////////////////////////////////////
// エッジ基準モデルのための辺
//
class EdgeForEdgeBased
{
public:
	int face;
	int vertex[2];
	int sibling[2];
	int back;
	bool hasuv;
	int validid;
	Vector2f uv[2];
public:
	EdgeForEdgeBased():face(-1),hasuv(false),validid(-1){}
	~EdgeForEdgeBased(){}
	//頂点v1,v2の辺か。
	//この順番じゃないとだめ。
	bool HasEdge(int v1,int v2)const
	{
		return HasEdge(iipair(v1,v2));
	}
	bool HasEdge(const EdgeForEdgeBased &e)const
	{
		return HasEdge(e.vertex[0],e.vertex[1]);
	}
	bool HasEdge(const iipair &e)const
	{
		if(vertex[0]==e.first&&vertex[1]==e.second)
			return true;
		return false;
	}
	//頂点vをもっているか。
	bool HasVertex(int v)const
	{
		if(vertex[0]==v)
				return true;
		if(vertex[1]==v)
				return true;
		return false;
	}
	//裏向きのエッジか
	bool IsBackEdge(EdgeForEdgeBased &e)const
	{
		if(e.vertex[0]==vertex[1]&&e.vertex[1]==vertex[0])
		{
			return true;
		}
		return false;
	}
	bool IsValid()const
	{
		return validid!=-1;
	}
	bool HasUV()const{return hasuv;}
	int &Next(){return sibling[1];}
	int &Prev(){return sibling[0];}
	int &Back(){return back;}
	int &Face(){return face;}
	int &Vertex(int i){return vertex[i];}

	const int &Next()const{return sibling[1];}
	const int &Prev()const{return sibling[0];}
	const int &Back()const{return back;}
	const int &Face()const{return face;}
	const int &Vertex(int i)const{return vertex[i];}
};
typedef std::vector<EdgeForEdgeBased> EdgeForEdgeBasedv;

class FaceForEdgeBased
{
public:
	int material;//マテリアルID
	int edge;//エッジID
public:
	FaceForEdgeBased():material(-1),edge(-1){}
	~FaceForEdgeBased(){}
};
typedef std::vector<FaceForEdgeBased> FaceForEdgeBasedv;

typedef std::list<Trianglev> Trianglevl;
typedef Trianglevl::iterator TrianglevlItr;
typedef std::vector<Trianglev> Trianglevv;
typedef std::list<Edgev> Edgevl;
typedef Edgevl::iterator EdgevlItr;


//////////////////////////////////////////////////
// メッシュ
//
class Mesh
{
public:
	int linked_bone;// 直接関連付けられたボーンID
	std::string name;// メッシュの名前
	Vector3fv vertex;// 頂点座標
	Vector3fv normals;// 各頂点のテキトーなノーマル
	Facev faces;// 面
	Trianglevl classified_triangles;// マテリアル別の三角形面
	Edgevl classified_edges;// マテリアル別のエッジ
	WeightLinkv weights;// スキニングの重み付け
	VertexTriangleListv transformed_triangles;// 座標割り当て済みのマテリアル別の三角形リスト
public:

	Mesh():linked_bone(-1){}

	Vector3f &CreateVertex(){Vector3f v;vertex.push_back(v);return vertex.back();}
	Face &CreateFace(){Face f;faces.push_back(f);return faces.back();}
	void Append(Vector3f &v){vertex.push_back(v);}
	void Append(Face &f){faces.push_back(f);}

	//面を三角形に分解する。
	//結果は引数tvに入れる
	//戻り値は追加した数
	int MakeTriangles(Trianglev &tv)
	{
		int fsz=faces.size();
		tv.reserve(fsz);
		int appended=0;
		for(int i=0;i<fsz;++i)
		{
			appended+=faces[i].MakeTriangles(tv);
		}
		return appended;
	}

	//面のうち辺形状を取り出す。
	//結果は引数evに入れる
	//戻り値は追加した数
	int MakeEdges(Edgev &ev)
	{
		int fsz=faces.size();
		ev.reserve(fsz);
		int appended=0;
		for(int i=0;i<fsz;++i)
		{
			appended+=faces[i].MakeEdges(ev);
		}
		return appended;
	}

	//メッシュを三角形に分解し、マテリアル別にソートして
	//trisに入れる
	//辺はedgesに入れる
	int SortTrisByMaterial()
	{
		Trianglev tv;
		Edgev ev;
		int fsz=faces.size();
		
		MakeTriangles(tv);
		MakeEdges(ev);
		
		//三角形
		int maxmat=-1;
		int tsz=tv.size();
		int esz=ev.size();
		for(int i=0;i<tsz;++i)
		{
			if(tv[i].material>maxmat)maxmat=tv[i].material;
		}
		classified_triangles.clear();
		for(int i=-1;i<=maxmat;++i)
		{
			Trianglev tvm;
			for(int j=0;j<tsz;++j)
			{
				if(tv[j].material==i)
				{
					tvm.push_back(tv[j]);
				}
			}
			if(!tvm.empty())classified_triangles.push_back(tvm);
		}
		
		//辺
		maxmat=-1;
		for(int i=0;i<esz;++i)
		{
			if(ev[i].material>maxmat)maxmat=ev[i].material;
		}
		classified_edges.clear();
		for(int i=-1;i<=maxmat;++i)
		{
			Edgev evm;
			for(int j=0;j<esz;++j)
			{
				if(ev[j].material==i)
				{
					evm.push_back(ev[j]);
				}
			}
			if(!evm.empty())classified_edges.push_back(evm);
		}
		return 0;
	}

	//頂点を共有しているグループごとに分ける
	//結果をresultに追加する
	int SplitByGroup(std::vector<Mesh> &result)const
	{
		int fsz=faces.size();
		int vsz=vertex.size();
		//どのグループに属するかをマーキングする
		intv mark;
		mark.resize(fsz);
		for(int i=0;i<fsz;i++)
		{
			mark[i]=-1;
		}
		int groupID=0;
		for(int i=0;i<fsz;i++)
		{
			int n=mark[i];
			if(n==-1)
			{
				n=groupID;
				mark[i]=n;
				groupID++;
			}
			for(int j=i+1;j<fsz;j++)
			{
				//iとjが共有頂点をもっているか
				if(faces[i].LinkedVertex(faces[j]))
				{
					if(mark[j]==-1)
						mark[j]=n;
					else//マークを張り替える
					{
						int m=mark[j];
						for(int k=0;k<fsz;k++)
							if(mark[k]==m)
								mark[k]=n;
					}
				}
			}
		}
		//マーク別にオブジェクトを分解する
		for(int i=0;i<groupID;i++)
		{
			Mesh mesh;
			mesh.vertex=vertex;
			mesh.name=name;//とりあえず同じ名前をつける
			
			for(int j=0;j<fsz;j++)
			{
				if(mark[j]==i)
					mesh.faces.push_back(faces[j]);
			}
			
			if(!mesh.faces.empty())
				result.push_back(mesh);
		}
		return 0;
	}

	//メッシュの内部に点pがあるかどうかを判定する
	//z向きのレイにぶつかるかどうかを判定して、交点数で内外を求める。
	bool Inside(const Trianglev &t,const Vector3f &p)const
	{
		//辺や頂点にぶつかったときのためのテンポラリバッファ
		intv exceptvtx;
		iipairv exceptedg;
		
		int ineg=0,ipos=0;
		int tsz=t.size();
		
		for(int i=0;i<tsz;i++)
		{
			int res=t[i].RayZ(p,vertex,exceptvtx,exceptedg);
			if(res==1)ipos++;
			if(res==-1)ineg++;
		}
		//+-の交点数がそれぞれ1以上で、かつ
		//交点数の差が偶数だったらメッシュ内部に点pがあるはず
		if((ineg==0)||(ipos==0))return false;
		if((ineg-ipos)&1)return false;
		return true;
	}

	//メッシュ内部の点に対する距離
	float InsideDistance(const Trianglev &t,const Vector3f &p)const
	{
		int tsz=t.size();
		float min=10000;
		for(int i=0;i<tsz;i++)
		{
			float res=t[i].Distance(p,vertex);
			if(res>0&&res<min)
			{
				min=res;
			}
		}
		return min;
	}

	//バウンディングボックスを求める
	//普通の点列のやつではなくて、
	//面や辺に実際に使われている頂点のみを対象とする
	int BoundingBox(Rect3f &r)
	{
		Vector3fv v;
		v.reserve(vertex.size());
		int fsz=faces.size();
		for(int i=0;i<fsz;i++)
		{
			for(int j=0;j<faces[i].vertex.size();j++)
			{
				v.push_back(vertex[faces[i].vertex[j]]);
			}
		}
		return Rydot::BoundingBox(v,r);
	}

	//各頂点のノーマルを求める(かなりテキトー)
	void CalcNormal()
	{
		size_t nv=vertex.size();
		normals.resize(nv);
		for(size_t i=0;i<nv;i++)
		{
			normals[i]=Vector3f(0,0,0);
		}
		for(TrianglevlItr itr=classified_triangles.begin();itr!=classified_triangles.end();++itr)
		{
			Trianglev fv=*itr;
			size_t sz=fv.size();
			
			for(size_t j=0;j<sz;++j)
			{
				Triangle &f=fv[j];
				Vector3f n=TriangleNormal(
					vertex[f.vertex[0]],
					vertex[f.vertex[1]],
					vertex[f.vertex[2]]);
				for(size_t k=0;k<3;k++)
				{
					normals[f.vertex[k]]+=n;
				}
			}
		}
		for(size_t i=0;i<nv;i++)
		{
			normals[i].Normalize();
		}
	}

	//座標割り当て済みの三角形群を作る
	void MakeTriangleList(Materialv &materials)
	{
		int sz=classified_triangles.size();
		transformed_triangles.resize(sz);
		int i=0;
		for(TrianglevlItr itr=classified_triangles.begin();itr!=classified_triangles.end();++itr,++i)
		{
			transformed_triangles[i].MakeTriangleList(*itr,vertex,materials);
		}
	}
	
	//vertex id に a,bをもつ辺を見つける
	int FindEdge(int a,int b,iipairv &edge)
	{
		int ne=edge.size();
		if(a<b)
		{
			for(int i=0;i<ne;i++)
			{
				if((edge[i].first==a)&&(edge[i].second==b))
					return i+1;
			}
		}
		if(a>b)
		{
			for(int i=0;i<ne;i++)
			{
				if((edge[i].first==b)&&(edge[i].second==a))
					return -(i+1);
			}
		}
		return 0;
	}
	//
	int EdgePrev(int n,int ne,iipairv &edge)
	{
		return (n>0)?edge[n-1].first:edge[(-n)-1+ne].second;
	}
	int EdgeCenter(int n,iipairv &edge)
	{
		return edge[abs(n)-1].second;
	}
	
	void FindFaceFromVertex(int v,intv &f)
	{
		int nf=faces.size();
		for(int i=0;i<nf;i++)
		{
			if(faces[i].HasVertex(v))
				f.push_back(i);
		}
	}
	
	//細分割する
	//現在は、すべて三角形面しか無理。
	//しかも、テクスチャはボロボロになりそう
	int Subdivision()
	{
		//make edge
		iipairv edge;
		Vector3iv edgerecord;//エッジ番号を持った面
		
		int nf=faces.size();
		int nv=vertex.size();
		
		edgerecord.reserve(nf);
		
		for(int i=0;i<nf;i++)
		{
			if(faces[i].vertex.size()!=3)
				return -1;

			intv &v=faces[i].vertex;

			if(v[0]<v[1])
				edge.push_back(iipair(v[0],v[1]));
			if(v[1]<v[2])
				edge.push_back(iipair(v[1],v[2]));
			if(v[2]<v[0])
				edge.push_back(iipair(v[2],v[0]));
		}
		
		int ne=edge.size();
		
		//三角形に辺のidをもたせる
		for(int i=0;i<nf;i++)
		{
			const Face &fc=faces[i];
			edgerecord.push_back(Vector3i(
				FindEdge(fc.vertex[0],fc.vertex[1],edge),
				FindEdge(fc.vertex[1],fc.vertex[2],edge),
				FindEdge(fc.vertex[2],fc.vertex[0],edge)
			));
		}
		
		//辺の中間点をとる
		for(int i=0;i<ne;i++)
		{
			iipair e=edge[i];
			Vector3f v=(vertex[e.first]+vertex[e.second])*0.5;
			vertex.push_back(v);
			edge[i].second=nv+i;
			e.first=nv+i;
			edge.push_back(e);
		}
		
		//面の中心をとる
		for(int i=0;i<nf;i++)
		{
			const Vector3i &tfc=edgerecord[i];
			int p1,p2,p3;
			int q1,q2,q3;
			p1=EdgeCenter(tfc.x,edge);
			p2=EdgeCenter(tfc.y,edge);
			p3=EdgeCenter(tfc.z,edge);
			edge.push_back(iipair(p1,p2));
			edge.push_back(iipair(p2,p3));
			edge.push_back(iipair(p3,p1));
			q1=EdgePrev(tfc.x,ne,edge);
			q2=EdgePrev(tfc.y,ne,edge);
			q3=EdgePrev(tfc.z,ne,edge);
			faces[i].vertex[0]=p1;faces[i].vertex[1]=p2;faces[i].vertex[2]=p3;
			Face f;
			f.vertex.resize(3);
			f.vertex[0]=q1;f.vertex[1]=p1;f.vertex[2]=p3;faces.push_back(f);
			f.vertex[0]=q2;f.vertex[1]=p2;f.vertex[2]=p1;faces.push_back(f);
			f.vertex[0]=q3;f.vertex[1]=p3;f.vertex[2]=p2;faces.push_back(f);
		}
		return 0;
	}
	
	//Catmull-Clarkの方法で細分割する
	int CatmullClark();
	
};
typedef std::vector<Mesh> Meshv;

/////////////////////////////////////////////////
// 辺ベースのメッシュ。
// テクスチャ未対応
class MeshEdgeBased
{
public:
	Vector3fv vertex;
	FaceForEdgeBasedv faces;
	EdgeForEdgeBasedv edges;
	intv validedge;//単エッジ化したときに有効なエッジ
	typedef FaceForEdgeBased f4eb;
	typedef EdgeForEdgeBased e4eb;
public:

	//メッシュから作る
	bool Make(Mesh &mesh)
	{
		vertex=mesh.vertex;

		int nf=mesh.faces.size();
		int nv=mesh.vertex.size();
		
		faces.reserve(nf);
		edges.reserve(nf*4);
		
		for(int i=0;i<nf;i++)
		{
			AppendFace(mesh.faces[i]);
		}
		LinkBackEdge();
		ValidateEdge();
		return true;
	}

	//面を追加する
	void AppendFace(Face &_face)
	{
		int nv=_face.vertex.size();
		int ne=edges.size();
		int nf=faces.size();
		f4eb f;
		bool hasuv=!_face.uv.empty();
		f.edge=ne;
		f.material=_face.material;
		faces.push_back(f);
		for(int i=0;i<nv;i++)
		{
			e4eb e;
			e.vertex[0]=_face.vertex[i];
			e.vertex[1]=_face.vertex[(i+1)%nv];
			e.Prev()=(i+nv-1)%nv+ne;
			e.Next()=(i+1)%nv+ne;
			e.Face()=nf;
			e.Back()=i+ne;
			
			if(hasuv)
			{
				e.hasuv=true;
				e.uv[0]=_face.uv[i];
				e.uv[1]=_face.uv[(i+1)%nv];
			}
			else
			{
				e.hasuv=false;
			}
			
			edges.push_back(e);
		}
	}

	//裏エッジとのリンクを作る
	void LinkBackEdge()
	{
		int ne=edges.size();
		for(int i=0;i<ne;i++)
		{
			for(int j=i+1;j<ne;j++)
			{
				if(edges[i].IsBackEdge(edges[j]))
				{
					edges[i].Back()=j;
					edges[j].Back()=i;
				}
			}
		}
	}

	//単エッジ化したときの有効性を決める
	// backあり&&a<b あるいは backなしのエッジを有効エッジにする
	// 単エッジ化したときの番号は0から振り、-1は無効エッジとする
	int ValidateEdge()
	{
		int ne=edges.size();
		validedge.clear();
		validedge.reserve(ne);
		for(int i=0,k=0;i<ne;i++)
		{
			edges[i].validid=-1;
			if(HasBack(i))
			{
				if(IsAscendingOrder(i))
				{
					validedge.push_back(i);
					edges[i].validid=k;
					k++;
				}
			}
			else
			{
				validedge.push_back(i);
				edges[i].validid=k;
				k++;
			}
		}
		return 0;
	}

	e4eb &NextEdge(e4eb &edge)
	{
		return edges[edge.Next()];
	}
	e4eb &PrevEdge(e4eb &edge)
	{
		return edges[edge.Prev()];
	}
	e4eb &BackEdge(e4eb &edge)
	{
		return edges[edge.Back()];
	}
	f4eb &RefFace(e4eb &edge)
	{
		return faces[edge.Face()];
	}
	
	const e4eb &NextEdge(const e4eb &edge)const
	{
		return edges[edge.Next()];
	}
	const e4eb &PrevEdge(const e4eb &edge)const
	{
		return edges[edge.Prev()];
	}
	const e4eb &BackEdge(const e4eb &edge)const
	{
		return edges[edge.Back()];
	}
	const f4eb &RefFace(const e4eb &edge)const
	{
		return faces[edge.Face()];
	}
	
	//頂点をさがす。面リンクから
	void FindVertexFromFace(int f,intv &vid)const
	{
		FindVertexFromFace(faces[f],vid);
	}
	void FindVertexFromFace(const f4eb &face,intv &vid)const
	{
		int initial_edge=face.edge;
		e4eb edge=edges[face.edge];
		while(1)
		{
			vid.push_back(edge.vertex[0]);
			if(edge.Next()==initial_edge)break;
			edge=NextEdge(edge);
		}
	}
	
	//辺をさがす。面リンクから
	void FindEdgeFromFace(int f,intv &eid)const
	{
		FindEdgeFromFace(faces[f],eid);
	}
	void FindEdgeFromFace(const f4eb &face,intv &eid)const
	{
		int initial_edge=face.edge;
		int e=face.edge;
		while(1)
		{
			eid.push_back(e);
			e=edges[e].Next();
			if(e==initial_edge)break;
		}
	}
	//faceが頂点vを含むか
	bool HasVertex(const f4eb &face,int v)const
	{
		int initial_edge=face.edge;
		e4eb edge=edges[face.edge];
		while(1)
		{
			if(edge.vertex[0]==v)return true;
			if(edge.Next()==initial_edge)break;
			edge=NextEdge(edge);
		}
		return false;
	}
	//対となる裏エッジがあるか
	bool HasBack(int e)const
	{
		return edges[e].Back()!=e;
	}
	//エッジが正方向か
	bool IsAscendingOrder(int e)const
	{
		return edges[e].vertex[0]<edges[e].vertex[1];
	}
	
	//面の法線を返す
	Vector3f FaceNormal(int f)const
	{
		return FaceNormal(faces[f]);
	}
	Vector3f FaceNormal(const f4eb &face)const
	{
		intv vid;
		FindVertexFromFace(face,vid);
		Vector3f n(0,0,0);
		int nv=vid.size();
		for(int i=0;i<nv;i++)
		{
			n+=TriangleNormal(vertex[vid[i]],vertex[vid[(i+1)%nv]],vertex[vid[(i+2)%nv]]);
		}
		n.Normalize();
		return n;
	}
	
	//エッジの持つ角度(ラジアン)を返す
	float EdgeAngle(int e)const
	{
		return EdgeAngle(edges[e]);
	}
	float EdgeAngle(const e4eb &edge)const
	{
		Vector3f n1=FaceNormal(RefFace(edge));
		Vector3f n2=FaceNormal(RefFace(BackEdge(edge)));
		return acos(n1.DotProd(n2));
	}

	//a,bを頂点にもつ辺を探しIDを返す
	int FindEdgeID(int a,int b)const
	{
		int ne=edges.size();
		for(int i=0;i<ne;i++)
		{
			if(edges[i].HasEdge(a,b))return i;
			if(edges[i].HasEdge(b,a))return i;
		}
		return -1;
	}
	
	//a,bの順で頂点をもつ辺をさがし、IDを返す
	int FindEdgeOrderedID(int a,int b)const
	{
		int ne=edges.size();
		int id=-1;
		for(int i=0;i<ne;i++)
		{
			if(edges[i].HasEdge(a,b)){return i;}
			if(edges[i].HasEdge(b,a))
			{
				if(HasBack(i))
				{
					return edges[i].Back();
				}
			}
		}
		return -1;
	}
	
	//頂点vをもつ辺をさがす(複数)
	void FindEdgeFromVertex(int v,intv &e)const
	{
		int ne=edges.size();
		for(int i=0;i<ne;i++)
		{
			if(edges[i].IsValid())
			{
				if(edges[i].HasVertex(v))
					e.push_back(i);
			}
		}
	}
	
	//頂点vをもつ面をさがす(複数)
	void FindFaceFromVertex(int v,intv &f)const
	{
		int nf=faces.size();
		for(int i=0;i<nf;i++)
		{
			if(HasVertex(faces[i],v))
				f.push_back(i);
		}
	}
	
	//エッジを単エッジ化したときのIDを返す
	int EdgeToValidEdge(int e)const
	{
		if(edges[e].IsValid())
			return edges[e].validid;
		else
			return edges[edges[e].Back()].validid;
	}

};
typedef std::vector<MeshEdgeBased> MeshEdgeBasedv;

//プリミティブメッシュを作るクラス
struct PrimitiveMeshCreator
{

	//方位角・偏角形式の球
	static int CreateSphere(Mesh &m,int N,int M)
	{
		m.vertex.clear();
		Vector3fv &v=m.vertex;
		v.push_back(Vector3f(0,0,1));
		for(int i=1;i<N;i++)
		{
			float si=sin((float)i/N*PI),ci=cos((float)i/N*PI);
			for(int j=0;j<M;j++)
			{
				float sj=sin(2.0*(float)j/M*PI),cj=cos(2.0*(float)j/M*PI);
				v.push_back(Vector3f(-si*sj,si*cj,ci));
			}
		}
		v.push_back(Vector3f(0,0,-1));
		
		m.faces.clear();
		Facev &f=m.faces;
		for(int i=0;i<M;i++)
		{
			f.push_back(Face(0,i+1,(i+1)%M+1));
		}
		for(int i=0;i<N-2;i++)
		{
			for(int j=0;j<M;j++)
			{
				f.push_back(Face(i*M+j+1,(i+1)*M+j+1,(i+1)*M+(j+1)%M+1));
				f.push_back(Face(i*M+j+1,(i+1)*M+(j+1)%M+1,i*M+(j+1)%M+1));
			}
		}
		for(int i=0;i<M;i++)
		{
			f.push_back(Face((N-2)*M+i+1,(N-1)*M+1,(N-2)*M+(i+1)%M+1));
		}
		return 0;
	}
	
	//ボックス
	static int CreateBox(Mesh &m)
	{
		m.vertex.clear();
		Vector3fv &v=m.vertex;
		v.push_back(Vector3f(1,1,1));
		v.push_back(Vector3f(-1,1,1));
		v.push_back(Vector3f(-1,-1,1));
		v.push_back(Vector3f(1,-1,1));
		v.push_back(Vector3f(1,1,-1));
		v.push_back(Vector3f(-1,1,-1));
		v.push_back(Vector3f(-1,-1,-1));
		v.push_back(Vector3f(1,-1,-1));

		m.faces.clear();
		Facev &f=m.faces;
		f.push_back(Face(0,1,2));
		f.push_back(Face(2,3,0));
		f.push_back(Face(0,4,5));
		f.push_back(Face(5,1,0));
		f.push_back(Face(1,5,6));
		f.push_back(Face(6,2,1));
		f.push_back(Face(2,6,7));
		f.push_back(Face(7,3,2));
		f.push_back(Face(3,7,4));
		f.push_back(Face(4,0,3));
		f.push_back(Face(4,7,6));
		f.push_back(Face(6,5,4));
		return 0;
	}
};


namespace MeshDeformer
{
	// バーンシュタイン基底関数
	double B_3(int i,double _t)
	{
		switch(i)
		{
			case 0:return (1-_t)*(1-_t)*(1-_t);
			case 1:return 3*(1-_t)*(1-_t)*_t;
			case 2:return 3*(1-_t)*_t*_t;
			case 3:return _t*_t*_t;
		}
		return 0;
	}

	// ベジェ空間
	// パラメータposと制御点controlを入力すれば結果の点が出てくる
	// posの各座標値は[0:1]の範囲でないとちょっとまずい。
	Vector3f beziercube(const Vector3fv &control,Vector3f &pos)
	{
		float Bx[]=
			{B_3(0,pos.x),B_3(1,pos.x),B_3(2,pos.x),B_3(3,pos.x)};
		float By[]=
			{B_3(0,pos.y),B_3(1,pos.y),B_3(2,pos.y),B_3(3,pos.y)};
		float Bz[]=
			{B_3(0,pos.z),B_3(1,pos.z),B_3(2,pos.z),B_3(3,pos.z)};
		Vector3f res(0,0,0);
		for(int a=0;a<64;a++)
		{
			int i=a%4,j=(a/4)%4,k=a/16;
			res+=control[a]*Bx[i]*By[j]*Bz[k];
		}
		return res;
	}

	int LatticeDeform(Mesh &m, const Vector3fv &control)
	{
		if(control.size() != 64)return 0;

		Rect3f bb;
		m.BoundingBox(bb);

		for(size_t i=0;i<m.vertex.size();++i)
		{
			m.vertex[i] = bb.TransformFrom(beziercube(control, bb.TransformTo(m.vertex[i])));
		}
		
		return 0;
	}
};


}// end of namespace Rydot

