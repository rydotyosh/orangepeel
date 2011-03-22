
// render.h
//
// render
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// 3DCGをレンダリングするやつ
// インタフェイスっぽいのがあるけど
// 中身はOpenGLしかない。
// なんか動的に変えるのがめんどくさそうだし・・・。

#pragma once

#include "model.h"
#include "motion.h"
#include "glex/texture.h"

namespace Rydot
{

class IRender
{
public:
	TextureManager texmgr;

public:
	virtual ~IRender(){}
	
	virtual int Init()=0;
	virtual int Draw(const Model &model,const Pose &pose)=0;
};

class NoRender:public IRender
{
public:
	~NoRender(){}
	int Init(){}
	int Draw(const Model &model,const Pose &pose){}
};

class GlRender:public IRender
{
public:
	GlRender(){}
	~GlRender(){}

	int Init()
	{
		return 0;
	}
	
	//普通に描画
	//ボーンに直接くっついているオブジェクトと
	//sdefしたオブジェクトを描画する。
	int Draw(const Model &model,const Pose &pose)
	{
		//ボーンに直接くっついているオブジェクト
		int nroots=pose.roots.size();
		for(int i=0;i<nroots;++i)
		{
			DrawSubBone(model,pose.roots[i],pose);
		}
		//sdefしたオブジェクト
		int nsdef=model.sdef.size();
		for(int i=0;i<nsdef;++i)
		{
			DrawMesh(model,model.sdefmeshes[i]);
		}
		return 0;
	}

	//ボーンとその全ての子ボーンに直接くっついているオブジェクトを描画する
	void DrawSubBone(const Model &model,int i,const Pose &pose)
	{
		const Bone &b=pose.bones[i];
		if(b.norm==-1)return;
		
		glPushMatrix();
		
		float ofs[3],rot[16];
		b.offset.ToArray(ofs);
		b.rotation.ToRotMatrix33().ToArray44(rot);
		
		glTranslatef(ofs[0],ofs[1],ofs[2]);
		
		glMultMatrixf(rot);
		
		for(int j=0;j<b.linked_meshes.size();j++)
		{
			DrawMesh(model,b.linked_meshes[j]);
		}
		
		for(int j=0;j<b.child.size();j++)
		{
			DrawSubBone(model,b.child[j],pose);
		}
		glPopMatrix();
	}

