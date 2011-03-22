
// sound.h
//
// sound
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

//�Z�ʂ̗����Ȃ����OpenAL�g���B
//Waveform�̊Ǘ����ǂ����ł���

#pragma once

#include <stdlib.h>
#include <AL/alut.h>

#include <stdio.h>
#include <math.h>
#include <vector>
#include <string>
#include <set>

#include "cthread.h"
#include "timecounter.h"
//#include <proxy.h>
#include "matrix.h"

#include <boost/smart_ptr.hpp>

#pragma comment(lib, "alut.lib")
#pragma comment(lib, "OpenAL32.lib")

namespace Rydot
{

enum LoopState
{
	LOOP_REPEAT=1,
	LOOP_NOREPEAT=0
};

struct WH
{
	unsigned char Header[4];
	long Size;
};
struct WF
{
	short FormatTag;
	short Channels;
	long SamplesPerSec;
	long AvgBytesPerSec;
	short BlockAlign;
	short BitsPerSample;
};

struct WFH//WaveFileHeader
{
	WH hRiff;
	BYTE hWave[4];
	WH hFmt;
	WF Format;
	WH hData;
};

class BinaryFileReader
{
private:
	FILE *fp;
public:
	void Open(std::string &fn)
	{
		Close();
		fp=fopen(fn.c_str(),"rb");
	}
	BinaryFileReader():fp(NULL){}
	~BinaryFileReader()
	{
		Close();
	}
	void Close()
	{
		if(fp)
		fclose(fp);
		fp=NULL;
	}
	size_t Read(std::vector<unsigned char> &r)
	{
		if(!fp)return 0;
		return fread(&r[0],1,r.size(),fp);
	}
	size_t Read(unsigned char *dat,int sz)
	{
		if(!fp)return 0;
		return fread(dat,1,sz,fp);
	}
	int Eof()
	{
		if(!fp)return 1;
		return feof(fp);
	}
	int Seek(int offset,int origin)
	{
		if(!fp)return 1;
		return fseek(fp,offset,origin);
	}
};

class IBuffer;
class Source;


//�g�`��ێ�����N���X

//�o�b�t�@�N���X�̃C���^�t�F�C�X
//���̉������Ⴞ�߂�
class IBuffer
{
public:
	IBuffer(){}
	virtual ~IBuffer(){}

	virtual int LoadFile(const std::string &fn)=0;
	virtual std::string GetName()=0;
	virtual bool IsStream()=0;
	//virtual void Restart()=0;
};

//���ʂ̃o�b�t�@
class Buffer:public IBuffer
{
private:
	ALuint m_buffer;
	std::string m_fn;

public:
	Buffer():m_buffer(0)
	{
	}
	~Buffer()
	{
		Release();
	}
	
	int LoadFile(const std::string &fn)
	{
		Release();
		m_fn=fn;
		
		m_buffer = alutCreateBufferFromFile(m_fn.c_str());
		return 1;
	}

	int SetWaveform16(const std::string &name,std::vector<short> wave,bool stereo,int freq)
	{
		Release();
		m_fn=name;

		alGenBuffers(1,&m_buffer);
		if(stereo)
			alBufferData(m_buffer,AL_FORMAT_STEREO16,&wave[0],wave.size()>>1,freq);
		else
			alBufferData(m_buffer,AL_FORMAT_MONO16,&wave[0],wave.size(),freq);

		return 1;
	}

	ALuint Get(){return m_buffer;}

	std::string GetName(){return m_fn;}
	bool IsStream(){return false;}

private:
	void Release()
	{
		if(m_buffer==0)return;
		alDeleteBuffers(1,&m_buffer);
		m_buffer=0;
	}	

};

//�X�g���[���p�̃o�b�t�@����
class StreamBufferInternal
{
private:
	std::vector<ALuint> m_buffer;
	std::string m_fn;
	unsigned int m_bufferread;
	unsigned int m_bufferwrite;
	
	BinaryFileReader m_file;
	ALenum m_format;
	ALsizei m_freq;
	
	int m_queuesize;
	int m_buffersize;

