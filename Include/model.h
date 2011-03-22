
// model.h
//
// model
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// 3Dモデルをテキトーに扱うクラス

#pragma once

#include "mesh.h"
#include "motion.h"

#include "modelloader.h"

namespace Rydot
{

//////////////////////////////////////////////////
// モデル
// メッシュとマテリアルとポーズの一式セット
// こいつでレンダリングできるはず。
//
class Model
{
public:
	Materialv materials;// マテリアル
	Meshv meshes;// メッシュ
	Meshv sdefmeshes;// 球面変形sdefしたメッシュ

	intv sdef;// sdef属性のメッシュのID

	bool material_active;

//	static TextureManager *texmgr;

	float outlineelevation;
	Vector3f outlinebias;
	Vector3f outlinerange;

	bool outline_only;
	
	bool texture_active;
	
	bool light_active;
	Vector3f light_pos;
	Vector3f light_color;
	Vector3f ambient_color;

//	static IModelLoader *loader;
public:

	Model():
		material_active(true),
		outlineelevation(2.0),
		outlinebias(0.3,0.3,0.3),
		outlinerange(0.5,0.43,0.4),
		outline_only(false),
		texture_active(true),
		light_active(true),
		light_color(1,1,1),
		light_pos(0,0,0),
		ambient_color(0.2,0.2,0.2)
		{}
	~Model(){}

/*	bool Load(std::string filename)
	{
		return loader->Load(filename,*this);
	}*/

	Material &CreateMaterial(){Material m;materials.push_back(m);return materials.back();}
	Mesh &CreateMesh(){Mesh m;meshes.push_back(m);return meshes.back();}
	void Append(Material &m){materials.push_back(m);}
	void Append(Mesh &m){meshes.push_back(m);}

	int GetMeshID(const std::string &name)const
	{
		int msz=meshes.size();
		for(int i=0;i<msz;i++)
		{
			if(name==meshes[i].name)
				return i;
		}
		return -1;
	}

	int GetMaterialID(const std::string &name)const
	{
		int msz=materials.size();
		for(int i=0;i<msz;i++)
		{
			if(name==materials[i].name)
				return i;
		}
		return -1;
	}

	////////////////////////////////////
	// 構築系
	//

