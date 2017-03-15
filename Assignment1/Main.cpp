/*Ashley Currie 10159991*/
#define _CRT_SECURE_NO_WARNINGS
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Mass.h"
#include "Spring.h"
#include "SpringSystem.h"
#include "Shader.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <vector>
#include <math.h>
#include <time.h>
#include <array>

#include <GLFW/glfw3.h>
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#define WIDTH 1000
#define HEIGHT 1000
#define M_PI 3.14

using namespace std;
using namespace glm;

void QueryGLVersion();
string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
void massSpringSim(SpringSystem * s);
void pendulumSim(SpringSystem * s);
void jelloSim(SpringSystem * s);
void clothSim(SpringSystem * s);
void SquareMesh(SpringSystem * sys, float div, float dim, int axis);

// --------------------------------------------------------------------------

// Functions to set up OpenGL shader programs for rendering
struct MyShader
{
	// OpenGL names for vertex and fragment shaders, shader program
	GLuint  vertex;
	GLuint  fragment;
	GLuint  program;
	GLuint mvpNum;
	mat4 mvp;
	// initialize shader and program names to zero (OpenGL reserved value)
	MyShader() : vertex(0), fragment(0), program(0), mvpNum(0)
	{}
};

//load, compile, and link shaders, returning true if successful
bool InitializeShaders(MyShader *shader)
{
	// load shader source from files
	string vertexSource = LoadSource(".\\vertex.glsl");
	string fragmentSource = LoadSource(".\\fragment.glsl");
	if (vertexSource.empty() || fragmentSource.empty()) return false;
	// compile shader source into shader objects
	shader->vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
	shader->fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
	// link shader program
	shader->program = LinkProgram(shader->vertex, shader->fragment);
	// check for OpenGL errors and return false if error occurred
	shader->mvpNum = glGetUniformLocation(shader->program, "mvp");
	return true;
}

// deallocate shader-related objects
void DestroyShaders(MyShader *shader)
{
	// unbind any shader programs and destroy shader objects
	glUseProgram(0);
	glDeleteProgram(shader->program);
	glDeleteShader(shader->vertex);
	glDeleteShader(shader->fragment);
}

