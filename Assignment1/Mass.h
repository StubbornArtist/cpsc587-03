#pragma once
#include <glm/common.hpp>
#include <glm/glm.hpp>

using namespace glm;

class Mass {
public:
	Mass();
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
private:
	float weight;
	vec3 position;
	vec3 velocity;
	vec3 force;
	bool anchored;
};
