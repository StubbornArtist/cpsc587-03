/*Ashley Currie 10159991*/
/*Use the left and right arrows to switch between assignment components.
Use the up and down arrows to increase/decrease the number of iterations*/

#define _CRT_SECURE_NO_WARNINGS
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Node.h"
#include "BSpline.h"

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
#define SPEED 21.0
#define GRAVITY 9.81


using namespace std;
using namespace glm;

void QueryGLVersion();
string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

void drawFloor(vector<Node *> * obj , Node * root,  float width, float imageWidth);
void bspline(vector<vec3> controlPoints, vector<vec3> * vertices, int iter);
void genTrack(string file, vector<Node *> * sceneObjects, Node * root,  BSpline * curve);
void supportPoles(BSpline * curve, vector<vec3> rail, vector<vec3> * vertices, float r);
void rail(vector<vec3> vertices, vector<vec3> * pipe, float r, vector<vec3> tangents, vector<vec3> norms, vector<vec3> binorms);
void railVertices(BSpline * curve, vector<vec3> * rightRail, vector<vec3> * leftRail, vector<vec3> * midRail, vector<vec3> * bars);
vec3 move(vec3 start, float * ind, vector<vec3> cPoints, float deltaS);
float max_height(vector<vec3> vertices);
void makeCart(vector<vec3> * vertices);
void fillColour(vec3 colour, vector<vec3> * colourBuffer, int num);
void genSkyBox(vector<Node *> * sceneObjects, Node * root);
void skyBox(Node * top, Node * bot, Node * left, Node * right, Node * front, Node * back, float imageWidth, float width);
void squareUV(vector<float> * map, float width);

