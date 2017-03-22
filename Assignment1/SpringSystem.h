#pragma once
#include "Spring.h"
#include <vector>
#include <math.h>
#include <random>

using namespace std;

#include "Plane.h"

class SpringSystem{
private:
	vector<Spring *> springs;
	vector<Mass *> masses;
	Plane * plane;
	float damping;
	vec3 gravity;
	float deltaT;
	float groundHeight;
	vec3 collisionForce(Mass * m, vec3 newPos);
public:
	SpringSystem();
	void addSpring(Spring * s);
	void addMass(Mass * m);
	void setDamping(float d);
	void setGravity(vec3 g);
	void setDeltaT(float t);
	void addPlane(Plane * p);
	float getDeltaT();
	void simulate();
	void reset();
	//void enableGround(float height);
	void getMesh(vector<float> * buf);

};