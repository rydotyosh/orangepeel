
#pragma once

#include "matrix.h"
#include <iostream>



class View
{
	typedef float Float;
	typedef Rydot::Rect2f	Rect2;
	typedef Rydot::Matrix44f	Matrix44;
	typedef Rydot::Vector2f	Vector2;
	typedef Rydot::Vector3f	Vector3;
public:
	View()
	{
	}



	// rectangle of view in pixel
	bool SetView(const Rect2 &view)
	{
		view_=view;
		return true;
	}



	// rectangle of view in pixel
	// angle in degree
	bool SetView(const Rect2 &view, Float angle)
	{
		view_=view;
		angle_=angle;
		return true;
	}



	Rect2 GetView(){return view_;}



	bool SetCamera(const Vector3 &eye, const Vector3 &at, const Vector3 &sky)
	{
		matrix_.LookAt(eye, at, sky);
		eye_=eye;
		return true;
	}



	bool Apply()
	{
		glViewport(
			GLint(view_.p1.x),
			GLint(view_.p1.y),
			GLint(view_.p2.x),
			GLint(view_.p2.y));

		glMatrixMode(GL_PROJECTION);
		Vector2 diag=view_.Diagonal();
		Matrix44 pers;
		pers.Perspective(
			angle_,
			diag.x/diag.y,
			0.1f,
			100.f);
		glLoadMatrixf((float*)&pers);

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf((float*)&matrix_);
		return true;
	}



	// returns [Original, Direction]
	std::pair<Vector3, Vector3> Ray(const Vector2 &screen)const
	{
		const double pi = atan(1.0)*4;
		double ratio = tan(pi/180.0*angle_*0.5)*2;
		const Vector2 diag = view_.Diagonal();
		const Vector3 v(
			float((screen.x - diag.x*0.5)/diag.y *ratio),
			float(-(screen.y - diag.y*0.5)/diag.y *ratio),
			-1.0f);

		//std::cerr<<screen.x<<"\t"<<screen.y<<"\n";

		return std::make_pair(eye_, matrix_.MultVector(v));
	}



private:
	Rect2 view_;
	Float angle_;
	Vector3 eye_;
	Matrix44 matrix_;
};


