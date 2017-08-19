#include <stdlib.h>
#include <stdio.h>
#include <tftgl.h>

//==============================================================================
int main(int argv, char** argc){
	// Initialize tftgl!
	if(tftglInit(TFTGL_LANDSCAPE) != TFTGL_OK){
		fprintf(stderr, "Failed to initialize TFTGL library! Error: %s\n",
			tftglGetErrorStr());
		return EXIT_FAILURE;
	}
	
	// Get display dimensions in pixels
	unsigned width, height;
	width = tftglGetWidth();
	height = tftglGetHeight();
	
	printf("Filling area of %dx%d\n", width, height);
	
	// Fill whole screen with red pixels
	// First is red, second is green, third is blue
	static const unsigned char color[3] = {255, 0, 0};
	// Start at position 0x0 and fill area of width x width pixels
	tftglFillColor(0, 0, width, height, color);
	
	// Set brightness to full 100%
	// 100% -> 255
	// 50% -> 128
	// 0% (OFF) -> 0
	tftgSetBrightness(255);
	
	// Terminate everything
	tftglTerminate();
	
	return EXIT_SUCCESS;
}