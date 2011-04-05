
// motion.h
//
// motion classes for skin-bone model
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// �X�L���{�[�����f���̃e�L�g�[�ȃ��[�V���������
// IK�ق����B

#pragma once

#include "matrix.h"
#include "matrixutil.h"
#include <vector>
#include <string>
#include "fileex.h"

namespace Rydot
{

typedef std::vector<int> intv;
typedef std::vector<Vector3f> Vector3fv;
typedef std::vector<Quaternionf> Quaternionfv; 

class IModelLoader;

class Model;

//////////////////////////////////////////////////
// Joint
// ���[�V�����̕��s�ړ��ϊ���S������
//
class Joint
{
public:
	std::string name;
	int link;
	int nscene;
	intv frame;
	Vector3fv position;
	int usespline;
public:
	Joint():usespline(1){}

	//p�̈ʒu�ŕ⊮����
	Vector3f Interpolate(float p,int loop,int end)
	{
		//p%=end;
		p=fmod(p,float(end));
		int a,b,i;
		for(i=0;i<nscene;i++)
		{
			if(frame[i]>p)
				break;
		}
		a=i-1;
		b=i;
		if(i==nscene)
		{
			if(loop)
				b=0;
			else
			{
				return position[i-1];
			}
		}
		int fa=frame[a],fb=frame[b];
		float ratio=(p-fa)/ ((end+fb-fa)%end);
		if(usespline)
			return CatmullRom
			(position[(a+nscene-1)%nscene],position[a],position[b],position[(b+1)%nscene],ratio);
		else
			//linear interpolate
			return position[a]*(1-ratio)+position[b]*(ratio);
	}
};


//////////////////////////////////////////////////
// Attitude
// ���[�V�����̉�]�ϊ���S������
//
class Attitude
{
public:
	std::string name;
	int link;
	int nscene;
	intv frame;
	Quaternionfv rotation;
	Quaternionfv splinecoef;
	int usespline;
public:
	Attitude():usespline(1){}
	void MakeSplineCoef()
	{
		if(nscene<=1)return;
		splinecoef.resize(nscene);
		for(int i=0;i<nscene;i++)
		{
			Quaternionf Q0=rotation[(i+nscene-1)%nscene],Q1=rotation[i],
			Q2=rotation[(i+1)%nscene];
//			Quaternionf q0=(Q0+Q1).Length2()<(Q0-Q1).Length2()?-Q0:Q0;
//			Quaternionf q2=(Q1+Q2).Length2()<(Q1-Q2).Length2()?-Q2:Q2;
//			Quaternionf q3=(Q2+Q3).Length2()<(Q2-Q3).Length2()?-Q3:Q3;
			splinecoef[i]=Quaternionf::Spline(Q0,Q1,Q2);
		}
	}
	Quaternionf Interpolate(float p,int loop,int end)
	{
		//p%=end;
		p=fmod(p,float(end));
		int a,b,i;
		for(i=0;i<nscene;i++)
		{
			if(frame[i]>p)
				break;
		}
		a=i-1;
		b=i;
		if(i==nscene)
		{
			if(loop)
				b=0;
			else
			{
				return rotation[i-1];
			}
		}
		int fa=frame[a],fb=frame[b];
		
		int w=(end+fb-fa)%end;
		float ratio=(p-fa)/ w;
		if(usespline)
		{
//			Quaternionf Q0=rotation[(a+nscene-1)%nscene],Q1=rotation[a],
//			Q2=rotation[b],Q3=rotation[(b+1)%nscene];
			Quaternionf Q1=rotation[a],Q2=rotation[b];
			Quaternionf A=splinecoef[a],B=splinecoef[b];
//			Quaternionf q0=(Q0+Q1).Length2()<(Q0-Q1).Length2()?-Q0:Q0;
//			Quaternionf q2=(Q1+Q2).Length2()<(Q1-Q2).Length2()?-Q2:Q2;
//			Quaternionf q3=(Q2+Q3).Length2()<(Q2-Q3).Length2()?-Q3:Q3;
//			Quaternionf A=Quaternionf::Spline(Q0,Q1,Q2);
//			Quaternionf B=Quaternionf::Spline(Q1,Q2,Q3);
			Quaternionf r=Quaternionf::Squad(Q1,A,B,Q2,ratio);
		//	r.Normalize();
			return r;
		}
		else
			//linear interpolate
			return Quaternionf::Slerp(rotation[a],rotation[b],ratio);
	}
};
typedef std::vector<Joint> Jointv;
typedef std::vector<Attitude> Attitudev;


//////////////////////////////////////////////////
// ���[�V����
// �L�[�t���[����p���ă��[�V������Ԃ��s��
//
class Motion
{
public:
	std::string name;
	Jointv joints;
	Attitudev attitudes;
	int endframe;
	int loop;
public:
	
	void Append(Attitude &a){attitudes.push_back(a);}
	void Append(Joint &j){joints.push_back(j);}
};
typedef std::vector<Motion> Motionv;


//////////////////////////////////////////////////
// �{�[��
//
class Bone
{
public:
	std::string name;
	int parent;// �e�{�[����ID
	intv child;// �q�{�[����ID
	intv linked_meshes;// ���ڊ֘A�t���Ă��郁�b�V����ID
	int root,top,norm;
	Vector3f originalcoord;// ���Ƃ̍��W
	Vector3f offset;// �e�{�[������̃I�t�Z�b�g
	Quaternionf originalattitude;// ���Ƃ̎p��
	Quaternionf rotation;// ��]
public:
	Bone():parent(-1){}
};
typedef std::vector<Bone> Bonev;


//////////////////////////////////////////////////
// �|�[�Y
//
class Pose
{
public:
	Bonev bones;//�{�[��
	Motionv motions;//���[�V����
	intv roots;//�{�[���̃��[�g

//	static IModelLoader *loader;
public:
	Pose(){}

/*	bool Load(std::string filename)
	{
		return loader->Load(filename,*this);
	}*/
	
