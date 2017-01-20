/*Ashley Currie 10159991*/
/*Use the left and right arrows to switch between assignment components.
Use the up and down arrows to increase/decrease the number of iterations*/

#define _CRT_SECURE_NO_WARNINGS
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Node.h"

#include <iostream>
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

# define M_PI   3.14159265358979323846

using namespace std;
using namespace glm;

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
void makeIcoSphere(int depth, vector<vec3> * newCoords);
void icosphere(int depth, vector<vec3> coords, vector<vec3> * answers);
void makeCylinder(float radius, float height, int segs, vector<vec3> * vertices);

const float WIDTH = 1000;
const float HEIGHT = 1000;


// --------------------------------------------------------------------------
// Functions to set up OpenGL shader programs for rendering
struct MyShader
{
	// OpenGL names for vertex and fragment shaders, shader program
	GLuint  vertex;
	GLuint  fragment;
	GLuint  program;
	GLuint mvpNum;
	GLuint ambientN;
	GLuint specularN;
	GLuint litFlag;
	GLuint view;
	GLuint light;
	mat4  mvp;
	vec3 ambient;
	vec3 specular;
	// initialize shader and program names to zero (OpenGL reserved value)
	MyShader() : vertex(0), fragment(0), program(0), litFlag(0)
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
	shader->ambientN = glGetUniformLocation(shader->program, "ambient");
	shader->specularN = glGetUniformLocation(shader->program, "specular");
	shader->litFlag = glGetUniformLocation(shader->program, "lit");
	shader->view = glGetUniformLocation(shader->program, "view");
	shader->light = glGetUniformLocation(shader->light, "light");

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

// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing geometry data

struct MyGeometry
{
	// OpenGL names for array buffer objects, vertex array object
	GLuint  vertexBuffer;
	GLuint colourBuffer;
	GLuint  textureBuffer;
	GLuint  vertexArray;
	GLsizei elementCount;

