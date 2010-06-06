#include <iostream>
#include "opeswf.h"
#include "main.h"

void display();

SWF* pSwf = new SWF;
SWF_HEADER header;

unsigned int main(int argc, char** argv)
{
	pSwf->LoadSWF("lol.swf");
	
	header = pSwf->GetHeader();
	
	glutInit(&argc, argv);
	glutInitWindowSize(header.rect->Xmax/20, header.rect->Ymax/20);
	glutCreateWindow("OpenSWF");
	glutDisplayFunc(&display); 
	
	glutMainLoop();
	
	delete pSwf;
	
	return 0;
}

void display()
{
	SWF_RGB backgroundColor = pSwf->GetBackgroundColor();
	glClearColor(backgroundColor.red, backgroundColor.green, backgroundColor.blue, 255);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0,0,header.rect->Xmax/20, header.rect->Ymax/20);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, header.rect->Xmax/20, 0, header.rect->Ymax/20);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	pSwf->Display();
	glFlush();
}
