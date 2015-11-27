/*
 * by Simon Jonassen, 2006.
 */
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#if defined _WIN32
# include <windows.h>
# include <gl\gl.h>
# include <gl\glu.h>
# include <gl\glut.h>
#else
# include <GL/gl.h>
# include <GL/glu.h>
# include <GL/glut.h>
#endif

#include "Vector.h"
#include "Object3D.h"

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif
clock_t last_time=0;
Scene s;
int mx, my;
int keyflag=0;
enum {MOVE_F=1,MOVE_B=2,MOVE_R=4,MOVE_L=8,MOVE_U=16,MOVE_D=32,ROLL_CW=64,ROLL_CCW=128};

GLfloat LightAmbient[4] = { 0.3f, 0.3f, 0.3f, 1.0f};
GLfloat LightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f};
GLfloat LightSpecular[4] ={ 1.0f, 1.0f, 1.0f, 1.0f};

void display (void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//set up projections and arrays
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1280.0/800, 0.1, 100000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
	
	//render scene
	glPushMatrix();
	s.Render();
	glPopMatrix();

	// disable old stuff and set up stencil
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
	
	// depth pass:
	glLoadIdentity();
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_POLYGON_BIT|GL_STENCIL_BUFFER_BIT);
	glDisable(GL_LIGHTING);					
	glDepthMask(GL_FALSE);					
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_STENCIL_TEST );				
	glClear(GL_STENCIL_BUFFER_BIT);
	glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
	glStencilFunc(GL_ALWAYS,1,0xFFFFFFFFL);

	//front faces, incr buffer
	glFrontFace(GL_CCW);
	glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
	glPushMatrix();
	s.Shadow();
	glPopMatrix();

	//back faces, decr buffer
	glFrontFace(GL_CW);
	glStencilOp(GL_KEEP,GL_KEEP,GL_DECR);
	glPushMatrix();
	s.Shadow();
	glPopMatrix();

	glFrontFace(GL_CCW );
	//render to colour buffer
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	glColor4f(0.0f,0.0f,0.0f,0.4f);
	glEnable(GL_BLEND );
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glStencilFunc(GL_NOTEQUAL,0,0xFFFFFFFFL);
	glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
	glPushMatrix();
	glLoadIdentity();
	glBegin(GL_QUADS);
		glVertex3f(-0.1f, 0.1f,-0.10f);
		glVertex3f(-0.1f,-0.1f,-0.10f);
		glVertex3f( 0.1f,-0.1f,-0.10f);
		glVertex3f( 0.1f, 0.1f,-0.10f);
	glEnd();
	glPopMatrix();
	glPopAttrib();


	/* xOr method
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	glDepthMask(0);
    glColorMask(0,0,0,0);
	glEnable(GL_STENCIL_TEST);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilFunc(GL_ALWAYS,0,~0u);
	glDisable(GL_CULL_FACE);
	glStencilOp(GL_KEEP,GL_KEEP,GL_INVERT);
   
	//render shadow volumes
	glPushMatrix();
	s.Shadow();
	glPopMatrix();
	
	//set up stencil buffer to reading
    glEnable(GL_CULL_FACE);
	glColorMask(1,1,1,1);
	glStencilFunc(GL_NOTEQUAL,0,~0u);
	glStencilOp(GL_KEEP,GL_KEEP, GL_KEEP);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	 
	//draw shadow image from stencil buffer
	glColor4f(0,0,0,0.5f);
	glBegin(GL_QUADS);
		glVertex3f( -10, 10,-10);
		glVertex3f( -10, -10,-10);
		glVertex3f( 10, -10,-10);
		glVertex3f( 10, 10,-10);
	glEnd();
    
	//reset all
    glDisable(GL_BLEND);
 	glDepthMask(1);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_LIGHTING);
*/
	//swap display buffers   
	glutSwapBuffers();
}


