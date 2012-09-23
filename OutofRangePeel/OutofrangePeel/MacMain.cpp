/*
 *  MacMain.cpp
 *  OutofrangePeel
 *
 *  Created by ryogo yoshimura on 11/03/08.
 *
 */

#include "MacMain.h"
#include <GLUT/glut.h>
#include <Carbon/Carbon.h>
#include <algorithm>
#include "boost/shared_ptr.hpp"
#include "OutofrangePeel.h"

#include "DefaultEvent.h"

//boost::shared_ptr<IGLEvents> events(new OrangePeel());
boost::shared_ptr<IGLEvents> events(new DefaultEvent());



int core_callback_animate()
{
	if(events)
		events->Animate();
	return 0;
}


int core_callback_disp()
{
	if(events)
		events->Display();
	return 0;
}



int core_callback_mouse(int button, int state, float x, float y)
{
	if(events)
		events->Mouse(MouseEvent(button, state, x, y, 0));
	return 0;
}



int core_callback_mousemove(float x, float y)
{
	if(events)
		events->Mouse(MouseEvent(-1, 1, x, y, 0));
	return 0;
}



int core_callback_mousepassivemove(float x, float y)
{
	if(events)
		events->Mouse(MouseEvent(-1, 0, x, y, 0));
	return 0;
}



int core_callback_mousewheel(int button, float dir, float x, float y)
{
	if(events)
		events->Mouse(MouseEvent(button, -1, x, y, dir));
	return 0;
}



int core_callback_resize(int width, int height)
{
	if(events)
		events->Resize(width, height);
	return 0;
}



int core_initialize()
{
//	glutInit(&argc,argv);
//	glutInitWindowPosition(100, 50);
//	glutInitWindowSize(640, 480);
//	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	
//	glutCreateWindow("Outofrange Peel project");
//	glutDisplayFunc(disp);
//	glutIdleFunc(animate);
//	glutMouseFunc(mouse);
//	glutMotionFunc(mousemove);
//	glutPassiveMotionFunc(mousepassivemove);
//	glutMouseWheelFunc(mousewheel);
//	glutReshapeFunc(resize);
	
	if(events)
		events->Initialize();
	
	return 0;
}





