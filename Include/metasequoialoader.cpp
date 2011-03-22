
// metasequoialoader.cpp
//
// metasequoia model loader
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// メタセコイアで作った3Dモデルを読み込んだりするクラス
// ついでにミコトを読み込んだりするクラスもある
// yaccとかlexとか使やいいのにこんなものを作って...

#include "matrix.h"
#include "Mesh.h"
#include "Motion.h"
#include "Model.h"
#include "ModelLoader.h"
#include "metasequoiaLoader.h"

namespace Rydot
{

//////////////////////////////////////////////////
// Mqoファイルを読み込む
//
bool MetasequoiaLoader::LoadMqo(IRender &render,Model &model)
{
	//header
	{
		char cs[256];
		(*i).getline(cs,256);
		if(!strcmp(cs,"Metasequoia Document"))return false;
		(*i).getline(cs,256);
		if(!strcmp(cs,"Format Text Ver 1.0"))return false;
	}
	
	while(1)
	{
		std::string s;
		ReadToken(s);

		if(s=="Object")
		{
			Mesh m;
			ReadLit(m.name);

			SkipPreChar('{');

			LoadObject(m);

			model.Append(m);
		}
		else if(s=="Material")
		{
			int n;
			(*i)>>n;//num of Materials

			SkipPreChar('{');

			model.materials.reserve(n);

			for(int j=0;j<n;j++)
			{
				Material m;
				LoadMaterial(m);
				if(!m.texture.empty())
				{
					int idx=render.texmgr->Load(m.texture.c_str());
					m.itexture=idx;
				}
				model.Append(m);
			}
			SkipPreChar('}');
		}
		else if(s=="Eof")break;
		else
		{
			SkipPreChar('}');
		}
	}
	return true;
}


bool MetasequoiaLoader::LoadObject(Mesh &mesh)
{
	//Vertex
	while(1)
	{
		std::string s;
		ReadToken(s);
		if(s=="vertex")
		{
			int n;
			(*i)>>n;//num of Vertex
			mesh.vertex.reserve(n);
			
			SkipPreChar('{');
			
			for(int j=0;j<n;j++)
			{
				Vector3f v;
				(*i)>>v.x>>v.y>>v.z;
				mesh.Append(v);
			}
			
			SkipPreChar('}');
			break;
		}
		else if(s=="BVertex")
		{
			int n;
			(*i)>>n;//num of Vertex
			mesh.vertex.reserve(n);
			
			SkipPreChar('\n');
			SkipPreChar('\n');//Vector xx[xx]
			
			for(int j=0;j<n;j++)
			{
				Vector3f v;
				char c[4];
				(*i).read(c,4);v.x=*((float*)(void*)c);
				(*i).read(c,4);v.y=*((float*)(void*)c);
				(*i).read(c,4);v.z=*((float*)(void*)c);
				mesh.Append(v);
			}
			
			SkipPreChar('}');
			break;
		}
		SkipPreChar('\n');
	}//end of Vertex
	
	//Face
	{
		std::string s;
		ReadToken(s);
		int n;
		(*i)>>n;//num of Faces
		
		mesh.faces.reserve(n);
		SkipPreChar('{');
		
		for(int j=0;j<n;j++)
		{
			Face face;
			LoadFace(face);
			mesh.Append(face);
		}
		
		SkipPreChar('}');
	}//end of Face
	
	SkipPreChar('}');//end of Object
	
	mesh.SortTrisByMaterial();
	mesh.CalcNormal();
	
	return true;
}


bool MetasequoiaLoader::LoadFace(Face &face)
{
	int n;
	(*i)>>n;//num of Vertex

	//attributes
	while(1)
	{
		std::string s;
		ReadToken(s);
		
		SkipPreChar('(');
		
		if(s=="V")//面の頂点数
		{
			face.vertex.reserve(n);
			for(int j=0;j<n;j++)
			{
				int v;
				(*i)>>v;
				face.vertex.push_back(v);
			}
		}
		else if(s=="M"){(*i)>>face.material;}//マテリアル
		else if(s=="UV")//テクスチャ座標
		{
			face.uv.reserve(n);
			for(int j=0;j<n;j++)
			{
				Vector2f u;
				(*i)>>u.x>>u.y;
				face.uv.push_back(u);
			}
		}
		else
		{
			float dummy;
			(*i)>>dummy;
		}
		
		SkipPreChar(')');
		
		int p=(*i).peek();
		if(p=='\n'||p=='\r')
			break;
	}
	
	return true;
}


bool MetasequoiaLoader::LoadMaterial(Material &material)
{
	material.diffusecolor[0]=1.0;
	material.diffusecolor[1]=1.0;
	material.diffusecolor[2]=1.0;
	material.specularcolor[0]=1.0;
	material.specularcolor[1]=1.0;
	material.specularcolor[2]=1.0;

	//name
	{
		std::string s;
		ReadLit(s);
		material.name=s;
	}

	//attributes
	while(1)
	{
		std::string s;
		ReadToken(s);

		SkipPreChar('(');

		if(s=="col")
		{
			(*i)>>material.color[0]
			    >>material.color[1]
			    >>material.color[2]
			    >>material.color[3];
		}
		else if(s=="dif"){(*i)>>material.diffuse;}
		else if(s=="amb"){(*i)>>material.ambient;}
		else if(s=="emi"){(*i)>>material.emission;}
		else if(s=="spc"){(*i)>>material.specular;}
		else if(s=="power"){(*i)>>material.power;}
		else if(s=="tex"){ReadLit(material.texture);}
		else if(s=="aplane"){std::string tmp;ReadLit(tmp);}
		else if(s=="bump"){std::string tmp;ReadLit(tmp);}
		else
		{
			float dummy;
			(*i)>>dummy;
		}
		
		SkipPreChar(')');

		int p=(*i).peek();
		if(p=='\n'||p=='\r')
			break;
	}

	return true;
}


//////////////////////////////////////////////////
// Mkmファイルを読み込む
//
bool MikotoLoader::LoadMkm(Pose &pose)
{
	//header
	{
		char cs[256];
		(*i).getline(cs,256);
		if(!strcmp(cs,"Mikoto Motion Ver 2"))return false;
	}


	while(1)
	{
		std::string s;
		ReadToken(s);

		if(s=="Motion")
		{
			SkipPreChar('{');

			Motion m;
			LoadMotion(m);

			SkipPreChar('}');

			//データからボーンへのリンクを張る
			int nbone=pose.bones.size();
			for(int i=0;i<m.attitudes.size();i++)
			{
				Attitude &a=m.attitudes[i];
				a.link=-1;
				for(int j=0;j<nbone;j++)
				{
					if(a.name==pose.bones[j].name)
					{
						a.link=j;
						break;
					}
				}
			}
			
			pose.Append(m);
		}
		else if("Eof")break;
	}
	return true;
}


bool MikotoLoader::LoadMotion(Motion &motion)
{
	{
		std::string s;
		ReadToken(s);//name
	}
	SkipPreChar('=');
	{
		std::string s;
		ReadLit(s);
		motion.name=s;
	}
	{
		std::string s;
		ReadToken(s);//endframe
	}
	SkipPreChar('=');
	(*i)>>motion.endframe;
	SkipPreChar('\n');//
	SkipPreChar('=');
	(*i)>>motion.loop;
	SkipPreChar('\n');//loop

	while(1)
	{
		SkipSpace();

		if((*i).peek()=='}')//end of Motion
			break;

		std::string s;
		ReadToken(s);

		if(s=="Quaternion")
		{
			SkipPreChar('{');
			Attitude a;
			{
				std::string s;
				ReadToken(s);//name
			}
			SkipPreChar('=');
			{
				std::string s;
				ReadLit(s);
				a.name=s;
			}

			SkipPreChar('\n');//
			SkipPreChar('\n');//class
			SkipPreChar('\n');//member
			SkipPreChar('\n');//curve

			while(1)
			{
				SkipSpace();

				if((*i).peek()=='}')break;

				int v;
				(*i)>>v;
				a.frame.push_back(v);

				SkipPreChar('(');

				Quaternionf q;
				(*i)>>q.x>>q.y>>q.z>>q.w;

				q.Normalize();

				a.rotation.push_back(q);
				
				SkipPreChar(')');
			}
			a.nscene=a.frame.size();
			
			//クォータニオンの裏返ってるやつを元に戻す
			for(int i=0;i<a.nscene-1;i++)
			{
				if(a.rotation[i].DotProd(a.rotation[i+1])<0)
				{
					a.rotation[i+1]*=-1;
				}
			}
			
			//スプライン係数を求めておく
			a.MakeSplineCoef();

			motion.Append(a);

			SkipPreChar('}');
		}
		else if(s=="Vector")
		{
			SkipPreChar('{');
			Joint a;
			{
				std::string s;
				ReadToken(s);//name
			}
			SkipPreChar('=');
			{
				std::string s;
				ReadLit(s);
				a.name=s;
			}

			SkipPreChar('\n');//
			SkipPreChar('\n');//class
			SkipPreChar('\n');//member
			SkipPreChar('\n');//curve

			while(1)
			{
				SkipSpace();

				if((*i).peek()=='}')break;

				int v;
				(*i)>>v;
				a.frame.push_back(v);

				SkipPreChar('(');

				Vector3f u;
				(*i)>>u.x>>u.y>>u.z;
				a.position.push_back(u);

				SkipPreChar(')');
			}
			a.nscene=a.frame.size();
			motion.Append(a);
			
			SkipPreChar('}');
		}
	}
	return true;
}

}//end of namespace Rydot
