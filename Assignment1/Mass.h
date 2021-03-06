#pragma once
#include <glm/common.hpp>
#include <glm/glm.hpp>

using namespace glm;

class Mass {
public:
	Mass();
	Mass(vec3 pos, float weight, bool anchored);
	vec3 getPosition();
	void setPosition(vec3 p);
	float getWeight();
	void setWeight(float w);
	vec3 getVelocity();
	void setVelocity(vec3 v);
	vec3 getForce();
	void addToForce(vec3 f);
	void clearForce();
	bool isAnchored();
	void assertAnchored();
	void reset();
private:
	float weight;
	vec3 position;
	vec3 origPos;
	vec3 velocity;
	vec3 force;
	bool anchored;
};
