/*
 * by Simon Jonassen, 2006.
 */
#ifndef Vector_HXR
#define Vector_HXR
#include <math.h>
//defines vector algebra
class tVector{
	public:
		float x, y, z;
		tVector(){x=y=z=0;}
		tVector(float xf,float yf,float zf){x=xf; y=yf; z=zf;}
		tVector operator+(const tVector& a){return tVector(x+a.x, y+a.y, z+a.z);}
		tVector operator-(const tVector& a){return tVector(x-a.x,y-a.y,z-a.z);}
		tVector operator*(float a){return tVector(a*x,a*y,a*z);}
		tVector operator/(float a){return tVector(x/a,y/a,z/a);}
		tVector& operator=(const tVector& a){x=a.x; y=a.y; z=a.z; return *this;}
		tVector cross(const tVector& a){return tVector(y*a.z-z*a.y,z*a.x-x*a.z,x*a.y-y*a.x);}
		float abs(){return sqrt(x*x+y*y+z*z);}
		float dot(const tVector& a){return x*a.x+y*a.y+z*a.z;}
};

inline void matrixMultMatrix(float *ma, float* mb, float* mm, int l, int m, int n){
	for (int i=0; i<l; i++){
		for (int j=0; j<n; j++){
			mm[i*n+j]=0;
			for (int k=0; k<m; k++){
				mm[i*n+j]+=ma[i*m+k]*mb[k*n+j];
			}
		}
	}
};

inline void invertMatrix3x3(float *mi, float *mo){
	float a=mi[0];
	float b=mi[1];
	float c=mi[2];
	float d=mi[3];
	float e=mi[4];
	float f=mi[5];
	float g=mi[6];
	float h=mi[7];
	float i=mi[8];
	float det=a*(e*i-f*h)-b*(d*i-f*g)+c*(d*h-e*g);

	mo[0]=(e*i-f*h)/det;
	mo[1]=(c*h-b*i)/det;
	mo[2]=(b*f-c*e)/det;
	mo[3]=(f*g-d*i)/det;
	mo[4]=(a*i-c*g)/det;
	mo[5]=(c*d-a*f)/det;
	mo[6]=(d*h-e*g)/det;
	mo[7]=(b*g-a*h)/det;
	mo[8]=(a*e-b*d)/det;
}

#endif

