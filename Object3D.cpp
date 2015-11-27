/*
 * by Simon Jonassen, 2006.
 */
#include "Object3D.h"
#include "BSpline.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
GLfloat LightPosition[4]= {0.0f,500.0f,0.0f, 1.0f};

//generate a random float between -1 and 1
float randch(){
	return (float)(rand()-RAND_MAX/2)/RAND_MAX;
};

//
//object funtions in depth:
//
void Object3D::Move(float dist1,float dist2,float dist3){
	//just increment position from rotation normals
	position=position+rotation.rmatmul(tVector(0,0,-1))*dist1+
		rotation.rmatmul(tVector(-1,0,0))*dist2+
		rotation.rmatmul(tVector(0,-1,0))*dist3;
};

void Object3D::Rotate(float pitch,float yaw,float roll){
	//use quaternion rotation
	rotation.rotate(false,pitch,yaw,roll);
};

Object3DComponent::Object3DComponent(){
	//set up position, rotation and bounds
	position=tVector(0,0,0);
	rotation=Quaternion(0,tVector(0,1,0));
	maxbound=tVector(0,0,0);
	minbound=tVector(0,0,0);
};

Object3DComponent::~Object3DComponent(){
	//remember to delete all the data
	delete [] edges;
	delete [] indices;
	delete [] vertices;
	delete [] vcolors;
	delete [] vnormals;
	delete [] inormals;
};

//object rendering
void Object3DComponent::Render(){
	//translate to object position
	glTranslatef(position.x,position.y,position.z);
	float* rmat=new float[16];
	rotation.rmat_gl(rmat);
	glMultMatrixf(rmat);
    
	//send arrays to render, clear and simple ;)
    glVertexPointer(3, GL_FLOAT, 0, (float*) vertices); 
	glNormalPointer(GL_FLOAT, 0, (float*) vnormals);
    glColorPointer(3,GL_FLOAT,0,(float*) vcolors);
	glDrawElements(GL_TRIANGLES, indicecount*3, GL_UNSIGNED_INT, (unsigned int*) indices);
};


//whole shadow volume rendering for composite objects
void Object3DComposite::Shadow(tVector lpos){
	float* rmat=new float[16];
	float* tmat=new float[16];	
	float* lposg=new float[4];
	float* lposl=new float[4];
	float* tmp1=new float[16];
	//translate viewpoint to object position, set up direction.
	glTranslatef(position.x,position.y,position.z);
	rotation.rmat_gl(rmat);
	glMultMatrixf(rmat);
	
	//find inverse rotation matrix; NOTE: rmat is inverted now!
	Quaternion q = rotation;
	q.rmat_gl(rmat);
	
	tmat[0]=1; tmat[1]=0; tmat[2]=0; tmat[3]=-position.x;
	tmat[4]=0; tmat[5]=1; tmat[6]=0; tmat[7]=-position.y;
	tmat[8]=0; tmat[9]=0; tmat[10]=1; tmat[11]=-position.z;
	tmat[12]=0; tmat[13]=0; tmat[14]=0; tmat[15]=1;
	
	//multiply rotation and translation
	matrixMultMatrix(rmat,tmat,tmp1,4,4,4);

	lposg[0]=lpos.x;
	lposg[1]=lpos.y;
	lposg[2]=lpos.z;
	lposg[3]=1;

	//translate global light position to object coordinate system
	matrixMultMatrix(tmp1,lposg,lposl,4,4,1);
	lpos = tVector(lposl[0]/lposl[3],lposl[1]/lposl[3],lposl[2]/lposl[3]);

	for (unsigned int i=0; i< childs.size(); i++){
		if (childs[i]->visible){
			glPushMatrix();
			childs[i]->Shadow(lpos);
			glPopMatrix();
		}
	}
	
	//clean up
	delete[] rmat;
	delete[] tmat;	
	delete[] lposg;
	delete[] lposl;
	delete[] tmp1;
};