	// メッシュを描画する
	int DrawMesh(const Model &model,const std::string &name)
	{
		return DrawMesh(model,model.GetMeshID(name));
	}
	int DrawMesh(const Model &model,int i)
	{
		return DrawMesh(model,model.meshes[i]);
	}
	int DrawMesh(const Model &model,const Mesh &o)
	{
		static std::vector<float> tmpvtx,tmpuv,tmpcol;

		if(!model.outline_only)
		{
			//三角形
			for(Trianglevl::const_iterator itr=o.classified_triangles.begin();itr!=o.classified_triangles.end();++itr)
			{
				const Trianglev &fv=*itr;
				int sz=fv.size();
	
				//material
				int istexture=SelectMaterial(model,fv[0].material);
	
				//vertex
				tmpvtx.resize(sz*(3*3));
				float *vtx=&tmpvtx[0];
				for(int j=0;j<sz;++j)
				{
					const Triangle &f=fv[j];
					for(int k=0;k<3;++k)
					{
						int v=f.vertex[k];
						o.vertex[v].ToArray(vtx+3*(k+3*j));
					}
				}
				glVertexPointer(3,GL_FLOAT,0,vtx);
	
				//color
				if(model.light_active)
				{
					tmpcol.resize(sz*(3*3));
					float *col=&tmpcol[0];
					for(int j=0;j<sz;++j)
					{
						const Triangle &f=fv[j];
						for(int k=0;k<3;++k)
						{
							int v=f.vertex[k];
							Vector3f c=LightAppliedColor(model,fv[0].material,o.vertex[v],o.normals[v]);
							c.ToArray(col+3*(k+3*j));
						}
					}
					glColorPointer(3,GL_FLOAT,0,col);
				}
				
				//uv
				float *uv=0;
				if(istexture)
				{
					tmpuv.resize(sz*(2*3));
					uv=&tmpuv[0];
					for(int j=0;j<sz;++j)
					{
						const Triangle &f=fv[j];
						if(f.uvexist)
						{
							for(int k=0;k<3;k++)
							{
								f.uv[k].ToArray(uv+2*(k+3*j));
							}
						}
						else
						{
							for(int k=0;k<6;k++)
							{
								uv[k+6*j]=0;
							}	
						}
					}
					glTexCoordPointer(2,GL_FLOAT,0,uv);
				}
				glDrawArrays(GL_TRIANGLES,0,3*sz);
			}
		}

		//輪郭線のようなやつ
		if(model.outlineelevation>0)
		{	
			for(Trianglevl::const_iterator itr=o.classified_triangles.begin();itr!=o.classified_triangles.end();++itr)
			{
				const Trianglev &fv=*itr;
				int sz=fv.size();
				
				//material
				int istexture=SelectMaterial2(model,fv[0].material);
				
				//vertex
				tmpvtx.resize(sz*(3*3));
				float *vtx=&tmpvtx[0];
				for(int j=0;j<sz;++j)
				{
					const Triangle &f=fv[j];
					for(int k=0;k<3;++k)
					{
						int v=f.vertex[k];
						Vector3f p=o.vertex[v];
						Vector3f n=o.normals[v];
						p-=n*model.outlineelevation;
						if(n.Abs()>2)
						{
							int a=0;
						}
						p.ToArray(vtx+3*(2-k+3*j));
					}
				}
				glVertexPointer(3,GL_FLOAT,0,vtx);
				
				//uv
				float *uv=0;
				if(istexture)
				{
					tmpuv.resize(sz*(2*3));
					float *uv=&tmpuv[0];
					for(int j=0;j<sz;++j)
					{
						const Triangle &f=fv[j];
						if(f.uvexist)
						{
							for(int k=0;k<3;k++)
							{
								f.uv[k].ToArray(uv+2*(k+3*j));
							}
						}
						else
						{
							for(int k=0;k<6;k++)
							{
								uv[k+6*j]=0;
							}	
						}
					}
					glTexCoordPointer(2,GL_FLOAT,0,uv);
				}
				glDrawArrays(GL_TRIANGLES,0,3*sz);
			}
		}

		//辺
		for(Edgevl::const_iterator itr=o.classified_edges.begin();itr!=o.classified_edges.end();++itr)
		{
			const Edgev &ev=*itr;
			int sz=ev.size();
			
			//material
			int istexture=SelectMaterial(model,ev[0].material);
			
			//vertex
			tmpvtx.resize(sz*(3*2));
			float *vtx=&tmpvtx[0];
			for(int j=0;j<sz;++j)
			{
				const Edge &e=ev[j];
				for(int k=0;k<2;++k)
				{
					int v=e.vertex[k];
					o.vertex[v].ToArray(vtx+3*(k+2*j));
				}
			}
			glVertexPointer(3,GL_FLOAT,0,vtx);
			
			//uv
			float *uv=0;
			if(istexture)
			{
				tmpuv.resize(sz*(2*2));
				float *uv=&tmpuv[0];
				for(int j=0;j<sz;++j)
				{
					const Edge &e=ev[j];
					if(e.uvexist)
					{
						for(int k=0;k<2;k++)
						{
							e.uv[k].ToArray(uv+2*(k+2*j));
						}
					}
					else
					{
						for(int k=0;k<6;k++)
						{
							uv[k+6*j]=0;
						}	
					}
				}
				glTexCoordPointer(2,GL_FLOAT,0,uv);
			}
			glDrawArrays(GL_LINES,0,2*sz);
		}
		return 0;
	}
	
	
	//座標割り当て済みの三角形を描画する
	int DrawMeshTransformed(const Model &model,std::string &name)
	{
		return DrawMeshTransformed(model,model.GetMeshID(name));
	}
	int DrawMeshTransformed(const Model &model,int i)
	{
		return DrawMeshTransformed(model,model.meshes[i]);
	}
	int DrawMeshTransformed(const Model &model,const Mesh &o)
	{
		int sz=o.transformed_triangles.size();
		for(int i=0;i<sz;++i)
		{
			const VertexTriangleList &tl=o.transformed_triangles[i];
			int tsz=tl.vertex.size();

			//material
			int istexture=SelectMaterial(model,tl.material);

			//vertex
			glVertexPointer(3,GL_FLOAT,0,&tl.vertex[0]);

			//uv
			if(istexture)
			{
				glTexCoordPointer(2,GL_FLOAT,0,&tl.uv[0]);
			}
			glDrawArrays(GL_TRIANGLES,0,tsz);
		}
		return 0;
	}

