#include "SpringSystem.h"

SpringSystem::SpringSystem() {
	damping = 0.0f;
	deltaT = 0.0f;
	groundHeight = NULL;
	springs = vector<Spring *>();
	masses = vector<Mass *>();
}
void SpringSystem :: setDamping(float d) {
	damping = d;
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
//indicate where the ground is in the scene
void SpringSystem :: enableGround(float height) {
	groundHeight = height;
}
//perform one step of the simulation
void SpringSystem::simulate() {
	//set the forces for each spring
	for (int i = 0; i < springs.size(); i++) {
		springs[i]->updateInternalForce();
	}
	//calculate new position for each mass
	for (int i = 0; i < masses.size(); i++) {
		Mass * m = masses[i];
		//only update position if the mass is not anchored
		if (!m->isAnchored()) {
			vec3 p = m->getPosition();
			vec3 v = m->getVelocity();
			float w = m->getWeight();
			//calculate total force on the mass
			vec3 f = m->getForce() + (vec3(0.0f, -9.81f, 0.0f) * w) - (damping * v);
			vec3 newVel = v + f * (deltaT / w);
			vec3 newPos = p + deltaT * newVel;
			//if a collision occured apply an opposing force
			if (groundHeight == NULL || newPos.y > groundHeight) {
				m->setVelocity(newVel);
				m->setPosition(newPos);
			}
			else {
				//calculate the approximate fraction of deltaT in which the collision occured
				float tPrime = (groundHeight - p.y) / (newPos.y - p.y);
				//calculate the exact change in time since the previous simulation when the collision occured
				tPrime = tPrime * deltaT;
				//set the velocity and position accordingly
				m->setVelocity(-(v + f * (tPrime / w)));
				m->setPosition(p + tPrime * m->getVelocity());
			}
		}
		m->clearForce();
	}
}
//return all masses to their original positions
void SpringSystem :: reset() {
	for (int i = 0; i < masses.size(); i++) {
		masses.at(i)->reset();
	}
}
//fill a buffer with the points between the springs for drawing
void SpringSystem :: getMesh(vector<float> * buf) {
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
