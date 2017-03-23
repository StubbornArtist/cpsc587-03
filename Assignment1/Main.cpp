/*Ashley Currie 10159991*/

//remove before submission
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

//add before submission
/*
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
*/
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
#define NUMSIM 4

using namespace std;
using namespace glm;

void massSpringSim(SpringSystem * s);
void pendulumSim(SpringSystem * s);
void jelloSim(SpringSystem * s, float width, float seg);
void clothSim(SpringSystem * s);
void connectMasses(vector<Mass *> masses, SpringSystem * s, float maxDist, float k, float d);
void squareMesh(vector<Mass *> * massBuf, SpringSystem * s, float div, float width, int axis, float offset);

//the current simulation number (there are four in total)
int simIndex = 0;

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
	cout << "GLFW ERROR " << error << ":" << endl;
	cout << description << endl;
}
//Query opengl version and renderer information
void QueryGLVersion()
{
	string version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	string glslver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
}
//A key callback function to switch between simulations
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
		else if (key == GLFW_KEY_RIGHT) {
			simIndex = (simIndex + 1) % NUMSIM;
		}
	}
}
//Perform a single draw of the vertices contained in the given Geometry using the given Shader 
void drawScene(Geometry * g, Shader * sh, mat4 mvp, GLFWwindow * w) {
	glClearColor(0, 0, 0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(sh->getProgram());
	glPointSize(9.0f);

	glUniformMatrix4fv(sh->getMVPNum(), 1, GL_FALSE, value_ptr(mvp));

	g->draw(GL_LINES);
	glUseProgram(0);
	glfwSwapBuffers(w);

}
//Initialize GLFW and the main window
//Set the error and key callback functions
GLFWwindow * initScene() {
	// initialize the GLFW windowing system
	if (!glfwInit()) {
		cout << "ERROR: GLFW failed to initilize, TERMINATING" << endl;
	}
	glfwSetErrorCallback(ErrorCallback);
	GLFWwindow * w = 0;
	// attempt to create a window with an OpenGL 4.1 core profile context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	w = glfwCreateWindow(WIDTH, HEIGHT, "CPSC 587 Assignment 3", 0, 0);
	glfwMakeContextCurrent(w);
	glfwSetKeyCallback(w, keys);
	glEnable(GL_DEPTH_TEST);
	QueryGLVersion();

	//remove before submission
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	return w;
}
//Cleanup then terminate the program
void destroyScene(Geometry * g, Shader * s, GLFWwindow * w) {
	s->destroy();
	g->destroy();
	glfwDestroyWindow(w);
	glfwTerminate();
}

// PROGRAM ENTRY POINT
int main(int argc, char *argv[])
{
	GLFWwindow * window = initScene();
	Shader * sh = new Shader();
	Geometry * geom = new Geometry();
	mat4 mvp;
	vector<float> vertices;
	vector<float> colours;
	vector<SpringSystem *> sims = { new SpringSystem(), new SpringSystem(), new SpringSystem(), new SpringSystem()};
	//generate each of the mass spring simulations
	massSpringSim(sims[0]);
	pendulumSim(sims[1]);
	jelloSim(sims[2], 4.0f, 0.5f);
	clothSim(sims[3]);

	//set up the view and projection matrices
	mvp = glm::perspective((75* (float)M_PI / 180), (float)(WIDTH / HEIGHT), 100.0f, 0.1f) *
		lookAt(vec3(0.0f, 0.0f, -10.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	sh->load("vertex.glsl", "fragment.glsl");

	clock_t diff;
	clock_t start;
	int prevSimIndex = -1;
	while (!glfwWindowShouldClose(window))
	{	
		//if a left/right arrow has been pressed move to the previous/next simulation
		if (prevSimIndex != simIndex) {
			if (prevSimIndex != - 1) sims[simIndex]->reset();
			//retrieve the vertices 
			sims[simIndex]->getMesh(&vertices);
			//refill the colour buffer
			colours.clear();
			for (int i = 0; i < vertices.size() / 3; i++) {
				colours.push_back(1.0f);
				colours.push_back(0.0f);
				colours.push_back(0.0f);
			}
			geom->reloadColours(colours);
			geom->reloadVertices(vertices);
			prevSimIndex = simIndex;
			//reset timer
			diff = 0.0f;
			start = clock();
		}
		//if deltaT time has passed then do the next simulation 
		//step for the current simulation
		if (diff >= sims[simIndex]->getDeltaT()) {
			sims[simIndex]->simulate();
			sims[simIndex]->getMesh(&vertices);
			geom->reloadVertices(vertices);
			diff -= sims[simIndex]->getDeltaT();
			start = clock();
		}
		//draw the vertices
		drawScene(geom, sh, mvp, window);
		//check for key events
		glfwPollEvents();

		diff += clock() - start;
		start = clock();
	}
	//cleanup then terminate
	destroyScene(geom, sh, window);
	return 0;
}

//---Functions that generate the mass spring systems displayed in this project---//
//Simple simulation of a mass on a spring
void massSpringSim(SpringSystem * s) {
	Mass * m1 = new Mass(vec3(0.0f, 2.0f, 0.0f), 2.0f, true);
	Mass * m2 = new Mass(vec3(0.0f, 0.0f, 0.0f), 2.0f, false);
	Spring * s1 = new Spring(m1, m2, 10.0f, 0.1f);

	s->setDeltaT(0.001f);
	s->addSpring(s1);
	s->addMass(m1);
	s->addMass(m2);
}
//Simulation of a chain of masses and springs
void pendulumSim(SpringSystem * s) {
	Mass * m = new Mass(vec3(-2.0f, 5.0f, 0.0f), 1.0f, true);
	s->addMass(m);

	for (float i = -1.9f; i <= 2.0f; i += 0.1f) {
		Mass * m1 = new Mass(vec3(i, 5.0f, 0.0f), 1.0f, false);
		Spring * s1 = new Spring(m, m1, 5000.0f, 0.5f);
		s->addSpring(s1);
		s->addMass(m1);

		m = m1;
	}
	s->setDamping(0.2f);
	s->setDeltaT(0.001f);
}
//Simulation of a cube of jello
void jelloSim(SpringSystem * s, float width, float seg) {
	vector<Mass *> masses = vector<Mass *>();
	float yOffset = width / 2.0f;
	float xSegs = width/seg;
	int i;
	//generate equally spaced square meshes of masses of width x width dimensions along the XZ axis
	for (int i = 0; i < xSegs; i++, yOffset-=seg) {
		squareMesh(&masses, s, seg, width, XZAXIS, yOffset);
	}
	//connect these square meshes with springs
	connectMasses(masses, s, sqrt(2 * seg * seg) , 1000.0f, 0.5f);

	s->setDamping(2.0f);
	s->setDeltaT(0.0009f);
	s->enableGround(-4.0f);
}
//Simulation of a hanging cloth
void clothSim(SpringSystem * s) {
	vector<Mass *> masses = vector<Mass *>();
	float x,y;
	int i,j;
	for (x = -5.0f, i = 0; i < 20; x += 0.5f, i++) {
		for (y = -5.0f, j = 0; j < 20; y += 0.5f, j++) {
			Mass * m = new Mass(vec3(x, 5.0f, y), 1.0f, (j == 19 && (i == 19 || i == 0))? true : false);
			masses.push_back(m);
			s->addMass(m);
		}
	}
	for (int h = 0; h < masses.size(); h++) {
		for (int k = h + 1; k < masses.size(); k++) {
			float d = length(masses.at(h)->getPosition() - masses.at(k)->getPosition());
			if (d <= sqrt(2.0f)/2.0f) {
				s->addSpring(new Spring(masses.at(h), masses.at(k), ((d == 0.5f) ? 9000.0f : 1000.0f), 0.8f));
			}
		}
	}
	s->setDamping(0.2f);
	s->setDeltaT(0.0009f);
}
//---Functions that help with the generation of the mass spring systems---//

//Connect all masses, in the masses vector given, that are less than or exactly a given distance apart
void connectMasses(vector<Mass *> masses, SpringSystem * s, float maxDist, float k, float d) {
	for (int h = 0; h < masses.size(); h++) {
		for (int k = h + 1; k < masses.size(); k++) {
			float d = length(masses.at(h)->getPosition() - masses.at(k)->getPosition());
			if (d <= maxDist) {
				s->addSpring(new Spring(masses.at(h), masses.at(k), k, d));
			}
		}
	}
}
//Generate a square mesh of dimensions width x width made up of masses along the XZ, XY, or YZ axis
void squareMesh(vector<Mass *> * massBuf, SpringSystem * s, float div, float width, int axis, float offset) {
	float x, y;
	int i, j;
	int numMass = (int)width / div;
	for (x = width / 2.0f, i = 0; i <= numMass; x -= div, i++) {
		for (y = width / 2.0f, j = 0; j <= numMass; y -= div, j++) {
			vec3 pos;
			//determine position of the mass based upon which two axes the square is on
			switch (axis) {
			case 1: pos = vec3(x, offset, y); break;
			case 2: pos = vec3(offset, x, y); break;
			case 3: pos = vec3(x, y, offset); break;
			}
			Mass * m = new Mass(pos, 1.0f, false);
			s->addMass(m);
			massBuf->push_back(m);
		}
	}
}
