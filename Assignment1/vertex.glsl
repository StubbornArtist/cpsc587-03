#version 410
// location indices for these attributes correspond to those specified in the
// InitializeGeometry() function of the main program
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexColour;


// output to be interpolated between vertices and passed to the fragment stage
out vec3 Colour;

uniform mat4 mvp;

void main()
{
    // assign vertex position without modification
    gl_Position = mvp * vec4(VertexPosition ,1.0);
    // assign output colour to be interpolated
    Colour = VertexColour;
}
