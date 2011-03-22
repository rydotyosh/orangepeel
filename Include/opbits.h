
// opbits.h
//
// operatable bits
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// ビット演算できる多倍長ビット列

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

namespace Rydot
{

//ビット演算できる多倍長ビット列
//文字列的に見たときの表現は、左詰め

//already be implemented:
// and
// or
// not
// xor
// []
// eq
// lt
// toVector
// toString
// cout<<

//is that needed?
// shift
// +,-,*,/


class OpBits
{
private:
	std::vector<unsigned int> bits;
	int n;
public:

	class setter
	{
	private:
		int ii;
		OpBits &bits;
		// private constructor
		// this cannot be accessed except by OpBits
		setter(int i,OpBits &b):ii(i),bits(b){}
	public:
		bool operator =(const bool b){bits.set(ii,b);return b;}
		operator bool()const{return bits[ii];}
		friend OpBits;
	};
	
	OpBits():n(0){}
	
	OpBits(const std::string &s)
	{
		resize(s.size());
		for(int i=0;i<size();i++)
		{
			bool b=(s[i]=='1');
			set(i,b);
		}
	}
	
	clear(){bits.clear();n=0;}
	int size()const{return n;}
	resize(int k){n=k;bits.resize((n-1)/32+1);}
	
	// or
	OpBits operator |(const OpBits &rhs)const
	{
		OpBits o=*this;
		if(rhs.size()>size())
		{
			o.resize(rhs.size());
		}
		int sz=rhs.bits.size();
		for(int i=0;i<sz;i++)
		{
			o.bits[i]|=rhs.bits[i];
		}
		return o;
	}
	OpBits &operator |=(const OpBits &rhs)
	{
		if(rhs.size()>size())
		{
			resize(rhs.size());
		}
		int sz=rhs.bits.size();
		for(int i=0;i<sz;i++)
		{
			bits[i]|=rhs.bits[i];
		}
		return *this;
	}
	
	// and
	OpBits operator &(const OpBits &rhs)const
	{
		OpBits o=*this;
		if(rhs.size()>size())
		{
			o.resize(rhs.size());
		}
		int sz=rhs.bits.size();
		for(int i=0;i<sz;i++)
		{
			o.bits[i]&=rhs.bits[i];
		}
		return o;
	}
	OpBits &operator &=(const OpBits &rhs)
	{
		if(rhs.size()>size())
		{
			resize(rhs.size());
		}
		int sz=rhs.bits.size();
		for(int i=0;i<sz;i++)
		{
			bits[i]&=rhs.bits[i];
		}
		return *this;
	}
	
	// not
	OpBits operator ~()const
	{
		OpBits o=*this;
		int sz=bits.size();
		for(int i=0;i<sz;i++)
		{
			o.bits[i]=~bits[i];
		}
		int sft=size()%32;
		if(sft==0)return o;
		int idx=size()/32;
		int isft=31-sft;
		unsigned int mask=(~0)>>isft;
		o.bits[idx]&=mask;
		return o;
	}
	
	// xor
	OpBits operator ^(const OpBits &rhs)const
	{
		OpBits o=*this;
		if(rhs.size()>size())
		{
			o.resize(rhs.size());
		}
		int sz=rhs.bits.size();
		for(int i=0;i<sz;i++)
		{
			o.bits[i]^=rhs.bits[i];
		}
		return o;
	}
	OpBits &operator ^=(const OpBits &rhs)
	{
		if(rhs.size()>size())
		{
			resize(rhs.size());
		}
		int sz=rhs.bits.size();
		for(int i=0;i<sz;i++)
		{
			bits[i]^=rhs.bits[i];
		}
		return *this;
	}
	
	//shift
	
	//arithmetic


	// match
	bool operator ==(const OpBits &rhs)const
	{
		if(this==&rhs)return true;//identical instance
		if(size()!=rhs.size())return false;
		int idx=size()/32;
		int sft=size()%32;
		for(int i=0;i<idx;i++)
		{
			if(bits[i]!=rhs.bits[i])return false;
		}
		if(sft==0)return true;
		int isft=31-sft;
		unsigned int mask=(~0)>>isft;
		if( (bits[idx]&mask)!=(rhs.bits[idx]&mask) )return false;
		return true;
	}
	bool operator !=(const OpBits &rhs)const
	{
		return !((*this)==rhs);
	}
	bool operator<(const OpBits &rhs)const
	{
		if(this==&rhs)return false;
		if(size()>rhs.size())return false;
		if(size()<rhs.size())return true;
		
		int idx=size()/32;
		int sft=size()%32;
		if(sft)
		{
			int isft=31-sft;
			unsigned int mask=(~0)>>isft;
			unsigned int a=(bits[idx]&mask);
			unsigned int b=(rhs.bits[idx]&mask);
			if( a>b )return false;
			if(a<b)return true;
		}
		for(int i=idx-1;i>=0;i++)
		{
			if(bits[i]>rhs.bits[i])return false;
			if(bits[i]<rhs.bits[i])return true;
		}
		return false;
	}

	// access
	bool at(const int i)const
	{
		int idx=i/32;
		int sft=i%32;
		return (bits[idx]>>sft)&1;
	}
	bool operator [](const int i)const
	{
		return at(i);
	}
	void set(const int i,const bool b)
	{
		int idx=i/32;
		int sft=i%32;
		unsigned int bb=b?1:0;
		unsigned int nb=b?0:1;
		bits[idx]|=bb<<sft;
		bits[idx]&=~((nb)<<sft);
	}
	setter at(const int i)
	{
		return setter(i,*this);
	}
	setter operator[](const int i)
	{
		return at(i);
	}

	// transform
	template <class T> std::vector<T> &toVector(std::vector<T> &v)const
	{
		int sz=size();
		v.resize(sz);
		for(int i=0;i<sz;i++)
		{
			v[i]=at(i);
		}
		return v;
	}
	template <class T>operator std::vector<T> ()const
	{
		std::vector<T> v;
		return toVector(v);
	}
	std::string &toString(std::string &s)const
	{
		int sz=size();
		s.resize(sz);
		for(int i=0;i<sz;i++)
		{
			s[i]=at(i)?'1':'0';
		}
		return s;
	}
	operator std::string ()const
	{
		std::string s;
		return toString(s);
	}

};

std::ostream &operator <<(std::ostream &o,const OpBits &b)
{
	for(int i=0;i<b.size();i++)
	{
		o<<(b[i]?"1":"0");
	}
	return o;
}

};//end of namespace Rydot

