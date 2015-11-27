/*
 * by Simon Jonassen, 2006.
 */
#ifndef Quaternion_HXR
#define Quaternion_HXR
#include <math.h>
class Quaternion{
	public:
		float q0, q1, q2, q3;
		Quaternion(){}
		//(angle,vector) constructor
		Quaternion(float w,tVector t){
			t=t/t.abs();
			q0=cos(w/2);
			q1=sin(w/2)*t.x;
			q2=sin(w/2)*t.y;
			q3=sin(w/2)*t.z;
			normalize();
		}
		
		//explicit constructor
		Quaternion(float q_0,float q_1,float q_2,float q_3){
			q0=q_0; q1=q_1; q2=q_2; q3=q_3;
		}
		
		//quaternion copy
		Quaternion& operator=(const Quaternion& a){
			q0=a.q0; q1=a.q1; q2=a.q2; q3=a.q3; return *this;
		}

		//quaternion product
		Quaternion operator*(const Quaternion& a){
			Quaternion q = Quaternion(
				q0*a.q0-q1*a.q1-q2*a.q2-q3*a.q3,
				q0*a.q1+q1*a.q0+q2*a.q3-q3*a.q2,
				q0*a.q2+q2*a.q0+q3*a.q1-q1*a.q3,
				q0*a.q3+q3*a.q0+q1*a.q2-q2*a.q1
				);
			q.normalize();
			return q;
		}

		//quaternion normalization
		void normalize(){
			float abs = sqrt(q0*q0+q1*q1+q2*q2+q3*q3);
			q0=q0/abs;
			q1=q1/abs;
			q2=q2/abs;
			q3=q3/abs;
		}

		//quaternion inversion
		void invert(){
			q1=-q1;
			q2=-q2;
			q3=-q3;
		}

		//quaternion rotation
		void rotate(bool intern, float pitch,float yaw, float roll){
			Quaternion qroll = Quaternion(cos(roll/2),0,0,sin(roll/2));
			Quaternion qpitch = Quaternion(cos(pitch/2),sin(pitch/2),0,0);
			Quaternion qyaw = Quaternion(cos(yaw/2),0,sin(yaw/2),0);
			Quaternion tmp = Quaternion(q0,q1,q2,q3);
			if (intern){
				*this = tmp*qyaw*qpitch*qroll;
			} else {
				*this = qroll*qpitch*qyaw*tmp;
			}
		}
		//rotation matrix to vector multiplication, used to find normal vectors
		tVector rmatmul(tVector t){
			return tVector(
				t.x*(1-2*(q2*q2+q3*q3))+t.y*2*(q1*q2-q0*q3)+t.z*2*(q0*q2+q1*q3),
				t.x*2*(q1*q2+q0*q3)+t.y*(1-2*(q1*q1+q3*q3))+t.z*2*(q2*q3-q0*q1),
				t.x*2*(q1*q3-q0*q2)+t.y*2*(q0*q1+q2*q3)+t.z*(1-2*(q1*q1+q2*q2))
			);
		}

		//NOTE! rmat_gl is an inverse to rmat
		void rmat_gl(float* mat){
			mat[0]=1-2*(q2*q2+q3*q3);
			mat[1]=2*(q1*q2+q0*q3);
			mat[2]=2*(q1*q3-q0*q2);
			mat[3]=0;
			mat[4]=2*(q1*q2-q0*q3);
			mat[5]=1-2*(q1*q1+q3*q3);
			mat[6]=2*(q0*q1+q2*q3);
			mat[7]=0;
			mat[8]=2*(q0*q2+q1*q3);
			mat[9]=2*(q2*q3-q0*q1);
			mat[10]=1-2*(q1*q1+q2*q2);
			mat[11]=0;
			mat[12]=0;
			mat[13]=0;
			mat[14]=0;
			mat[15]=1;
		}

		void rmat(float* mat){
			mat[0]=1-2*(q2*q2+q3*q3);
			mat[1]=2*(q1*q2-q0*q3);
			mat[2]=2*(q0*q2+q1*q3);
			mat[3]=0;
			mat[4]=2*(q1*q2-q0*q3);
			mat[5]=1-2*(q1*q1+q3*q3);
			mat[6]=2*(q2*q3-q0*q1);
			mat[7]=0;
			mat[8]=2*(q1*q3-q0*q2);
			mat[9]=2*(q0*q1+q2*q3);
			mat[10]=1-2*(q1*q1+q2*q2);
			mat[11]=0;
			mat[12]=0;
			mat[13]=0;
			mat[14]=0;
			mat[15]=1;
		}
};
#endif