//whole collision detection for composite objects
void Object3DComposite::Collide(tVector& ps, tVector& pf){
	//set up tables
	//TODO preallocate this data some place
	float* rmat=new float[16];
	float* tmat=new float[16];	
	float* tmp1=new float[16];
	float* sposg=new float[4];
	float* fposg=new float[4];
	float* sposl=new float[4];
	float* fposl=new float[4];
	float* pp = new float[9];
	float* p = new float[3];
	float* solv = new float[3];
	
	//NOTE! rmat is inverted!
	rotation.rmat_gl(rmat);
	
	tmat[0]=1; tmat[1]=0; tmat[2]=0; tmat[3]=-position.x;
	tmat[4]=0; tmat[5]=1; tmat[6]=0; tmat[7]=-position.y;
	tmat[8]=0; tmat[9]=0; tmat[10]=1; tmat[11]=-position.z;
	tmat[12]=0; tmat[13]=0; tmat[14]=0; tmat[15]=1;

	//calculate Rmat*Tmat
	matrixMultMatrix(rmat,tmat,tmp1,4,4,4);
	
	sposg[0]=ps.x;
	sposg[1]=ps.y;
	sposg[2]=ps.z;
	sposg[3]=1;


	fposg[0]=pf.x;
	fposg[1]=pf.y;
	fposg[2]=pf.z;
	fposg[3]=1;

	//translate both positions from global to local frame
	//TODO use different object states to translate start and final positions
	matrixMultMatrix(tmp1,sposg,sposl,4,4,1);
	matrixMultMatrix(tmp1,fposg,fposl,4,4,1);

	tVector lps = tVector(sposl[0]/sposl[3],sposl[1]/sposl[3],sposl[2]/sposl[3]);
	tVector lpf = tVector(fposl[0]/fposl[3],fposl[1]/fposl[3],fposl[2]/fposl[3]);

	//now! collide with childs using local coordinates
	for (unsigned int i=0; i< childs.size(); i++)
		childs[i]->Collide(lps,lpf);

	//translate the endpoint back to global
	Quaternion q = rotation;
	q.invert();
	q.rmat_gl(rmat);
	tmat[0 ]=1; tmat[1 ]=0; tmat[2 ]=0; tmat[3 ]=position.x;
	tmat[4 ]=0; tmat[5 ]=1; tmat[6 ]=0; tmat[7 ]=position.y;
	tmat[8 ]=0; tmat[9 ]=0; tmat[10]=1; tmat[11]=position.z;
	tmat[12]=0; tmat[13]=0; tmat[14]=0; tmat[15]=1;

	matrixMultMatrix(tmat,rmat,tmp1,4,4,4);
	
	fposl[0]=lpf.x;
	fposl[1]=lpf.y;
	fposl[2]=lpf.z;
	fposl[3]=1;
	matrixMultMatrix(tmp1,fposl,fposg,4,4,1);
	pf=tVector(fposg[0]/fposg[3],fposg[1]/fposg[3],fposg[2]/fposg[3]);
	delete[] rmat;
	delete[] tmat;	
	delete[] tmp1;
	delete[] sposg;
	delete[] fposg;
	delete[] sposl;
	delete[] fposl;
	delete[] solv;
	delete[] p;
	delete[] pp;
};