	criticalsection cs;

public:
	StreamBufferInternal():m_bufferread(0),m_bufferwrite(0),m_queuesize(16),m_buffersize(8192)
	{
	}
	~StreamBufferInternal()
	{
		Release();
	}

	//���[�h����O�ɂ���Ăق���
	void SetQueuesize(int q){m_queuesize=q;}
	void SetBuffersize(int b){m_buffersize=b;}
	
	int LoadFile(const std::string &fn)
	{
		Release();
		m_fn=fn;
		
		InitStream();

		return 1;
	}

	//�Đ��L���[�ɓ����o�b�t�@
	ALuint Queue()
	{
		synchronized sync(cs);

		int r=m_buffer[m_bufferread];
		if( (m_bufferread+1)%m_buffer.size()==m_bufferwrite )
		{
			return 0;
		}
		else
		{
			++m_bufferread;
			m_bufferread%=m_buffer.size();
			return r;
		}
	}

	//�t�@�C������ǂݍ��ރo�b�t�@
	ALuint Unqueue()
	{
		return m_buffer[m_bufferwrite];
	}
	
	//�Đ��I���o�b�t�@������������
	//���[�v�������炨�K�ɓ�������
	int Reflesh(int loop)
	{
		synchronized sync(cs);

		if(!ReadStream(loop))
		{
			return 0;
		}
		return 1;
	}

	void Top()
	{
		m_file.Seek(sizeof(WFH),SEEK_SET);
	}

	void Restart()
	{
		synchronized sync(cs);

		Top();
		m_bufferwrite=0;
		m_bufferread=0;

		while(ReadStream(0));
	}

	std::string GetName()
	{
		return m_fn;
	}
	
private:

	void Release()
	{
		if(m_buffer.empty())return;
		alDeleteBuffers(m_buffer.size(),&m_buffer[0]);
		m_buffer.clear();
	}

	void InitStream()
	{
		m_buffer.resize(m_queuesize);
		alGenBuffers(m_queuesize,&m_buffer[0]);
		
		m_bufferwrite=0;
		m_bufferread=0;
		
		m_file.Open(m_fn);
		
		WFH w;
		unsigned long r=m_file.Read((unsigned char*)&w,sizeof(WFH));
		
		if(memcmp(w.hRiff.Header,"RIFF",4))
		{m_file.Close();return;}
		if(memcmp(w.hFmt.Header,"fmt ",4))
		{m_file.Close();return;}
		if(memcmp(w.hData.Header,"data",4))
		{m_file.Close();return;}
		
		if((w.Format.Channels==1)&&(w.Format.BitsPerSample==8))
			m_format=AL_FORMAT_MONO8;
		else if((w.Format.Channels==2)&&(w.Format.BitsPerSample==8))
			m_format=AL_FORMAT_STEREO8;
		else if((w.Format.Channels==1)&&(w.Format.BitsPerSample==16))
			m_format=AL_FORMAT_MONO16;
		else if((w.Format.Channels==2)&&(w.Format.BitsPerSample==16))
			m_format=AL_FORMAT_STEREO16;
		
		m_freq=w.Format.SamplesPerSec;
		
		for(int i=0;i<m_queuesize;++i)
		{
			ReadStream(0);
		}
	}
	
	int ReadStream(int loop)
	{
		if((m_bufferwrite+1)%m_queuesize==m_bufferread)return 0;

		std::vector<unsigned char> dat(m_buffersize);
		int sz=m_file.Read(dat);

		if(loop)
		{
			if(sz<m_buffersize)
			{
				Top();
				int dd=m_buffersize-sz;
				m_file.Read(&dat[sz],dd);
			}
		}
		else
		{
			if(dat.empty())return 0;
			if(m_file.Eof())return 0;
		}
		
		alBufferData(m_buffer[m_bufferwrite],m_format,&dat[0],dat.size(),m_freq);
		
		++m_bufferwrite;
		m_bufferwrite%=m_buffer.size();
		return 1;
	}
};

//�X�g���[���Đ��p�o�b�t�@
class Stream:public IBuffer
{
private:
	std::string m_fn;
	
