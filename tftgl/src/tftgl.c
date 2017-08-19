#include <tftgl.h>
#include <stdio.h>

unsigned int errorCode = TFTGL_OK;

#include "bcm2835.h"

static volatile uint32_t* gpioData = NULL;

#define GPIO_GPFSET0 (BCM2835_GPSET0/4)
#define GPIO_GPFCLR0 (BCM2835_GPCLR0/4)
 
#define GPIO_WRITE_PIN(pinnum, pinstate) \
	*(gpioData + (pinstate ? GPIO_GPFSET0 : GPIO_GPFCLR0)) = (1 << pinnum) ;

#define PULSE_LOW(reg) GPIO_WRITE_PIN(reg, LOW); GPIO_WRITE_PIN(reg, HIGH);
#define PULSE_HIGH(reg) GPIO_WRITE_PIN(reg, HIGH); GPIO_WRITE_PIN(reg, LOW);
#define SWAP(i, j) {typeof(i) t = i; i = j; j = t;}

// Include display
#include "tftgl_ssd1963.h"

// Include touchscreen driver
#include "tftgl_ads7843.h"

static unsigned char* areaPixels = NULL;
static TftglEglData eglData;

unsigned int tftglInitEgl(unsigned int flags){
	const EGLint configAttribs[] = {
		EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_ALPHA_SIZE, 0,
		EGL_DEPTH_SIZE, 0,
		EGL_STENCIL_SIZE, 1,
		EGL_SAMPLE_BUFFERS, (flags & TFTGL_MSAA ? 1 : 0),
		EGL_SAMPLES, (flags & TFTGL_MSAA ? 4 : 0),
		//EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	}; 
	  
	const EGLint pbufferAttribs[] = {
		EGL_WIDTH, tftglGetWidth(),
		EGL_HEIGHT, tftglGetHeight(),
		EGL_NONE,
	};

	const EGLint contextAttribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	
	if(tftglGetWidth() <= 0 || tftglGetWidth() > 2048){
		errorCode = TFTGL_BAD_WIDTH;
		return TFTGL_ERROR;
	}
	
	if(tftglGetHeight() <= 0 || tftglGetHeight() > 2048){
		errorCode = TFTGL_BAD_HEIGHT;
		return TFTGL_ERROR;
	}
	
	if((eglData.display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY){
		//fprintf(stderr, "Failed to get EGL display!\n");
		tftglTerminateEgl();
		errorCode = TFTGL_NO_DISPLAY;
		return TFTGL_ERROR;
	}
	
	if(eglInitialize(eglData.display, &eglData.versionMajor, &eglData.versionMinor) == EGL_FALSE){
		//fprintf(stderr, "Failed to get EGL version!\n");
		tftglTerminateEgl();
		errorCode = TFTGL_BAD_DISPLAY;
		return TFTGL_ERROR;
	}
	
	EGLint numConfigs;
	if(!eglChooseConfig(eglData.display, configAttribs, &eglData.config, 1, &numConfigs)){
		//fprintf(stderr, "Failed to get EGL config!\n");
		tftglTerminateEgl();
		errorCode = TFTGL_BAD_CONFIG;
		return TFTGL_ERROR;
	}
	
	if(eglBindAPI(EGL_OPENGL_ES_API) == EGL_FALSE){
		//fprintf(stderr, "Failed to bind GLES API!\n");
		tftglTerminateEgl();
		errorCode = TFTGL_BAD_GLAPI;
		return TFTGL_ERROR;
	}
	
	eglData.surface = eglCreatePbufferSurface(eglData.display, eglData.config, pbufferAttribs);
	if(eglData.surface == EGL_NO_SURFACE){
		//fprintf(stderr, "Failed to create EGL surface!\n");
		tftglTerminateEgl();	
		errorCode = TFTGL_BAD_SURFACE;
		return TFTGL_ERROR;
	}
	
	eglData.context = eglCreateContext(eglData.display, eglData.config, EGL_NO_CONTEXT, contextAttribs);
	if(eglData.context == EGL_NO_CONTEXT){
		//fprintf(stderr, "Failed to create EGL context!%s\n");
		tftglTerminateEgl();
		errorCode = TFTGL_BAD_CONTEXT;
		return TFTGL_ERROR;
	}
	
	tftglEglMakeCurrent();
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	return TFTGL_OK;
}

unsigned int tftglEglMakeCurrent(){
	if(eglMakeCurrent(eglData.display, eglData.surface, eglData.surface, eglData.context) != EGL_TRUE){
		return TFTGL_ERROR;
	}
	glViewport(0, 0, tftglGetWidth(), tftglGetHeight());
	return TFTGL_OK;
}

void tftglUploadFbo(){
	tftglUploadFboArea(0, 0, tftglGetWidth(), tftglGetHeight());
}

void tftglUploadFboArea(unsigned int x, unsigned int y, unsigned int w, unsigned int h){
	unsigned int width, height;
	width = tftglGetWidth();
	height = tftglGetHeight();
	
	
	
	if(x >= width || y >= height)return;
	if(w == 0 || h == 0)return;
	
	// Check area dimensions
	if(x + w >= LCD_WIDTH){
		w = LCD_WIDTH - x;
	}
	if(y + h >= LCD_HEIGHT){
		h = LCD_HEIGHT - y;
	}
	
	if(areaPixels == NULL){
		// Create pixel buffer that can hold entire screen area
		areaPixels = (unsigned char*)malloc(width * height * 3);
		if(areaPixels == NULL){
			errorCode = TFTGL_OUT_OF_MEM;
			return;
		}
	}
	
	// Get pixels from current GL framebuffer and fill the screen
	glReadPixels(x, LCD_HEIGHT - y - h, w, h, GL_RGB, GL_UNSIGNED_BYTE, areaPixels);
	tftglFillPixels(x, y, w, h, areaPixels);
}

void tftglTerminateEgl(){
	eglDestroyContext(eglData.display, eglData.context);
	eglDestroySurface(eglData.display, eglData.surface);
	eglTerminate(eglData.display);
	
	if(areaPixels != NULL){
		free(areaPixels);
		areaPixels = NULL;
	}
}

unsigned int tftglInit(unsigned int flags){
	unsigned int res;
	
	res = bcm2835_init();
	if(res != 1){
		errorCode = TFTGL_GPIO_ERROR;
		return TFTGL_ERROR;
	}
	
	gpioData = bcm2835_regbase(BCM2835_REGBASE_GPIO);
	
	res = tftglInitTouch(flags);
	if(res != TFTGL_OK)return res;
	
	res = tftglInitDisplay(flags);
	if(res != TFTGL_OK)return res;
	
	res = tftglInitEgl(flags);
	if(res != TFTGL_OK)return res;
	
	return TFTGL_OK;
}

void tftglTerminate(){
	tftglTerminateTouch();
	tftglTerminateDisplay();
	bcm2835_close();
	tftglTerminateEgl();
}

TftglEglData* tftglGetEglData(){
	return &eglData;
}

unsigned int tftglGetError(){
	unsigned int cpy = errorCode;
	errorCode = TFTGL_OK;
	return cpy;
}

const char* tftglGetErrorStr(){
	unsigned int cpy = errorCode;
	errorCode = TFTGL_OK;
	
	switch(cpy){
		case TFTGL_ERROR: return "TFTGL_ERROR (Generic error!)";
		case TFTGL_OK: return "TFTGL_OK";
		case TFTGL_SPI_ERROR: return "TFTGL_MMAP_ERROR (SPI not initialized! Are you running as root/sudo?)";
		case TFTGL_GPIO_ERROR: return "TFTGL_GPIO_ERROR (GPIO not initialized! Are you running as root/sudo?)";
		case TFTGL_NO_DISPLAY: return "TFTGL_NO_DISPLAY (Failed to get EGL display!)";
		case TFTGL_BAD_DISPLAY: return "TFTGL_BAD_DISPLAY (EGL display is invalid!)";
		case TFTGL_BAD_CONFIG: return "TFTGL_BAD_CONFIG (EGL config is invalid!)";
		case TFTGL_BAD_GLAPI: return "TFTGL_BAD_GLAPI (Could not bind OpenGL ES 2 to EGL!)";
		case TFTGL_BAD_SURFACE: return "TFTGL_BAD_SURFACE (Could not create EGL surface!)";
		case TFTGL_BAD_CONTEXT: return "TFTGL_BAD_CONTEXT (Could not create EGL context!)";
		case TFTGL_BAD_WIDTH: return "TFTGL_BAD_WIDTH (LCD has invalid width!)";
		case TFTGL_BAD_HEIGHT: return "TFTGL_BAD_HEIGHT (LCD has invalid height!)";
		case TFTGL_OUT_OF_MEM: return "TFTGL_OUT_OF_MEM (System is out of memory!)";
		default: return "TFTGL_UNKNOWN_ERROR";
	}
	return "TFTGL_UNKNOWN_ERROR";
}