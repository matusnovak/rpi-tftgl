// Define the following into 1 only if D0 to D15 are consecutive
// GPIO pins without gaps!
// Example: D0 -> 12, D1 -> 13, ..., D15 -> 27
// Using consecutive GPIO increases performance.
#define LCD_DATA_CONSECUTIVE 1

// Select GPIO pins. The numbers must match GPIO number
// See here: https://learn.sparkfun.com/tutorials/raspberry-gpio/gpio-pinout
// Example: LCD_D0 is set to GPIO 12 -> physical pin 32
#define LCD_D0 12
#define LCD_D1 13
#define LCD_D2 14
#define LCD_D3 15
#define LCD_D4 16
#define LCD_D5 17
#define LCD_D6 18
#define LCD_D7 19
#define LCD_D8 20
#define LCD_D9 21
#define LCD_D10 22
#define LCD_D11 23
#define LCD_D12 24
#define LCD_D13 25
#define LCD_D14 26
#define LCD_D15 27

#define LCD_WR 3
#define LCD_RS 4
#define LCD_CS 5
#define LCD_RESET 6

// What are the display dimensions? (in pixels)
static unsigned int LCD_WIDTH = 800;
static unsigned int LCD_HEIGHT = 480;
	
static unsigned int displayInitialized = TFTGL_ERROR;
static unsigned int displayPortait = 0;
static unsigned int displayRotate = 0;

// Use GPIO_WRITE_PIN(pin, HIGH or LOW) to write to pin
// Use PULSE_LOW to create a pulse (needed by writing pixels) or PULSE_HIGH
// Use SWAP instead of std::swap (there is no C++ here!)
// Use if(gpioData != NULL){ // OK } else { // ERROR } to check
// if GPIO has been initialized!

static void tftglDisplayData(unsigned char data){
	GPIO_WRITE_PIN(LCD_RS, HIGH);
	GPIO_WRITE_PIN(LCD_D0, data & 0x01);
	GPIO_WRITE_PIN(LCD_D1, data & 0x02);
	GPIO_WRITE_PIN(LCD_D2, data & 0x04);
	GPIO_WRITE_PIN(LCD_D3, data & 0x08);
	GPIO_WRITE_PIN(LCD_D4, data & 0x10);
	GPIO_WRITE_PIN(LCD_D5, data & 0x20);
	GPIO_WRITE_PIN(LCD_D6, data & 0x40);
	GPIO_WRITE_PIN(LCD_D7, data & 0x80);
	PULSE_LOW(LCD_WR);
}

static void tftglDisplayCom(unsigned char data){
	GPIO_WRITE_PIN(LCD_RS, LOW);
	GPIO_WRITE_PIN(LCD_D0, data & 0x01);
	GPIO_WRITE_PIN(LCD_D1, data & 0x02);
	GPIO_WRITE_PIN(LCD_D2, data & 0x04);
	GPIO_WRITE_PIN(LCD_D3, data & 0x08);
	GPIO_WRITE_PIN(LCD_D4, data & 0x10);
	GPIO_WRITE_PIN(LCD_D5, data & 0x20);
	GPIO_WRITE_PIN(LCD_D6, data & 0x40);
	GPIO_WRITE_PIN(LCD_D7, data & 0x80);
	PULSE_LOW(LCD_WR);
}

#define COMMAND(X) tftglDisplayCom(X)
#define DATA(X) tftglDisplayData(X)

static void tftglDisplaySetXY(unsigned int x, unsigned int y, unsigned int w, unsigned int h){
	if(displayPortait){
		SWAP(x, y);
		SWAP(w, h);
	}
	w = w - 1 + x;
	h = h - 1 + y;
	COMMAND(0x2a); 
  	DATA(x>>8);
  	DATA(x);
  	DATA(w>>8);
  	DATA(w);
	COMMAND(0x2b); 
  	DATA(y>>8);
  	DATA(y);
  	DATA(h>>8);
  	DATA(h);
	COMMAND(0x2c); 
}

