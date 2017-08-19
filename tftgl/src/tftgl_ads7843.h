#define CHIP_SELECT_PIN BCM2835_SPI_CS0

#define CMD_START 0x80
#define CMD_12BIT 0x00
#define CMD_8BIT 0x04
#define CMD_DFR 0x00
#define CMD_POS_X (0x5 << 4)
#define CMD_POS_Y (0x1 << 4)
#define CMD_POS_Z1 (0x3 << 4)
#define CMD_POS_Z2 (0x4 << 4)
#define CMD_PWR 0x3

#define TFTGL_IGNORE_TOUCH (0x10)
	
static double remap(double value, double InMin, double InMax, double OutMin, double OutMax) {
	return (value - InMin)*(OutMax - OutMin) / (InMax - InMin) + OutMin;
}

static unsigned int minTouchPressure = 100; // Default
static double calibrationData[4][2] = {
	{0, 0}, {0, 0}, {0, 0}, {0, 0},
};

unsigned int tftglInitTouch(unsigned int flags) {
	if(flags & TFTGL_IGNORE_TOUCH){
		return TFTGL_OK;
	}
	
	if (!bcm2835_spi_begin()){
		errorCode = TFTGL_SPI_ERROR;
		return TFTGL_ERROR;
    }
	
	//bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    //bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
    //bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_2048);
    //bcm2835_spi_chipSelect(CHIP_SELECT_PIN);                      // The default
    //bcm2835_spi_setChipSelectPolarity(CHIP_SELECT_PIN, LOW);      // the default
	return TFTGL_OK;
}

void tftglTerminateTouch() {
	bcm2835_spi_end();
}

static unsigned int tftglGetTouchRawData16(unsigned int target, unsigned int c){
	static unsigned char out[3];
	static unsigned char buf[3];
	unsigned int i, ret;
	buf[0] = (CMD_START | CMD_12BIT | CMD_DFR | target | CMD_PWR);
	
	ret = 0;
	for(i = 0; i < c; i++){
		bcm2835_spi_transfernb(buf, out, 3);
		ret += (out[1] << 8 | out[2])>>3;
	}
	return (ret / c);
}

static unsigned int tftglGetTouchRawData8(unsigned int target, unsigned int c){
	static unsigned char out[2];
	static unsigned char buf[2];
	unsigned int i, ret;
	buf[0] = (CMD_START | CMD_8BIT | CMD_DFR | target | CMD_PWR);
	
	ret = 0;
	for(i = 0; i < c; i++){
		bcm2835_spi_transfernb(buf, out, 2);
		ret += out[1];
	}
	return (ret / c);
}

void tftglGetTouchRaw(unsigned int* x, unsigned int* y, unsigned int* z){
	// 2048 is about 122 Khz
	// Anything higher may not be correct reading
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_4096);
	bcm2835_spi_chipSelect(CHIP_SELECT_PIN);
	
	if(z != NULL)*z = tftglGetTouchRawData16(CMD_POS_Z1, 8);
	if(x != NULL)*x = tftglGetTouchRawData16(CMD_POS_X, 8);
	if(y != NULL)*y = tftglGetTouchRawData16(CMD_POS_Y, 8);
}

unsigned int tftglGetTouch(unsigned int* x, unsigned int* y){
	unsigned int i, press;
	// 2048 is about 122 Khz
	// Anything higher may not be correct reading
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_4096);
	bcm2835_spi_chipSelect(CHIP_SELECT_PIN);
	
	press = tftglGetTouchRawData16(CMD_POS_Z1, 1);
	if(press > minTouchPressure){
		// Calculate only if either X or Y are not null!
		if(x != NULL){
			double rawx = (double)tftglGetTouchRawData16(CMD_POS_X, 8);
			double weight = remap(rawx, calibrationData[0][0], calibrationData[1][0], 0.0, 1.0);
			
			//printf("Remapped X: %f weight: %f\n", rawx, weight);
			*x = (unsigned int)remap(weight, 0.0, 1.0, calibrationData[0][1], calibrationData[1][1]);
		}
		if(y != NULL){
			
			double rawy = (double)tftglGetTouchRawData16(CMD_POS_Y, 8);
			double weight = remap(rawy, calibrationData[2][0], calibrationData[3][0], 0.0, 1.0);
			
			//printf("Remapped Y: %f weight: %f\n", rawy, weight);
			*y = (unsigned int)remap(weight, 0.0, 1.0, calibrationData[2][1], calibrationData[3][1]);
		}
		return TFTGL_GOT_TOUCH;
	}
	return TFTGL_NO_TOUCH;
}

void tftglSetTouchSensitivity(unsigned int val){
	minTouchPressure = val;
}

void tftglSetTouchCalibration(unsigned int which, unsigned int val, unsigned int pos){
	switch(which){
		case TFTGL_CALIB_MIN_X: calibrationData[0][0] = (double)val; calibrationData[0][1] = (double)pos; break;
		case TFTGL_CALIB_MAX_X: calibrationData[1][0] = (double)val; calibrationData[1][1] = (double)pos; break;
		case TFTGL_CALIB_MIN_Y: calibrationData[2][0] = (double)val; calibrationData[2][1] = (double)pos; break;
		case TFTGL_CALIB_MAX_Y: calibrationData[3][0] = (double)val; calibrationData[3][1] = (double)pos; break;
		default: break;
	}
}