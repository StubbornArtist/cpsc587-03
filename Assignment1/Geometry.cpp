#include "Geometry.h"

Geometry::Geometry() {
	vertexArray = 0;
	vertexBuffer = 0;
	colourBuffer = 0;
	verticeCount = 0;
}
Geometry::Geometry(vector<float> vertices, vector<float> colours) {
	initialize(vertices, colours);
}
void Geometry::initialize(vector<float> vertices, vector<float> colours) {
	const GLuint VERTEX_INDEX = 0;
	const GLuint COLOUR_INDEX = 1;
	verticeCount = vertices.size();

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &colourBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colourBuffer);
	glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(float), colours.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(VERTEX_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(VERTEX_INDEX);

	glBindBuffer(GL_ARRAY_BUFFER, colourBuffer);
	glVertexAttribPointer(COLOUR_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(COLOUR_INDEX);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void Geometry::reloadVertices(vector<float> vertices) {
	verticeCount = vertices.size();
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
}

void Geometry::reloadColours(vector<float> colours) {
	glBindBuffer(GL_ARRAY_BUFFER, colourBuffer);
	glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(float), colours.data(), GL_STATIC_DRAW);
}

void Geometry::draw(GLenum mode) {
	glBindVertexArray(vertexArray);
	glDrawArrays(mode, 0, verticeCount);
	glBindVertexArray(0);
}

void Geometry::destroy() {
	glDeleteVertexArrays(1, &vertexArray);
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &colourBuffer);
}