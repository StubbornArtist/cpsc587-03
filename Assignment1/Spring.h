#pragma once
#include "Mass.h"
class Spring {
public:
	Spring();
	Spring(Mass * m1, Mass * m2, float k);
	float getStiffness();
	void setStiffness(float k);
	Mass * getFirstMass();
	void setFirstMass(Mass * m);
	Mass * getSecondMass();
	void setSecondMass(Mass * m);
	float getRestLength();
	void setRestLength(float r);
	vec3 updateInternalForce();
private:
	float k;
	float restLen;
	Mass * m1;
	Mass * m2;

};