	//マテリアルを採用する
	//現在は色とテクスチャのみ設定する
	int SelectMaterial(const Model &model,int i)
	{
		if(!model.material_active)
		{
			return 0;
		}
		int istexture=0;
		if(i>=0)
		{
			const Material &m=model.materials[i];
			glColor4f(m.color[0],m.color[1],m.color[2],m.color[3]);

			if(m.itexture>=0&&model.texture_active)
			{
				istexture=1;
				glEnable(GL_TEXTURE_2D);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				texmgr->Activate(m.itexture);
			}
			else
			{
				glDisable(GL_TEXTURE_2D);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}
			
			if(model.light_active)
			{
				glEnableClientState(GL_COLOR_ARRAY);
			}
			else
			{
				glDisableClientState(GL_COLOR_ARRAY);
			}
		}
		else
		{
			glColor3f(1,1,1);
			glDisable(GL_TEXTURE_2D);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);

			if(model.light_active)
			{
				glEnableClientState(GL_COLOR_ARRAY);
			}
			else
			{
				glDisableClientState(GL_COLOR_ARRAY);
			}
		}
		return istexture;
	}
	
	//マテリアルを採用する
	//輪郭線用
	int SelectMaterial2(const Model &model,int i)
	{
		if(!model.material_active)return 0;
		int istexture=0;
		if(i>=0)
		{
			const Material &m=model.materials[i];
			glColor3f(
				model.outlinebias.x+m.color[0]*model.outlinerange.x,
				model.outlinebias.y+m.color[1]*model.outlinerange.y,
				model.outlinebias.z+m.color[2]*model.outlinerange.z);
			if(m.itexture>=0&&model.texture_active)
			{
				istexture=1;
				glEnable(GL_TEXTURE_2D);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				texmgr->Activate(m.itexture);
			}
			else
			{
				glDisable(GL_TEXTURE_2D);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}
		}
		else
		{
			glColor3f(1,1,1);
			glDisable(GL_TEXTURE_2D);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		glDisableClientState(GL_COLOR_ARRAY);
		
		return istexture;
	}
	
	//ライト適用時の色を返す
	Vector3f LightAppliedColor(const Model &model,int material,const Vector3f &pos,const Vector3f &normal)
	{
		if(!model.material_active)return Vector3f(1,1,1);
		if(!model.light_active)return Vector3f(1,1,1);
		if(material>=0)
		{
			Matrix44f mx;
			glGetFloatv(GL_MODELVIEW_MATRIX ,(float*)mx.m);
			
		//	std::cout<<mx<<std::endl;
			
			Vector3f p=mx*pos;
			Vector3f n=mx*(pos+normal)-p;
			Vector3f lp=mx*model.light_pos;
			Vector3f lc=model.light_color;
			
			const Material &m=model.materials[material];
			float lev=n.Norm().DotProd((p-lp).Norm());
			if(lev<0)lev=0;
			Vector3f emi(m.color[0]*m.emission,m.color[1]*m.emission,m.color[2]*m.emission);
			Vector3f amb=model.ambient_color*m.ambient;
			//Vector3f dif(m.color[0]*lc.x*(m.diffuse*lev),m.color[1]*lc.y*(m.diffuse*lev),m.color[2]*lc.z*(m.diffuse*lev));
			Vector3f dif(m.diffusecolor[0]*lc.x*(m.diffuse*lev),m.diffusecolor[1]*lc.y*(m.diffuse*lev),m.diffusecolor[2]*lc.z*(m.diffuse*lev));
			Vector3f c=amb+emi+dif;
			return c;
		}
		return Vector3f(1,1,1);
	}
};


}//end of namespace Rydot