void tftglFillColor(unsigned int x, unsigned int y, unsigned int w, unsigned int h, const unsigned char* color){
	unsigned int i;
	
	if(displayInitialized == TFTGL_ERROR)return;
	if(x >= LCD_WIDTH || y >= LCD_HEIGHT)return;
	if(w == 0 || h == 0)return;
	
	// Check area dimensions
	if(x + w >= LCD_WIDTH){
		w = LCD_WIDTH - x;
	}
	if(y + h >= LCD_HEIGHT){
		h = LCD_HEIGHT - y;
	}
	
	//GPIO_WRITE_PIN(LCD_CS, LOW);
	tftglDisplaySetXY(x, y, w, h);
	GPIO_WRITE_PIN(LCD_RS, HIGH);
	
#if defined(LCD_DATA_CONSECUTIVE) && LCD_DATA_CONSECUTIVE == 1
	// Convert RGB-888 to RGB-565
	unsigned int rgb = ((color[0] >> 3) << 11) | ((color[1] >> 2) << 5) | color[2] >> 3;
	// Set GPIO pins by setting bitmask to one
	*(gpioData + GPIO_GPFSET0) = (rgb << LCD_D0);
	// Clear GPIO pins by settings bitmaks to one
	// FYI: ~X inverts bits
	*(gpioData + GPIO_GPFCLR0) = ((~rgb) << LCD_D0);
#else 
	GPIO_WRITE_PIN(LCD_D0, color[2] & 0x08);
	GPIO_WRITE_PIN(LCD_D1, color[2] & 0x10);
	GPIO_WRITE_PIN(LCD_D2, color[2] & 0x20);
	GPIO_WRITE_PIN(LCD_D3, color[2] & 0x40);
	GPIO_WRITE_PIN(LCD_D4, color[2] & 0x80);
	
	GPIO_WRITE_PIN(LCD_D5, color[1] & 0x04);
	GPIO_WRITE_PIN(LCD_D6, color[1] & 0x08);
	GPIO_WRITE_PIN(LCD_D7, color[1] & 0x10);
	GPIO_WRITE_PIN(LCD_D8, color[1] & 0x20);
	GPIO_WRITE_PIN(LCD_D9, color[1] & 0x40);
	GPIO_WRITE_PIN(LCD_D10, color[1] & 0x80);
	
	GPIO_WRITE_PIN(LCD_D11, color[0] & 0x08);
	GPIO_WRITE_PIN(LCD_D12, color[0] & 0x10);
	GPIO_WRITE_PIN(LCD_D13, color[0] & 0x20);
	GPIO_WRITE_PIN(LCD_D14, color[0] & 0x40);
	GPIO_WRITE_PIN(LCD_D15, color[0] & 0x80);
#endif
	
	for(i = 0; i < w*h; i++){
		PULSE_LOW(LCD_WR);
	}
	
	//GPIO_WRITE_PIN(LCD_CS, HIGH);
}

void tftglFillPixels(unsigned int x, unsigned int y, unsigned int w, unsigned int h, const unsigned char* pixels){
	int u, v;
	
	if(displayInitialized == TFTGL_ERROR)return;
	if(x >= LCD_WIDTH || y >= LCD_HEIGHT)return;
	if(w == 0 || h == 0)return;
	
	unsigned int stride = w * 3;
	
	// Check area dimensions
	if(x + w >= LCD_WIDTH){
		w = LCD_WIDTH - x;
	}
	if(y + h >= LCD_HEIGHT){
		h = LCD_HEIGHT - y;
	}
	
	//GPIO_WRITE_PIN(LCD_CS, LOW);
	tftglDisplaySetXY(x, y, w, h);
	GPIO_WRITE_PIN(LCD_RS, HIGH);
	
	for(v = h -1; v >= 0; v--){
		for(u = 0; u < w; u++){
			const unsigned char* px = &pixels[v * stride + u * 3];
#if defined(LCD_DATA_CONSECUTIVE) && LCD_DATA_CONSECUTIVE == 1
			unsigned int rgb = ((px[0] >> 3) << 11) | ((px[1] >> 2) << 5) | px[2] >> 3;
			*(gpioData + GPIO_GPFSET0) = (rgb << LCD_D0);
			*(gpioData + GPIO_GPFCLR0) = ((~rgb) << LCD_D0);
#else 
			GPIO_WRITE_PIN(LCD_D0, px[2] & 0x08);
			GPIO_WRITE_PIN(LCD_D1, px[2] & 0x10);
			GPIO_WRITE_PIN(LCD_D2, px[2] & 0x20);
			GPIO_WRITE_PIN(LCD_D3, px[2] & 0x40);
			GPIO_WRITE_PIN(LCD_D4, px[2] & 0x80);
			
			GPIO_WRITE_PIN(LCD_D5, px[1] & 0x04);
			GPIO_WRITE_PIN(LCD_D6, px[1] & 0x08);
			GPIO_WRITE_PIN(LCD_D7, px[1] & 0x10);
			GPIO_WRITE_PIN(LCD_D8, px[1] & 0x20);
			GPIO_WRITE_PIN(LCD_D9, px[1] & 0x40);
			GPIO_WRITE_PIN(LCD_D10, px[1] & 0x80);
			
			GPIO_WRITE_PIN(LCD_D11, px[0] & 0x08);
			GPIO_WRITE_PIN(LCD_D12, px[0] & 0x10);
			GPIO_WRITE_PIN(LCD_D13, px[0] & 0x20);
			GPIO_WRITE_PIN(LCD_D14, px[0] & 0x40);
			GPIO_WRITE_PIN(LCD_D15, px[0] & 0x80);
#endif
			PULSE_LOW(LCD_WR);
		}
	}
	
	//GPIO_WRITE_PIN(LCD_CS, HIGH);
}