	// initialize object names to zero (OpenGL reserved value)
	MyGeometry() : vertexBuffer(0), textureBuffer(0), vertexArray(0), elementCount(0)
	{}
};

void initNode(Node * node){
	vector<GLfloat> vert = vector<GLfloat>();
	vector<GLfloat> colours = vector<GLfloat>();
	for (unsigned int i = 0; i < node->vertices.size(); i++){
		vert.push_back(node->vertices.at(i).x);
		vert.push_back(node->vertices.at(i).y);
		vert.push_back(node->vertices.at(i).z);
		colours.push_back(0);
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
// create buffers and fill with geometry data, returning true if successful
//bool InitializeGeometry(MyGeometry *geometry, vector<vec3> vertices, vector<float> map)
bool InitializeGeometry(MyGeometry *geometry, vector<vec3> vertices, vector<vec3> colours)
{
	vector<GLfloat> vert = vector<GLfloat>();
	for (unsigned int i = 0; i < vertices.size(); i++){
		vert.push_back(vertices.at(i).x);
		vert.push_back(vertices.at(i).y);
		vert.push_back(vertices.at(i).z);
	}
	geometry->elementCount = vert.size()/3;
	
	// these vertex attribute indices correspond to those specified for the
	// input variables in the vertex shader
	const GLuint VERTEX_INDEX = 0;
	const GLuint COLOUR_INDEX = 1;
	//const GLuint TEXTURE_INDEX = 2;

	// create an array buffer object for storing our vertices
	glGenBuffers(1, &geometry->vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vert.size() * sizeof(GLfloat), vert.data(), GL_STATIC_DRAW);
	// create another one for storing our texture
	//glGenBuffers(1, &geometry->textureBuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, geometry->textureBuffer);
	//glBufferData(GL_ARRAY_BUFFER, map.size() * sizeof(GLfloat), map.data(), GL_STATIC_DRAW);
	glGenBuffers(1, &geometry->colourBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
	glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(GLfloat), colours.data(), GL_STATIC_DRAW);

	// create a vertex array object encapsulating all our vertex attributes
	glGenVertexArrays(1, &geometry->vertexArray);
	glBindVertexArray(geometry->vertexArray);

	// associate the position array with the vertex array object
	glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
	glVertexAttribPointer(VERTEX_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(VERTEX_INDEX);

	glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
	glVertexAttribPointer(COLOUR_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(COLOUR_INDEX);

	// Tell openGL how the data is formatted
	//glBindBuffer(GL_ARRAY_BUFFER, geometry->textureBuffer);
	//glVertexAttribPointer(TEXTURE_INDEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(TEXTURE_INDEX);

	// unbind our buffers, resetting to default state
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CheckGLErrors();
	// check for OpenGL errors and return false if error occurred
	return true;
}

// deallocate geometry-related objects
void DestroyGeometry(MyGeometry *geometry)
{
	// unbind and destroy our vertex array object and associated buffers
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &geometry->vertexArray);
	glDeleteBuffers(1, &geometry->vertexBuffer);
	glDeleteBuffers(1, &geometry->textureBuffer);
}
bool InitializeTexture(Texture* texture, GLuint target = GL_TEXTURE_2D)
{
	if (texture->data != nullptr)
	{
		texture->target = target;
		glGenTextures(1, &texture->textureID);
		glBindTexture(texture->target, texture->textureID);
		glTexImage2D(texture->target, 0, texture->format, texture->width, texture->height, 0, texture->format, GL_UNSIGNED_BYTE, texture->data);

		// Note: Only wrapping modes supported for GL_TEXTURE_RECTANGLE when defining
		// GL_TEXTURE_WRAP are GL_CLAMP_TO_EDGE or GL_CLAMP_TO_BORDER
		glTexParameteri(texture->target, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(texture->target, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(texture->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(texture->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Clean up
		glBindTexture(texture->target, 0);

		return !CheckGLErrors();
	}
	return true; //error
}

void LoadTexture(Texture * texture, string filePath){

	stbi_set_flip_vertically_on_load(true);
	int numComp;

	unsigned char * data = stbi_load(filePath.c_str(), &texture->width, &texture->height, &numComp, 0);
	texture->data = data;
	texture->format = GL_RGB;

	InitializeTexture(texture, GL_TEXTURE_RECTANGLE);
}

// deallocate texture-related objects
void DestroyTexture(Texture *texture)
{
	glBindTexture(texture->target, 0);
	glDeleteTextures(1, &texture->textureID);
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

	window = glfwCreateWindow(WIDTH, HEIGHT, "CPSC 453 Assignment 1", 0, 0);
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
			else if (key == GLFW_KEY_ENTER){
				con->animate = !con->animate;
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
	InitializeShaders(&shader);

	MyGeometry geom;
	mat4 projection;
	mat4 view;
	//grab textures
	//Texture * glass = new Texture();
	//LoadTexture(glass, "bead.jpg");
	//create scene objects
	vector<Node *> sceneObjects = vector<Node *>();
	Node * root = new Node();
	Node * rod = new Node();
	Node * bead = new Node();
	//setup hierarchy 
	root->addChild(rod);
	root->addChild(bead);
	rod->setScale(vec3(0.1));
	rod->lit = false;
	sceneObjects.push_back(rod);
	bead->setScale(vec3(0.1));
	bead->lit = false;
	sceneObjects.push_back(bead);
	
	//generate sphere
	//vector<vec3> vertices = vector<vec3>();
	//vector<vec3> colours = vector<vec3>();
	makeCylinder(0.5f, 5.0f, 100, &(rod->vertices));
	makeIcoSphere(5, &(bead->vertices));
	//for (int i = 0; i < rod->vertices.size(); i++){
		//colours.push_back(vec3(0, 0, 0));
	//}
	//glass->setUV(vertices);
	
	for (int i = 0; i < sceneObjects.size(); i++){
		initNode(sceneObjects.at(i));
	}
	//InitializeGeometry(&geom, vertices, colours);
	// run an event-triggered main loop
	while (!glfwWindowShouldClose(window))
	{	
		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shader.program);
	
		view = lookAt(vec3(0,0,4), vec3(0,0,3), vec3(0,1,0));
		projection = perspective((50 * (float)M_PI /180), WIDTH/HEIGHT, 1000.0f, 0.1f);
	
		mat4 transform = translate(mat4(1.0f), vec3(0, 0, context.ztrans)) * rotate(mat4(1.0f),
			((float)(2 * M_PI) - context.phi), vec3(0, 1, 0)) * rotate(mat4(1.0f), (float)context.theta, vec3(0, 0, 1));
		root->transform(transform);
		mat4 invertTrans = translate(mat4(1.0f), vec3(0, 0, context.ztrans)) * rotate(mat4(1.0f), context.phi, vec3(0, 1, 0)) * rotate(mat4(1.0f), (float)(M_PI - context.theta), vec3(0, 0, 1));

		vec4 newView = invertTrans * vec4(0,0,4,1);
		vec4 newLight = transform * vec4(0, 0, 0, 1);
		glUniform3f(shader.view, newView.x, newView.y, newView.z);
		glUniform3f(shader.light, newLight.x, newLight.y, newLight.z);

		for (int i = 0; i < sceneObjects.size(); i++){

			glBindVertexArray(sceneObjects.at(i)->vertexArray);
			shader.mvp = projection * view * sceneObjects.at(i)->getGlobalTransform();
			//Texture * tex = sceneObjects.at(i)->getTexture();
			//glBindBuffer(GL_ARRAY_BUFFER, geom.textureBuffer);
			//glBufferData(GL_ARRAY_BUFFER, tex->UVmap.size() * sizeof(GLfloat), tex->UVmap.data(), GL_STATIC_DRAW);
			glUniform3d(shader.ambientN, 1, 1, 1);
			glUniform3d(shader.specularN, 0.5, 0.5, 0.5);
			//send mvp matrix to shader
			glUniformMatrix4fv(shader.mvpNum, 1, GL_FALSE, value_ptr(shader.mvp));
			glUniform1ui(shader.litFlag, sceneObjects.at(i)->lit ? 1 : 0);

			//draw triangles for current shape
			glDrawArrays(GL_TRIANGLES, 0, sceneObjects.at(i)->vertices.size()/3);
			glBindVertexArray(0);
		}
		glUseProgram(0);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	DestroyGeometry(&geom);
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

void icosphere(int depth, vector<vec3> coords, vector<vec3> * answers){
	if (depth == 0){
		answers->push_back(coords.at(0));
		answers->push_back(coords.at(1));
		answers->push_back(coords.at(2));
	}
	else{
		for (unsigned int i = 0; i < coords.size(); i += 3){
			vec3 p1 = coords.at(i);
			vec3 p2 = coords.at(i+1);
			vec3 p3 = coords.at(i+2);

			vec3 lerp1 = normalize((p1 + p2) * 0.5f);
			vec3 lerp2 = normalize((p2 + p3) * 0.5f);
			vec3 lerp3 = normalize((p1 + p3) * 0.5f);

			icosphere(depth -1 , vector < vec3 > {p1, lerp1, lerp3}, answers);
			icosphere(depth -1 , vector < vec3 > {lerp3, lerp2, p3}, answers);
			icosphere(depth -1, vector < vec3 > {lerp3, lerp1, lerp2}, answers);
			icosphere(depth -1, vector < vec3 > {lerp1, p2, lerp2}, answers);
		}
	}
	
}

void makeIcoSphere(int depth, vector<vec3> * newCoords){
	newCoords->clear();
	GLfloat oneOverRootTwo = 1 / sqrt(2);
	vector<vec3> coords = {
		normalize(vec3(0, 1, oneOverRootTwo)),
		normalize(vec3(1, 0, -oneOverRootTwo)),
		normalize(vec3(0, -1, oneOverRootTwo)),
		normalize(vec3(0, 1, oneOverRootTwo)),
		normalize(vec3(-1, 0, -oneOverRootTwo)),
		normalize(vec3(1, 0, -oneOverRootTwo)),
		normalize(vec3(0, 1, oneOverRootTwo)),
		normalize(vec3(0, -1, oneOverRootTwo)),
		normalize(vec3(-1, 0, -oneOverRootTwo)),
		normalize(vec3(0, -1, oneOverRootTwo)),
		normalize(vec3(-1, 0, -oneOverRootTwo)),
		normalize(vec3(1, 0, -oneOverRootTwo))
	};

	icosphere(depth, coords, newCoords);
}

void makeCylinder(float radius, float height, int segs, vector<vec3> * vertices){

	for (float i = 0.0f; i < 2 * M_PI; i+= (2 * M_PI/ segs)){

		vertices->push_back(vec3(radius * cos(i), radius * sin(i), 0));
		vertices->push_back(vec3(radius * cos(i), radius * sin(i), height));
		vertices->push_back(vec3(radius * cos(i + (2 * M_PI / segs)), radius * sin(i + (2 * M_PI / segs)), 0));

		vertices->push_back(vec3(radius * cos(i + (2 * M_PI / segs)), radius * sin(i + (2 * M_PI / segs)), 0));
		vertices->push_back(vec3(radius * cos(i), radius * sin(i), height));
		vertices->push_back(vec3(radius * cos(i + (2 * M_PI / segs)), radius * sin(i + (2 * M_PI / segs)), height));
	}
}


