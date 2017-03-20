#include "Mass.h"

Mass::Mass() {
	weight = 0;
	position,origPos = vec3(0);
	velocity = vec3(0);
	force = vec3(0);
	anchored = false;
}
Mass::Mass(vec3 pos, float w, bool anchor) {
	position = pos;
	origPos = pos;
	weight = w;
	anchored = anchor;
	velocity = vec3(0);
	force = vec3(0);
}
vec3 Mass::getPosition() {
	return position;
}
void Mass::setPosition(vec3 p) {
	position = p;
}
float Mass::getWeight() {
	return weight;
}
void Mass::setWeight(float w) {
	weight = w;
}
vec3 Mass::getVelocity() {
	return velocity;
}
void Mass::setVelocity(vec3 v) {
	velocity = v;
}
vec3 Mass::getForce() {
	return force;
}
void Mass::addToForce(vec3 f) {
	force += f;
}
void Mass::clearForce() {
	force = vec3(0);
}
bool Mass :: isAnchored(){
	return anchored;
}
void Mass::assertAnchored() {
	anchored = true;
}

void Mass::reset() {
	position = origPos;
	setVelocity(vec3(0.0f));
	clearForce();
}