	int m_queuesize;
	int m_buffersize;

public:
	Stream():m_queuesize(16),m_buffersize(8192)
	{
	}
	~Stream()
	{
	}

	//���[�h����O�ɂ���Ăق���
	void SetQueuesize(int q){m_queuesize=q;}
	void SetBuffersize(int b){m_buffersize=b;}
	
	//�t�@�C�������[�h�Ƃ������t�@�C������ݒ�Ƃ�����
	int LoadFile(const std::string &fn)
	{
		m_fn=fn;
		return 1;
	}

	std::string GetName(){return m_fn;}
	bool IsStream(){return true;}

	boost::shared_ptr<StreamBufferInternal> CreateInternal()
	{
		boost::shared_ptr<StreamBufferInternal> p(new StreamBufferInternal());
		p->SetQueuesize(m_queuesize);
		p->SetBuffersize(m_buffersize);
		p->LoadFile(m_fn);
		return p;
	}

};

//�X�g���[���̃L���[�C���O���s���X���b�h
class StreamHandler:public thread
{
private:
	StreamBufferInternal &buffer;
	ALuint source;
	int count;
	int loop;
	int finish;
public:
	StreamHandler(StreamBufferInternal &buf,ALuint src,int lp):buffer(buf),source(src),count(0),loop(lp),finish(0){}
	
	int run()
	{
		int end=0;
		TimeInterval ti(0.002);
		while(isvalid())
		{
			ALint queue;
			alGetSourceiv(source,AL_BUFFERS_QUEUED,&queue);
			if(queue==0)break;

			ALint processed;
			alGetSourceiv(source,AL_BUFFERS_PROCESSED,&processed);
			
			while(processed>0)
			{
				ALuint w=buffer.Unqueue();

				alSourceUnqueueBuffers(source,1,&w);

				if(!buffer.Reflesh(loop))
				{
					end=1;
				}
				count++;

				ALuint r=buffer.Queue();
				if(r==0)break;

				if(!end)
				{
					alSourceQueueBuffers(source,1,&r);
				}

				--processed;
			}
			
			ti.update();
			
		}
		finish=1;
		return 0;
	}
};

//�����N���X
//�X�s�[�J�[�Ƃ��v���C���̂悤�Ȃ���
//����͂����AL�܂���(AL�̒P�Ȃ郉�b�v)
class Source
{
private:
	ALuint m_source;
	LoopState m_loop;
	IBuffer *m_buf;
	boost::shared_ptr<StreamBufferInternal> m_bufinternal;
	StreamHandler *m_sh;
public:
	Source():m_source(0),m_loop(LOOP_NOREPEAT),m_buf(NULL),m_sh(NULL)
	{
		Create();
	}
	Source(const Source &src):m_source(0),m_loop(LOOP_NOREPEAT),m_buf(NULL),m_sh(NULL)
	{
		Copy(src);
	}
	Source &operator=(const Source &src)
	{
		if(&src==this)return *this;

		//�X�g���[���n���h�������ɑ���
		if(m_sh)
		{
			m_sh->invalidate();//������
			m_sh=NULL;
		}

		Copy(src);

		return *this;
	}

	~Source()
	{
		Release();
	}

	//src����R�s�[����
	void Copy(const Source &src)
	{
		Create();
		m_loop=src.m_loop;
		m_buf=src.m_buf;
	}
	
	void Create()
	{
		if(alIsSource(m_source))return;
		
		alGenSources(1,&m_source);
	}
	
	void Release()
	{
		if(alIsSource(m_source))
			alDeleteSources(1,&m_source);
		
		m_source=0;

		if(m_sh)
		{
			m_sh->invalidate();
			m_sh=NULL;
		}
	}
	
	//�o�b�t�@���w�肷��
	void AttachBuffer(IBuffer *buf)
	{
		m_buf=buf;
		if(!m_buf->IsStream())
			alSourcei(m_source,AL_BUFFER,((Buffer*)buf)->Get());
	}
	
