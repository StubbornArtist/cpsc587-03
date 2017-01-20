#include "Node.h"


Node::Node(){
	globalModel = mat4(1.0f);
	translation = mat4(1.0f);
	rotation = mat4(1.0f);
	scale = mat4(1.0f);
}
void Node::transform(mat4 transform){
	globalModel = transform * translation * rotation * scale;
	for (int i = 0; i < children.size(); i++){
		children[i]->transform(globalModel);
	}
}
void Node::addChild(Node * n){
	children.push_back(n);
}
mat4 Node :: getGlobalTransform(){
	return globalModel;
}

void Node :: setTranslation(vec3 trans){
	translation = glm::translate(translation, trans);
}

void Node::setRotation(float angle, vec3 rot){
	rotation = glm::rotate(rotation, angle, rot);
}

void Node::setScale(vec3 s){
	scale = glm::scale(scale, s);
}
/**void Node::setTexture(Texture * tex){
	texture = tex;
}
Texture * Node::getTexture(){
	return texture;
}**/