unsigned int tftglInitDisplay(unsigned int flags){
	if(gpioData == NULL){
		errorCode = TFTGL_GPIO_ERROR;
		return TFTGL_ERROR;
	}
	
	displayPortait = flags & TFTGL_PORTRAIT;
	displayRotate = flags & TFTGL_ROTATE_180;
	
	if(displayPortait){
		SWAP(LCD_WIDTH, LCD_HEIGHT);
	}
	
	bcm2835_gpio_fsel(LCD_D0, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LCD_D1, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LCD_D2, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LCD_D3, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LCD_D4, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LCD_D5, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LCD_D6, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LCD_D7, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LCD_D8, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LCD_D9, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LCD_D10, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LCD_D11, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LCD_D12, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LCD_D13, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LCD_D14, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LCD_D15, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LCD_WR, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LCD_RS, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LCD_CS, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LCD_RESET, BCM2835_GPIO_FSEL_OUTP);
	
	GPIO_WRITE_PIN(LCD_CS, LOW);
	
	usleep(100 * 1000);

	GPIO_WRITE_PIN(LCD_RESET, HIGH);
	usleep(5 * 1000); // 5
	GPIO_WRITE_PIN(LCD_RESET, LOW);
	usleep(15 * 1000); // 15
	GPIO_WRITE_PIN(LCD_RESET, HIGH);
	usleep(15 * 1000); // 15
	//GPIO_WRITE_PIN(LCD_CS, LOW);

	COMMAND(0xE2);		//PLL multiplier, set PLL clock to 120M
	DATA(0x23);	    //N=0x36 for 6.5M, 0x23 for 10M crystal
	DATA(0x02);
	DATA(0x04);
	COMMAND(0xE0);		// PLL enable
	DATA(0x01);
	usleep(10 * 1000);
	COMMAND(0xE0);
	DATA(0x03);
	usleep(10 * 1000);
	COMMAND(0x01);		// software reset
	usleep(100 * 1000);
	COMMAND(0xE6);		//PLL setting for PCLK, depends on resolution
	DATA(0x04);
	DATA(0x93);
	DATA(0xE0);
  
	COMMAND(0xB0);		//LCD SPECIFICATION
	DATA(0x00);	// 0x24
	DATA(0x00);
	DATA(0x03);		//Set HDP	799
	DATA(0x1F);
	DATA(0x01);		//Set VDP	479
	DATA(0xDF);
	DATA(0x00);
  
	COMMAND(0xB4);		//HSYNC
	DATA(0x03);		//Set HT	928
	DATA(0xA0);
	DATA(0x00);		//Set HPS	46
	DATA(0x2E);
	DATA(0x30);		//Set HPW	48
	DATA(0x00);		//Set LPS	15
	DATA(0x0F);
	DATA(0x00);
  
	COMMAND(0xB6);		//VSYNC
	DATA(0x02);		//Set VT	525
	DATA(0x0D);
	DATA(0x00);		//Set VPS	16
	DATA(0x10);
	DATA(0x10);		//Set VPW	16
	DATA(0x00);		//Set FPS	8
	DATA(0x08);
  
	COMMAND(0xBA);
	DATA(0x05);		//GPIO[3:0] out 1
  
	COMMAND(0xB8);
	DATA(0x07);	    //GPIO3=input, GPIO[2:0]=output
	DATA(0x01);		//GPIO0 normal
  
	COMMAND(0x36);		//rotation
	if(displayPortait){
		if(displayRotate){
			DATA(0x22);
		} else {
			DATA(0x21);
		}
	} else {
		if(displayRotate){
			DATA(0x00);
		} else {
			DATA(0x03);
		}
	}
	//DATA(0x50);	// 0x50 - landscape #1 // 0x40
	//DATA(0x01); // 0x01 - landscape #2
	//DATA(0x22);
  
	COMMAND(0xF0);		//pixel data interface
	DATA(0x03);
  
	usleep(10 * 1000);
  
	tftglDisplaySetXY(0, 0, 799, 479);
	COMMAND(0x29);		//display on
  
	COMMAND(0xBE);		//set PWM for B/L
	DATA(0x06);
	DATA(0x00); // Initial brightness to 0%
	DATA(0x01);
	DATA(0xF0);
	DATA(0x00);
	DATA(0x00);
  
	COMMAND(0xD0); 
	DATA(0x0D);	
	COMMAND(0x2C);

	//GPIO_WRITE_PIN(LCD_CS, HIGH);
	
	displayInitialized = TFTGL_OK;
	
	static const unsigned char color[3] = {255, 255, 255};
	tftglFillColor(0, 0, LCD_WIDTH, LCD_HEIGHT, color);
	
	return TFTGL_OK;
}


void tftgSetBrightness(unsigned char val){
	//GPIO_WRITE_PIN(LCD_CS, LOW);
	COMMAND(0xBE); //set PWM for B/L
	DATA(0x06);
	DATA(val);
	DATA(0x01);
	DATA(0xF0);
	DATA(0x00);
	DATA(0x00);
	//GPIO_WRITE_PIN(LCD_CS, HIGH);
}

unsigned int tftglDisplayIsInit(){
	return (displayInitialized == TFTGL_ERROR ? TFTGL_ERROR : TFTGL_OK);
}

void tftglTerminateDisplay(){
	displayInitialized = TFTGL_ERROR;
}

unsigned int tftglGetWidth(){
	return LCD_WIDTH;
}

unsigned int tftglGetHeight(){
	return LCD_HEIGHT;
}