/*
 * by Simon Jonassen, 2006.
 */
#ifndef Spline_HXR
#define Spline_HXR
#include <math.h>
#include "Vector.h"

//general nonunifom B-Spline blending function for vectors
tVector blendtVector(float u, tVector pip, tVector pi, tVector pin, tVector pinn){
	tVector r1=pinn-pin*3+pi*3-pip;
	tVector r2=pip*3-pi*6+pin*3;
	tVector r3=pin*3-pip*3;
	tVector r4=pip+pi*4+pin;
	return tVector(
		(r1.x*u*u*u+r2.x*u*u+r3.x*u+r4.x)/6,
		(r1.y*u*u*u+r2.y*u*u+r3.y*u+r4.y)/6,
		(r1.z*u*u*u+r2.z*u*u+r3.z*u+r4.z)/6
	);
}

//nonperiodic mesh blending; generates also normals, indices and edges
void blendMesh(int lx, int ly, int sx, tVector* cpoints, tVector* vertices,tVector* vnormals, indice* indices, tVector* inormals){
	//extend vertically
	tVector* cbpoints=new tVector[lx*ly*sx];
	for (int i=0; i<ly;i++){
		for (int j=0; j<lx;j++){
			for (int k=0;k<sx;k++){
				tVector pip,pi,pin,pinn;
				pi=cpoints[i*lx+j];
				if (j>0) pip=cpoints[i*lx+j-1];
				else pip=cpoints[i*lx+j];
				if (j<(lx-1)) pin=cpoints[i*lx+j+1];
				else pin=cpoints[i*lx+j];
				if (j<(lx-2)) pinn=cpoints[i*lx+j+2];
				else pinn=cpoints[i*lx+j];
				cbpoints[i*lx*sx+j*sx+k]=blendtVector((float)k/sx,pip,pi,pin,pinn);
			}
		}
	}
	//extend horisontally
	for (int i=0; i<ly;i++){
		for (int j=0; j<lx*sx;j++){
			for (int k=0;k<sx;k++){
				tVector pip,pi,pin,pinn;
				pi=cbpoints[i*lx*sx+j];
				if (i>0) pip=cbpoints[(i-1)*lx*sx+j];
				else pip=cbpoints[i*lx*sx+j];
				if (i<(ly-1)) pin=cbpoints[(i+1)*lx*sx+j];
				else pin=cbpoints[i*lx*sx+j];
				if (i<(ly-2)) pinn=cbpoints[(i+2)*lx*sx+j];
				else pinn=cbpoints[i*lx*sx+j];
				vertices[i*lx*sx*sx+lx*sx*k+j]=blendtVector((float)k/sx,pip,pi,pin,pinn);
			}
		}
	}
	//generate indices, edges and normals
	for (int i=0; i<ly*sx-1;i++){
		for (int j=0; j<lx*sx-1;j++){

			int ax = i*lx*sx+j;
			int bx = i*lx*sx+(j+1);
			int cx = (i+1)*lx*sx+(j+1);
			int dx = (i+1)*lx*sx+j;

			indices[2*(i*(lx*sx-1)+j)]=indice(dx,bx,ax);
			indices[2*(i*(lx*sx-1)+j)+1]=indice(bx,dx,cx);
			tVector norm1 = (vertices[dx]-vertices[ax]).cross(vertices[bx]-vertices[ax]);
			tVector norm2 = (vertices[bx]-vertices[cx]).cross(vertices[dx]-vertices[cx]);
			norm1=norm1/(norm1.abs()*4);
			norm2=norm2/(norm2.abs()*4);
			tVector mnorm12=(norm1+norm2)/2;
			inormals[2*(i*(lx*sx-1)+j)]=norm1;
			inormals[2*(i*(lx*sx-1)+j)+1]=norm2;

			vnormals[ax]=vnormals[ax]+norm1;
			vnormals[bx]=vnormals[bx]+mnorm12;
			vnormals[dx]=vnormals[dx]+mnorm12;
			vnormals[cx]=vnormals[cx]+norm2;
		}
	}
	//edges must be fixed outside this method
	delete [] cbpoints;
}

//periodic mesh blending; generates also normals, indices and edges
void blendMeshPeriodic(int lx, int ly, int sx, tVector* cpoints, tVector* vertices,tVector* vnormals, indice* indices, tVector* inormals, edge* edges){
	//extend in both directions
	tVector* cbpoints=new tVector[lx*ly*sx];
	for (int i=0; i<ly;i++){
		for (int j=0; j<lx;j++){
			for (int k=0;k<sx;k++){
				tVector pip,pi,pin,pinn;
				pi=cpoints[i*lx+j];
				pip=cpoints[i*lx+(j+(lx-1))%lx];
				pin=cpoints[i*lx+(j+1)%lx];
				pinn=cpoints[i*lx+(j+2)%lx];
				cbpoints[i*lx*sx+j*sx+k]=blendtVector((float)k/sx,pip,pi,pin,pinn);
			}
		}
	}
	for (int i=0; i<ly;i++){
		for (int j=0; j<lx*sx;j++){
			for (int k=0;k<sx;k++){
				tVector pip,pi,pin,pinn;
				pi=cbpoints[i*lx*sx+j];
				pip=cbpoints[((i+(ly-1))%ly)*lx*sx+j];
				pin=cbpoints[((i+1)%ly)*lx*sx+j];
				pinn=cbpoints[((i+2)%ly)*lx*sx+j];
				vertices[i*lx*sx*sx+lx*sx*k+j]=blendtVector((float)k/sx,pip,pi,pin,pinn);
			}
		}
	}

	//generate indices, edges and normals
	for (int i=0; i<ly*sx;i++){
		for (int j=0; j<lx*sx;j++){
			int ax = i*lx*sx+j;
			int bx = i*lx*sx+(j+1)%(lx*sx);
			int cx = ((i+1)%(ly*sx))*lx*sx+(j+1)%(lx*sx);
			int dx = ((i+1)%(ly*sx))*lx*sx+j;

			indices[2*(i*lx*sx+j)]=indice(dx,bx,ax);
			indices[2*(i*lx*sx+j)+1]=indice(bx,dx,cx);
			tVector norm1 = (vertices[dx]-vertices[ax]).cross(vertices[bx]-vertices[ax]);
			tVector norm2 = (vertices[bx]-vertices[cx]).cross(vertices[dx]-vertices[cx]);
			norm1=norm1/(norm1.abs()*4);
			norm2=norm2/(norm2.abs()*4);
			inormals[2*(i*lx*sx+j)]=norm1;
			inormals[2*(i*lx*sx+j)+1]=norm2;

			tVector mnorm12=(norm1+norm2)/2;

			vnormals[ax]=vnormals[ax]+norm1;
			vnormals[bx]=vnormals[bx]+mnorm12;
			vnormals[dx]=vnormals[dx]+mnorm12;
			vnormals[cx]=vnormals[cx]+norm2;

			edges[(i*lx*sx+j)*3]   = edge(ax,bx,2*(((i+ly*sx-1)%(ly*sx))*lx*sx+j)+1,2*(i*lx*sx+j));
			edges[(i*lx*sx+j)*3+1] = edge(dx,bx,2*(i*lx*sx+j),2*(i*lx*sx+j)+1);
			edges[(i*lx*sx+j)*3+2] = edge(ax,dx,2*(i*lx*sx+j),2*(i*lx*sx+(j+lx*sx-1)%(lx*sx))+1);
		}
	}
	delete [] cbpoints;
}
#endif
