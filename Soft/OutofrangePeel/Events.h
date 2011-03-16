
#pragma once

class MouseEvent
{
private:
	int button_;
	int state_;
	int x_;
	int y_;
	int dir_;//wheel

public:
	MouseEvent(){}
	MouseEvent(int button, int state, int x, int y, int dir):button_(button),state_(state),x_(x),y_(y),dir_(dir){}

	int Button()const{return button_;}
	int State()const{return state_;}
	int X()const{return x_;}
	int Y()const{return y_;}
	int Dir()const{return dir_;}

	bool IsClicked()const{return button_>=0 && state_>=0;}
	bool IsWheel()const{return state_<0;}
};



class IGLEvents
{
public:
	virtual void Initialize(){}

	virtual void Display(){}
	virtual void Animate(){}

	virtual void Mouse(const MouseEvent &me){}

	virtual void Resize(int width, int height){}
};

