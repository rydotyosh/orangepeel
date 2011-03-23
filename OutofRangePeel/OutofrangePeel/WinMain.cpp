
#include <stdio.h>
#include <stdlib.h>
#include <gl/freeglut.h>
#include <math.h>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include "timecounter.h"

#include "OutofrangePeel.h"



boost::shared_ptr<IGLEvents> events(new OrangePeel());



Rydot::TimeInterval ti(0.016);



void animate()
{
	if(events)
	{
		ti.update();
		events->Animate();
		glutPostRedisplay();
	}
}



void disp()
{
	if(events)
	{
		events->Display();
		glutSwapBuffers();
	}
}



void mouse(int button, int state, int x, int y)
{
	if(events)
		events->Mouse(MouseEvent(button, state, x, y, 0));
}



void mousemove(int x, int y)
{
	if(events)
		events->Mouse(MouseEvent(-1, 1, x, y, 0));
}



void mousepassivemove(int x, int y)
{
	if(events)
		events->Mouse(MouseEvent(-1, 0, x, y, 0));
}



void mousewheel(int button, int dir, int x, int y)
{
	if(events)
		events->Mouse(MouseEvent(button, -1, x, y, dir));
}



void resize(int width, int height)
{
	if(events)
		events->Resize(width, height);
}



int main(int argc,char **argv)
{
	glutInit(&argc,argv);
	glutInitWindowPosition(100, 50);
	glutInitWindowSize(640, 480);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	glutCreateWindow("Outofrange Peel project");
	glutDisplayFunc(disp);
	glutIdleFunc(animate);
	glutMouseFunc(mouse);
	glutMotionFunc(mousemove);
	glutPassiveMotionFunc(mousepassivemove);
	glutMouseWheelFunc(mousewheel);
	glutReshapeFunc(resize);

	if(events)
		events->Initialize();

	glutMainLoop();
	return 0;
}



int WINAPI WinMain(
	HINSTANCE hInstance, 
	HINSTANCE hPrevInstance, 
	LPSTR lpCmdLine, 
	int nCmdShow
)
{
	return main(0, NULL);
}