//whole shadow volume rendering for component objects
void Object3DComponent::Shadow(tVector lpos){
	float* rmat=new float[16];
	float* tmat=new float[16];	
	float* lposg=new float[4];
	float* lposl=new float[4];
	float* tmp1=new float[16];
	//translate viewpoint to object position, set up direction.
	glTranslatef(position.x,position.y,position.z);
	rotation.rmat_gl(rmat);
	glMultMatrixf(rmat);
	
	//find inverse rotation matrix; NOTE: rmat is inverted now!
	Quaternion q = rotation;
	q.rmat_gl(rmat);
	
	tmat[0]=1; tmat[1]=0; tmat[2]=0; tmat[3]=-position.x;
	tmat[4]=0; tmat[5]=1; tmat[6]=0; tmat[7]=-position.y;
	tmat[8]=0; tmat[9]=0; tmat[10]=1; tmat[11]=-position.z;
	tmat[12]=0; tmat[13]=0; tmat[14]=0; tmat[15]=1;
	
	//multiply rotation and translation
	matrixMultMatrix(rmat,tmat,tmp1,4,4,4);

	lposg[0]=lpos.x;
	lposg[1]=lpos.y;
	lposg[2]=lpos.z;
	lposg[3]=1;

	//translate global light position to object coordinate system
	matrixMultMatrix(tmp1,lposg,lposl,4,4,1);
	tVector l = tVector(lposl[0]/lposl[3],lposl[1]/lposl[3],lposl[2]/lposl[3]);
	
	edge e;
	tVector v1, v2, v3, v4, n1, n2, lv;
	glBegin(GL_QUADS);
	//iterate
	for (int i=0; i < edgecount; i++){
		e = edges[i];
		v1 = vertices[e.v1];
		v2 = vertices[e.v2];
		n1 = inormals[e.i1];

		if (e.i2<0) n2=tVector(0,0,0)-n1;
		else n2 = inormals[e.i2];

		lv = (v1+v2)/2 - l;
		//check if indices faces light differently
		if ( n1.dot(lv)*n2.dot(lv) < 0){
			if (n1.dot(lv)<0){
				tVector tmp=v1;
				v1=v2;
				v2=tmp;
			}
			v4= v1+(v1-l)*10000;
			v3= v2+(v2-l)*10000;
			//TODO: reverse direction if the first face shadows 
		
			//draw the band
			glVertex3f(v1.x,v1.y,v1.z);
			glVertex3f(v2.x,v2.y,v2.z);
			glVertex3f(v3.x,v3.y,v3.z);
			glVertex3f(v4.x,v4.y,v4.z);

		}
	}	
	glEnd();	
	//clean up
	delete[] rmat;
	delete[] tmat;	
	delete[] lposg;
	delete[] lposl;
	delete[] tmp1;
}



