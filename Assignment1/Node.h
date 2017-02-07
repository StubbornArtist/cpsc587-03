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
	uint textureBuffer;
	vector<vec3> vertices;
	vector<vec3> colours;
	vector<float> map;
	Texture tex;
	void transform(mat4 transform);
	void addChild(Node * child);
	void setRotation(float angle, vec3 rot);
	void setRotation(mat4 rot);
	void setTranslation(vec3 trans);
	void setScale(vec3 scale);
	mat4 getGlobalTransform();
	Node();

private:
	mat4 translation;
	mat4 rotation;
	mat4 scale;
	mat4 globalModel;
	vector<Node *> children;
};