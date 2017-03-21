#pragma once
#include "Spring.h"
#include <vector>
#include <math.h>
#include <random>

using namespace std;

class SpringSystem{
private:
	vector<Spring *> springs;
	vector<Mass *> masses;
	float damping;
	vec3 gravity;
	float deltaT;
	bool windOn;
	int simCount;
	float groundHeight;
	vec3 wind(float t);
public:
	SpringSystem();
	void addSpring(Spring * s);
	void addMass(Mass * m);
	void setDamping(float d);
	void setGravity(vec3 g);
	void setDeltaT(float t);
	float getDeltaT();
	void enableWind();
	void simulate();
	void reset();
	void enableGround(float height);
	void getMesh(vector<float> * buf);

};