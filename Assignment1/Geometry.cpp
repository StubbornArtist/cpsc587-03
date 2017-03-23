#include "Geometry.h"

Geometry::Geometry() {
	vertexArray = 0;
	vertexBuffer = 0;
	colourBuffer = 0;
	verticeCount = 0;
	initialize();
}
void Geometry::initialize() {
	const GLuint VERTEX_INDEX = 0;
	const GLuint COLOUR_INDEX = 1;

	glGenBuffers(1, &vertexBuffer);
	glGenBuffers(1, &colourBuffer);

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
//change the vertices
void Geometry::reloadVertices(vector<float> vertices) {
	verticeCount = vertices.size() / 3;
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
}
//change the colours
void Geometry::reloadColours(vector<float> colours) {
	glBindBuffer(GL_ARRAY_BUFFER, colourBuffer);
	glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(float), colours.data(), GL_STATIC_DRAW);
}
//draw vertices in given colours
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