
// model.h
//
// model
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// 3D���f�����e�L�g�[�Ɉ����N���X

#pragma once

#include "mesh.h"
#include "motion.h"

#include "modelloader.h"

namespace Rydot
{

//////////////////////////////////////////////////
// ���f��
// ���b�V���ƃ}�e���A���ƃ|�[�Y�̈ꎮ�Z�b�g
// �����Ń����_�����O�ł���͂��B
//
class Model
{
public:
	Materialv materials;// �}�e���A��
	Meshv meshes;// ���b�V��
	Meshv sdefmeshes;// ���ʕό`sdef�������b�V��

	intv sdef;// sdef�����̃��b�V����ID

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
	// �\�z�n
	//

	//�{�[���𒊏o����
	//���o�����{�[����pose�ɏo�͂����B
	//�v�Z�@��mikoto�̌`�����̗p
	//���ӂ͓��ɈӖ��Ȃ�
	//���ӂ��{�[���̎���\���A
	//�Z�ӂɂ������Ă��钸�_�����̃{�[���̃��[�J�����_
	//�Z�ӂ̕������{�[����sky����
	void ExtractBone(Pose &pose)
	{
		//Mqo�̃I�u�W�F�N�g�Q����{�[���I�u�W�F�N�g��������
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
		//�e�ʁE�ӂ��{�[���E�u���b�W�ɕϊ�����
		for(int j=0;j<nfc;j++)
		{
			Face &f=m.faces[j];
			int nvtx=f.vertex.size();
			//�O�p�`�̏ꍇ�̓{�[��
			if(nvtx==3)
			{
				//�ӂ̒����̏��Ԃɂ����
				//���̎O�p�`���ǂ������{�[���Ȃ̂������߂�B
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

				//���W���蓖��
				b.offset=m.vertex[b.root];
				b.originalcoord=b.offset;
				b.rotation=Vector3ToQuaternion(m.vertex[b.root],m.vertex[b.top],m.vertex[b.norm]);
				b.originalattitude=b.rotation;

				//�{�[���̖��O�́A�O�p�`�Ɋ֘A�t����ꂽ�}�e���A�����Ƃ���
				if(f.material>=0)
				{
					std::string s=materials[f.material].name;
					
					//���E���݂���`�����ǂ����B
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

				//�����̂̍��W��ϊ�����
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
			//�ӂ̏ꍇ�́A�{�[���ƃ{�[�����Ȃ��u���b�W
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

		//�ӌ`��̕����𐮂���B
		//(�e�{�[������q�{�[���֌������悤�ɂ���)
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

		//�{�[���̃����N�𒣂�
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
					if(b.root==c.top)//c�̐�[��b�̍����t���Ă���
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

		//���W���K�w�\���ɂ���
		for(int j=0;j<nb;j++)
		{
			if(pose.bones[j].parent==-1)
			{
				pose.roots.push_back(j);
				pose.MakeSubCoords(j);
			}
		}

		//�ό`�I�u�W�F�N�g�𒊏o����
		FindSdef(sdef);
		for(int i=0;i<sdef.size();i++)
		{
			ExtractSdef(sdef[i],pose);
		}
	}

	//���ʕό`���b�V���𒊏o����
	//n:���b�V��ID
	void ExtractSdef(int n,Pose &pose)
	{
		Mesh &m=meshes[n];

		intv anchor;
		FindAnchor(m.name.substr(5),anchor);

		//�e�A���J�[�ɕ�����
		Meshv anchormesh;
		for(int i=0;i<anchor.size();i++)
		{
			meshes[anchor[i]].SplitByGroup(anchormesh);
		}

		Trianglevv anctris;
		//�A���J�[�ɖ��O�����A���̖��O�̃{�[���Ɋ֘A�t����
		//���łɎO�p�`�ɕ�������
		for(int i=0;i<anchormesh.size();i++)
		{
			if(anchormesh[i].faces[0].material!=-1)//�}�e���A�����疼�O������
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
		
		//Sdef�I�u�W�F�N�g�̊e���_�ɂ��āA�{�[���̏d�݂��v�Z����
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
			//�d�ݕt���𐳋K��
			m.weights[i].Normalize();
		}
	}

	//�A���J�[��������
	void FindAnchor(std::string link,intv &anchor)
	{
		//Mqo�̃I�u�W�F�N�g�Q����A���J�[�u�W�F�N�g��������
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
	//Sdef��������
	void FindSdef(intv &sdef)
	{
		//Mqo�̃I�u�W�F�N�g�Q����Sdef�u�W�F�N�g��������
		int nmesh=meshes.size();
		for(int i=0;i<nmesh;i++)
		{
			if(meshes[i].name.substr(0,5)!="sdef:")
				continue;

			sdef.push_back(i);
		}
	}
	//���_���W���蓖�čς݂̎O�p�`�����
	//meshID:���b�V��ID
	void MakeTriangleList(int meshID)
	{
		meshes[meshID].MakeTriangleList(materials);
	}

	////////////////////////////////////
	// �ό`�n
	//

	void Deform(const Pose &pose)
	{
		int bsz=pose.bones.size();
		static Vector3fv coords;//�ق�Ƃ��̓X���b�h�ŗL�������B
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

	//Sdef�̂������b�V�����{�[�������Ƃɕό`����
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
	//���_�ʒu���{�[�������ƂɈړ�����
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

