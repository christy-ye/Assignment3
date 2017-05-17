#pragma once
#ifndef _LINE_H
#define _LINE_H_

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
// Use of degrees is deprecated. Use radians instead.
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Line
{
public:
	Line(glm::vec3 initial, glm::vec3 terminal);
	~Line();

	glm::mat4 toWorld;
	GLuint uProjection, uModelview;

	void draw(GLint, glm::mat4);
	void update();
	void spin(float);

	// These variables are needed for the shader program
	GLuint VBO, VAO, EBO;
};

// Define the coordinates and indices needed to draw the cube. Note that it is not necessary
// to use a 2-dimensional array, since the layout in memory is the same as a 1-dimensional array.
// This just looks nicer since it's easy to tell what coordinates/indices belong where.


// Note that GL_QUADS is deprecated in modern OpenGL (and removed from OSX systems).
// This is why we need to draw each face as 2 triangles instead of 1 quadrilateral

#endif
