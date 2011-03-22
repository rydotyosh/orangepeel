
// filter1d.h
//
// filter for 1d data
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// 音声用のデジタルフィルタっぽいやつ
// FIRが微妙に未実装...

#pragma once

#include <math.h>
#include <complex>

namespace Rydot
{

static double pi=3.14159265358979;

class Filter1
{
public:
	Filter1(){}
	virtual ~Filter1(){}
	
	virtual void Reset(){}
	
	virtual int Operate(int n,float *in,float *out){return 0;}
	
	virtual void GetFreqChara(int n,float *out,float at){}
};

// 2nd Order IIR Filter
class IIR2:public Filter1
{
private:
	// sampling frequency
	int m_fs;
	// filter coefficents
	float m_a[3],m_b[3];
	// previous delay line
	float m_x[3],m_y[3];
public:
	IIR2()
	{
		SetFs(44100);
		Reset();
	}
	
	//サンプリング周波数をセット
	void SetFs(int fs)
	{
		m_fs=fs;
	}
	int GetFs(){return m_fs;}
	
	//入出力履歴をリセット
	void Reset()
	{
		m_x[0]=0;m_x[1]=0;m_x[2]=0;
		m_y[0]=0;m_y[1]=0;m_y[2]=0;
	}
	
	//Operator
	int Operate(int n,float *in,float *out)
	{
		//Op PreviousDelayLine
		out[0]=
			(m_b[0]/m_a[0])*in[0]+
			(m_b[1]/m_a[0])*m_x[1]+
			(m_b[2]/m_a[0])*m_x[0]-
			(m_a[1]/m_a[0])*m_y[1]-
			(m_a[2]/m_a[0])*m_y[0];
		out[1]=
			(m_b[0]/m_a[0])*in[1]+
			(m_b[1]/m_a[0])*in[0]+
			(m_b[2]/m_a[0])*m_x[1]-
			(m_a[1]/m_a[0])*out[0]-
			(m_a[2]/m_a[0])*m_y[1];

		//Op Main
		int i;
		for(i=2;i<n;i++)
		{
			out[i]=
			(m_b[0]/m_a[0])*in[i]+
			(m_b[1]/m_a[0])*in[i-1]+
			(m_b[2]/m_a[0])*in[i-2]-
			(m_a[1]/m_a[0])*out[i-1]-
			(m_a[2]/m_a[0])*out[i-2];
		}

		//SaveDelay
		m_x[0]=in[n-2];
		m_x[1]=in[n-1];
		m_y[0]=out[n-2];
		m_y[1]=out[n-1];

		return n;
	}

	float *GetA(){return m_a;}
	float *GetB(){return m_b;}

	//周波数特性を求める
	void GetFreqChara(int n,float *out,float at)
	{
		typedef std::complex<double> cx;
		double m=(double)n;

		for(int i=0;i<n;i++)
		{
			double j=pow(10,4.6f*(1-i/m));
			double omega=2.0f*pi*(at/m_fs);
			cx w(0,omega/j);
			cx w2(0,2.0f*omega/j);
			cx z1=std::exp(w);
			cx z2=std::exp(w2);
			out[i]=
				(float)sqrt(std::norm(
					((double)m_b[0]+(double)m_b[1]*z1+(double)m_b[2]*z2)/
					((double)m_a[0]+(double)m_a[1]*z1+(double)m_a[2]*z2)));
		}
	}
	
	
	//諸特性を示すフィルタを簡単に作成


	//Peaking
	//freq付近をgainだけ増減
	void CalcMP(float freq,float gain,float edg=1.0f/1.414213f)
	{
		float omega=2.0f*pi*(freq/m_fs);
		float s=(float)sin(omega);
		float c=(float)cos(omega);
		float fQ=edg;
		float A=(float)pow(10.0,gain/40.0);
		float alpha=s/(2.0f*fQ);

		m_b[0]=( 1.0f+alpha*A);
		m_b[1]=(-2.0f*c      );
		m_b[2]=( 1.0f-alpha*A);
		m_a[0]=( 1.0f+alpha/A);
		m_a[1]=(-2.0f*c      );
		m_a[2]=( 1.0f-alpha/A);
	}
	