void fillBuffers(uint * vertexArray, uint * colourBuffer, uint * vertexBuffer, vector<float> * vertices, vector<float> * colours){

	const GLuint VERTEX_INDEX = 0;
	const GLuint COLOUR_INDEX = 1;

	glGenBuffers(1, vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, *vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices->size() * sizeof(float), vertices->data(), GL_STATIC_DRAW);

	glGenBuffers(1, colourBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, *colourBuffer);
	glBufferData(GL_ARRAY_BUFFER, colours->size() * sizeof(float), colours->data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, vertexArray);
	glBindVertexArray(*vertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, *vertexBuffer);
	glVertexAttribPointer(VERTEX_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(VERTEX_INDEX);

	glBindBuffer(GL_ARRAY_BUFFER, *colourBuffer);
	glVertexAttribPointer(COLOUR_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(COLOUR_INDEX);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
	cout << "GLFW ERROR " << error << ":" << endl;
	cout << description << endl;
}

// PROGRAM ENTRY POINT
int main(int argc, char *argv[])
{
	// initialize the GLFW windowing system
	if (!glfwInit()) {
		cout << "ERROR: GLFW failed to initilize, TERMINATING" << endl;
		return -1;
	}
	glfwSetErrorCallback(ErrorCallback);
	// attempt to create a window with an OpenGL 4.1 core profile context
	GLFWwindow *window = 0;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(WIDTH, HEIGHT, "CPSC 587 Assignment 3", 0, 0);
	glfwMakeContextCurrent(window);
	glEnable(GL_DEPTH_TEST);
	// query and print out information about our OpenGL environment
	QueryGLVersion();
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	// call function to load and compile shader programs
	//MyShader shader;
	Shader * sh = new Shader();
	sh->load(".\\vertex.glsl", ".\\fragment.glsl");
	mat4 projection;
	mat4 view;
	vector<float> vertices;
	vector<float> colours;
	uint vertexArray = 0;
	uint colourBuffer = 0;
	uint vertexBuffer = 0;
	SpringSystem * sim1 = new SpringSystem();
	massSpringSim(sim1);
	SpringSystem * sim2 = new SpringSystem();
	pendulumSim(sim2);
	SpringSystem * sim3 = new SpringSystem();
	clothSim(sim3);

	//set up the projection matrix
	projection = glm::perspective((75* (float)M_PI / 180), (float)(WIDTH / HEIGHT), 100.0f, 0.1f);
	view = lookAt(vec3(0.0f, 0.0f, -10.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

	//InitializeShaders(&shader);
	fillBuffers(&vertexArray, &colourBuffer, &vertexBuffer, &vertices, &colours);
	clock_t diff = 0.0f;
	clock_t start = clock();
	bool init = true;
	while (!glfwWindowShouldClose(window))
	{	
		if (diff >= sim3->getDeltaT()) {
			sim3->simulate();
			sim3->getMesh(&vertices);
			start = clock();
		}
		if (init && vertices.size() > 0) {

			for (int i = 0; i < vertices.size()/3; i++) {
				colours.push_back(1.0f);
				colours.push_back(0.0f);
				colours.push_back(0.0f);
			}
			glBindBuffer(GL_ARRAY_BUFFER, colourBuffer);
			glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(float), colours.data(), GL_STATIC_DRAW);
			init = false;
		}
		glClearColor(0, 0, 0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(sh->getProgram());

		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
		glBindVertexArray(vertexArray);

		glUniformMatrix4fv(sh->getMVPNum(), 1, GL_FALSE, value_ptr(projection * view));

		glDrawArrays(GL_LINES, 0, vertices.size());
		glBindVertexArray(0);
		glUseProgram(0);
		glfwSwapBuffers(window);

		diff = clock() - start;
	}

	glDeleteVertexArrays(1,&vertexArray);
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &colourBuffer);

	//DestroyShaders(&shader);
	sh->destroy();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
void QueryGLVersion()
{
	// query opengl version and renderer information
	string version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	string glslver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
}

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
	string source;

	ifstream input(filename.c_str());
	if (input) {
		copy(istreambuf_iterator<char>(input),
			istreambuf_iterator<char>(),
			back_inserter(source));
		input.close();
	}
	else {
		cout << "ERROR: Could not load shader source from file "
			<< filename << endl;
	}
	return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
	// allocate shader object name
	GLuint shaderObject = glCreateShader(shaderType);

	// try compiling the source as a shader of the given type
	const GLchar *source_ptr = source.c_str();
	glShaderSource(shaderObject, 1, &source_ptr, 0);
	glCompileShader(shaderObject);

	// retrieve compile status
	GLint status;
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)
	{
		GLint length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		string info(length, ' ');
		glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
		cout << "ERROR compiling shader:" << endl << endl;
		cout << source << endl;
		cout << info << endl;
	}

	return shaderObject;
}

//creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
	// allocate program object name
	GLuint programObject = glCreateProgram();

	// attach provided shader objects to this program
	if (vertexShader)   glAttachShader(programObject, vertexShader);
	if (fragmentShader) glAttachShader(programObject, fragmentShader);

	// try linking the program with given attachments
	glLinkProgram(programObject);

	// retrieve link status
	GLint status;
	glGetProgramiv(programObject, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length;
		glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
		string info(length, ' ');
		glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
		cout << "ERROR linking shader program:" << endl;
		cout << info << endl;
	}

	return programObject;
}

void massSpringSim(SpringSystem * s) {
	Spring * s1 = new Spring();
	s1->setRestLength(2.0f);
	s1->setStiffness(10.0f);

	Mass * m1 = new Mass();
	m1->setWeight(2.0f);
	m1->setPosition(vec3(0.0f, 2.0f, 0.0f));
	m1->assertAnchored();

	Mass * m2 = new Mass();
	m2->setWeight(2.0f);
	m2->setPosition(vec3(0.0f, 0.0f, 0.0f));

	s1->setFirstMass(m1);
	s1->setSecondMass(m2);

	s->setGravity(vec3(0.0f, -9.81f, 0.0f));
	s->setDamping(0.5f);
	s->setDeltaT(0.01f);
	s->addSpring(s1);
	s->addMass(m1);
	s->addMass(m2);
}
void pendulumSim(SpringSystem * s) {
	Mass * m = new Mass();
	m->setPosition(vec3(-2.0f, 2.0f, 0.0f));
	m->setWeight(1.0f);
	m->assertAnchored();
	s->addMass(m);

	for (float i = -1.9f; i <= 2.0f; i += 0.1f) {
		Mass * m1 = new Mass();
		m1->setPosition(vec3(i, 2.0f, 0.0f));
		m1->setWeight(1.0f);

		Spring * s1 = new Spring();
		s1->setRestLength(0.1f);
		s1->setStiffness(1000.0);
		s1->setFirstMass(m);
		s1->setSecondMass(m1);

		s->addSpring(s1);
		s->addMass(m1);

		m = m1;
	}

	s->setGravity(vec3(0, -9.81, 0));
	s->setDamping(0.2f);
	s->setDeltaT(0.001f);
}
void jelloSim(SpringSystem * s) {
	int i, j;
	float x, y;
	for (i = 0, x = 2.0f; i < 20; i++, x -= 0.1f) {
		for (j = 0, y = 2.0f; j < 20; j++, y -= 0.1f) {
			


		}
	}


}
void clothSim(SpringSystem * s) {
	Mass * massGrid[20][20];
	float x,y;
	int i,j;
	for (x = -5.0f, i = 0; i < 20; x += 0.5f, i++) {
		for (y = -5.0f, j = 0; j < 20; y += 0.5f, j++) {
			Mass * m = new Mass();
			m->setWeight(1.0f);
			m->setPosition(vec3(x, y, 0.0f));
			massGrid[i][j] = m;
			s->addMass(m);
		}
	}
	massGrid[19][19]->assertAnchored();
	massGrid[0][19]->assertAnchored();

	for (int i = 0; i < 19; i++) {
		for (int j = 0; j < 19; j++) {
			Mass * m1 = massGrid[i][j];
			Mass * m2 = massGrid[i][j + 1];
			Mass * m3 = massGrid[i + 1][j];
			Mass * m4 = massGrid[i + 1][j + 1];

			Spring * sp = new Spring();
			Spring * sp2 = new Spring();
			Spring * sp3 = new Spring();
			Spring * sp4 = new Spring();

			sp->setFirstMass(m1);
			sp->setSecondMass(m2);
			sp->setStiffness(1000.0f);
			sp->setRestLength(0.5f);

			sp2->setFirstMass(m1);
			sp2->setSecondMass(m3);
			sp2->setStiffness(1000.0f);
			sp2->setRestLength(0.5f);
			
			sp3->setFirstMass(m1);
			sp3->setSecondMass(m4);
			sp3->setStiffness(1000.0f);
			sp3->setRestLength(sqrt(2.0f)/2.0f);

			sp4->setFirstMass(m2);
			sp4->setSecondMass(m3);
			sp4->setStiffness(1000.0f);
			sp4->setRestLength(sqrt(2.0f)/2.0f);
			
			if (j == 18) {
				Spring * leftSp = new Spring();
				leftSp->setFirstMass(m2);
				leftSp->setSecondMass(m4);
				leftSp->setStiffness(1000.0f);
				leftSp->setRestLength(0.5f);
				s->addSpring(leftSp);
			}
			if (i == 18) {
				Spring * botSp = new Spring();
				botSp->setFirstMass(m3);
				botSp->setSecondMass(m4);
				botSp->setStiffness(1000.0f);
				botSp->setRestLength(0.5f);
				s->addSpring(botSp);
			}
			s->addSpring(sp);
			s->addSpring(sp2);
			s->addSpring(sp3);
			s->addSpring(sp4);

		}
	}
	s->setDamping(0.2f);
	s->setGravity(vec3(0.0f, -9.81f, 0.0f));
	s->setDeltaT(0.0001f);
}

void SquareMesh(SpringSystem * sys, float div, float dim, int axis) {
	vector<vector<Mass *>> massGrid = vector<vector<Mass *>>();
	float x, y;
	int i, j;
	int numMass = (int)dim / div;
	for (x = dim/2.0f, i = 0; i < numMass; x -= div, i++) {
		massGrid.push_back(vector<Mass *>());
		for (y = dim/2.0f, j = 0; j < numMass; y -=div, j++) {
			Mass * m = new Mass();
			m->setWeight(1.0f);
			switch (axis) {
			case 1: m->setPosition(vec3(x, 0.0f, y)); break;
			case 2: m->setPosition(vec3(0.0f, x, y)); break;
			case 3: m->setPosition(vec3(x, y, 0.0f)); break;
			}
			massGrid.at(i).push_back(m);
			sys->addMass(m);
		}
	}
	for (int i = 0; i < numMass - 1; i++) {
		for (int j = 0; j < numMass - 1 ; j++) {
			Mass * m1 = massGrid.at(i).at(j);
			Mass * m2 = massGrid.at(i).at(j + 1);
			Mass * m3 = massGrid.at(i + 1).at(j);

			Spring * sp1 = new Spring(m1, m2, 1000.0f);
			Spring * sp2 = new Spring(m1, m3, 1000.0f);

			if (i == numMass - 2) {
				Mass * m4 = massGrid.at(i + 1).at(j + 1);
				Spring * botSp = new Spring(m3, m4, 1000.0f);
				sys->addSpring(botSp);
			}

			if (j == numMass - 2) {
				Mass * m4 = massGrid.at(i + 1).at(j + 1);
				Spring * rightSp = new Spring(m2, m4, 1000.0f);
				sys->addSpring(rightSp);

			}
			sys->addSpring(sp1);
			sys->addSpring(sp2);
		}
	}
}