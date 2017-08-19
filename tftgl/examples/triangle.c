#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h> 

// Add TFTGL library
#include <tftgl.h>

// Add our helper for creating OpenGL shaders and buffers
#include "gl_helper.h"

// The following array holds vec3 data of 
// three vertex positions
static const GLfloat vboVertexData[] = {
   -1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	0.0f,  1.0f, 0.0f,
};

// The following array holds vec3 dara of 
// three vertex colors 
static const GLfloat vboColorData[] = {
    1.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
};

// The following are GLSL shaders for rendering
// a triangle on the screen
// Tutorial: https://www.khronos.org/assets/uploads/books/openglr_es_20_programming_guide_sample.pdf
#define STRINGIFY(x) #x
static const char* vertexShaderCode = STRINGIFY(
	attribute vec3 pos;
	attribute vec3 col;
	varying vec3 v_col;
	uniform float rot;
	void main() {
		v_col = col;
		float r = rot * 0.0174533; // degrees to radians
		float x = pos.x * cos(r) - pos.y * sin(r);
		float y = pos.x * sin(r) + pos.y * cos(r);
		gl_Position = vec4(x, y, pos.z, 1.0);
	}
);

static const char* fragmentShaderCode = STRINGIFY(
	varying vec3 v_col;
	void main() {
		gl_FragColor = vec4(v_col, 1.0);
	}
);

//==============================================================================
int main(int argv, char** argc){
	unsigned int i;
	struct timeval t1, t2;
	GLuint vbo[2];
	GLuint program;
	double elapsedTime;
	
	// Initialize tftgl!
	// Alternatively, run tftglGpioInit(), then tftglInitDisplay(),
	// and then tftglInitEgl()
	if(tftglInit(TFTGL_LANDSCAPE) != TFTGL_OK){
		fprintf(stderr, "Failed to initialize TFTGL library! Error: %s\n",
			tftglGetErrorStr());
		return EXIT_FAILURE;
	}
	
	// Set brightness to full 100%
	tftgSetBrightness(255);
	
	// Get viewport size
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	
	printf("TFT display initialized with EGL! Screen size: %dx%d\n",
		viewport[2], viewport[3]);
	
	printf("OpenGL version is (%s)\n", glGetString(GL_VERSION));
	
	// See gl_helper.h for more infor or read any OpenGL tutorial for compiling
	// GLSL shaders (there are quite many on Google).
	program = compileShader(vertexShaderCode, fragmentShaderCode);
	if(program == 0){
		printf("Failed to create OpenGL shader!\n");
		tftglTerminate();
		return EXIT_FAILURE;
	}
	
	// Create two Vertex Buffer Objects
	glGenBuffers(2, vbo);
	if(vbo[0] == 0 || vbo[1] == 0){
		printf("Failed to create GL_ARRAY_BUFFER! Error: %s\n", glGetErrorStr());
		tftglTerminate();
		return EXIT_FAILURE;
	}
	
	// The first buffer holds vertex positions
	// 3 vertices of 3D vectors of float data
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 3 * 3 * sizeof(float), vboVertexData, GL_STATIC_DRAW);
	
	// The second buffer holds color data for each vertex
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 3 * 3 * sizeof(float), vboColorData, GL_STATIC_DRAW);
	
	// Get attribute and uniform pointers from GLSL shader (program)
	glUseProgram(program);
	GLint posLoc = glGetAttribLocation(program, "pos");
	GLint colorLoc = glGetAttribLocation(program, "col");
	GLint rotLoc = glGetUniformLocation(program, "rot");
	
	// Start rendering
	gettimeofday(&t1, NULL);
	for(i = 0; i <= 36; i++){
		// Clean screen with black color
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	
		// Bind first vertex buffer
		glEnableVertexAttribArray(posLoc);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		
		// Bind second vertex buffer
		glEnableVertexAttribArray(colorLoc);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		
		// Set rotation
		glUniform1f(rotLoc, i * 10.0f);
		
		// Draw triangles, total of 3 vertices
		glDrawArrays(GL_TRIANGLES, 0, 3);
		
		// Copy pixesl from OpenGL buffer into TFT LCD Display (time costly!)
		tftglUploadFbo();
	}
	// End of rendering
	gettimeofday(&t2, NULL);
	
	// Calculate duration
	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0;
	printf("Rendering of 37 frames took: %fms\n", elapsedTime);
	printf("Calculated FPS: %f\n", 1.0 / (elapsedTime / 37000.0));
	
	// Cleanup GLSL shader and two buffers
	glDeleteProgram(program);
	glDeleteBuffers(2, vbo);
	
	// Terminates everything (GPIO, LCD, and EGL)
	tftglTerminate();
	
	return EXIT_SUCCESS;
}