	//HighShelf
	//freq以上をgainだけ増減
	void CalcHS(float freq,float gain,float edg=1.0f/1.414213f)
	{
		float omega=2.0f*pi*(freq/m_fs);
		float s=(float)sin(omega);
		float c=(float)cos(omega);
		float fQ=edg;
		float A=(float)pow(10.0,gain/40.0);
		float alpha=s/(2.0f*fQ);
		float beta=(float)sqrt(A/fQ);

		m_b[0]=(      A*((A+1.0f)+(A-1.0f)*c+beta*s));
		m_b[1]=(-2.0f*A*((A-1.0f)+(A+1.0f)*c)       );
		m_b[2]=(      A*((A+1.0f)+(A-1.0f)*c-beta*s));
		m_a[0]=(         (A+1.0f)-(A-1.0f)*c+beta*s );
		m_a[1]=(   2.0f*((A-1.0f)-(A+1.0f)*c)       );
		m_a[2]=(         (A+1.0f)-(A-1.0f)*c-beta*s );
	}
	
	//LowShelf
	//freq以下をgainだけ増減
	void CalcLS(float freq,float gain,float edg=1.0f/1.414213f)
	{
		float omega=2.0f*pi*(freq/m_fs);
		float s=(float)sin(omega);
		float c=(float)cos(omega);
		float fQ=edg;
		float A=(float)pow(10.0,gain/40.0);
		float alpha=s/(2.0f*fQ);
		float beta=(float)sqrt(A/fQ);

		m_b[0]=(     A*((A+1.0f)-(A-1.0f)*c+beta*s));
		m_b[1]=(2.0f*A*((A-1.0f)-(A+1.0f)*c)       );
		m_b[2]=(     A*((A+1.0f)-(A-1.0f)*c-beta*s));
		m_a[0]=(        (A+1.0f)+(A-1.0f)*c+beta*s );
		m_a[1]=( -2.0f*((A-1.0f)+(A+1.0f)*c)       );
		m_a[2]=(        (A+1.0f)+(A-1.0f)*c-beta*s );
	}
	
	
	//LowPassFilter
	void CalcLPF(float freq,float edg)
	{
		float omega=2.0f*pi*(freq/m_fs);
		float s=(float)sin(omega);
		float c=(float)cos(omega);
		float fQ=edg;
		float alpha=s/(2.0f*fQ);

		m_b[0]=(1.0f-c)/2.0f;
		m_b[1]= 1.0f-c      ;
		m_b[2]=(1.0f-c)/2.0f;
		m_a[0]= 1.0f+alpha;
		m_a[1]=-2.0f*c    ;
		m_a[2]= 1.0f-alpha;
	}
	
	//HighPassFilter
	void CalcHPF(float freq,float edg)
	{
		float omega=2.0f*pi*(freq/m_fs);
		float s=(float)sin(omega);
		float c=(float)cos(omega);
		float fQ=edg;
		float alpha=s/(2.0f*fQ);

		m_b[0]= (1.0f+c)/2.0f;
		m_b[1]=-(1.0f+c)    ;
		m_b[2]= (1.0f+c)/2.0f;
		m_a[0]=  1.0f+alpha;
		m_a[1]= -2.0f*c    ;
		m_a[2]=  1.0f-alpha;
	}
	
	
	//BandPassFilter
	void CalcBPF(float freq,float band)
	{
		float omega=2.0f*pi*(freq/m_fs);
		float s=(float)sin(omega);
		float c=(float)cos(omega);
		float alpha=s*sinh(log(2.0f)/2.0f*band*omega/s);

		m_b[0]= alpha;
		m_b[1]= 0;
		m_b[2]=-alpha;
		m_a[0]= 1.0f+alpha;
		m_a[1]=-2.0f*c;
		m_a[2]= 1.0f-alpha;
	}
	