	//ボーンを抽出する
	//抽出したボーンはposeに出力される。
	//計算法はmikotoの形式を採用
	//長辺は特に意味なし
	//中辺がボーンの軸を表し、
	//短辺にくっついている頂点がそのボーンのローカル原点
	//短辺の方向がボーンのsky方向
	void ExtractBone(Pose &pose)
	{
		//Mqoのオブジェクト群からボーンオブジェクトを見つける
		int nmesh=meshes.size();
		int target=-1;
		for(int i=0;i<nmesh;i++)
		{
			if(meshes[i].name.substr(0,4)=="bone")
			{
				target=i;
				break;
			}
		}
		if(target==-1)return;
		
		Mesh &m=meshes[target];
		
		int nfc=m.faces.size();
		//各面・辺をボーン・ブリッジに変換する
		for(int j=0;j<nfc;j++)
		{
			Face &f=m.faces[j];
			int nvtx=f.vertex.size();
			//三角形の場合はボーン
			if(nvtx==3)
			{
				//辺の長さの順番によって
				//その三角形がどういうボーンなのかを求める。
				Bone b;
				int max=0,min=1;

				float d[3];
				d[0]=m.vertex[f.vertex[0]].Dist(m.vertex[f.vertex[1]]);
				d[1]=m.vertex[f.vertex[1]].Dist(m.vertex[f.vertex[2]]);
				d[2]=m.vertex[f.vertex[2]].Dist(m.vertex[f.vertex[0]]);

				float tmp=d[0];

				if(tmp<d[1]){tmp=d[1];max=1;}
				if(tmp<d[2]){max=2;}

				tmp=d[1];
				if(tmp>d[0]){tmp=d[0];min=0;}
				if(tmp>d[2]){min=2;}

				int rot=(3+max-min)%3;//1 or 2=-1

				int r,n,t;
				r=(min+rot-1)%3;
				n=(r+rot)%3;
				t=(n+rot)%3;

				b.root=f.vertex[r];
				b.norm=f.vertex[n];
				b.top=f.vertex[t];

				//座標割り当て
				b.offset=m.vertex[b.root];
				b.originalcoord=b.offset;
				b.rotation=Vector3ToQuaternion(m.vertex[b.root],m.vertex[b.top],m.vertex[b.norm]);
				b.originalattitude=b.rotation;

				//ボーンの名前は、三角形に関連付けられたマテリアル名とする
				if(f.material>=0)
				{
					std::string s=materials[f.material].name;
					
					//左右存在する形式かどうか。
					b.name=s;

					if(s.length()-2>=0)
					{
						if(s.substr(s.length()-2,2)=="[]")
						{
							std::string s2;
							if(m.vertex[f.vertex[r]].x>0)
								s2="[L]";
							else
								s2="[R]";

							b.name=s.substr(0,s.length()-2);
							b.name+=s2;
						}
					}
				}

				int lname=b.name.length();

				//元物体の座標を変換する
				for(int k=0;k<nmesh;k++)
				{
					Mesh &m=meshes[k];
					int pos=m.name.length()-lname-1;
					if(pos<0)continue;
					if(m.name.substr(pos)==std::string("-")+b.name)
					{
						Quaternionf q=b.rotation.Inv();
						for(int l=0;l<m.vertex.size();l++)
						{
							m.vertex[l]=q.Rotate(m.vertex[l]-b.offset);
						}
						b.linked_meshes.push_back(k);
						
						m.CalcNormal();
					}	
				}
				pose.Append(b);
			}
			//辺の場合は、ボーンとボーンをつなぐブリッジ
			else if(nvtx==2)
			{
				Bone b;

				b.root=f.vertex[0];
				b.top=f.vertex[1];
				b.norm=-1;

				pose.Append(b);
			}
		}

		int nb=pose.bones.size();

		//辺形状の方向を整える。
		//(親ボーンから子ボーンへ向かうようにする)
		for(int j=0;j<nb;j++)
		{
			Bone &b=pose.bones[j];
			if(b.norm==-1)
			{
				for(int k=0;k<nb;k++)
				{
					if(k==j)continue;
					Bone &c=pose.bones[k];
					if(c.norm==-1)continue;
					if(b.root==c.top)
					{
						b.parent=k;
						c.child.push_back(j);
					}
					else if(b.top==c.top)
					{
						b.parent=k;
						std::swap(b.top,b.root);
						c.child.push_back(j);
					}
					b.offset=m.vertex[b.root];
				}
			}
		}

		//ボーンのリンクを張る
		for(int j=0;j<nb;j++)
		{
			Bone &b=pose.bones[j];
			if(b.norm>=0)
			{
				for(int k=0;k<nb;k++)
				{
					if(k==j)continue;
					int l=k;
					Bone &c=pose.bones[l];
					if(b.root==c.top)//cの先端にbの根が付いている
					{
						while(1)
						{
							Bone &c=pose.bones[l];

							if(c.norm==-1)
								l=c.parent;
							else
								break;
						}
						Bone &c=pose.bones[l];
						b.parent=l;
						c.child.push_back(j);
					}
				}
			}
		}

		//座標を階層構造にする
		for(int j=0;j<nb;j++)
		{
			if(pose.bones[j].parent==-1)
			{
				pose.roots.push_back(j);
				pose.MakeSubCoords(j);
			}
		}

		//変形オブジェクトを抽出する
		FindSdef(sdef);
		for(int i=0;i<sdef.size();i++)
		{
			ExtractSdef(sdef[i],pose);
		}
	}

