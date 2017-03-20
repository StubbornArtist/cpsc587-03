#pragma once
#include "Spring.h"
#include <vector>

using namespace std;

class SpringSystem{
private:
	vector<Spring *> springs;
	vector<Mass *> masses;
	vector<vec3> forces;
	float damping;
	vec3 gravity;
	float deltaT;
public:
	SpringSystem();
	void addSpring(Spring * s);
	void addMass(Mass * m);
	void setDamping(float d);
	void setGravity(vec3 g);
	void setDeltaT(float t);
	void addForce(vec3 f);
	float getDeltaT();
	void simulate();
	void reset();
	void getMesh(vector<float> * buf);

};