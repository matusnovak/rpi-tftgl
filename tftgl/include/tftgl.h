#ifndef _TFTGL_LIB_H_
#define _TFTGL_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

// Return values
#define TFTGL_ERROR (0)
#define TFTGL_OK (1)

#define TFTGL_GOT_TOUCH (1)
#define TFTGL_NO_TOUCH (0)

// Error codes
#define TFTGL_GPIO_ERROR (2)
#define TFTGL_SPI_ERROR (3)
#define TFTGL_NO_DISPLAY (4)
#define TFTGL_BAD_DISPLAY (5)
#define TFTGL_BAD_CONFIG (6)
#define TFTGL_BAD_GLAPI (7)
#define TFTGL_BAD_SURFACE (8)
#define TFTGL_BAD_CONTEXT (9)
#define TFTGL_BAD_WIDTH (10)
#define TFTGL_BAD_HEIGHT (11)
#define TFTGL_OUT_OF_MEM (12)

// Flags
#define TFTGL_LANDSCAPE (0x0)
#define TFTGL_PORTRAIT (0x1)
#define TFTGL_ROTATE_180 (0x2)
#define TFTGL_MSAA (0x4)

#define TFTGL_CALIB_MIN_X (0)
#define TFTGL_CALIB_MAX_X (1)
#define TFTGL_CALIB_MIN_Y (2)
#define TFTGL_CALIB_MAX_Y (3)

typedef struct TftglEglDataStruct {
	EGLDisplay display;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;
	int versionMajor;
	int versionMinor;
} TftglEglData;

// Common TFTGL functions
extern unsigned int tftglInit(unsigned int flags);
extern void tftglTerminate();
extern unsigned int tftglGetWidth();
extern unsigned int tftglGetHeight();
extern unsigned int tftglGetError();
extern const char* tftglGetErrorStr();

// LCD functions
extern void tftgSetBrightness(unsigned char val);
extern void tftglFillPixels(unsigned int x, unsigned int y, 
	unsigned int w, unsigned int h, 
	const unsigned char* pixels);
extern void tftglGetTouchRaw(unsigned int* x, unsigned int* y, unsigned int* z);
extern unsigned int tftglGetTouch(unsigned int* x, unsigned int* y);
extern void tftglSetTouchSensitivity(unsigned int val);
extern void tftglSetTouchCalibration(unsigned int which, unsigned int val, unsigned int pos);

// EGL/OpenGL ES functions
extern unsigned int tftglEglMakeCurrent();
extern void tftglTerminateEgl();
extern TftglEglData* tftglGetEglData();
extern void tftglUploadFbo();
extern void tftglUploadFboArea(unsigned int x, unsigned int y, 
	unsigned int w, unsigned int h);

#ifdef __cplusplus
}
#endif

#endif