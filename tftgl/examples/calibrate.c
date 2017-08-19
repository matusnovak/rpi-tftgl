#include <stdlib.h>
#include <stdio.h>
#include "kbhit.h"

// Add TFTGL library
#include <tftgl.h>

// Add NANOVG library
#include <nanovg.h>
#define NANOVG_GLES2_IMPLEMENTATION	// Use GLES 2 implementation.
#include <nanovg_gl.h>

//==============================================================================
int main(int argv, char** argc){
	unsigned int x, y, z;
	unsigned int i;
	unsigned int width, height;
	unsigned int calibData[4][2];
	unsigned int calibXMin, calibXMax, calibYMin, calibYMax;
	
	// Initialize tftgl!
	if(tftglInit(TFTGL_LANDSCAPE) != TFTGL_OK){
		fprintf(stderr, "Failed to initialize TFTGL library! Error: %s\n",
			tftglGetErrorStr());
		return EXIT_FAILURE;
	}
	
	width = tftglGetWidth();
	height = tftglGetHeight();
	
	// Set brightness to full 100%
	// 100% -> 255
	// 50% -> 128
	// 0% (OFF) -> 0
	tftgSetBrightness(255);
	
	// In case you need to change the sensitivity, use this.
	// Default: 100
	tftglSetTouchSensitivity(150);
	
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	
	struct NVGcontext* vg = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
	
	// Four points on the screen, each 10% from the edges
	unsigned int pos[4][2] = {
		{ width / 10, height / 10}, // 10% X and 10% Y
		{ width - width / 10, height / 10}, // 90% X and 10% Y
		{ width - width / 10, height - height / 10}, // 90% X and 90% Y
		{ width / 10, height - height / 10}, // 10% X and 90% Y
	};
	
	// Render the 4 points
	for(i = 0; i < 4; i++){
		printf("Press the red dot on the screen.\n");
		printf("Press any key on the keyboard to terminate!\n");
		
		// Begin nanovg drawing
		nvgBeginFrame(vg, width, height, 1.0);
		
		glClear(GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
		
		// Display red dot on the screen
		nvgBeginPath(vg);
		nvgCircle(vg, pos[i][0], pos[i][1], 10.0);
		nvgFillColor(vg, nvgRGBA(255, 0, 0, 255));
		nvgFill(vg);
		
		nvgEndFrame(vg);
		tftglUploadFbo();
		
		// Wait for touch
		while(1){
			// Keyboard press detected, terminate application!
			if(kbhit()){
				nvgDeleteGLES2(vg);
				tftglTerminate();
				return EXIT_SUCCESS;
			}
			
			// Check if we have a touch, we do not care about x and y
			// by tftglGetTouch() as the data would be meaningless.
			// We only want x and y as RAW data from tftglGetTouchRaw()
			if(tftglGetTouch(NULL, NULL) == TFTGL_GOT_TOUCH){
				tftglGetTouchRaw(&x, &y, NULL);
				printf("Got touch!\n");
				usleep(500 * 1000); // Some delay for slow fingers
				break;
			} else {
				// Nothing, wait
			}
		}
		
		calibData[i][0] = x;
		calibData[i][1] = y;
	}
	
	// Min X is composed from left points (top left, bottom left)
	// Max X is composed from right points (top right, bottom right)
	calibXMin = (calibData[0][0] + calibData[3][0]) / 2;
	calibXMax = (calibData[1][0] + calibData[2][0]) / 2;
	
	// Min Y is composed from top points (top left, top right)
	// Max Y is composed from bottom points (bottom left, bottom right)
	calibYMin = (calibData[0][1] + calibData[1][1]) / 2;
	calibYMax = (calibData[2][1] + calibData[3][1]) / 2;
	
	printf("Calibration data:\n");
	printf("X min: %d at pos: %d\n", calibXMin, pos[0][0]);
	printf("X max: %d at pos: %d\n", calibXMax, pos[1][0]);
	printf("Y min: %d at pos: %d\n", calibYMin, pos[0][1]);
	printf("Y max: %d at pos: %d\n", calibYMax, pos[3][1]);
	printf("Add the following into your code:\n");

    printf("tftglSetTouchCalibration(TFTGL_CALIB_MIN_X, %d, %d);\n", calibXMin, pos[0][0]);
    printf("tftglSetTouchCalibration(TFTGL_CALIB_MAX_X, %d, %d);\n", calibXMax, pos[1][0]);
    printf("tftglSetTouchCalibration(TFTGL_CALIB_MIN_Y, %d, %d);\n", calibYMin, pos[0][1]);
    printf("tftglSetTouchCalibration(TFTGL_CALIB_MAX_Y, %d, %d);\n", calibYMax, pos[3][1]);
    printf("This is hardware specific data! You will need to include this\n");
    printf("in your program in order to use the touchscreen!\n");

	tftglSetTouchCalibration(TFTGL_CALIB_MIN_X, calibXMin, pos[0][0]); // 3617
	tftglSetTouchCalibration(TFTGL_CALIB_MAX_X, calibXMax, pos[1][0]); // 430
	tftglSetTouchCalibration(TFTGL_CALIB_MIN_Y, calibYMin, pos[0][1]); // 3543
	tftglSetTouchCalibration(TFTGL_CALIB_MAX_Y, calibYMax, pos[3][1]); // 812
	
	// Run forever
    int gotTouch = 0;
	while(1){
		// Begin nanovg drawing
		nvgBeginFrame(vg, width, height, 1.0);
		//glClear(GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
		
		// Wait for touch
		while(1){
			// Keyboard press detected, terminate application!
			if(kbhit()){
				nvgDeleteGLES2(vg);
				tftglTerminate();
				return EXIT_SUCCESS;
			}
			
			// Check if we have a touch
			if(gotTouch == 0 && tftglGetTouch(&x, &y) == TFTGL_GOT_TOUCH){
                gotTouch = 1; // Mark flag so we know we already got touch!
				printf("Got touch at %d, %d\n", x, y);
				nvgBeginPath(vg);
				nvgCircle(vg, x, y, 10.0);
				nvgFillColor(vg, nvgRGBA(0, 0, 255, 255));
				nvgFill(vg);
				break;
			} else {
				// Nothing, wait
                gotTouch = 0;
			}
		}
		
		nvgEndFrame(vg);
		tftglUploadFboArea(x-10, y-10, 20, 20);
	}
	
	nvgDeleteGLES2(vg);
	
	// Terminate everything
	tftglTerminate();
	
	return EXIT_SUCCESS;
}