#include "Screen.h"
#include "Line.h"
Cube * trial; 
Skybox * leftSky;
Skybox * rightSky; 

GLuint Window::tempfbo; 
using namespace std; 
using namespace glm;  

Screen::Screen(glm::vec3 pa, glm::vec3 pb, glm::vec3 pc, glm::vec3 pe) {

	SWorld = glm::mat4(1.0f) * glm::scale(glm::mat4(1.0f), glm::vec3(40.0f, 40.0f, 40.0f)); 
	trial = new Cube(); 
	leftSky = new Skybox(); 
	rightSky = new Skybox(); 

	vector <const GLchar *> faces;
	vector <const GLchar *> leftFaces;
	vector <const GLchar*> rightFaces; 

	for (int i = 0; i < 6; i++) {
		faces.push_back("H:/Assignment3/Assignment2/textures/vr_test_pattern.ppm");
	}

	trial->loadCubemap(faces);

	//load left eye
	leftFaces.push_back("H:/Assignment3/Assignment2/textures/left-ppm/px.ppm");
	leftFaces.push_back("H:/Assignment3/Assignment2/textures/left-ppm/nx.ppm");
	leftFaces.push_back("H:/Assignment3/Assignment2/textures/left-ppm/py.ppm");
	leftFaces.push_back("H:/Assignment3/Assignment2/textures/left-ppm/ny.ppm");
	leftFaces.push_back("H:/Assignment3/Assignment2/textures/left-ppm/pz.ppm");
	leftFaces.push_back("H:/Assignment3/Assignment2/textures/left-ppm/nz.ppm");

	leftSky->loadCubemap(leftFaces); 

	//load right eye
	rightFaces.push_back("H:/Assignment3/Assignment2/textures/right-ppm/px.ppm");
	rightFaces.push_back("H:/Assignment3/Assignment2/textures/right-ppm/nx.ppm");
	rightFaces.push_back("H:/Assignment3/Assignment2/textures/right-ppm/py.ppm");
	rightFaces.push_back("H:/Assignment3/Assignment2/textures/right-ppm/ny.ppm");
	rightFaces.push_back("H:/Assignment3/Assignment2/textures/right-ppm/pz.ppm");
	rightFaces.push_back("H:/Assignment3/Assignment2/textures/right-ppm/nz.ppm");

	rightSky->loadCubemap(rightFaces); 

	//resize
	leftSky->skyWorld = leftSky->skyWorld * glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.0f, 0.0f));
	rightSky->skyWorld = rightSky->skyWorld * glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f));

	sa = pa;
	sb = pb;
	sc = pc;
	pos = pe; 

	glm::vec3 a = glm::vec3(SWorld * glm::vec4(sa, 1.f));
	glm::vec3 b = glm::vec3(SWorld * glm::vec4(sb, 1.f));
	glm::vec3 c = glm::vec3(SWorld * glm::vec4(sc, 1.f));
	glm::vec3 e = glm::vec3(SWorld * glm::vec4(pos, 1.0f));

	//cerr << a.x << " " << a.y << "  " << a.z << endl;

	Screen::Projection = projection(a, b, c, e, 0.1f, 1000.0f);

	//Screen::Projection = projection(sa, sb, sc, pos, 0.1f, 1000.0f);

	//////////////////////////////////////////////////////////////////////////////
	// Setup screen VAO
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), &screenVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glBindVertexArray(0);

	// Framebuffers
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	// Create a color attachment texture
	textureColorbuffer = generateAttachmentTexture(false, false);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
	
	/////////////////////////////
	//second framebuffer for stereo
	//glGenFramebuffers(1, &framebufferR);
	//glBindFramebuffer(GL_FRAMEBUFFER, framebufferR);
	//// Create a color attachment texture
	//textureColorbufferR = generateAttachmentTexture(false, false);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbufferR, 0);
	/////////////////////////////


	// Create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	//SAME SIZE 
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, Window::width/2, Window::height); // Use a single renderbuffer object for both a depth AND stencil buffer.
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo); // Now actually attach it
						
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureColorbuffer, 0);
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureColorbufferR, 0);	

	//std::cout << "Color buffer: " << textureColorbuffer << std::endl;
	//std::cout << "Depth buffer: " << rbo << std::endl;
	//cerr << "x: " << Window::width << endl; 
	//cerr << "y: " << Window::height << endl;

	// Now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

Screen::~Screen()
{
	// Delete previously generated buffers. Note that forgetting to do this can waste GPU memory in a 
	// large project! This could crash the graphics driver due to memory leaks, or slow down application performance!
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteFramebuffers(1, &framebuffer);
	//glDeleteFramebuffers(1, &framebufferR);
	delete(trial); 
	delete(leftSky); 
}

