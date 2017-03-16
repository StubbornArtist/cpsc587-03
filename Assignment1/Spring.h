#pragma once
#include "Mass.h"
class Spring {
public:
	Spring();
	Spring(Mass * m1, Mass * m2, float k, float d);
	float getStiffness();
	void setStiffness(float k);
	float getDamping();
	void setDamping(float d);
	Mass * getFirstMass();
	void setFirstMass(Mass * m);
	Mass * getSecondMass();
	void setSecondMass(Mass * m);
	float getRestLength();
	void setRestLength(float r);
	void updateInternalForce();
private:
	float k;
	float restLen;
	float damp;
	Mass * m1;
	Mass * m2;

};
