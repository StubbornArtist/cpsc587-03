#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GLFW\glfw3.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iterator>

using namespace glm;
using namespace std;
class Shader {

private:
	GLuint vertex;
	GLuint fragment;
	GLuint program;
	GLuint mvpNum;
	string loadSource(string name);
	GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);
	GLuint compile(GLenum shaderType, const string &source);

public:
	void load(string vertexPath, string fragPath);
	void destroy();
	GLuint getProgram();
	GLuint getMVPNum();
};