//whole collision detection for component objects
void Object3DComponent::Collide(tVector& ps, tVector& pf){

	//set up tables
	//TODO preallocate this data some place
	float* rmat=new float[16];
	float* tmat=new float[16];	
	float* tmp1=new float[16];
	float* sposg=new float[4];
	float* fposg=new float[4];
	float* sposl=new float[4];
	float* fposl=new float[4];
	float* pp = new float[9];
	float* p = new float[3];
	float* solv = new float[3];
	
	//NOTE! rmat is inverted!
	rotation.rmat_gl(rmat);
	
	tmat[0]=1; tmat[1]=0; tmat[2]=0; tmat[3]=-position.x;
	tmat[4]=0; tmat[5]=1; tmat[6]=0; tmat[7]=-position.y;
	tmat[8]=0; tmat[9]=0; tmat[10]=1; tmat[11]=-position.z;
	tmat[12]=0; tmat[13]=0; tmat[14]=0; tmat[15]=1;

	//calculate Rmat*Tmat
	matrixMultMatrix(rmat,tmat,tmp1,4,4,4);
	
	sposg[0]=ps.x;
	sposg[1]=ps.y;
	sposg[2]=ps.z;
	sposg[3]=1;


	fposg[0]=pf.x;
	fposg[1]=pf.y;
	fposg[2]=pf.z;
	fposg[3]=1;

	//translate both positions from global to local frame
	//TODO use different object states to translate start and final positions
	matrixMultMatrix(tmp1,sposg,sposl,4,4,1);
	matrixMultMatrix(tmp1,fposg,fposl,4,4,1);

	tVector lps = tVector(sposl[0]/sposl[3],sposl[1]/sposl[3],sposl[2]/sposl[3]);
	tVector lpf = tVector(fposl[0]/fposl[3],fposl[1]/fposl[3],fposl[2]/fposl[3]);
	
	//check bounding box collision
	if ( lpf.x>minbound.x && lpf.x<maxbound.x &&
		lpf.y>minbound.y && lpf.y<maxbound.y &&
		lpf.z>minbound.z && lpf.z<maxbound.z
	){
		//for all indices...
		for (int i=0; i <indicecount; i++){
			tVector norm = inormals[i];
			indice ind = indices[i];
			float a=norm.x;
			float b=norm.y;
			float c=norm.z;
			tVector v1=vertices[ind.v1];
			float d = - (a*v1.x + b*v1.y + c*v1.z);
			//check indices for plane-line-intersetction
		    //TODO insert a pretest with distance to point vs max polygon radius here
			if ((a*lps.x+b*lps.y+c*lps.z+d)*(a*lpf.x+b*lpf.y+c*lpf.z+d) < 0){

				//solve time-impact equation
				indice ind = indices[i];
							
				tVector v2=vertices[ind.v2];
				tVector v3=vertices[ind.v3];

				tVector p1=v2-v1;
				tVector p2=v3-v1;

				pp[0]=lps.x-lpf.x;
				pp[1]=p1.x;
				pp[2]=p2.x;
				pp[3]=lps.y-lpf.y;
				pp[4]=p1.y;
				pp[5]=p2.y;
				pp[6]=lps.z-lpf.z;
				pp[7]=p1.z;
				pp[8]=p2.z;

				invertMatrix3x3(pp,pp);
				
				p[0]=lps.x-v1.x;
				p[1]=lps.y-v1.y;
				p[2]=lps.z-v1.z;
		
				matrixMultMatrix(pp,p,solv,3,3,1);
				//if the impact point inside indice
				if ( solv[1]>0 && solv[2]>0 && solv[1]+solv[2]<1){
					tVector rest=(lpf-lps)*(1-solv[0]);
					if (norm.dot(norm)!=0){
						//bounce!!!
						lpf=lpf-norm*(norm.dot(rest)/norm.dot(norm))*2;
					}
				}
			}
		}
	}
	//translate the endpoint back to global
	Quaternion q = rotation;
	q.invert();
	q.rmat_gl(rmat);
	tmat[0 ]=1; tmat[1 ]=0; tmat[2 ]=0; tmat[3 ]=position.x;
	tmat[4 ]=0; tmat[5 ]=1; tmat[6 ]=0; tmat[7 ]=position.y;
	tmat[8 ]=0; tmat[9 ]=0; tmat[10]=1; tmat[11]=position.z;
	tmat[12]=0; tmat[13]=0; tmat[14]=0; tmat[15]=1;

	matrixMultMatrix(tmat,rmat,tmp1,4,4,4);
	
	fposl[0]=lpf.x;
	fposl[1]=lpf.y;
	fposl[2]=lpf.z;
	fposl[3]=1;
	matrixMultMatrix(tmp1,fposl,fposg,4,4,1);
	pf=tVector(fposg[0]/fposg[3],fposg[1]/fposg[3],fposg[2]/fposg[3]);
	delete[] rmat;
	delete[] tmat;	
	delete[] tmp1;
	delete[] sposg;
	delete[] fposg;
	delete[] sposl;
	delete[] fposl;
	delete[] solv;
	delete[] p;
	delete[] pp;

};

