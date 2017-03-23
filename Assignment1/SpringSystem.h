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
	float deltaT;
	float groundHeight;
public:
	SpringSystem();
	void addSpring(Spring * s);
	void addMass(Mass * m);
	void setDamping(float d);
	void setDeltaT(float t);
	float getDeltaT();
	void simulate();
	void reset();
	void enableGround(float height);
	void getMesh(vector<float> * buf);

};