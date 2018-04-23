#include <wiringPi.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <wiringPiI2C.h>

#include <string.h>
#include <lcd.h>

// The wiringPi pin the button is connected to
#define BUTTON 24

// The registers for the accelerometer
#define X_REG 0x32
#define Y_REG 0x34
#define Z_REG 0x36

// Variables for the accelerometer and the LCD
int fd1 = 0;
int fd2 = 0;
short int data = 0;	
short int data2 = 0;
int datasimple = 0;	

// Variables to handle what to display to the LCD
int mode = 0;
int nDeg = 0;
float nRad = 0;

// Helper functions
void ButtonISR();
void getAngle();
void updateLCD();
short int axis_sample_average(int axis, int fd);
short int axis_sample(int axis,int fd);
int configure();





int main(int argc, char *argv[])
{
	configure();

	lcdPosition(fd2, 0, 0);
    lcdPrintf(fd2, "%s", "     Level      ");
	
    // loops indefinitely
	while(1){

		getAngle();
		updateLCD();
		usleep(100000);
	}

	return 0;
}

// Interrupt Service Routine for button presses, which control operational mode
void ButtonISR(){
	if (mode==1){
		mode=0;
		printf("degrees\n");
	}
	else if (mode==0){
		mode=1;
		printf("radians\n");
	}
}