//generate a terrain
Terrain::Terrain(int lx, int ly, int sx, float r){
	roughness=r;
	srand(time(NULL));
	visible=false;
	
	verticecount=lx*ly*sx*sx;
	vertices=new tVector[verticecount];
	vcolors=new tVector[verticecount];
	vnormals=new tVector[verticecount];

	indicecount=(lx*sx-1)*(ly*sx-1)*2;
	indices=new indice[indicecount];
	inormals=new tVector[indicecount];
	
	tVector* cpoints=new tVector[lx*ly];
	cpoints[0].y=roughness*randch();
	float maxy=-999999;
	float miny=999999;
	for (int i=0; i<ly;i++){
		for (int j=0; j<lx;j++){
			if  (i==0 && j==0)
				cpoints[i*lx+j]=tVector(j*100,roughness*randch(),i*100);
			else if (i==0)
				cpoints[i*lx+j]=tVector(j*100,cpoints[i*lx+j-1].y+roughness*randch(),i*100);
			else if (j==0)
				cpoints[i*lx+j]=tVector(j*100,cpoints[(i-1)*lx+j].y+roughness*randch(),i*100);
			else
				cpoints[i*lx+j]=tVector(j*100,0.5*(cpoints[(i-1)*lx+j].y+cpoints[i*lx+j-1].y)+roughness*randch(),i*100);
			if (cpoints[i*lx+j].y>maxy) maxy = cpoints[i*lx+j].y;
			if (cpoints[i*lx+j].y<miny) miny = cpoints[i*lx+j].y;
		}
	}
	//get object geometry from this mesh!
    blendMesh(lx,ly,sx,cpoints,vertices,vnormals,indices,inormals);
	delete [] cpoints;

	for (int i=0; i<ly*sx;i++){
		for (int j=0; j<lx*sx;j++){
			vcolors[i*lx*sx+j]=tVector(0.341,0.404,0.24)+(tVector(1.0,1.0,1.0)-tVector(0.341,0.404,0.24))*pow((vertices[i*lx*sx+j].y-miny)/(maxy-miny),2);
		}
	}

	edgecount=0;
	vector<edge> ve_edges;
	/*
	/ Note: This part of code works, but runs in O(lx^2 * ly^2 * sx^4), so it was removed to accelerate performance =/
	for (int i=0; i<indicecount; i++){
		int v1 = indices[i].v1;
		int v2 = indices[i].v2;
		int v3 = indices[i].v3;

		int f1[] = {indices[i].v1,indices[i].v2,indices[i].v3};
		for (int k=0; k<3; k++){
			//tripwire, for each edge try to find another edge that match, if not the edge have only one face!
			bool found=false;
			for (int j=i+1; j<indicecount; j++){
				
				int f2[] = {indices[j].v1,indices[j].v2,indices[j].v3};
				for (int l=0; l<3; l++){
					if (f1[k]==f2[l] && f1[(k+1)%3]==f2[(l+2)%3]){
						ve_edges.push_back(edge(f1[k],f1[(k+1)%3],i,j));
						edgecount++;
						found=true; l=3; j=indicecount;
						break;
					}
				}
			}
			if (!found) ve_edges.push_back(edge(f1[k],f1[(k+1)%3],i,-1));
		}
	}
	*/
	edges = new edge[edgecount];
	for (int i=0; i<edgecount; i++){
		edges[i]=ve_edges[i];
	}
	rot=tVector(0,0,0);
	mov=tVector(0,0,0);
	maxbound=tVector(100*lx,maxy,100*ly);
	minbound=tVector(-100*lx,miny,-100*ly);
};

//generate a torus
Torus::Torus(int sx, float r1, float r2, float r3){
	this->r1=r1;
	this->r2=r2;
	this->r3=r3;
	
	int lx=4;
	int ly=4;	
	visible=true;
	
	verticecount=lx*ly*sx*sx;
	vertices=new tVector[verticecount];
	vcolors=new tVector[verticecount];
	vnormals=new tVector[verticecount];

	indicecount=lx*sx*ly*sx*2;
	indices=new indice[indicecount];
	inormals=new tVector[indicecount];
	
	edgecount=3*lx*sx*ly*sx;
	edges=new edge[edgecount];

	tVector* cpoints=new tVector[lx*ly];
	cpoints[0] =tVector(-r1,-r3,-r1);
	cpoints[1] =tVector(-r2,-r3,-r2);
	cpoints[2] =tVector(-r2, r3,-r2);
	cpoints[3] =tVector(-r1, r3,-r1);
	cpoints[4] =tVector(-r1,-r3, r1);
	cpoints[5] =tVector(-r2,-r3, r2);
	cpoints[6] =tVector(-r2, r3, r2);
	cpoints[7] =tVector(-r1, r3, r1);
	cpoints[8] =tVector( r1,-r3, r1);
	cpoints[9] =tVector( r2,-r3, r2);
	cpoints[10]=tVector( r2, r3, r2);
	cpoints[11]=tVector( r1, r3, r1);
	cpoints[12]=tVector( r1,-r3,-r1);
	cpoints[13]=tVector( r2,-r3,-r2);
	cpoints[14]=tVector( r2, r3,-r2);
	cpoints[15]=tVector( r1, r3,-r1);

	//convert mesh to geometry
	blendMeshPeriodic(lx,ly,sx,cpoints,vertices,vnormals,indices,inormals,edges);
	delete [] cpoints;
	for (int i=0; i<ly*sx;i++){
		for (int j=0; j<lx*sx;j++){
			vcolors[i*lx*sx+j]=tVector((vertices[i*lx*sx+j].x+r2)/(2*r2),(vertices[i*lx*sx+j].y+r3)/(2*r3),(vertices[i*lx*sx+j].z+r2)/(2*r2));
		}
	}
	
	rot=tVector(rand()*0.01/RAND_MAX,rand()*0.01/RAND_MAX,rand()*0.01/RAND_MAX);
	//rot=tVector(0,0,0);
	mov=tVector(0,0,0);
	maxbound=tVector(r2,r3,r2);
	minbound=tVector(-r2,-r3,-r2);
};