	//�Đ�����
	//void Play(LoopState loop=LOOP_NOREPEAT)
	void Play()
	{
		//�o�b�t�@���w�肳��ĂȂ�
		if(!m_buf)return;

		//�Đ����������牽�����Ȃ�
		if(IsPlaying())return;

		//SetLoop(loop);
		if(m_buf->IsStream())
		{
			alSourcei(m_source,AL_LOOPING,AL_FALSE);

			//�X�g���[���n���h�������ɑ���
			if(m_sh)
			{
				m_sh->invalidate();//������
				m_sh=NULL;
			}

			if(m_bufinternal)
			{
				if(m_bufinternal->GetName()!=m_buf->GetName())
				{
					m_bufinternal=((Stream*)m_buf)->CreateInternal();
				}
			}
			else
			{
				m_bufinternal=((Stream*)m_buf)->CreateInternal();
			}

			while(1)
			{
				ALuint b=m_bufinternal->Queue();
				if(b==0)break;
				
				alSourceQueueBuffers(m_source,1,&b);
			}

			m_sh=new StreamHandler(*m_bufinternal,m_source,m_loop);
			m_sh->start();
		}
		else
		{
			if(m_loop==LOOP_REPEAT)
				alSourcei(m_source,AL_LOOPING,AL_TRUE);
			else
				alSourcei(m_source,AL_LOOPING,AL_FALSE);
		}
		alSourcePlay(m_source);
	}
	
	//�o�b�t�@���w�肵�čĐ�����
	/*void Play(Buffer &buf,LoopState loop=LOOP_NOREPEAT)
	{
		AttachBuffer(buf);
		Play(loop);
	}
	void Play(Stream &buf,LoopState loop=LOOP_NOREPEAT)
	{
		AttachBuffer(buf);
		Play(loop);
	}*/
	
	void Pause(){alSourcePause(m_source);}
	void Stop()
	{
		alSourceStop(m_source);
		if(m_sh)
		{
			m_sh->invalidate();
			m_sh=NULL;
		}
		FlushQueue();
		if(m_bufinternal)
		{
			m_bufinternal->Restart();
		}
	}
	void FlushQueue()
	{
		while(1)
		{
			ALuint q=0;
			alSourceUnqueueBuffers(m_source,1,&q);
			if(q==0)break;
		}
	}
	
	//����������
	void Rewind(){alSourceRewind(m_source);}
	
	void SetPitch(float pitch){alSourcef(m_source,AL_PITCH,pitch);}
	void SetGain(float gain)  {alSourcef(m_source,AL_GAIN,gain);}
	void SetCone(float inner,float outer,float ogain)
	{
		alSourcef(m_source,AL_CONE_INNER_ANGLE,inner);
		alSourcef(m_source,AL_CONE_OUTER_ANGLE,outer);
		alSourcef(m_source,AL_CONE_OUTER_GAIN,ogain);
	}
	void SetPosition(Vector3f &pos){alSourcefv(m_source,AL_POSITION,(float*)&pos);}
	void SetVelocity(Vector3f &vel){alSourcefv(m_source,AL_VELOCITY,(float*)&vel);}
	void SetDirection(Vector3f &dr){alSourcefv(m_source,AL_DIRECTION,(float*)&dr);}

	void SetLoop(LoopState loop)
	{
		m_loop=loop;
	}

	bool IsPlaying()
	{
		ALint p;
		alGetSourcei(m_source,AL_SOURCE_STATE,&p);

		if(p==AL_PLAYING)
			return true;

		return false;
	}

	IBuffer *GetBuffer(){return m_buf;}
};

typedef boost::shared_ptr<Source> Sound;

//�T�E���h�����Ɠ���I�Ɉ������
//�g����
//1.�t�@�C����,�X�g���[��or�o�b�t�@,���[�v���w��
//2.�g�`,�t�H�[�}�b�g,�T���v�����O���g��,���[�v���w��
//�n���h�������炦�邩��Đ��ł���~�ł��Ȃ�ł�
//�n���h���ǂ�������Ă����O������΂Ȃ�Ƃ�
class SoundManager
{
private:
	std::vector< Sound > m_sources;
	std::vector< IBuffer* > m_buffers;
public:

