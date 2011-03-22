
// ring.h
//
// ring
//
// by Ryogo Yoshimura
// mailto:ry@jyoken.net
//
// BSD license

// 有限可換環 Z/nZ っぽいやつ
// 要は自然数nでループしていて+と*で閉じているやつ
//
// 体 F になってほしいときはnは素数じゃないと困るよ。
// あんまり値が大きいと困るよ。
//
// リングバッファとかザ・リングとかロードオブザリングとかじゃないよ。
//
// あんまり理解していないうえに使いどころがない件。

#pragma once

struct Ring
{
public:
	int v;
	int n;
public:
	Ring();
	Ring(Ring &_r):v(_r.v),n(_r.n){}
	Ring(int _v,int _n):v(_v),n(_n){}
	Ring &operator=(const Ring &_r){v=_r.v;n=_r.n;return *this;}
	bool operator==(const Ring &_r){if(v==_r.v&&n==_r.n)return true;return false;}
	bool operator!=(const Ring &_r){if(v==_r.v&&n==_r.n)return false;return true;}
	
	//nが異なる場合は被演算側に合わされる。
	Ring operator+(const Ring &_r){return Ring((v+_r.v)%n,n);}
	Ring operator*(const Ring &_r){return Ring((v*_r.v)%n,n);}
	Ring &operator+=(const Ring &_r){v=(v+_r.v)%n;return *this;}
	Ring &operator*=(const Ring &_r){v=(v*_r.v)%n;return *this;}
	
	//数をそのまま入れると可換環で返ってくる
	Ring operator+(const int _v){return Ring((v+_v)%n,n);}
	Ring operator*(const int _v){return Ring((v*_v)%n,n);}
	Ring &operator+=(const int _v){v=(v+_v)%n;return *this;}
	Ring &operator*=(const int _v){v=(v*_v)%n;return *this;}
};