void Screen::render(GLuint shaderProgram, GLuint frameShader, GLuint skyShader, int mode) {

	//glm::vec3 a = glm::vec3(SWorld * glm::vec4(sa, 1.f));
	//glm::vec3 b = glm::vec3(SWorld * glm::vec4(sb, 1.f));
	//glm::vec3 c = glm::vec3(SWorld * glm::vec4(sc, 1.f));
	//glm::vec3 e = glm::vec3(SWorld * glm::vec4(pos, 1.0f));

	////cerr << a.x << " " << a.y << "  " << a.z << endl;

	//Screen::Projection = projection(a, b, c, e, 0.1f, 1000.0f);

	if (mode == 0) {
		//// First pass
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glViewport(Window::xPos, Window::yPos, Window::width/2,Window::height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
		glEnable(GL_DEPTH_TEST);
	
		trial->draw(shaderProgram);
		
		//if (mode == 0) {
		//leftSky->draw(skyShader);
		//}
		//if (mode == 1) {
		//	rightSky->draw(skyShader); 
		//}
		glBindFramebuffer(GL_FRAMEBUFFER, Window::tempfbo); // back to default
		//glViewport(Window::xPos, Window::yPos, Window::width/2, Window::height);
	}
	if (mode == 1) {
		/////////
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glViewport(Window::xPos, Window::yPos, Window::width/2, Window::height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
		glEnable(GL_DEPTH_TEST);

		trial->draw(shaderProgram);
		//rightSky->draw(skyShader);

		glBindFramebuffer(GL_FRAMEBUFFER, Window::tempfbo); // back to default
		//glViewport(Window::xPos, Window::yPos, Window::width/2, Window::height);
	}
		//glDisable(GL_DEPTH_TEST);

	//////////////
	glUseProgram(frameShader);

	glm::mat4 modelview = Window::V * SWorld;
	// We need to calcullate this because modern OpenGL does not keep track of any matrix other than the viewport (D)
	// Consequently, we need to forward the projection, view, and model matrices to the shader programs
	// Get the location of the uniform variables "projection" and "modelview"
	uProjection = glGetUniformLocation(frameShader, "sprojection");
	uModelview = glGetUniformLocation(frameShader, "smodelview");
	// Now send these values to the shader program

	//////////////////////////////
	//NOTE:: changed the perspective to take in P calculated from Screen
	glUniformMatrix4fv(uProjection, 1, GL_FALSE, &Window::P[0][0]);
	glUniformMatrix4fv(uModelview, 1, GL_FALSE, &modelview[0][0]);
	glBindVertexArray(quadVAO);
	//glActiveTexture(GL_TEXTURE0); // diff 
	//glUniform1i(glGetUniformLocation(frameShader, "screenTexture"), 0);//diff
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	//cerr << "frame: " << framebuffer << endl; 
}

glm::mat4 Screen::projection(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 e, float n, float f) {

	glm::vec3 va, vb, vc; 
	glm::vec3 vr, vu, vn; 
	glm::mat4 SPerspective = glm::mat4(1.0f); 

	float l, r, b2, t, d;

	vr = b - a;
	vu = c - a; 

	vr = glm::normalize(vr);
	vu = glm::normalize(vu); 

	vn = glm::normalize(glm::cross(vr, vu)); 
	
	cerr << vn.x << " " << vn.y << "  " << vn.z << endl;

	va = a - e; 
	vb = b - e;
	vc = c - e;

	d = -1.0f * glm::dot(va, vn); 

	l = glm::dot(vr, va) * n / d;
	r = glm::dot(vr, vb) * n / d;
	b2 = glm::dot(vu, va) * n / d;
	t = glm::dot(vu, vc) * n / d;

	glm::mat4 tempP = glm::mat4(1.0f); 

	tempP = glm::frustum(l,r,b2,t,n,f); 

	SPerspective= glm::mat4( glm::vec4(vr, 0.0f), glm::vec4(vu,0.0f), glm::vec4(vn, 0.0f), glm::vec4(0.0f,0.0f,0.0f,1.0f)); 
	SPerspective = glm::transpose(SPerspective);
	glm::mat4 temp =  tempP * SPerspective * glm::translate(glm::mat4(1.0f), (-1.0f * e));

	cerr << "this: " << this << endl; 
	cerr << "Sp x: " << SPerspective[2].x << endl; 
	cerr << "Sp y: " << SPerspective[2].y << endl;
	cerr << "Sp z: " << SPerspective[2].z << endl;

	return temp; 
}

// Generates a texture that is suited for attachments to a framebuffer
GLuint Screen::generateAttachmentTexture(GLboolean depth, GLboolean stencil)
{

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Window::width/2, Window::height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	cerr << "to return: " << textureID << endl; 
	return textureID;
}


void Screen::debugMode(glm::mat4 headPose, GLint shaderProgram, int mode) {

	vec4 upsd2; 
	glm::vec3 sd = sb - sa + sc;
	glm::vec3 eyes = glm::vec3(headPose[3]);
	
	
	if (mode == 0) {
		eyes = glm::vec3(headPose[3]) + glm::vec3(Window::Loffsetx, Window::Loffsety, Window::Loffsetz);
	}
	if (mode == 1) {
		eyes = glm::vec3(headPose[3]) + glm::vec3(Window::Roffsetx, Window::Roffsety, Window::Roffsetz);
		cerr << "in right" << endl; 
	}

	//cerr << "eyes " << eyes.x << " " << eyes.y << " " << eyes.z << endl;
	glm::mat4 temp;

	vec4 upsd = vec4(sd, 1.0f);
	upsd = SWorld  * upsd;

	Line *line1 = new Line(eyes, vec3(upsd));
	line1->draw(shaderProgram, headPose);
	//cerr << "sd: " << sd.x << " " << sd.y << " " << sd.z << endl;

	upsd2 = SWorld * upsd2 ;

	Line *line2 = new Line(eyes, vec3(upsd2));
	line2->draw(shaderProgram, headPose);
	//cerr << "sa: " << sa.x << " " << sa.y << " " << sa.z << endl;

	vec4 upsd3 = vec4(sc, 1.0f);
	upsd3 = SWorld * upsd3;

	Line *line3 = new Line(eyes, vec3(upsd3));
	line3->draw(shaderProgram, headPose);
	//cerr << "sc: " << sc.x << " " << sc.y << " " << sc.z << endl;

	vec4 upsd4 = vec4(sb, 1.0f);
	upsd4 = SWorld * upsd4 ;

	Line *line4 = new Line(eyes, upsd4);
	line4->draw(shaderProgram, headPose);
	//cerr << "sb: " << sb.x << " " << sb.y << " " << sb.z << endl;

}