// Idle, do update!
void idle (void) {
	if (clock()-last_time>0.01){
		last_time=clock();
		if ((keyflag & MOVE_F)!=0) s.cam.Move(1.0f,0.0f,0.0f);
		if ((keyflag & MOVE_B)!=0) s.cam.Move(-1.0f,0.0f,0.0f);
		if ((keyflag & MOVE_L)!=0) s.cam.Move(0.0f,1.0f,0.0f);
		if ((keyflag & MOVE_R)!=0) s.cam.Move(0.0f,-1.0f,0.0f);
		if ((keyflag & MOVE_U)!=0) s.cam.Move(0.0f,0.0f,1.0f);
		if ((keyflag & MOVE_D)!=0) s.cam.Move(0.0f,0.0f,-1.0f);
		if ((keyflag & ROLL_CW)!=0) s.cam.Rotate(0.0f,0.0f,-0.01f);
		if ((keyflag & ROLL_CCW)!=0) s.cam.Rotate(0.0f,0.0f,0.01f);
		s.Update();
	}
   glutPostRedisplay();
}



//  MouseDown callback
void mouseF(int button, int state, int x, int y){
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) mx=x; my=y;
}

//  MouseUp callback
void mouseM(int x, int y){
	s.cam.Rotate((y-my)*0.001,-(x-mx)*0.001,0);
	mx=x;
	my=y;
}

//  KeyboardDown callback
void keyboardDown(unsigned char key, int x, int y) {
	switch(key){
		case  'w':
			keyflag|=MOVE_F;
			break;
		case  's':
			keyflag|=MOVE_B;
			break;
		case  'd':
			keyflag|=MOVE_R;
			break;
		case  'a':
			keyflag|=MOVE_L;
			break;
		case  'z':
			keyflag|=MOVE_U;
			break;
		case  'x':
			keyflag|=MOVE_D;
			break;
		case  'q':
			keyflag|=ROLL_CCW;
			break;
		case  'e':
			keyflag|=ROLL_CW;
			break;
		case 'f':
			if (glIsEnabled(GL_FOG))glDisable(GL_FOG);
			else glEnable(GL_FOG);
			break;
		case 'k':
			exit(0);
	}
}
//  KeyboardUp callback
void keyboardUp(unsigned char key, int x, int y) {
	switch(key){
		case  'w':
			keyflag&=~MOVE_F;
			break;
		case  's':
			keyflag&=~MOVE_B;
			break;
		case  'a':
			keyflag&=~MOVE_L;
			break;
		case  'd':
			keyflag&=~MOVE_R;
			break;
		case  'z':
			keyflag&=~MOVE_U;
			break;
		case  'x':
			keyflag&=~MOVE_D;
			break;
		case  'q':
			keyflag&=~ROLL_CCW;
			break;
		case  'e':
			keyflag&=~ROLL_CW;
			break;
	}
}

//Initializing GLUT, lights and scene objects
int main (int argc, char **argv) {
    //GLUT
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(1280, 800);
    glutCreateWindow("acidtrip");
    glutFullScreen();

	//scene and objects
	Terrain *t= new Terrain(15,15,10,300.0f);
    t->position=tVector(-1000,-300,-1000);
    s.objects.push_back(t);


	Torus *tt2 = new Torus(10, 10,50,12);
    tt2->position=tVector(-100,200,-100);
    s.objects.push_back(tt2);

    
	Torus *tt1 = new Torus(1, 50,100,25);
    tt1->position=tVector(0,0,0);
    
	Model3D *cube = new Model3D(20, "cube.obj");
	
	cube->position=tVector(0,0,0);

	Object3DComposite *box= new Object3DComposite();
	box->position=tVector(0,0,0);
	tt1->rot=cube->rot=tVector(0,0,0);

	box->childs.push_back(tt1);
	tt1->parent=box;
	box->childs.push_back(cube);
	cube->parent=box;
	s.objects.push_back(box);


	Model3D *monkey = new Model3D(50, "monkey.obj");
	monkey->position=tVector(100,200,100);
	monkey->visible=false;
	s.objects.push_back(monkey);

    s.cam.rotation=Quaternion(0.7,tVector(0,1,0));
	s.cam.Rotate(-0.03,0,0);
    s.cam.position=tVector(500,500,500);

	//light
    glShadeModel(GL_SMOOTH);
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glLightfv(GL_LIGHT1,GL_AMBIENT,LightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);	
	glLightfv(GL_LIGHT1, GL_SPECULAR,LightSpecular);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	
	glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);

	//callback functions
    glutKeyboardFunc(keyboardDown);
	glutKeyboardUpFunc(keyboardUp);
    glutMouseFunc(mouseF);
    glutMotionFunc(mouseM);
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}
