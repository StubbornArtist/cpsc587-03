#include "Spring.h"

Spring::Spring() {
	k = 0.0f;
	m1 = 0;
	m2 = 0;
	restLen = 0;
}
float Spring::getStiffness() {
	return k;
}
void Spring::setStiffness(float s) {
	k = s;
}
Mass * Spring::getFirstMass() {
	return m1;
}
void Spring::setFirstMass(Mass * m) {
	m1 = m;
}
Mass * Spring::getSecondMass() {
	return m2;
}
void Spring::setSecondMass(Mass * m) {
	m2 = m;
}
float Spring::getRestLength() {
	return restLen;
}
void Spring::setRestLength(float r) {
	restLen = r;
}
vec3 Spring::updateInternalForce() {
	vec3 curLen = m2->getPosition() - m1->getPosition();
	vec3 f =  -k * (length(curLen) - restLen) * normalize(curLen);
	m1->addToForce(-f);
	m2->addToForce(f);
	return f;
}

