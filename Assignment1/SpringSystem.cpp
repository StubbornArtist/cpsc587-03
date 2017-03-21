#include "SpringSystem.h"

SpringSystem::SpringSystem() {
	damping = 0.0f;
	gravity = vec3(0.0f);
	deltaT = 0.0f;
	simCount = 0;
	windOn = false;
	groundHeight = NULL;
	springs = vector<Spring *>();
	masses = vector<Mass *>();
}
void SpringSystem :: setDamping(float d) {
	damping = d;
}
void SpringSystem::setGravity(vec3 g) {
	gravity = g;
}
void SpringSystem :: addSpring(Spring * s) {
	springs.push_back(s);
}
void SpringSystem::addMass(Mass * m) {
	masses.push_back(m);
}
void SpringSystem::setDeltaT(float t) {
	deltaT = t;
}
float SpringSystem::getDeltaT() {
	return deltaT;
}
void SpringSystem :: enableWind() {
	windOn = true;
}
void SpringSystem :: enableGround(float height) {
	groundHeight = height;
}
void SpringSystem::simulate() {
	for (int i = 0; i < springs.size(); i++) {
		springs[i]->updateInternalForce();
	}
	for (int i = 0; i < masses.size(); i++) {
		Mass * m = masses[i];
		if (!m->isAnchored()) {
			vec3 p = m->getPosition();
			vec3 v = m->getVelocity();
			float w = m->getWeight();
			vec3 f = m->getForce() + (gravity * w) - (damping * v);
			vec3 newVel = v + f * (deltaT / w);
			vec3 newPos = p + deltaT * newVel;
			if (groundHeight == NULL || newPos.y > groundHeight) {
				m->setVelocity(newVel);
				m->setPosition(newPos);
			}
		}
		m->clearForce();
	}
}
void SpringSystem::reset() {
	for (int i = 0; i < masses.size(); i++) {
		masses.at(i)->reset();
	}
	simCount = 0;
}
void SpringSystem::getMesh(vector<float> * buf) {
	buf->clear();
	for (int i = 0; i < springs.size(); i++) {
		vec3 posOne = springs[i]->getFirstMass()->getPosition();
		vec3 posTwo = springs[i]->getSecondMass()->getPosition();

		buf->push_back(posOne.x);
		buf->push_back(posOne.y);
		buf->push_back(posOne.z);
		buf->push_back(posTwo.x);
		buf->push_back(posTwo.y);
		buf->push_back(posTwo.z);
	}
}
vec3 SpringSystem::wind(float t) {
	return vec3(rand());
}