const float WIDTH = 2000;
const float HEIGHT = 2000;
bool firstPerson = true;
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
//give a node in the scene graph a vertex, colour, and texture buffer
void initNode(Node * node){
	vector<GLfloat> vert = vector<GLfloat>();
	vector<GLfloat> colours = vector<GLfloat>();
	//convert vertices to a vector of floats instead of a vector of vec3
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
	glfwMakeContextCurrent(window);
	glEnable(GL_DEPTH_TEST);
	// query and print out information about our OpenGL environment
	QueryGLVersion();
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	//keyboard input function which switches between first person and thrid person views
	auto keys = [](GLFWwindow * window, int key, int scancode, int action, int mods){
		if (action == GLFW_PRESS){
			if (key == GLFW_KEY_V){
				firstPerson = !firstPerson;
			}
		}
	};
	glfwSetKeyCallback(window, keys);

	// call function to load and compile shader programs
	MyShader shader;
	mat4 projection;
	mat4 view;
	//set up the projection matrix
	projection = perspective((75 * (float)M_PI / 180), WIDTH / HEIGHT, 5000.0f, 0.1f);
	
	//list of drawable objects in the scene
	vector<Node *> sceneObjects = vector<Node *>();
	//root of the scene graph
	Node * root = new Node();
	BSpline * curve = new BSpline();

	vec3 pos; 
	float ind = 0.0f;
	float H;
	float speed;
	bool decc = false;
	float deccSpeed;
	int i = 0;

	//genSkyBox(&sceneObjects, root);
	drawFloor(&sceneObjects, root, 1000, 1);
	genTrack("points3.txt", &sceneObjects, root, curve);

	//create a cart 
	Node * cart = new Node();
	root->addChild(cart);
	sceneObjects.push_back(cart);
	makeCart(&(cart->vertices));
	cart->setScale(vec3(0.5, 2.0, 0.5));
	fillColour(vec3(0, 0, 0), &(cart->colours), cart->vertices.size());

	//starting position of the cart
	pos = curve->vertices.at(0);
	//maximum height on the coaster
	H = max_height(curve->vertices);
	
	InitializeShaders(&shader);
	//initialize each of the object's vertex buffer, texture buffer, and colour buffer
	for (int i = 0; i < sceneObjects.size(); i++){
		initNode(sceneObjects.at(i));
	}
	while (!glfwWindowShouldClose(window))
	{	
		glClearColor(0.196078, 0.6, 0.8, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shader.program);

		root->transform(mat4(1.0f));
		//begin decceleration here
		if (i >= 400){
			if (!decc){
				//speed before deceleration begins
				deccSpeed = SPEED + sqrt(2.0f * GRAVITY * (H - pos.y));
				decc = true;
			}
			//the end of the track
			vec3 end = curve->vertices.at(curve->vertices.size() - 1);
			//distance from current position to the end 
			float dDec = length(end - pos);
			//distance from the point where deceleration starts to the end
			float lDec = length(end - curve->vertices.at(400));
			//new speed
			speed = deccSpeed * (dDec / lDec);
		}
		else{
			decc = false;
			speed = SPEED + sqrt(2.0f * GRAVITY * (H - pos.y));
		}
		//calculate the change in position
		float deltaS = speed * DELTA_T;
		//determine where along the curve to move the cart based upon how far the cart moved
		pos = move(pos, &ind, curve->vertices, deltaS);

		//move the cart along the curve deltaS, and rotate it so that it remains on the rails
		i = (int)ind;
		vec3 gravityComp = normalize(curve->norms.at(i) * (speed  * speed) / length(curve->norms.at(i)) + vec3(0, GRAVITY, 0));
		vec3 binorm = normalize(cross(curve->tangents.at(i), gravityComp));
		cart->setRotation(mat4(vec4(curve->tangents.at(i), 0), vec4(gravityComp, 0), vec4(binorm, 0), vec4(0, 0, 0, 1)));
		//add 0.07 to the pos inorder to account for the height of the cart
		cart->setTranslation(pos + curve->norms.at(i) * 0.07f);

		//switch between first and third person 
		if (firstPerson){
			//move the camera to follow the cart
			view = lookAt(pos + 1.5f * curve->norms.at(i) + 1.5f * curve->tangents.at(i), curve->vertices.at((i + 20) % curve->vertices.size()), curve->norms.at(i));
		}
		else{
			view = lookAt(vec3(0,0,150), vec3(0,0,0), vec3(0, 1, 0));
		}
		for (int i = 0; i < sceneObjects.size(); i++){
			Node * n = sceneObjects.at(i);
			glBindVertexArray(n->vertexArray);
			//calculate mvp matrix for this object
			shader.mvp = projection * view * n->getGlobalTransform();
			glUniformMatrix4fv(shader.mvpNum, 1, GL_FALSE, value_ptr(shader.mvp));
			glUniform1i(shader.texturize, (i <= 0) ? 1 : 0);

			glBindTexture(n->tex.target, n->tex.textureID);
			glTexImage2D(n->tex.target, 0, n->tex.format, n->tex.width, n->tex.height, 0, n->tex.format, GL_UNSIGNED_BYTE, n->tex.data);

			glClear(GL_DEPTH_BUFFER_BIT);
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
	vec3 newpos;
	int cur = (int)(*ind);
	int next = (cur + 1) % cPoints.size();
	float s = (float)(*ind) - cur;
	float newDS = length(cPoints.at(next) - start);
	if (newDS > deltaS){
		if (cPoints.at(next) != cPoints.at(cur)){
			newpos = start + deltaS * normalize(cPoints.at(next) - cPoints.at(cur));
			float pr = (float)(cur + length(newpos - cPoints.at(cur)) / length(cPoints.at(next) - cPoints.at(cur)));
			*ind = pr;
		}
		else {
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
	while (newDS + length(cPoints.at(next) - cPoints.at(cur)) < deltaS){
		newDS = newDS + length(cPoints.at(next) - cPoints.at(cur));
		cur = next;
		next = (next + 1) % cPoints.size();
	}
	if (cPoints.at(next) != cPoints.at(cur)){
		newpos = cPoints.at(cur) + (deltaS - newDS) * normalize(cPoints.at(next) - cPoints.at(cur));
		float pr = (float)(cur + length(newpos - cPoints.at(cur)) / length(cPoints.at(next) - cPoints.at(cur)));
		*ind = pr;
	}
	else {
		newpos = cPoints.at(next);
		*ind = (float)next;
	}
	if (*ind >= cPoints.size()){
		*ind -= cPoints.size();
	}	
	return newpos;
}
//----Functions that create vertices for 3D rails of the track-----

void genTrack(string file, vector<Node *> * sceneObjects, Node * root, BSpline * curve){
	Node * track = new Node();
	vector<vec3> controlPoints;
	vector<vec3> trackLeft;
	vector<vec3> trackRight;
	vector<vec3> midTrack;
	vector<vec3> bars;

	readControlPoints(file, &controlPoints);
	bspline(controlPoints, &(curve->vertices), 6);
	//get vertices for the curve of the left, right, and center tracks as well as the tangents, normals, and binormals of the curve
	railVertices(curve, &trackRight, &trackLeft, &midTrack, &bars);
	//vertices for the right, left, and center tracks
	rail(trackLeft, &(track->vertices), 0.1f, curve->tangents, curve->norms, curve->binorms);
	rail(trackRight, &(track->vertices), 0.1f, curve->tangents, curve->norms, curve->binorms);
	rail(midTrack, &(track->vertices), 0.2f, curve->tangents, curve->norms, curve->binorms);
	for (int i = 0; i < bars.size(); i++){
		track->vertices.push_back(bars.at(i));
	}
	//poles that shoot down from the curve every 8 points
	supportPoles(curve, midTrack, &(track->vertices), 0.1f);
	//fill the colour buffer for the track with a scarlet color
	fillColour(vec3(0.55, 0.09, 0.09), &(track->colours), track->vertices.size());
	root->addChild(track);
	sceneObjects->push_back(track);
}
//A function which generates a series of poles coming down from the curve every 8 points
void supportPoles(BSpline * curve, vector<vec3> rail, vector<vec3> * vertices, float r){
	//the tangent of a horizontal pole will be a pointing down in the y axis
	vec3 tangent = vec3(0, -1, 0);
	//the normal will be along the x axis
	vec3 norm = vec3(-1, 0, 0);
	//the binormal is perpendicular to both
	vec3 binorm = normalize(cross(tangent, norm));

	//for every 8th point create a pole
	for (int i = 0; i < rail.size(); i+=10){
		if (curve->norms.at(i).y >= 0){
			vec3 point = rail.at(i);
			vec3 point2 = vec3(point.x, 0, point.z);
			float inc = (2.0f * M_PI) / 100;
			//for each segment (there will be 100) along a circle of radius r that is centered at the point on the roller coaster
			//form a rectangle that extends to the ground 
			for (float u = 0.0f, j = 0; j < 100; u+= inc, j++){
				vertices->push_back(r * cos(u) * binorm + r *sin(u) * cross(tangent, binorm) + point);
				vertices->push_back(r * cos(u + inc) * binorm + r *sin(u + inc) * cross(tangent, binorm) + point2);
				vertices->push_back(r * cos(u) * binorm + r *sin(u) * cross(tangent, binorm) + point2);

				vertices->push_back(r * cos(u) * binorm + r *sin(u) * cross(tangent, binorm) + point);
				vertices->push_back(r * cos(u + inc) * binorm + r *sin(u + inc) * cross(tangent, binorm) + point);
				vertices->push_back(r * cos(u + inc) * binorm + r *sin(u + inc) * cross(tangent, binorm) + point2);
			}
		}

	}
}
//A function which takes any set of vertices and creates circles around each point then connects those circles to make a curved rail
void rail(vector<vec3> vertices, vector<vec3> * pipe, float r, vector<vec3> tangents, vector<vec3> norms, vector<vec3> binorms){
	const int div = 20;
	const float inc = (2.0f * M_PI) / div;
	vector<vec3> tempPipe = vector<vec3>();
	for (int i = 0; i < vertices.size(); i++){		
		for (float u = 0.0f, j = 0; j < div; u += inc, j++){
			tempPipe.push_back(r * cos(u) * binorms.at(i) + r * sin(u) * cross(tangents.at(i), binorms.at(i)) + vertices.at(i));
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
}
//A function that generates the right rail, left rail, central (lower) rail, joints between rails, and the tangent, normal, binormals of a given curve
void railVertices(BSpline * curve, vector<vec3> * rightRail, vector<vec3> * leftRail, vector<vec3> * midRail, vector<vec3> * bars){
	
	for (int i = 0; i < curve->vertices.size(); i++){
		vec3 norm;
		vec3 binorm;
		vec3 point = curve->vertices.at(i == 0 ? curve->vertices.size() - 1 : i - 1); 
		vec3 point2 = curve->vertices.at((i< curve->vertices.size() - 1) ? i + 1 : 0);

		vec3 tangent = normalize(point2 - point);
		vec3 v = cross(vec3(1, 0, 0), tangent);
		float s = length(v);
		float c = dot(tangent, vec3(1, 0, 0));
		if (tangent == vec3(-1, 0, 0)){
			norm = vec3(0, 1, 0);
			binorm = vec3(0, 0, 1);
		}
		else{
			//http://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d
			mat3 skew = mat3(vec3(0.0f, v.z, -v.y), vec3(-v.z, 0.0f, v.x), vec3(v.y, -v.x, 0.0f));
			mat3 R = mat3(1.0f) + skew + (skew * skew) * ((1.0f - c) / (s * s));

			norm = R * vec3(0, 1, 0);
			binorm = R * vec3(0, 0, 1);
		}
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

		curve->tangents.push_back(tangent);
		curve->norms.push_back(norm);
		curve->binorms.push_back(binorm);
	}
}

//Functions that creates a b-spline curve using the subdivision method
void bspline(vector<vec3> controlPoints, vector<vec3> * vertices, int iter){
	if (iter == 0){
		for (int i = 0; i < controlPoints.size(); i++){
			vertices->push_back(controlPoints.at(i));
		}
	}
	else{
		//find the point between each current point and add it to a new list of points
		vector<vec3> temp = vector<vec3>();
		for (int i = 0; i < controlPoints.size(); i++){
			vec3 point1 = controlPoints.at(i);
			vec3 point2 = controlPoints.at((i + 1) % controlPoints.size());
			vec3 newPoint = (point1 + point2) * 0.5f;

			temp.push_back(point1);
			temp.push_back(newPoint);
		}
		vector<vec3> newControlPoints = vector<vec3>();
		//shift the set of new points so that each point is between it's original position 
		//and it's neighbours original position
		for (int i = 0; i < temp.size(); i++){
			newControlPoints.push_back(0.5f * (temp.at(i) + temp.at((i + 1) % temp.size())));
		}
		//continue to subdivide with these new points
		bspline(newControlPoints, vertices, iter - 1);
	}
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
	LoadTexture(&(rightBox->tex), "posx.png");
	LoadTexture(&(leftBox->tex), "negx.png");
	LoadTexture(&(topBox->tex), "posy.png");
	LoadTexture(&(botBox->tex), "negy.png");
	LoadTexture(&(frontBox->tex), "posz.png");
	LoadTexture(&(backBox->tex), "negz.png");
	//generate vertices and texture coordinates for each face
	skyBox(topBox, botBox, leftBox, rightBox, frontBox, backBox, rightBox->tex.width, 500);
}

void drawFloor(vector<Node *> * obj , Node * root, float width, float imageWidth){
	float d = width / 2;
	Node * floor = new Node();
	LoadTexture(&(floor->tex), "negy.png");
	//bottom side
	floor->vertices.push_back(vec3(-d, 0, -d));
	floor->vertices.push_back(vec3(d, 0, -d));
	floor->vertices.push_back(vec3(d, 0, d));

	floor->vertices.push_back(vec3(d, 0, d));
	floor->vertices.push_back(vec3(-d, 0, d));
	floor->vertices.push_back(vec3(-d, 0, -d));

	squareUV(&(floor->map), floor->tex.width);
	obj->push_back(floor);
	root->addChild(floor);

}
//A function that generates the vertices for each of the faces of the sky box
void skyBox(Node * top, Node * bot, Node * left, Node * right, Node * front, Node * back, float imageWidth, float width){
	float d = width / 2;
	//front side
	front->vertices.push_back(vec3(-d, width, d));
	front->vertices.push_back(vec3(d, width, d));
	front->vertices.push_back(vec3(d, 0, d));

	front->vertices.push_back(vec3(d, 0, d));
	front->vertices.push_back(vec3(-d, 0, d));
	front->vertices.push_back(vec3(-d, width, d));

	squareUV(&(front->map), imageWidth);

	//back side
	back->vertices.push_back(vec3(-d, width, -d));
	back->vertices.push_back(vec3(d, width, -d));
	back->vertices.push_back(vec3(d, 0, -d));

	back->vertices.push_back(vec3(d, 0, -d));
	back->vertices.push_back(vec3(-d, 0, -d));
	back->vertices.push_back(vec3(-d, width, -d));

	squareUV(&(back->map), imageWidth);

	//left side
	left->vertices.push_back(vec3(-d, width, -d));
	left->vertices.push_back(vec3(-d, width, d));
	left->vertices.push_back(vec3(-d, 0, d));

	left->vertices.push_back(vec3(-d, 0, d));
	left->vertices.push_back(vec3(-d, 0, -d));
	left->vertices.push_back(vec3(-d, width, -d));
	
	squareUV(&(left->map), imageWidth);

	//right side
	right->vertices.push_back(vec3(d, width, d));
	right->vertices.push_back(vec3(d, width, -d));
	right->vertices.push_back(vec3(d, 0, -d));

	right->vertices.push_back(vec3(d, 0, -d));
	right->vertices.push_back(vec3(d, 0, d));
	right->vertices.push_back(vec3(d, width, d));

	squareUV(&(right->map), imageWidth);

	//top side
	top->vertices.push_back(vec3(-d, width, -d));
	top->vertices.push_back(vec3(d, width, -d));
	top->vertices.push_back(vec3(d, width, d));

	top->vertices.push_back(vec3(d, width, d));
	top->vertices.push_back(vec3(-d, width, d));
	top->vertices.push_back(vec3(-d, width, -d));

	squareUV(&(top->map), imageWidth);

	//bottom side
	bot->vertices.push_back(vec3(-d,0, -d));
	bot->vertices.push_back(vec3(d,0, -d));
	bot->vertices.push_back(vec3(d,0, d));

	bot->vertices.push_back(vec3(d, 0, d));
	bot->vertices.push_back(vec3(-d,0, d));
	bot->vertices.push_back(vec3(-d,0, -d));

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