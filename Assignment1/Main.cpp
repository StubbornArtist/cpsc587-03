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
#define SPEED 100.0
#define GRAVITY 9.81


using namespace std;
using namespace glm;

void QueryGLVersion();
string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

void position(float t, vector<vec3> controlPoints, vector<float> knots, int degree, vec3 * point);
void closedKnots(int num, vector<float> * knots);
void bspline(vector<vec3> controlPoints, vector<float> knots, int degree, int segs, vector<vec3> * vertices, vector<vec3> * pipe);
float basis(int i, int k, vector<float> knots, float t);
void rail(vector<vec3> vertices, vector<vec3> * pipe, float r, vector<vec3> tangents, vector<vec3> norms, vector<vec3> binorms);
void railVertices(vector<vec3> spline, vector<vec3> * rightRail, vector<vec3> * leftRail, vector<vec3> * midRail, vector<vec3> * bars, vector<vec3> * tangents, vector<vec3> * norms, vector<vec3> * binorms);
vec3 move(vec3 start, float * ind, vector<vec3> cPoints, float deltaS);
float max_height(vector<vec3> vertices);
void makeCart(vector<vec3> * vertices);
void fillColour(vec3 colour, vector<vec3> * colourBuffer, int num);
void gravitationalForce(vector<vec3> vertices, int i, float speed, vec3 * N);
void genSkyBox(vector<Node *> * sceneObjects, Node * root);
void skyBox(Node * top, Node * bot, Node * left, Node * right, Node * front, Node * back, float imageWidth, float width);
void squareUV(vector<float> * map, float width);

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
	GLuint  texturize;
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
	shader->texturize = glGetUniformLocation(shader->program, "texturize");
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
void readControlPoints(string objectFile,vector<vec3> * points){
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
		if (node->colours.size() > 0){
			colours.push_back(node->colours.at(i).r);
			colours.push_back(node->colours.at(i).g);
			colours.push_back(node->colours.at(i).b);
		}
	}
	const GLuint VERTEX_INDEX = 0;
	const GLuint COLOUR_INDEX = 1;
	const GLuint TEXTURE_INDEX = 2;

	glGenBuffers(1, &node->vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, node->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vert.size() * sizeof(GLfloat), vert.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &node->colourBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, node->colourBuffer);
	glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(GLfloat), colours.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &node->textureBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, node->textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, node->map.size() * sizeof(GLfloat), node->map.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &node->vertexArray);
	glBindVertexArray(node->vertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, node->vertexBuffer);
	glVertexAttribPointer(VERTEX_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(VERTEX_INDEX);

	glBindBuffer(GL_ARRAY_BUFFER, node->colourBuffer);
	glVertexAttribPointer(COLOUR_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(COLOUR_INDEX);

	glBindBuffer(GL_ARRAY_BUFFER, node->textureBuffer);
	glVertexAttribPointer(TEXTURE_INDEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(TEXTURE_INDEX);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void LoadTexture(Texture * texture, string filePath){

	stbi_set_flip_vertically_on_load(true);
	int numComp;

	unsigned char * data = stbi_load(filePath.c_str(), &texture->width, &texture->height, &numComp, 0);
	texture->data = data;
	texture->format = GL_RGB;

	if (texture->data != nullptr)
	{
		texture->target = GL_TEXTURE_RECTANGLE;
		glGenTextures(1, &texture->textureID);
		glBindTexture(texture->target, texture->textureID);
		glTexImage2D(texture->target, 0, texture->format, texture->width, texture->height, 0, texture->format, GL_UNSIGNED_BYTE, texture->data);

		// Note: Only wrapping modes supported for GL_TEXTURE_RECTANGLE when defining
		// GL_TEXTURE_WRAP are GL_CLAMP_TO_EDGE or GL_CLAMP_TO_BORDER
		glTexParameteri(texture->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(texture->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(texture->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(texture->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Clean up
		glBindTexture(texture->target, 0);
	}
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
	//mouse input
	MyContext context;
	glfwSetWindowUserPointer(window, &context);
	auto keys = [](GLFWwindow * window, int key, int scancode, int action, int mods){
		MyContext * con = GetContext(window);
		if (action == GLFW_PRESS){
			if (key == GLFW_KEY_EQUAL){
				con->ztrans += 1.5f;
			}
			else if (key == GLFW_KEY_MINUS){
				con->ztrans -= 1.5f;
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
	// query and print out information about our OpenGL environment
	QueryGLVersion();
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	glEnable(GL_DEPTH_TEST);
	//glFrontFace(GL_CW);
	// call function to load and compile shader programs
	MyShader shader;
	mat4 projection;
	mat4 view;
	view = lookAt(vec3(0, 4, 100), vec3(0, 0, 0), vec3(0, 1, 0));
	projection = perspective((50 * (float)M_PI / 180), WIDTH / HEIGHT, 1000.0f, 0.1f);
	
	//create scene objects
	vector<Node *> sceneObjects = vector<Node *>();
	vector<vec3> controlPoints;
	vector<float> knots;
	vector<vec3> trackLeft;
	vector<vec3> trackRight;
	vector<vec3> midTrack;
	vector<vec3> bars;
	vector<vec3> tangents;
	vector<vec3> norms;
	vector<vec3> binorms;
	vec3 pos; 
	float ind = 0.0f;
	float H;
	float speed;
	Node * root = new Node();
	Node * track = new Node();
	Node * curve = new Node();
	Node * cart = new Node();
	root->addChild(track);
	root->addChild(cart);
	sceneObjects.push_back(track);
	sceneObjects.push_back(cart);
	//genSkyBox(&sceneObjects, root);
	
	
	readControlPoints("points2.txt", &controlPoints);
	closedKnots(4 + controlPoints.size() + 1, &knots);
	bspline(controlPoints, knots, 3, 400, &(curve->vertices),&(track->vertices));
	railVertices(curve->vertices, &trackRight, &trackLeft, &midTrack, &bars, &tangents, &norms, &binorms);
	rail(trackLeft, &(track->vertices), 0.1f, tangents, norms, binorms);
	rail(trackRight, &(track->vertices), 0.1f, tangents, norms, binorms);
	rail(midTrack, &(track->vertices), 0.2f, tangents, norms, binorms);
	for (int i = 0; i < bars.size(); i++){
		track->vertices.push_back(bars.at(i));
	}
	pos = curve->vertices.at(0);
	H = max_height(curve->vertices);
	makeCart(&(cart->vertices));
	cart->setScale(vec3(0.5,2.0,0.5));

	fillColour(vec3(0.55, 0.09, 0.09), &(track->colours), track->vertices.size());
	fillColour(vec3(0, 0, 0), &(cart->colours), cart->vertices.size());
	
	InitializeShaders(&shader);
	for (int i = 0; i < sceneObjects.size(); i++){
		initNode(sceneObjects.at(i));
	}
	while (!glfwWindowShouldClose(window))
	{	
		glClearColor(0.196078, 0.6, 0.8, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shader.program);

		mat4 transform = translate(mat4(1.0f), vec3(0, 0, context.ztrans)) * rotate(mat4(1.0f),
			((float)(2 * M_PI) - context.phi), vec3(0, 1, 0)) * rotate(mat4(1.0f), (float)context.theta, vec3(0, 0, 1));
		root->transform(transform);

		speed = SPEED + sqrt(2.0f * abs(GRAVITY) * (H - pos.y));
		float deltaS =  speed * DELTA_T;
		pos = move(pos, &ind, curve->vertices, deltaS);

		int i = (int)ind;
		cart->setRotation(mat4(vec4(tangents.at(i), 0), vec4(norms.at(i), 0), vec4(binorms.at(i), 0), vec4(0, 0, 0, 1)));
		cart->setTranslation(pos - curve->vertices.at(0) + norms.at(i) * 0.07f);

		for (int i = 0; i < sceneObjects.size(); i++){
			Node * n = sceneObjects.at(i);
			glBindVertexArray(n->vertexArray);
			//calculate mvp matrix for this object
			shader.mvp = projection * view * n->getGlobalTransform();
			glUniformMatrix4fv(shader.mvpNum, 1, GL_FALSE, value_ptr(shader.mvp));

			glBindTexture(n->tex.target, n->tex.textureID);
			glTexImage2D(n->tex.target, 0, n->tex.format, n->tex.width, n->tex.height, 0, n->tex.format, GL_UNSIGNED_BYTE, n->tex.data);

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawArrays(GL_TRIANGLES, 0, n->vertices.size());
			glBindTexture(n->tex.target, 0);
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

//---Functions related to movement of the cart in the scene-----

//a function that determines the maximum height that the roller coaster can reach
float max_height(vector<vec3> vertices){
	float max = vertices.at(0).y;
	for (int i = 0; i < vertices.size(); i++){
		if (vertices.at(i).y > max){
			max = vertices.at(i).y;
		}
	}
	return max;
}

//A function which determines the new point along the curve after moving a distance of deltaS
//based on the algorithm for parameterizing a curve given in the tutorial notes
vec3 move(vec3 start, float* ind, vector<vec3> cPoints, float deltaS){
	int cur = (int)(*ind);
	int next = (cur + 1) % cPoints.size();
	float s = (float)(*ind)-cur;
	float newDS = length(cPoints.at(next) - start);
	if (newDS > deltaS){
		vec3 newpos;
		if (cPoints.at(next) != cPoints.at(cur)){
			newpos = start + deltaS * normalize(cPoints.at(next) - cPoints.at(cur));
			float pr = (float)(cur + length(newpos - cPoints.at(cur)) / length(cPoints.at(next) - cPoints.at(cur)));
			*ind = pr;
		} else {
			newpos = cPoints.at(next);
			*ind = (float)next;
		}

		if (*ind >= cPoints.size()){
			*ind -= cPoints.size();
		}
		return newpos;
	}

	cur = next;
	next = (cur + 1) % cPoints.size();
	while(newDS + length(cPoints.at(next) - cPoints.at(cur)) < deltaS){
		newDS = newDS + length(cPoints.at(next) - cPoints.at(cur));
		cur = next;
		next = (next + 1) % cPoints.size();
	}
	vec3 newpos;
	if (cPoints.at(next) != cPoints.at(cur)){
		newpos = cPoints.at(cur)+ (deltaS - newDS) * normalize(cPoints.at(next) - cPoints.at(cur));
		float pr = (float)(cur + length(newpos - cPoints.at(cur)) / length(cPoints.at(next) - cPoints.at(cur)));
		*ind = pr;
	} else {
		newpos = cPoints.at(next);
		*ind = (float)next;
	}
	if (*ind >= cPoints.size()){
		*ind -= cPoints.size();
	}
	return newpos;
}

void gravitationalForce(vector<vec3> vertices, int i, float speed, vec3 * N){
	vec3 n;
	vec3 f;
	if (i + 1 == vertices.size()){
		n = vertices.at(0);
		f = vertices.at(i - 1);
	}
	else if (i - 1 < 0){
		f = vertices.at(0);
		n = vertices.at(i + 1);
	}
	else{
		n = vertices.at(i + 1);
		f = vertices.at(i - 1);
	}
	n = (vertices.at(i) + n) / 2.0f;
	f = (vertices.at(i) + f) / 2.0f;
	float x = 0.5f * length(n - 2.0f * vertices.at(i) + f);
	float c = 0.5f * length(n - f);
	float r = ((x * x) + (c * c)) / (2.0f * c);
	vec3 norm = normalize(n - 2.0f * vertices.at(i) + f);
	*N = normalize(((speed * speed) / r) * norm + vec3(0, GRAVITY, 0));
}

//----Functions that create vertices for 3D rails of the track-----

//A function which takes any set of vertices and creates circles around each point then connects those circles to make a curved rail
void rail(vector<vec3> vertices, vector<vec3> * pipe, float r, vector<vec3> tangents, vector<vec3> norms, vector<vec3> binorms){
	const int div = 20;
	const float inc = (2.0f * M_PI) / div;
	vector<vec3> tempPipe = vector<vec3>();
	for (int i = 0; i < vertices.size(); i++){		
		for (float u = 0.0f, j = 0; j < div; u += inc, j++){
			tempPipe.push_back(r * cos(u) * binorms.at(i) + r * sin(u) * cross(tangents.at(i), binorms.at(i)) + vertices.at(i));
			//tempPipe.push_back(r * cos(u + inc) * binorms.at(i) + r * sin(u + inc) * cross(tangents.at(i), binorms.at(i)) + vertices.at(i));
		}
	}
	for (int i = 0; i < vertices.size(); i++){
		int offset = i * div;
		int offset2 = ((i + 1) * div) % tempPipe.size();
		for (int j = 0; j < div; j++){
			vec3 p1 = tempPipe.at(offset + j);
			vec3 p2 = tempPipe.at(offset + ((j + 1) % div));
			vec3 p3 = tempPipe.at((offset2 + j));
			vec3 p4 = tempPipe.at(offset2 + ((j + 1) % div));
			pipe->push_back(p1);
			pipe->push_back(p3);
			pipe->push_back(p2);
			pipe->push_back(p2);
			pipe->push_back(p3);
			pipe->push_back(p4);
		}
	}
	/*
	int numPerCirc = (tempPipe.size() / vertices.size()) - 1;
	for (int i = 0; i < tempPipe.size(); i += numPerCirc){
		for (int j = 0; j <= numPerCirc; j++){
			vec3 p1 = tempPipe.at((i + j) % tempPipe.size());
			vec3 p2 = tempPipe.at((i + numPerCirc + j) % tempPipe.size());
			vec3 p3 = tempPipe.at((i + numPerCirc + j + 1) % tempPipe.size());
			vec3 p4 = tempPipe.at((i + j + 1) % tempPipe.size());
			pipe->push_back(p1);
			pipe->push_back(p2);
			pipe->push_back(p3);
			pipe->push_back(p1);
			pipe->push_back(p3);
			pipe->push_back(p4);
		}
	}*/
}
//A function that generates the right rail, left rail, central (lower) rail, joints between rails, and the tangent, normal, binormals of a given curve
void railVertices(vector<vec3> spline, vector<vec3> * rightRail, vector<vec3> * leftRail, vector<vec3> * midRail, vector<vec3> * bars, vector<vec3> * tangents, vector<vec3> * norms, vector<vec3> * binorms){
	for (int i = 0; i < spline.size(); i++){
		vec3 point = spline.at(i == 0 ? spline.size() - 1 : i - 1); 
		vec3 point2 = spline.at((i<spline.size() - 1) ? i + 1 : 0);

		vec3 tangent = normalize(point2 - point);
		vec3 v = cross(vec3(1, 0, 0), tangent);
		float s = length(v);
		float c = dot(tangent, vec3(1, 0, 0));

		//http://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d
		mat3 skew = mat3(vec3(0.0f, v.z, -v.y), vec3(-v.z, 0.0f, v.x), vec3(v.y, -v.x, 0.0f));
		//mat3 R = (v == vec3(0.0f)) ? mat3(1.0f) : mat3(1.0f) + skew + (skew * skew) * ((1.0f - c) / (s * s));
		mat3 R = mat3(1.0f) + skew + (skew * skew) * ((1.0f - c) / (s * s));
		
		vec3 norm = R * vec3(0, 1, 0);
		vec3 binorm = R * vec3(0, 0, -1);

		vec3 right = (binorm * 0.5f) + point;
		vec3 left = point - (binorm * 0.5f);
		rightRail->push_back(right);
		leftRail->push_back(left);

		vec3 mid = (right + left) * 0.5f + norm * -0.5f;
		midRail->push_back(mid);

		if (i % 2 == 0){
			bars->push_back(right);
			bars->push_back(right + norm * 0.1f);
			bars->push_back(mid);
			bars->push_back(mid + norm * 0.2f);
			bars->push_back(mid);
			bars->push_back(right + norm * 0.1f);

			bars->push_back(left);
			bars->push_back(mid);
			bars->push_back(left + norm * 0.1f);
			bars->push_back(mid + norm * 0.2f);
			bars->push_back(left + norm * 0.1f);
			bars->push_back(mid);
		}

		tangents->push_back(tangent);
		norms->push_back(norm);
		binorms->push_back(binorm);
	}
}


//-----Functions that create a b-spline curve---- 

//the bspline at some given t value where t is between 0 and 1
void position(float t, vector<vec3> controlPoints, vector<float> knots, int degree, vec3 * point){
	vec3 p = vec3(0, 0, 0);
	for (int i = 0; i < controlPoints.size(); i++){
		p += controlPoints.at(i) * basis(i, degree, knots, t);
	}
	point->x = p.x;
	point->y = p.y;
	point->z = p.z;
}
//A function which generates the knot vector as defined in the b-spline algorithm for a closed curve
void closedKnots(int num, vector<float> * knots){
	for (int i = 0; i <= num; i++){
		knots->push_back((float)i / (float)num);
	}
}
//a function that samples along the b-spline and collects vertices
void bspline(vector<vec3> controlPoints, vector<float> knots, int degree, int segs, vector<vec3> * vertices, vector<vec3> * pipe){
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
//The b-spline basis function as defined on the webpage of the above url
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
//Funtion that adds vertices for a cart object
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
	vertices->push_back(vec3(1.5, 0.5, 1));
	vertices->push_back(vec3(1.5, 0, 1));

	vertices->push_back(vec3(-1.5, 0, 1));
	vertices->push_back(vec3(-1.5, 0.5, 1));
	vertices->push_back(vec3(1.5, 0.5, 1));

	//right side of the cart broken into two triangles
	vertices->push_back(vec3(-1.5, 0, -1));
	vertices->push_back(vec3(1.5, 0.5, -1));
	vertices->push_back(vec3(1.5, 0, -1));

	vertices->push_back(vec3(-1.5, 0, -1));
	vertices->push_back(vec3(-1.5, 0.5, -1));
	vertices->push_back(vec3(1.5, 0.5, -1));

	//back side of the cart broken into two triangles
	vertices->push_back(vec3(-1.5, 0, 1));
	vertices->push_back(vec3(-1.5, 0.5, -1));
	vertices->push_back(vec3(-1.5, 0, -1));

	vertices->push_back(vec3(-1.5, 0, 1));
	vertices->push_back(vec3(-1.5, 0.5, 1));
	vertices->push_back(vec3(-1.5, 0.5, -1));

	//front side of the cart broken into two triangles
	vertices->push_back(vec3(1.5, 0, 1));
	vertices->push_back(vec3(1.5, 0.5, -1));
	vertices->push_back(vec3(1.5, 0, -1));

	vertices->push_back(vec3(1.5, 0, 1));
	vertices->push_back(vec3(1.5, 0.5, 1));
	vertices->push_back(vec3(1.5, 0.5, -1));

	//back fin
	vertices->push_back(vec3(-1.5, 0.5, 1));
	vertices->push_back(vec3(-1.5, 0.8, -1));
	vertices->push_back(vec3(-1.5, 0.5, -1));

	vertices->push_back(vec3(-1.5, 0, 1));
	vertices->push_back(vec3(-1.5, 0.8, 1));
	vertices->push_back(vec3(-1.5, 0.8, -1));

	//left triangle
	vertices->push_back(vec3(0.5, 0.5, 1));
	vertices->push_back(vec3(1.5, 0.5, 1));
	vertices->push_back(vec3(0.5, 0.8, 1));

	//right triangle
	vertices->push_back(vec3(0.5, 0.5, -1));
	vertices->push_back(vec3(1.5, 0.5, -1));
	vertices->push_back(vec3(0.5, 0.8, -1));

	//front square
	vertices->push_back(vec3(1.5, 0.5, 1));
	vertices->push_back(vec3(0.5, 0.8, -1));
	vertices->push_back(vec3(1.5, 0.5, -1));

	vertices->push_back(vec3(1.5, 0.5, 1));
	vertices->push_back(vec3(0.5, 0.8, 1));
	vertices->push_back(vec3(0.5, 0.8, -1));

}
void fillColour(vec3 colour, vector<vec3> * colourBuffer, int num){
	for (int i = 0; i < num; i++){
		colourBuffer->push_back(colour);
	}
}
//-----Functions that help create a sky box------

//A function which that creates several faces for the sky box and adds them to the scene, with loaded textures
void genSkyBox(vector<Node *> * sceneObjects, Node * root){
	//one object for each face of the box
	Node * rightBox = new Node();
	Node * leftBox = new Node();
	Node * topBox = new Node();
	Node * botBox = new Node();
	Node * frontBox = new Node();
	Node * backBox = new Node();
	//add all faces to the root of the scene
	root->addChild(rightBox);
	root->addChild(leftBox);
	root->addChild(topBox);
	root->addChild(botBox);
	root->addChild(frontBox);
	root->addChild(backBox);
	//add all face to the list of drawable objects in the scene 
	sceneObjects->push_back(rightBox);
	sceneObjects->push_back(leftBox);
	sceneObjects->push_back(topBox);
	sceneObjects->push_back(botBox);
	sceneObjects->push_back(frontBox);
	sceneObjects->push_back(backBox);
	//load images for each face
	LoadTexture(&(rightBox->tex), "posx.jpg");
	LoadTexture(&(leftBox->tex), "negx.jpg");
	LoadTexture(&(topBox->tex), "posy.jpg");
	LoadTexture(&(botBox->tex), "negy.jpg");
	LoadTexture(&(frontBox->tex), "posz.jpg");
	LoadTexture(&(backBox->tex), "negz.jpg");
	//generate vertices and texture coordinates for each face
	skyBox(topBox, botBox, leftBox, rightBox, frontBox, backBox, rightBox->tex.width, 200);
}
//A function that generates the vertices for each of the faces of the sky box
void skyBox(Node * top, Node * bot, Node * left, Node * right, Node * front, Node * back, float imageWidth, float width){
	float d = width / 2;
	//front side
	front->vertices.push_back(vec3(-d, d, d));
	front->vertices.push_back(vec3(d, d, d));
	front->vertices.push_back(vec3(d, -d, d));

	front->vertices.push_back(vec3(d, -d, d));
	front->vertices.push_back(vec3(-d, -d, d));
	front->vertices.push_back(vec3(-d, d, d));

	squareUV(&(front->map), imageWidth);

	//back side
	back->vertices.push_back(vec3(-d, d, -d));
	back->vertices.push_back(vec3(d, d, -d));
	back->vertices.push_back(vec3(d, -d, -d));

	back->vertices.push_back(vec3(d, -d, -d));
	back->vertices.push_back(vec3(-d, -d, -d));
	back->vertices.push_back(vec3(-d, d, -d));

	squareUV(&(back->map), imageWidth);

	//left side
	left->vertices.push_back(vec3(-d, d, -d));
	left->vertices.push_back(vec3(-d, d, d));
	left->vertices.push_back(vec3(-d, -d, d));

	left->vertices.push_back(vec3(-d, -d, d));
	left->vertices.push_back(vec3(-d, -d, -d));
	left->vertices.push_back(vec3(-d, d, -d));
	
	squareUV(&(left->map), imageWidth);

	//right side
	right->vertices.push_back(vec3(d, d, d));
	right->vertices.push_back(vec3(d, d, -d));
	right->vertices.push_back(vec3(d, -d, -d));

	right->vertices.push_back(vec3(d, -d, -d));
	right->vertices.push_back(vec3(d, -d, d));
	right->vertices.push_back(vec3(d, d, d));

	squareUV(&(right->map), imageWidth);

	//top side
	top->vertices.push_back(vec3(-d, d, -d));
	top->vertices.push_back(vec3(d, d, -d));
	top->vertices.push_back(vec3(d, d, d));

	top->vertices.push_back(vec3(d, d, d));
	top->vertices.push_back(vec3(-d, d, d));
	top->vertices.push_back(vec3(-d, d, -d));

	squareUV(&(top->map), imageWidth);

	//bottom side
	bot->vertices.push_back(vec3(-d, -d, -d));
	bot->vertices.push_back(vec3(d, -d, -d));
	bot->vertices.push_back(vec3(d, -d, d));

	bot->vertices.push_back(vec3(d, -d, d));
	bot->vertices.push_back(vec3(-d, -d, d));
	bot->vertices.push_back(vec3(-d, -d, -d));

	squareUV(&(bot->map), imageWidth);
}
//A function that creates a uv map for a square image of a given width
void squareUV(vector<float> * map, float width){
	map->push_back(0.0);
	map->push_back(width);

	map->push_back(width);
	map->push_back(width);

	map->push_back(width);
	map->push_back(0.0);

	map->push_back(width);
	map->push_back(0.0);

	map->push_back(0.0);
	map->push_back(0.0);

	map->push_back(0.0);
	map->push_back(width);
}