#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h> 

// Add TFTGL library
#include <tftgl.h>

// Add NANOVG library
#include <nanovg.h>
#define NANOVG_GLES2_IMPLEMENTATION	// Use GLES 2 implementation.
#include <nanovg_gl.h>

static const double graphSamples[6] = {0.2, 0.4, 0.45, 0.6, 0.7, 0.5};

//==============================================================================
int main(int argv, char** argc){
	double pxRatio;
	unsigned int i;
	
	// Initialize tftgl!
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
		
	GLint width = viewport[2];
	GLint height = viewport[3];
	
	glClear(GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	
	struct NVGcontext* vg = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
	
	// Begin nanovg drawing
	nvgBeginFrame(vg, width, height, 1.0);
	
	// Draw font
	int font = nvgCreateFont(vg, "sans", "FreeSans.ttf");
	nvgFontFaceId(vg, font);
	nvgFontSize(vg, 32);
	nvgFillColor(vg, nvgRGBA(255,255,255,255));
	nvgText(vg, 20, 30, "TrueType font with nanovg!", NULL);
	
	// Draw graph
	// First, draw graph background
	NVGpaint bg = nvgLinearGradient(vg, 0,height * 0.2,0, height, nvgRGBA(0,160,192,0), nvgRGBA(0,160,192,192));
	
	nvgBeginPath(vg);
	nvgMoveTo(vg, 0, height/2); // Left middle corner
	for (i = 0; i < 6; i++){
		nvgLineTo(vg, (width / 6) * (i + 1), height * graphSamples[i]);
	}
	nvgLineTo(vg, width, height); // Bottom right
	nvgLineTo(vg, 0, height); // Bottom left
	nvgFillPaint(vg, bg);
	nvgFill(vg);
	
	// Next, draw graph line
	nvgBeginPath(vg);
	nvgMoveTo(vg, 0, height/2);
	for (i = 0; i < 6; i++){
		nvgLineTo(vg, (width / 6) * (i + 1), height * graphSamples[i]);
	}
	nvgStrokeColor(vg, nvgRGBA(0, 160, 192, 255));
	nvgStrokeWidth(vg, 3.0f);
	nvgStroke(vg);
	
	// Last, draw graph points
	nvgBeginPath(vg);
	for (i = 0; i < 6; i++){
		nvgCircle(vg, (width / 6) * (i + 1), height * graphSamples[i], 4.0);
	}
	nvgFillColor(vg, nvgRGBA(0, 160, 192, 255));
	nvgFill(vg);
	
	nvgBeginPath(vg);
	for (i = 0; i < 6; i++){
		nvgCircle(vg, (width / 6) * (i + 1), height * graphSamples[i], 2.0);
	}
	nvgFillColor(vg, nvgRGBA(220, 220, 220, 255));
	nvgFill(vg);
	
	// Draw blured font
	nvgFillColor(vg, nvgRGBA(192,80,0,255));
	nvgFontBlur(vg, 4.0);
	nvgFontSize(vg, 64);
	nvgText(vg, 20, height/2, "With Blur!", NULL);
	
	nvgFillColor(vg, nvgRGBA(128,0,255,255));
	nvgFontBlur(vg, 0.0);
	nvgFontSize(vg, 32);
	nvgText(vg, 20, height/2 + 40, "And alpha blending!", NULL);
	
	// End nanovg drawing
	nvgEndFrame(vg);
	
	// Copy pixesl from OpenGL buffer into TFT LCD Display (time costly!)
	tftglUploadFbo();
	
	nvgDeleteGLES2(vg);
	
	// Terminates everything (GPIO, LCD, and EGL)
	tftglTerminate();
	
	return EXIT_SUCCESS;
}