	void Append(Bone &b){bones.push_back(b);}
	void Append(Motion &m){motions.push_back(m);}
	void Append(int &r){roots.push_back(r);}

	//���[�V�����̖��O���烂�[�V����ID��T��
	//�Ȃ����-1��Ԃ��B
	int GetMotionID(const std::string &name)const
	{
		int msz=int(motions.size());
		for(int i=0;i<msz;i++)
		{
			if(name==motions[i].name)
				return i;
		}
		return -1;
	}

	//�{�[���̖��O����{�[��ID��T��
	//�Ȃ����-1��Ԃ��B
	int GetBoneID(std::string &name)
	{
		int bsz=int(bones.size());
		for(int i=0;i<bsz;i++)
		{
			if(name==bones[i].name)
				return i;
		}
		return -1;
	}

	//�|�[�Y���Ƃ点��B
	//(�w�肵�����[�V�����̎w�肵���t���[�����{�[���̎p���ɓK�p����B)
	//nmotion:���[�V�����ԍ�
	//count:���̃��[�V�����̈ʒu�Bmotions[nmotion].end�𒴂����0�ɖ߂�B
	void SetPose(int nmotion,float count)
	{
		Motion &m=motions[nmotion];
		int asz=m.attitudes.size();
		int rsz=roots.size();
		int loop=m.loop;
		int end=m.endframe;
		for(int i=0;i<asz;i++)
		{
			Attitude &a=m.attitudes[i];
			bones[a.link].rotation=a.Interpolate(count,loop,end);
		}
		for(int i=0;i<rsz;i++)
		{
			bones[roots[i]].offset=m.joints[0].Interpolate(count,loop,end);
		}
	}

	//���݂̃|�[�Y�ɁA����䗦ratio�ŕʂ̃|�[�Y���|�����킹��
	//���݂̃{�[���̎p���ɁA�w�肵�����[�V�����̎w�肵���t���[���̎p�����������킹��B
	//�D���Ȃ����|�[�Y�������悤�B
	void InterpolatePose(int nmotion,float count,float ratio)
	{
		Motion &m=motions[nmotion];
		int asz=m.attitudes.size();
		int rsz=roots.size();
		int loop=m.loop;
		int end=m.endframe;
		for(int i=0;i<asz;i++)
		{
			Attitude &a=m.attitudes[i];
			bones[a.link].rotation=Quaternionf::Slerp(
				bones[a.link].rotation,
				a.Interpolate(count,loop,end),ratio);
		}
		for(int i=0;i<rsz;i++)
		{
			bones[roots[i]].offset=
				bones[roots[i]].offset*(1-ratio)+
				m.joints[0].Interpolate(count,loop,end)*(ratio);
		}
	}

	//���݂�bone[i]�̎p�����v�Z����
	int CalcPresentAttitude(int i,Vector3f &coord,Quaternionf &atti)const
	{
		const Bone &b=bones[i];
		if(b.parent==-1)
		{
			coord=b.offset;
			atti=b.rotation;
			return 0;
		}
		CalcPresentAttitude(b.parent,coord,atti);
		coord+=atti.Rotate(b.offset);
		atti*=b.rotation;
		return 0;
	}

	//�{�[���̍����W���A�e�{�[������̑��΍��W�ɕϊ�����
	void MakeSubCoords(int n)
	{
		Bone &b=bones[n];
		size_t nc=b.child.size();
		for(size_t i=0;i<nc;i++)
		{
			int j=b.child[i];
			Bone &c=bones[j];
			if(c.norm==-1)continue;
			MakeSubCoords(j);
			c.offset-=b.offset;
			Quaternionf q=b.rotation.Inv();
			Vector3f v=q.Rotate(c.offset);
			c.offset=v;
			c.rotation=b.rotation.Inv()*c.rotation;
		}
	}

	void DbgBoneTree()
	{
		file f("dbgbonetree.txt","wt");
		//FILE *fp=fopen("dbgbonetree.txt","wt");

		for(size_t i=0;i<bones.size();i++)
		{
			if(bones[i].parent!=-1)
			{
			fprintf(f.get(),"%s(%d):[parent:%s(%d)]\n",
				bones[i].name.c_str(),int(i),
				bones[bones[i].parent].name.c_str(),bones[i].parent
				);
			}else
			{
				fprintf(f.get(),"%s(%d):\n",bones[i].name.c_str(),int(i));
			}
			for(size_t j=0;j<bones[i].child.size();j++)
			{
				fprintf(f.get()," %s(%d)\n",bones[bones[i].child[j]].name.c_str(),bones[i].child[j]);
			}
		}

		fprintf(f.get(),"\ntree\n");

		for(size_t i=0;i<roots.size();i++)
		{
			fprintf(f.get(),"root\n");
			DbgBone(f,roots[i]);
		}
	}

	void DbgBone(file &f,int i)
	{
		fprintf(f.get(),"%s\n",bones[i].name.c_str());

		for(int j=0;j<bones[i].child.size();j++)
		{
			DbgBone(f,bones[i].child[j]);
		}
	}
};

}//end of namespace Rydot

