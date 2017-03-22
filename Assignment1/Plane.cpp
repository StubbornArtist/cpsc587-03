#include "Plane.h"

Plane::Plane(vec3 norm, vec3 p) {
	normal = norm;
	point = p;
}

bool Plane::collision(vec3 p1, vec3 p2, vec3 * i) {
	vec3 dir = normalize(p1 - p2);
	float d = dot(normal, dir);
	if (abs(d) > FLT_EPSILON) {
		float t = dot(point - p1, normal)/d;
		if (t >= 0.0f) {
			*i = p2 + dir * t;
			return true;
		}
	}
	return false;
}

vec3 Plane::getNormal() {
	return normal;
}

vec3 Plane::getPoint() {
	return point;
}