	//球面変形メッシュを抽出する
	//n:メッシュID
	void ExtractSdef(int n,Pose &pose)
	{
		Mesh &m=meshes[n];

		intv anchor;
		FindAnchor(m.name.substr(5),anchor);

		//各アンカーに分ける
		Meshv anchormesh;
		for(int i=0;i<anchor.size();i++)
		{
			meshes[anchor[i]].SplitByGroup(anchormesh);
		}

		Trianglevv anctris;
		//アンカーに名前をつけ、その名前のボーンに関連付ける
		//ついでに三角形に分解する
		for(int i=0;i<anchormesh.size();i++)
		{
			if(anchormesh[i].faces[0].material!=-1)//マテリアルから名前をつける
			{
				anchormesh[i].name=materials[anchormesh[i].faces[0].material].name;
			}

			std::string s=anchormesh[i].name;
			if(s.length()-2>=0)
			{
				if(s.substr(s.length()-2,2)=="[]")
				{
					std::string s2;
					if(anchormesh[i].vertex[anchormesh[i].faces[0].vertex[0]].x>0)
						s2="[L]";
					else
						s2="[R]";

					anchormesh[i].name=s.substr(0,s.length()-2);
					anchormesh[i].name+=s2;
				}
			}

			for(int j=0;j<pose.bones.size();j++)
			{
				if(anchormesh[i].name==pose.bones[j].name)
				{
					anchormesh[i].linked_bone=j;
					break;
				}
			}
			Trianglev t;
			anchormesh[i].MakeTriangles(t);
			anctris.push_back(t);
		}
		
		//Sdefオブジェクトの各頂点について、ボーンの重みを計算する
		int vsz=m.vertex.size();
		int asz=anchormesh.size();
		m.weights.resize(vsz);
		for(int i=0;i<vsz;i++)
		{
			Vector3f v=m.vertex[i];
			for(int j=0;j<asz;j++)
			{
				if(anchormesh[j].Inside(anctris[j],v))
				{
					float d=anchormesh[j].InsideDistance(anctris[j],v);
					m.weights[i].Append(anchormesh[j].linked_bone,d);
				}
			}
			//重み付けを正規化
			m.weights[i].Normalize();
		}
	}

	//アンカーをさがす
	void FindAnchor(std::string link,intv &anchor)
	{
		//Mqoのオブジェクト群からアンカーブジェクトを見つける
		int nmesh=meshes.size();
		for(int i=0;i<nmesh;i++)
		{
			if(meshes[i].name.substr(0,6)!="anchor")
				continue;

			Mesh &m=meshes[i];
			std::string l=m.name.substr(m.name.find('|')+1);
			if(l==link)
				anchor.push_back(i);
		}
	}
	//Sdefをさがす
	void FindSdef(intv &sdef)
	{
		//Mqoのオブジェクト群からSdefブジェクトを見つける
		int nmesh=meshes.size();
		for(int i=0;i<nmesh;i++)
		{
			if(meshes[i].name.substr(0,5)!="sdef:")
				continue;

			sdef.push_back(i);
		}
	}
	//頂点座標割り当て済みの三角形を作る
	//meshID:メッシュID
	void MakeTriangleList(int meshID)
	{
		meshes[meshID].MakeTriangleList(materials);
	}

	////////////////////////////////////
	// 変形系
	//

	void Deform(const Pose &pose)
	{
		int bsz=pose.bones.size();
		static Vector3fv coords;//ほんとうはスレッド固有がいい。
		static Quaternionfv attis;
		coords.resize(bsz);
		attis.resize(bsz);
		for(int i=0;i<bsz;i++)
		{
			pose.CalcPresentAttitude(i,coords[i],attis[i]);
		}
		
		sdefmeshes.resize(sdef.size());
		for(int i=0;i<sdef.size();i++)
		{
			DeformMesh(sdef[i],sdefmeshes[i],coords,attis,pose);
		}
	}

	//Sdefのついたメッシュをボーンをもとに変形する
	void DeformMesh(int n,Mesh &result,const Vector3fv &coords,const Quaternionfv &attis,const Pose &pose)const
	{	
		const Mesh &m=meshes[n];
		if(m.weights.empty())return;
		
		result.faces=m.faces;
		result.classified_triangles=m.classified_triangles;
		result.vertex.resize(m.vertex.size());
		
		int vsz=m.vertex.size();
		for(int i=0;i<vsz;i++)
		{
			result.vertex[i]=DeformVertex(m.vertex[i],m.weights[i],coords,attis,pose);
		}
		result.CalcNormal();
	}
	//頂点位置をボーンをもとに移動する
	Vector3f DeformVertex(const Vector3f &v,const WeightLink &w,const Vector3fv &coords,const Quaternionfv &attis,const Pose &pose)const
	{
		Vector3f result(0,0,0);
		for(int i=0;i<w.weight.size();i++)
		{
			int lk=w.link[i];
			float wt=w.weight[i];
			
			Vector3f d=v-pose.bones[lk].originalcoord;
			Vector3f nd=pose.bones[lk].originalattitude.Inv().Rotate(d);
			
			Vector3f rd=attis[lk].Rotate(nd);
			Vector3f trd=rd+coords[lk];
			result+=trd*wt;
		}
		return result;
	}

};

};//end of namespace Rydot

