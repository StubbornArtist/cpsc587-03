/*Ashley Currie 10159991*/
/*Use the left and right arrows to switch between assignment components.
Use the up and down arrows to increase/decrease the number of iterations*/

#define _CRT_SECURE_NO_WARNINGS
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Node.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <vector>
#include <math.h>
#include <GLFW/glfw3.h>
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define M_PI   3.14159265358979323846
#define DT 0.0001
#define DELTA_T 0.001
#define SPEED 5


using namespace std;
using namespace glm;

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
void makeCylinder(vec3 start, float radius, float height, int segs, vector<vec3> * vertices);
float arcLength(vector<vec3> vertices);
void position(float t, vector<vec3> controlPoints, vector<float> knots, int degree, vec3 * point);
void closedKnots(int num, vector<float> * knots);
void bspline(vector<vec3> controlPoints, vector<float> knots, int degree, int segs, vector<vec3> * vertices);
float basis(int i, int k, vector<float> knots, float t);
float move(float deltaS);
void tangent(float t, vector<vec3> controlPoints, vector<float> knots, int degree, vec3 * tangent);
void makeCart(vector<vec3> * vertices);

const float WIDTH = 2000;
const float HEIGHT = 2000;

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
	CheckGLErrors();
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

//get a given number of floats from a line of characters and put them in a given vector
void extractFloats(string str, int num, std::vector<float> * data){
	stringstream ss(str);
	float f;
	for (int i = 0; i < num; i++){
		ss >> f;
		data->push_back(f);
	}

}
void readControlPoints(string objectFile,vector<vec3> * points ){
	string objects = LoadSource(objectFile);
	stringstream ss(objects);
	string line;
	vector<float> data;

	while (getline(ss, line)){
		if (line[0] != '#' && line != ""){
			extractFloats(line, 3, &data);
			points->push_back(vec3(data.at(0), data.at(1), data.at(2)));
			data.clear();
		}
	}
}
void initNode(Node * node){
	vector<GLfloat> vert = vector<GLfloat>();
	vector<GLfloat> colours = vector<GLfloat>();
	for (unsigned int i = 0; i < node->vertices.size(); i++){
		vert.push_back(node->vertices.at(i).x);
		vert.push_back(node->vertices.at(i).y);
		vert.push_back(node->vertices.at(i).z);
		colours.push_back(1);
		colours.push_back(0);
		colours.push_back(0);
	}
	const GLuint VERTEX_INDEX = 0;
	const GLuint COLOUR_INDEX = 1;

	glGenBuffers(1, &node->vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, node->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vert.size() * sizeof(GLfloat), vert.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &node->colourBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, node->colourBuffer);
	glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(GLfloat), colours.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &node->vertexArray);
	glBindVertexArray(node->vertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, node->vertexBuffer);
	glVertexAttribPointer(VERTEX_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(VERTEX_INDEX);

	glBindBuffer(GL_ARRAY_BUFFER, node->colourBuffer);
	glVertexAttribPointer(COLOUR_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(COLOUR_INDEX);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

struct MyContext{

	GLfloat xOrigin;
	GLfloat yOrigin;
	GLfloat phi;
	GLfloat theta;
	GLfloat ztrans;
	bool dragging;
	bool animate;

	MyContext(): xOrigin(0), yOrigin(0),ztrans(0), phi(2 * M_PI), theta(M_PI), dragging(false), animate(true){}

};
MyContext * GetContext(GLFWwindow * w){
	return static_cast<MyContext *>(glfwGetWindowUserPointer(w));
}

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
	cout << "GLFW ERROR " << error << ":" << endl;
	cout << description << endl;
}

// ==========================================================================
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

	window = glfwCreateWindow(WIDTH, HEIGHT, "CPSC 587 Assignment 1", 0, 0);
	if (!window) {
		cout << "Program failed to create GLFW window, TERMINATING" << endl;
		glfwTerminate();
		return -1;
	}
	//mouse input
	MyContext context;
	glfwSetWindowUserPointer(window, &context);
	auto keys = [](GLFWwindow * window, int key, int scancode, int action, int mods){
		MyContext * con = GetContext(window);
		if (action == GLFW_PRESS){
			if (key == GLFW_KEY_EQUAL){
				con->ztrans += 0.1f;
			}
			else if (key == GLFW_KEY_MINUS){
				con->ztrans -= 0.1f;
			}
		}
	};
	auto mouseMove = [](GLFWwindow * window, double xpos, double ypos){
		MyContext * con = GetContext(window);
		if (con->dragging){
			con->phi+= (xpos - con->xOrigin) * 0.01f;
			con->theta += (ypos - con->yOrigin) * 0.01f;
			if (con->phi > 2.0f * M_PI){
				con->phi = 0.0f;
			}
			if (con->theta > M_PI){
				con->theta = 0.0f;
			}
			con->xOrigin = xpos;
			con->yOrigin = ypos;
		}
	};
	auto mouseButton = [](GLFWwindow * window, int button, int action, int mods){
		MyContext * con = GetContext(window);
		if (button == GLFW_MOUSE_BUTTON_LEFT){
			if (action == GLFW_PRESS){
				double x;
				double y;
				glfwGetCursorPos(window, &x, &y);
				con->dragging = true;
				con->xOrigin = x;
				con->yOrigin = y;
			}
			else if (action == GLFW_RELEASE){
				con->dragging = false;
			}
		}
	};
	// set keyboard callback function and make our context current (active)
	glfwSetKeyCallback(window, keys);
	glfwSetCursorPosCallback(window, mouseMove);
	glfwSetMouseButtonCallback(window, mouseButton);
	glfwMakeContextCurrent(window);
	glEnable(GL_DEPTH_TEST);

	// query and print out information about our OpenGL environment
	QueryGLVersion();

	//initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	// call function to load and compile shader programs
	MyShader shader;
	mat4 projection;
	mat4 view;
	view = lookAt(vec3(0, 0, -4), vec3(0, 0, 3), vec3(0, 1, 0));
	projection = perspective((50 * (float)M_PI / 180), WIDTH / HEIGHT, 1000.0f, 0.1f);
	
	//create scene objects
	vector<Node *> sceneObjects = vector<Node *>();
	vector<vec3> controlPoints;
	vector<float> knots;
	float length;
	Node * root = new Node();
	Node * track = new Node();

	root->addChild(track);
	track->drawingPrimitive = GL_LINE_LOOP;
	sceneObjects.push_back(track);

	readControlPoints("test.txt", &controlPoints);
	closedKnots(4 + controlPoints.size() + 1, &knots);
	bspline(controlPoints, knots, 4, 100, &(track->vertices));
	length = arcLength(track->vertices);
	track->setScale(vec3(0.05f));
	
	InitializeShaders(&shader);
	for (int i = 0; i < sceneObjects.size(); i++){
		initNode(sceneObjects.at(i));
	}

	// run an event-triggered main loop
	while (!glfwWindowShouldClose(window))
	{	
		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shader.program);
	
		mat4 transform = translate(mat4(1.0f), vec3(0, 0, context.ztrans)) * rotate(mat4(1.0f),
			((float)(2 * M_PI) - context.phi), vec3(0, 1, 0)) * rotate(mat4(1.0f), (float)context.theta, vec3(0, 0, 1));
		root->transform(transform);

		//later speed will be calculated
		float deltaS = SPEED * DELTA_T;
		float t = move(deltaS);
		for (int i = 0; i < sceneObjects.size(); i++){
			glBindVertexArray(sceneObjects.at(i)->vertexArray);
			//calculate mvp matrix for this object
			shader.mvp = projection * view * sceneObjects.at(i)->getGlobalTransform();
			glUniformMatrix4fv(shader.mvpNum, 1, GL_FALSE, value_ptr(shader.mvp));
			//draw triangles for current shape
			glDrawArrays(sceneObjects.at(i)->drawingPrimitive, 0, sceneObjects.at(i)->vertices.size());
			glBindVertexArray(0);
		}
		glUseProgram(0);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	for (int i = 0; i < sceneObjects.size(); i++){
		glDeleteVertexArrays(1, &sceneObjects.at(i)->vertexArray);
		glDeleteBuffers(1, &sceneObjects.at(i)->vertexBuffer);
		glDeleteBuffers(1, &sceneObjects.at(i)->colourBuffer);
	}
	DestroyShaders(&shader);
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
	// query opengl version and renderer information
	string version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	string glslver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

	cout << "OpenGL [ " << version << " ] "
		<< "with GLSL [ " << glslver << " ] "
		<< "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors()
{
	return true;
}
// --------------------------------------------------------------------------
// OpenGL shader support functions

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

void makeCylinder(vec3 start, float radius, float height, int segs, vector<vec3> * vertices){

	for (float i = 0.0f; i < 2 * M_PI; i+= (2 * M_PI/ segs)){

		vertices->push_back(vec3(radius * cos(i) + start.x, radius * sin(i) + start.y, start.z));
		vertices->push_back(vec3(radius * cos(i) + start.x, radius * sin(i) + start.y, height + start.z));
		vertices->push_back(vec3(radius * cos(i + (2 * M_PI / segs)) + start.x, radius * sin(i + (2 * M_PI / segs)) + start.y, start.z));

		vertices->push_back(vec3(radius * cos(i + (2 * M_PI / segs)) + start.x, radius * sin(i + (2 * M_PI / segs)) + start.y, start.z));
		vertices->push_back(vec3(radius * cos(i) + start.x, radius * sin(i) + start.y, height + start.z));
		vertices->push_back(vec3(radius * cos(i + (2 * M_PI / segs)) + start.x, radius * sin(i + (2 * M_PI / segs)) + start.y, height + start.z));
	}
}
float reParametrize(vector<vec3> vertices, vector<float> * uValues, float length){
	float deltaS = length;
	float u = 0.0f;
	int count = 0;
	while (u < 1.0f){
		vec3 p = 




	}


}
float arcLength(vector<vec3> vertices){
	float len = 0.0f;
	for (int i = 0; i < vertices.size(); i++){
		len += length(vertices.at((i + 1)%(vertices.size() - 1)) - vertices.at(i));
	}
	return len;
}
void tangent(float t, vector<vec3> controlPoints, vector<float> knots, int degree, vec3 * tangent){
	vec3 p1;
	vec3 p2;
	position(t, controlPoints, knots, degree, &p1);
	position((t + (float)DT), controlPoints, knots, degree, &p2);
	p1 = (p2 - p1) / (float)DT;
	tangent->x = p1.x;
	tangent->y = p1.y;
	tangent->z = p1.z;
}
void position(float t, vector<vec3> controlPoints, vector<float> knots, int degree, vec3 * point){
	vec3 p = vec3(0, 0, 0);
	for (int i = 0; i < controlPoints.size(); i++){
		p += controlPoints.at(i) * basis(i, degree, knots, t);
	}
	point->x = p.x;
	point->y = p.y;
	point->z = p.z;
}

void closedKnots(int num, vector<float> * knots){
	for (int i = 0; i <= num; i++){
		knots->push_back((float)i / (float)num);
	}
}
void bspline(vector<vec3> controlPoints, vector<float> knots, int degree, int segs, vector<vec3> * vertices){

	for (int j = 0; j < segs; j++){
		//current point along the curve (0 to 1)
		float t = j * (1 / (float)segs);
		//point that is at distance t along the curve
		vec3 point;
		position(t, controlPoints, knots, degree, &point);
		vertices->push_back(point);
	}
}
//http://stackoverflow.com/questions/30035970/b-spline-algorithm
float basis(int i, int k, vector<float> knots, float t){
	if (k == 0){
		if (t < knots.at(i + 1) && t >= knots.at(i)){
			return 1.0f;
		}
		return 0.0f;
	}
	float p1, p2;
	if (knots.at(i + k) == knots.at(i)){
		p1 = 0.0f;
	}
	else{
		p1 = ((t - knots.at(i)) / ((knots.at(i + k) - knots.at(i))) * basis(i, k - 1, knots, t));
	}
	if (knots.at(i + k + 1) == knots.at(i + 1)){
		p2 = 0.0f;
	}
	else{
		p2 = (((knots.at(i + k + 1) - t) / (knots.at(i + k + 1) - knots.at(i + 1))) * basis(i + 1, k - 1, knots, t));
	}
	return  p1 + p2;
}

void makeCart(vector<vec3> * vertices){
	//base of the cart broken into two triangles
	vertices->push_back(vec3(1.5, 0, -1));
	vertices->push_back(vec3(1.5, 0, 1));
	vertices->push_back(vec3(-1.5, 0, 1));

	vertices->push_back(vec3(-1.5, 0, 1));
	vertices->push_back(vec3(1.5, 0, -1));
	vertices->push_back(vec3(-1.5, 0, -1));

	//left side of the cart broken into two triangles
	vertices->push_back(vec3(-1.5, 0, 1));
	vertices->push_back(vec3(1.5, -0.5, 1));
	vertices->push_back(vec3(1.5, 0, 1));

	vertices->push_back(vec3(-1.5, 0, 1));
	vertices->push_back(vec3(-1.5, -0.5, 1));
	vertices->push_back(vec3(1.5, -0.5, 1));

	//right side of the cart broken into two triangles
	vertices->push_back(vec3(-1.5, 0, -1));
	vertices->push_back(vec3(1.5, -0.5, -1));
	vertices->push_back(vec3(1.5, 0, -1));

	vertices->push_back(vec3(-1.5, 0, -1));
	vertices->push_back(vec3(-1.5, -0.5, -1));
	vertices->push_back(vec3(1.5, -0.5, -1));

	//back side of the cart broken into two triangles
	vertices->push_back(vec3(-1.5, 0, 1));
	vertices->push_back(vec3(-1.5, -0.5, -1));
	vertices->push_back(vec3(-1.5, 0, -1));

	vertices->push_back(vec3(-1.5, 0, 1));
	vertices->push_back(vec3(-1.5, -0.5, 1));
	vertices->push_back(vec3(-1.5, -0.5, -1));

	//front side of the cart broken into two triangles
	vertices->push_back(vec3(1.5, 0, 1));
	vertices->push_back(vec3(1.5, -0.5, -1));
	vertices->push_back(vec3(1.5, 0, -1));

	vertices->push_back(vec3(1.5, 0, 1));
	vertices->push_back(vec3(1.5, -0.5, 1));
	vertices->push_back(vec3(1.5, -0.5, -1));

	//back fin
	vertices->push_back(vec3(-1.5, -0.5, 1));
	vertices->push_back(vec3(-1.5, -0.8, -1));
	vertices->push_back(vec3(-1.5, -0.5, -1));

	vertices->push_back(vec3(-1.5, -0, 1));
	vertices->push_back(vec3(-1.5, -0.8, 1));
	vertices->push_back(vec3(-1.5, -0.8, -1));

	//left triangle
	vertices->push_back(vec3(0.5, -0.5, 1));
	vertices->push_back(vec3(1.5, -0.5, 1));
	vertices->push_back(vec3(0.5, -0.8, 1));

	//right triangle
	vertices->push_back(vec3(0.5, -0.5, -1));
	vertices->push_back(vec3(1.5, -0.5, -1));
	vertices->push_back(vec3(0.5, -0.8, -1));

	//front square
	vertices->push_back(vec3(1.5, -0.5, 1));
	vertices->push_back(vec3(0.5, -0.8, -1));
	vertices->push_back(vec3(1.5, -0.5, -1));

	vertices->push_back(vec3(1.5, -0.5, 1));
	vertices->push_back(vec3(0.5, -0.8, 1));
	vertices->push_back(vec3(0.5, -0.8, -1));

}