	//Notch
	void CalcNotch(float freq,float edg)
	{
		float omega=2.0f*pi*(freq/m_fs);
		float s=(float)sin(omega);
		float c=(float)cos(omega);
		float fQ=edg;
		float alpha=s/(2.0f*fQ);

		m_b[0]= 1.0f;
		m_b[1]=-2.0f*c;
		m_b[2]= 1.0f;
		m_a[0]= 1.0f+alpha;
		m_a[1]=-2.0f*c;
		m_a[2]= 1.0f-alpha;
	}
};

// IIR filter
class IIR:Filter1
{
private:
	float *m_a,*m_b,*m_x,*m_y;
	int m_n;
	int m_fs;
public:
	IIR()
	:m_a(0),m_b(0),m_x(0),m_y(0)
	{
		m_n=0;
		SetFs(44100);
	}
	
	IIR(IIR2 &iir2)
	:m_a(0),m_b(0),m_x(0),m_y(0)
	{
		m_n=0;
		SetFs(iir2.GetFs());
		SetCoef(2,iir2.GetA(),iir2.GetB());
	}
	
	~IIR()
	{
		if(m_a)
		{
			delete m_a;
			delete m_b;
			delete m_x;
			delete m_y;
		}
	}
	
	//サンプリング周波数をセット
	void SetFs(int fs)
	{
		m_fs=fs;
	}
	int GetFs(){return m_fs;}
	
	void Reset()
	{
		for(int i=0;i<m_n;i++)
		{
			m_x[i]=0;
			m_y[i]=0;
		}
	}
	
	//係数を設定する
	// n:Order
	// a:a[n+1] deno
	// b:b[n+1] num
	void SetCoef(int n,const float *a,const float *b)
	{
		if(m_n!=0 && m_n!=n)
		{
			delete m_a;
			delete m_b;
			delete m_x;
			delete m_y;
		}
		
		m_n=n;
		m_a=new float[n+1];
		m_b=new float[n+1];
		m_x=new float[n];
		m_y=new float[n];

		for(int i=0;i<m_n+1;i++)
		{
			m_a[i]=a[i];
			m_b[i]=b[i];
		}

		Reset();
	}

	int Operate(int n,float *in,float *out)
	{
		float sum;
		for(int i=0;i<m_n;i++)
		{
			sum=m_b[0]*in[i];
			for(int j=0;j<i;j++)
			{
				sum+=m_b[j+1]*in[i-j];
				sum-=m_a[j+1]*out[i-j];
			}
			for(int j=i;j<m_n;j++)
			{
				sum+=m_b[j+1]*m_x[m_n-j-1];
				sum-=m_a[j+1]*m_y[m_n-j-1];
			}
			out[i]=sum/m_a[0];
		}

		for(int i=m_n;i<n;i++)
		{
			sum=m_b[0]*in[i];
			for(int j=0;j<m_n;j++)
			{
				sum+=m_b[j+1]*in[i-j-1];
				sum-=m_a[j+1]*out[i-j-1];
			}
			out[i]=sum/m_a[0];
		}

		for(int i=0;i<m_n;i++)
		{
			m_x[i]=in[n-m_n+i];
			m_y[i]=out[n-m_n+i];
		}

		return n;
	}
	
	//Frequency Characteristic
	void GetFreqChara(int n,float *out,float at)
	{
		typedef std::complex<double> cx;
		double m=(double)n;

		for(int i=0;i<n;i++)
		{
			double j=pow(10,4.6f*(1-i/m));
			double omega=2.0f*pi*(at/m_fs);
			cx x((double)m_b[0],0),y((double)m_a[0],0);
			for(int n=1;n<m_n;n++)
			{
				cx z=std::exp(cx(0,n*omega/j));
				x+=z*(double)m_b[n];
				y+=z*(double)m_a[n];
			}
			out[i]=(float)sqrt(std::norm(x/y));
		}
	}
};

// FIR filter
class FIR
{
private:
	float *m_a,*m_x;
	int m_n;
public:
	FIR();
	void Reset();// zero x
	void SetCoef(int n,const float *a);

	int Operate(const float *pIn,int nInSpl,float *pOut);

	//Frequency Characteristic
	void GetFreqChara(int N,float *pOut);
};

}