	SoundManager()
	{
	}
	~SoundManager()
	{
		Release();
	}

	//fn:�t�@�C����
	//stream:�X�g���[���Ȃ�true
	//loop:���[�v����Ȃ�LOOP_REPEAT,���Ȃ��Ȃ�LOOP_NOREPEAT
	//return:�V����source
	Sound Load(const std::string &fn,bool stream=false,LoopState loop=LOOP_NOREPEAT)
	{
		Sound s(new Source);
		s->SetLoop(loop);
		m_sources.push_back(s);

		for(int i=0;i<m_buffers.size();++i)
		{
			//���łɑ���
			if(m_buffers[i]->GetName()==fn)
			{
				s->AttachBuffer(m_buffers[i]);
				return s;
			}
		}

		IBuffer *b;

		if(stream)
		{
			b=new Stream;
		}
		else
		{
			b=new Buffer;
		}

		b->LoadFile(fn);
		s->AttachBuffer(b);
		m_buffers.push_back(b);

		return s;
	}
	//name:
	//wave:�g�`
	//stereo:
	//freq:�T���v�����O���g��
	//loop:
	//return:�V����source
	Sound Wave(const std::string &name,std::vector<short> &wave,bool stereo,int freq=44100,LoopState loop=LOOP_NOREPEAT)
	{
		Sound s(new Source);
		s->SetLoop(loop);
		m_sources.push_back(s);

		for(int i=0;i<m_buffers.size();++i)
		{
			//���łɑ���
			if(m_buffers[i]->GetName()==name)
			{
				s->AttachBuffer(m_buffers[i]);
				return s;
			}
		}

		Buffer *b=new Buffer;

		b->SetWaveform16(name,wave,stereo,freq);
		s->AttachBuffer(b);
		m_buffers.push_back(b);

		return s;
	}

	//����buffer������source���R�s�[����
	//return:�V����source
	Sound Copy(const Sound &snd)
	{
		Sound s(new Source);
		m_sources.push_back(s);	

		s->Copy(*snd);
		return s;
	}

	//�g���ĂȂ�source�Ǝg���ĂȂ�buffer������
	//�n���h�����ێ�����ĂȂ��āA�Đ�������ĂȂ���Ύg���ĂȂ��Ƃ݂Ȃ��B
	int Reduce()
	{
		std::set<IBuffer*> usedbuffer;

		for(std::vector<Sound>::iterator i=m_sources.begin();i!=m_sources.end();)
		{
			if(!(*i).unique())//�ǂ����Ŏg���Ă���
			{
				usedbuffer.insert((*i)->GetBuffer());
			}
			else
			{
				if(!(*i)->IsPlaying())//�Đ�������Ȃ�
				{
					i=m_sources.erase(i);
					continue;
				}
			}
			++i;
		}

		for(std::vector<IBuffer*>::iterator i=m_buffers.begin();i!=m_buffers.end();)
		{
			if(usedbuffer.find(*i)==usedbuffer.end())
			{
				delete (*i);
				i=m_buffers.erase(i);
				continue;
			}
			++i;
		}

		return 1;
	}

	//�����I�ɑS������
	int Release()
	{
		m_sources.clear();

		for(int i=0;i<m_buffers.size();++i)
		{
			delete m_buffers[i];
			m_buffers[i]=NULL;
		}
		m_buffers.clear();

		return 1;
	}

	//�Đ����Ă���\�[�X�������true
	bool IsPlaying()
	{
		for(std::vector<Sound>::iterator i=m_sources.begin();i!=m_sources.end();)
		{
			if((*i)->IsPlaying())return true;
			++i;
		}
		return false;
	}

	//
	void Play(const std::string &name)
	{
		for(int i=0;i<m_buffers.size();++i)
		{
			if(m_buffers[i]->GetName()==name)
			{
				std::cout<<"found"<<std::endl;
				Sound s(new Source);
				m_sources.push_back(s);	
				s->AttachBuffer(m_buffers[i]);
				s->Play();
				return;
			}
		}
	}
	
};




}//end of namespace Rydot