Model3D::Model3D(int sx, char const * filename){
    verticecount=0, indicecount=0, edgecount=0;
	printf("Loading model %s: ",filename);
    FILE* inFile;
    inFile = fopen(filename, "r");
    if(inFile == NULL){
		printf("error\n");
    }
	else{
	
	char curr[1024];
	vector<tVector> ve_vertices;
	vector<tVector> ve_vnormals;
	vector<edge> ve_edges;
	vector<indice> ve_indices;
	float x=0,y=0,z=0;
	int a=0,b=0,c=0, tmp=0;

    while(fscanf(inFile, "%s", curr) != EOF){
		if(strcmp(curr, "v") == 0){
			verticecount++;
			fscanf(inFile, "%f", &x);
			fscanf(inFile, "%f", &y);
			fscanf(inFile, "%f", &z);
			ve_vertices.push_back(tVector(sx*x,sx*y,sx*z));
		}
		else if (strcmp(curr, "f") == 0 ){
			
			fscanf(inFile, "%d", &a);
			fscanf(inFile, "%d", &b);
			fscanf(inFile, "%d", &c);
			do{
				indicecount++;
				ve_indices.push_back(indice(a-1 ,b-1, c-1));
				b=c;
			} while (fscanf(inFile, "%d", &c) && b!=c);
		}
	}
	maxbound=tVector(0,0,0);
	minbound=tVector(0,0,0);
	vertices = new tVector[verticecount];
	vnormals = new tVector[verticecount];
	for (int i=0; i<verticecount; i++){
		vertices[i]=ve_vertices[i];
		vnormals[i]=tVector(0,0,0);
		if (vertices[i].x>maxbound.x) maxbound.x=vertices[i].x;
		else if (vertices[i].x<minbound.x) minbound.x=vertices[i].x;
		if (vertices[i].y>maxbound.y) maxbound.y=vertices[i].y;
		else if (vertices[i].y<minbound.y) minbound.y=vertices[i].y;
		if (vertices[i].z>maxbound.z) maxbound.z=vertices[i].z;
		else if (vertices[i].z<minbound.z) minbound.z=vertices[i].z;
	}

	indices = new indice[indicecount];
	inormals = new tVector[indicecount];
	for (int i=0; i<indicecount; i++){
		indices[i]=ve_indices[i];
		int v1 = indices[i].v1;
		int v2 = indices[i].v2;
		int v3 = indices[i].v3;
		tVector fn = (vertices[v2] - vertices[v1]).cross(vertices[v3]-vertices[v1]);
		inormals[i]=fn/fn.abs();
		vnormals[v1]=vnormals[v1]+fn;
		vnormals[v2]=vnormals[v2]+fn;
		vnormals[v3]=vnormals[v3]+fn;

		//find edges here
		for (int j=i+1; j<indicecount; j++){
			int f1[] = {ve_indices[i].v1,ve_indices[i].v2,ve_indices[i].v3};
			int f2[] = {ve_indices[j].v1,ve_indices[j].v2,ve_indices[j].v3};
			for (int k=0; k<3; k++){
				for (int l=0; l<3; l++){
					if (f1[k]==f2[l] && f1[(k+1)%3]==f2[(l+2)%3]){
						ve_edges.push_back(edge(f1[k],f1[(k+1)%3],i,j));
						edgecount++;
					}
				}
			}
		}
	}
	vcolors = new tVector[verticecount];
	for (int i=0; i<verticecount; i++){
		vnormals[i]=vnormals[i]/vnormals[i].abs();
		vcolors[i]=tVector(
			(vertices[i].x-minbound.x)/(maxbound.x-minbound.x),
			(vertices[i].y-minbound.y)/(maxbound.y-minbound.y),
			(vertices[i].z-minbound.z)/(maxbound.z-minbound.z)
		);
	}
	
	edges = new edge[edgecount];
	for (int i=0; i<edgecount; i++){
		edges[i]=ve_edges[i];
	}
	printf("%d %d %d \n",verticecount,indicecount,edgecount);
	}
	
	rot=tVector(rand()*0.01/RAND_MAX,rand()*0.01/RAND_MAX,rand()*0.01/RAND_MAX);
	mov=tVector(0,0,0);
	visible=true;
}

