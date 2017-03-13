#include "SpringSystem.h"

SpringSystem::SpringSystem() {
	damping = 0;
	gravity = vec3(0);
	deltaT = 0;
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
void SpringSystem::simulate() {
	for (int i = 0; i < masses.size(); i++) {
		masses[i]->clearForce();
	}
	for (int i = 0; i < springs.size(); i++) {
		springs[i]->updateInternalForce();
	}
	for (int i = 0; i < masses.size(); i++) {
		Mass * m = masses[i];
		if (!m->isAnchored()) {
			vec3 v = m->getVelocity();
			float w = m->getWeight();
	
			m->setVelocity(v + (m->getForce() + (gravity * w) - (damping * v)) * (deltaT / w));
			m->setPosition(m->getPosition() + deltaT * m->getVelocity());
		}
	}

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

