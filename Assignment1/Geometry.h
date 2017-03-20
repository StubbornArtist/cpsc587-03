#pragma once
#include <GL/glew.h>
#include <GLFW\glfw3.h>
#include<vector>

using namespace std;

class Geometry {
private:
	GLuint vertexArray;
	GLuint vertexBuffer;
	GLuint colourBuffer;
	int verticeCount;
public:
	Geometry();
	void initialize();
	void reloadVertices(vector<float> vertices);
	void reloadColours(vector<float> colours);
	void draw(GLenum mode);
	void destroy();
};