// define camera functions

void Camera::Update(){
	// on update need to recalculate mov and rot vectors, calculate new position,
    // collide, recalculate mov vector, 
	mov=mov*0.95;
	rot=rot*0.95;

	rotation.rotate(true,rot.x,rot.y,rot.z);
	tVector	nposition=position+mov;
	scene->Collide(position,nposition);
	mov=nposition-position;
	position=nposition;
};

void Camera::Move(float acc1,float acc2,float acc3){
	// just get rotation projections, move in all 3 directions
	mov=mov+rotation.rmatmul(tVector(0,0,-1))*acc1+
		rotation.rmatmul(tVector(-1,0,0))*acc2+
		rotation.rmatmul(tVector(0,-1,0))*acc3;
};

void Camera::Rotate(float pitch,float yaw,float roll){
	//just increment vector
	rot=rot+tVector(pitch,yaw,roll);
};

void Scene::Shadow(){
	//translate to camera position
	float* rmat=new float[16];
	Quaternion qq = cam.rotation;
	qq.invert();
	qq.rmat_gl(rmat);
	glMultMatrixf(rmat);
	glTranslatef(-cam.position.x,-cam.position.y,-cam.position.z);
	
	//render all visible childs
	tVector lpos=tVector(LightPosition[0]/LightPosition[3],LightPosition[1]/LightPosition[3],LightPosition[2]/LightPosition[3]);
	for (unsigned int i=0; i<objects.size(); i++){
		if (objects[i]->visible){
			glPushMatrix();
			objects[i]->Shadow(lpos);
			glPopMatrix();
		}
	}
	
};

void Scene::Render(){
	//translate to camera position
	float* rmat=new float[16];
	Quaternion qq = cam.rotation;
	qq.invert();
	qq.rmat_gl(rmat);
	glMultMatrixf(rmat);
	glTranslatef(-cam.position.x,-cam.position.y,-cam.position.z);
	
	//set the light NOW!
	glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);

	//render each object now
	for (unsigned int i=0; i<objects.size(); i++){
		glPushMatrix();
		objects[i]->Render();
		glPopMatrix();
	}
	
};

void Scene::Update(){
	// just iterate over objects and update them. Update the camera.
	for (unsigned int i=0; i<objects.size(); i++) 
		objects[i]->Update();
	cam.Update();
};

void Scene::Collide(tVector &ps,tVector &pf){
	//just iterates through scene objects
	for (unsigned int i=0; i<objects.size(); i++){
		objects[i]->Collide(ps,pf);
	}
};
