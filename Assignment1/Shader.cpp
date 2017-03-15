#include "Shader.h"

void Shader::load(string vertexPath, string fragPath) {
	string vertexSource = loadSource(vertexPath);
	string fragmentSource = loadSource(fragPath);
	vertex = compile(GL_VERTEX_SHADER, vertexSource);
	fragment = compile(GL_FRAGMENT_SHADER, fragmentSource);
	// link shader program
	program = linkProgram(vertex, fragment);
	// check for OpenGL errors and return false if error occurred
	mvpNum = glGetUniformLocation(program, "mvp");
}

void Shader::destroy() {
	glUseProgram(0);
	glDeleteProgram(program);
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

GLuint Shader::getProgram() {
	return program;
}
GLuint Shader::getMVPNum() {
	return mvpNum;
}
string Shader:: loadSource(string filename) {
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

GLuint Shader::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
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

GLuint Shader:: compile(GLenum shaderType, const string &source)
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
