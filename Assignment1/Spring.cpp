#include "Spring.h"

Spring::Spring() {
	k = 0.0f;
	m1 = 0;
	m2 = 0;
	damp = 0.0f;
	restLen = 0;
}
Spring::Spring(Mass * mOne, Mass * mTwo, float stiffness, float d) {
	m1 = mOne;
	m2 = mTwo;
	k = stiffness;
	damp = d;
	restLen = length(m1->getPosition() - m2->getPosition());
}
float Spring::getStiffness() {
	return k;
}
void Spring::setStiffness(float s) {
	k = s;
}
float Spring :: getDamping() {
	return damp;
}
void Spring :: setDamping(float d) {
	damp = d;
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
//update the forces on each mass in this spring
void Spring::updateInternalForce() {
	vec3 curLen = m1->getPosition() - m2->getPosition();
	vec3 f =  -k * (length(curLen) - restLen) * normalize(curLen);
	//add force exterted by the spring to each mass attached to it
	m1->addToForce(f);
	m2->addToForce(-f);
	//add damping to each mass 
	if (!(f.x == 0.0f && f.y == 0.0f && f.z == 0.0f)) {
		vec3 fn = normalize(f);
		vec3 fd = -damp * (dot(m1->getVelocity() - m2->getVelocity(), fn) / dot(fn, fn)) * fn;
		m1->addToForce(fd);
		m2->addToForce(-fd);
	}
}

