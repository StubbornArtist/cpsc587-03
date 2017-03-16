/*Ashley Currie 10159991*/
#define _CRT_SECURE_NO_WARNINGS
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Mass.h"
#include "Spring.h"
#include "SpringSystem.h"
#include "Shader.h"
#include "Geometry.h"

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
#define XZAXIS 1
#define YZAXIS 2
#define XYAXIS 3
#define NUMSIM 3

using namespace std;
using namespace glm;

void massSpringSim(SpringSystem * s);
void pendulumSim(SpringSystem * s);
void jelloSim(SpringSystem * s);
void clothSim(SpringSystem * s);
void squareMesh(vector<vector<Mass *>> * mesh, float div, float dim, int axis, float offset);


int simIndex = 0;
// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
	cout << "GLFW ERROR " << error << ":" << endl;
	cout << description << endl;
}

void QueryGLVersion()
{
	// query opengl version and renderer information
	string version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	string glslver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
}

void keys(GLFWwindow * window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT) {
			if (simIndex == 0) {
				simIndex = NUMSIM - 1;
			}
			else {
				simIndex--;
			}
		}
		if (key == GLFW_KEY_RIGHT) {
			simIndex = (simIndex + 1) % NUMSIM;
		}
	}
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
	glfwSetKeyCallback(window, keys);
	glEnable(GL_DEPTH_TEST);
	// query and print out information about our OpenGL environment
	QueryGLVersion();
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	// call function to load and compile shader programs
	Shader * sh = new Shader();
	Geometry * geom = new Geometry();
	mat4 projection;
	mat4 view;
	vector<float> vertices;
	vector<float> colours;
	vector<SpringSystem *> sims = { new SpringSystem(), new SpringSystem(), new SpringSystem() };
	//SpringSystem * sim1 = new SpringSystem();
	massSpringSim(sims[0]);
	//SpringSystem * sim2 = new SpringSystem();
	pendulumSim(sims[1]);
	//SpringSystem * sim3 = new SpringSystem();
	clothSim(sims[2]);

	//set up the projection matrix
	projection = glm::perspective((75* (float)M_PI / 180), (float)(WIDTH / HEIGHT), 100.0f, 0.1f);
	view = lookAt(vec3(0.0f, 0.0f, -10.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

	sims[simIndex]->getMesh(&vertices);
	for (int i = 0; i < vertices.size() / 3; i++) {
		colours.push_back(1.0f);
		colours.push_back(0.0f);
		colours.push_back(0.0f);
	}
	geom->initialize(vertices, colours);
	sh->load(".\\vertex.glsl", ".\\fragment.glsl");

	clock_t diff = 0.0f;
	clock_t start = clock();
	int prevSimIndex = 0;
	while (!glfwWindowShouldClose(window))
	{	
		if (diff >= sims[simIndex]->getDeltaT()) {
			sims[simIndex]->simulate();
			sims[simIndex]->getMesh(&vertices);
			geom->reloadVertices(vertices);
			start = clock();
		}
		if (prevSimIndex != simIndex) {
			colours.clear();
			for (int i = 0; i < vertices.size() / 3; i++) {
				colours.push_back(1.0f);
				colours.push_back(0.0f);
				colours.push_back(0.0f);
			}
			geom->reloadColours(colours);
			prevSimIndex = simIndex;
		}
		glClearColor(0, 0, 0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(sh->getProgram());
		glPointSize(9.0f);

		glUniformMatrix4fv(sh->getMVPNum(), 1, GL_FALSE, value_ptr(projection * view));
		geom->draw(GL_LINES);
		geom->draw(GL_POINTS);
		glUseProgram(0);
		glfwSwapBuffers(window);

		diff = clock() - start;
	}
	sh->destroy();
	geom->destroy();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
void massSpringSim(SpringSystem * s) {
	Mass * m1 = new Mass(vec3(0.0f, 2.0f, 0.0f), 2.0f, true);
	Mass * m2 = new Mass(vec3(0.0f, 0.0f, 0.0f), 2.0f, false);
	Spring * s1 = new Spring(m1, m2, 10.0f, 0.5f);

	s->setGravity(vec3(0.0f, -9.81f, 0.0f));
	s->setDamping(0.7f);
	s->setDeltaT(0.001f);
	s->addSpring(s1);
	s->addMass(m1);
	s->addMass(m2);
}
void pendulumSim(SpringSystem * s) {
	Mass * m = new Mass(vec3(-2.0f, 2.0f, 0.0f), 1.0f, true);
	s->addMass(m);

	for (float i = -1.9f; i <= 2.0f; i += 0.1f) {
		Mass * m1 = new Mass(vec3(i, 2.0f, 0.0f), 1.0f, false);
		Spring * s1 = new Spring(m, m1, 1000.0f, 0.5f);
		s->addSpring(s1);
		s->addMass(m1);

		m = m1;
	}
	s->setGravity(vec3(0, -9.81, 0));
	s->setDamping(0.2f);
	s->setDeltaT(0.0001f);
}
void jelloSim(SpringSystem * s, float width, float seg) {
	vector<vector<Mass *>> massGrid = vector<vector<Mass *>>();
	int numSegs = (int)(width / seg);
	float h;
	int j;
	for (int i = 0; i < numSegs * 4; i++) {
		for (j = 0, h = -(width/2.0f); j < numSegs; j++, h +=seg) {
			if (i / numSegs < 1.0f) {
				vec3 pos = vec3(i* seg - (width / 2.0f), 0.0f, h);
			}
			else if (i / numSegs < 2.0f) {

			}




		}
	}

}
void clothSim(SpringSystem * s) {
	Mass * massGrid[20][20];
	float x,y;
	int i,j;
	for (x = -5.0f, i = 0; i < 20; x += 0.5f, i++) {
		for (y = -5.0f, j = 0; j < 20; y += 0.5f, j++) {
			Mass * m = new Mass(vec3(x, 0.0f, y), 1.0f, (j == 19 && (i == 19 || i == 0))? true : false);
			massGrid[i][j] = m;
			s->addMass(m);
		}
	}
	for (int i = 0; i < 19; i++) {
		for (int j = 0; j < 19; j++) {
			Mass * m1 = massGrid[i][j];
			Mass * m2 = massGrid[i][j + 1];
			Mass * m3 = massGrid[i + 1][j];
			Mass * m4 = massGrid[i + 1][j + 1];

			Spring * sp = new Spring(m1, m2, 9000.0f, 0.8f);
			Spring * sp2 = new Spring(m1, m3, 9000.0f, 0.8f);
			Spring * sp3 = new Spring(m1,  m4, 1000.0f, 0.8f);
			Spring * sp4 = new Spring(m2, m3, 1000.0f, 0.8f);
			
			if (j == 18) {
				Spring * leftSp = new Spring(m2, m4, 9000.0f, 0.8f);
				s->addSpring(leftSp);
			}
			if (i == 18) {
				Spring * botSp = new Spring(m3, m4, 9000.0f, 0.8f);
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
	s->setDeltaT(0.0009f);
}

void squareMesh(vector<vector<Mass *>> * mesh, float div, float width, int axis, float offset) {
	float x, y;
	int i, j;
	int numMass = (int)width / div;
	for (x = width / 2.0f, i = 0; i < numMass; x -= div, i++) {
		mesh->push_back(vector<Mass *>());
		for (y = width / 2.0f, j = 0; j < numMass; y -= div, j++) {
			Mass * m = new Mass();
			m->setWeight(1.0f);
			switch (axis) {
			case 1: m->setPosition(vec3(x, offset, y)); break;
			case 2: m->setPosition(vec3(offset, x, y)); break;
			case 3: m->setPosition(vec3(x, y, offset)); break;
			}
			mesh->at(i).push_back(m);
		}
	}
}
