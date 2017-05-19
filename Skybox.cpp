#include "Skybox.h"
#include "Window.h"

Skybox::Skybox()
{
	glm::vec3 cam_pos(0.0f, 0.0f, 20.0f);		// e  | Position of camera
	glm::vec3 cam_look_at(0.0f, 0.0f, 0.0f);	// d  | This is where the camera looks at
	glm::vec3 cam_up(0.0f, 1.0f, 0.0f);			// up | What orientation "up" is

	skyWorld = glm::mat4(1.0f);
	skyWorld = skyWorld* glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, 100.0f, 100.0f));

	staticCam = glm::lookAt(cam_pos, cam_look_at, cam_up); 

	// Create array object and buffers. Remember to delete your buffers when the object is destroyed!
	glGenVertexArrays(1, &skyVAO);
	glGenBuffers(1, &skyVBO);
//	glGenBuffers(1, &skyEBO);

	// Bind the Vertex Array Object (VAO) first, then bind the associated buffers to it.
	// Consider the VAO as a container for all your buffers.
	glBindVertexArray(skyVAO);

	// Now bind a VBO to it as a GL_ARRAY_BUFFER. The GL_ARRAY_BUFFER is an array containing relevant data to what
	// you want to draw, such as vertices, normals, colors, etc.
	glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
	// glBufferData populates the most recently bound buffer with data starting at the 3rd argument and ending after
	// the 2nd argument number of indices. How does OpenGL know how long an index spans? Go to glVertexAttribPointer.
	glBufferData(GL_ARRAY_BUFFER, sizeof(Skyvertices), Skyvertices, GL_STATIC_DRAW);
	// Enable the usage of layout location 0 (check the vertex shader to see what this is)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
		3, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		3 * sizeof(GLfloat), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

					 // We've sent the vertex data over to OpenGL, but there's still something missing.
					 // In what order should it draw those vertices? That's why we'll need a GL_ELEMENT_ARRAY_BUFFER for this.
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyEBO);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Skyindices), Skyindices, GL_STATIC_DRAW);

	// Unbind the currently bound buffer so that we don't accidentally make unwanted changes to it.
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Unbind the VAO now so we don't accidentally tamper with it.
	// NOTE: You must NEVER unbind the element array buffer associated with a VAO!
	glBindVertexArray(0);
}

Skybox::~Skybox()
{
	// Delete previously generated buffers. Note that forgetting to do this can waste GPU memory in a 
	// large project! This could crash the graphics driver due to memory leaks, or slow down application performance!
	glDeleteVertexArrays(1, &skyVAO);
	glDeleteBuffers(1, &skyVBO);
	glDeleteBuffers(1, &skyEBO);
}

void Skybox::draw(GLuint shaderProgram, glm::mat4 Projection)
{
	glUseProgram(shaderProgram); 
	//glDepthMask(GL_FALSE); 

	//find the position 
	glm::vec4 a = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec4 b = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	glm::vec4 c = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

	glm::vec3 trans = glm::vec3(Window::V[3].x, Window::V[3].y, Window::V[3].z);

	glm::mat4 tempHP = glm::mat4(a, b, c, glm::vec4(trans, 1.0f));

	// Calculate the combination of the model and view (camera inverse) matrices
	glm::mat4 modelview = tempHP * skyWorld;
	// We need to calcullate this because modern OpenGL does not keep track of any matrix other than the viewport (D)
	// Consequently, we need to forward the projection, view, and model matrices to the shader programs
	// Get the location of the uniform variables "projection" and "modelview"
	SProjection = glGetUniformLocation(shaderProgram, "skyprojection");
	SModelview = glGetUniformLocation(shaderProgram, "skymodelview");
	// Now send these values to the shader program
	glUniformMatrix4fv(SProjection, 1, GL_FALSE, &Projection[0][0]);
	glUniformMatrix4fv(SModelview, 1, GL_FALSE, &modelview[0][0]);
	// Now draw the cube. We simply need to bind the VAO associated with it.
	glBindVertexArray(skyVAO);

	glActiveTexture(GL_TEXTURE0); // diff 
	glUniform1i(glGetUniformLocation(shaderProgram, "skycube"), 0);//diff

	glBindTexture(GL_TEXTURE_CUBE_MAP, skytextureID); //diff

												   // Tell OpenGL to draw with triangles, using 36 indices, the type of the indices, and the offset to start from
	//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_TRIANGLES, 0,36);
	
	// Unbind the VAO when we're done so we don't accidentally draw extra stuff or tamper with its bound buffers
	glBindVertexArray(0);

	//glDepthFunc(GL_LESS); // diff

}


unsigned char* Skybox::loadPPM(const char* filename, int& width, int& height) {

	const int BUFSIZE = 128;
	FILE* fp;
	unsigned int read;
	unsigned char* rawData;
	char buf[3][BUFSIZE];
	char* retval_fgets;
	size_t retval_sscanf;

	if ((fp = fopen(filename, "rb")) == NULL)
	{
		std::cerr << "error reading ppm file, could not locate " << filename << std::endl;
		width = 0;
		height = 0;
		return NULL;
	}
	// Read magic number:
	retval_fgets = fgets(buf[0], BUFSIZE, fp);

	// Read width and height:
	do
	{
		retval_fgets = fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');
	retval_sscanf = sscanf(buf[0], "%s %s", buf[1], buf[2]);
	width = atoi(buf[1]);
	height = atoi(buf[2]);

	// Read maxval:
	do
	{
		retval_fgets = fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');

	// Read image data:
	rawData = new unsigned char[width * height * 3];
	read = fread(rawData, width * height * 3, 1, fp);
	fclose(fp);
	if (read != 1)
	{
		std::cerr << "error parsing ppm file, incomplete data" << std::endl;
		delete[] rawData;
		width = 0;
		height = 0;
		return NULL;
	}

	return rawData;

}

// This function loads a texture from file. Note: texture loading functions like these are usually
// managed by a 'Resource Manager' that manages all resources (like textures, models, audio).
// For learning purposes we'll just define it as a utility function.
void Skybox::loadCubemap(std::vector<const GLchar*> faces)
{
	glGenTextures(1, &skytextureID);

	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, skytextureID);

	for (unsigned int i = 0; i < faces.size(); i++) {

		image = loadPPM(faces[i], width, height);
		//  cerr<<"passed loadPPM: "<<endl;
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	}
	// cerr<<"outside loop"<<endl;
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	//cerr<<"textureID: "<<textureID<<endl;
	//cerr<<"end of loadcubemap"<<endl;

}

