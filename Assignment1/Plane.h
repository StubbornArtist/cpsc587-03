#pragma once
#include <glm/common.hpp>
#include<glm/glm.hpp>

using namespace glm;
class Plane {
private:
	vec3 normal;
	vec3 point;

public:
	Plane(vec3 norm, vec3 p);
	bool collision(vec3 p1, vec3 p2, vec3 * i);
	vec3 getNormal();
	vec3 getPoint();
};
