#include <glm/common.hpp>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<vector>
#include "Texture.h"

using namespace glm;
using namespace std;

class Node{
public:
	uint vertexBuffer;
	uint colourBuffer;
	uint vertexArray;
	vector<vec3> vertices;
	void transform(mat4 transform);
	void addChild(Node * child);
	vector<vec3> getGeometry();
	void setRotation(float angle, vec3 rot);
	void setTranslation(vec3 trans);
	void setScale(vec3 scale);
	mat4 getGlobalTransform();
	//void setTexture(Texture * tex);
	///Texture * getTexture();
	bool lit;
	Node();

private:
	mat4 translation;
	mat4 rotation;
	mat4 scale;
	mat4 globalModel;
	vector<Node *> children;
	//Texture * texture;
};