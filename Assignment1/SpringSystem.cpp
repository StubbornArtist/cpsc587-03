#include "SpringSystem.h"

SpringSystem::SpringSystem() {
	damping = 0.0f;
	gravity = vec3(0.0f);
	deltaT = 0.0f;
	plane = NULL;
	//groundHeight = NULL;
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

void SpringSystem :: addPlane(Plane * p) {
	plane = p;
}
//void SpringSystem :: enableGround(float height) {
	//groundHeight = height;
//}
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
			vec3 f = m->getForce() + (gravity * w) - (damping * v);
			vec3 newVel = v + f * (deltaT / w);
			vec3 newPos = p + deltaT * newVel;
			//if a collision occured apply an opposing force
			f += collisionForce(m, newPos);
			//change the velocity and position of the mass accordingly
			m->setVelocity(v + f * (deltaT/w));
			m->setPosition(p + deltaT * m->getVelocity());
		}
		m->clearForce();
	}
}
//return all masses to their original positions
void SpringSystem::reset() {
	for (int i = 0; i < masses.size(); i++) {
		masses.at(i)->reset();
	}
}
//fill a buffer with the points between the springs for drawing
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
//calculate the force of collision if any collision occured
vec3 SpringSystem::collisionForce(Mass * m, vec3 nextPos) {
	vec3 p = m->getPosition();
	vec3 v = m->getVelocity();
	float w = m->getWeight();
	vec3 inter = vec3(0.0f);
	if (plane != NULL && plane->collision(p, nextPos, &inter)) {
		vec3 pN = plane->getNormal();
		vec3 pP = plane->getPoint();
		//apply force to push mass away from the plane
		vec3 f = pN * -sign(dot(pN, normalize(inter - pP))) * -10.0f * dot((inter - pP), pN);
		return f;
	}
	return vec3(0.0f);
}
/*
if (groundHeight == NULL || newPos.y > groundHeight) {
	m->setVelocity(newVel);
	m->setPosition(newPos);
}
else {
	float tPrime = (groundHeight - p.y) / (newPos.y - p.y);
	tPrime = tPrime * deltaT;
	m->setVelocity(-newVel);
	m->setPosition(p + tPrime * m->getVelocity());
}*/