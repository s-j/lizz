/*
 * by Simon Jonassen, 2006.
 */
#ifndef O3D_HXR
#define O3D_HXR

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
#include "Quaternion.h"
#include <stdio.h>
#include <vector>
using std::vector;

//simple indice class
class indice{
	public:
		unsigned int v1, v2, v3;
		indice(){};
		indice(unsigned int v_1,unsigned  int v_2,unsigned  int v_3){ v1=v_1; v2=v_2; v3=v_3; };
};

//simple edge class
class edge{
	public:
		unsigned int v1, v2, i1, i2;	
		edge(){};
		edge(unsigned int v_1,unsigned  int v_2,unsigned  int i_1,unsigned int i_2){
			v1=v_1; v2=v_2; i1=i_1; i2=i_2;
		};
};

//3D object class
class Object3D{
	public:
		tVector mov;
		tVector rot;
		tVector position;
		bool visible;
		Quaternion rotation;
		Object3D(){};
		~Object3D(){};
		virtual void Move(float,float,float);
		virtual void Rotate(float,float,float);
		virtual void Update(){};
		virtual void Render(){};
		virtual void Shadow(tVector){};
		virtual void Collide(tVector&, tVector&){};

};

//composite objects, just iterates over child objects
class Object3DComposite: public Object3D{
	public:
		vector<Object3D*> childs;
		Object3D* parent;
		Object3DComposite(){
			rot=tVector(rand()*0.01/RAND_MAX,rand()*0.01/RAND_MAX,rand()*0.01/RAND_MAX);
			mov=tVector(0,0,0);
		};
		~Object3DComposite(){
			for (unsigned int i=0; i< childs.size(); i++){
				delete childs[i];
			}		
		};
		virtual void Update(){
			Rotate(rot.x,rot.y,rot.z);Move(mov.x,mov.y,mov.z);
			for (unsigned int i=0; i< childs.size(); i++){
				childs[i]->Update();
			}
		};
		virtual void Render(){
			glTranslatef(position.x,position.y,position.z);
			float* rmat=new float[16];
			rotation.rmat_gl(rmat);
			glMultMatrixf(rmat);
			for (unsigned int i=0; i< childs.size(); i++){
				glPushMatrix();
				childs[i]->Render();
				glPopMatrix();
			}
		};
		virtual void Shadow(tVector lpos);
		virtual void Collide(tVector& ps, tVector& pf);
};

//component object, implements almost everything in collision, rendering, shadowing
class Object3DComponent: public Object3D{
	public:
		Object3D* parent;
		int verticecount;
		int indicecount;
		int edgecount;
		tVector* vertices;
		tVector* vcolors;
		tVector* vnormals;
		tVector* inormals;
		indice* indices;
		edge* edges;
		tVector maxbound;
		tVector minbound;
		Object3DComponent();
		~Object3DComponent();
		virtual void Update(){Rotate(rot.x,rot.y,rot.z);Move(mov.x,mov.y,mov.z);};
		virtual void Render();
		virtual void Shadow(tVector lpos);
		virtual void Collide(tVector& ps, tVector& pf);

};

//nonperiodic spline example object
class Terrain: public Object3DComponent{
	public:
		float roughness;

		//lx,ly,sx
		Terrain(int lx,int ly,int sx, float roughness);
};

//periodic spline example object
class Torus: public Object3DComponent{
	public:
		float r1,r2,r3;
		Torus(int sx, float r1, float r2, float r3);
};


//nonperiodic spline example object
class Model3D: public Object3DComponent{
	public:
		Model3D(int sx, char const * filename);
};

class Scene;

//simple camera class
class Camera{
	public:
		tVector position;
		tVector mov;
		tVector rot;
		Quaternion rotation;
		Scene *scene;
		Camera(){};
		void Update();
		void Move(float,float,float);
		void Rotate(float,float,float);
};

//scene keeps the universe together
class Scene{
	public:
		vector<Object3D*> objects;
		Camera cam;
		Scene(){cam.scene=this;}
		void Render();
		void Shadow();
		void Update();
		void Collide(tVector& ps, tVector& pf);
};
#endif

