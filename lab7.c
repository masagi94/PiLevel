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

void getAngle(){}

/* 
	This function will print out the values we get from the accelerometer
	to the LCD according to the mode we're in. If the mode is 0, the LCD will
	print out degrees. If mode = 1, it will print radians.
	
	ex. mode = 0
	 _______________
	|     Level     |             
	|  + 155 deg    |
	|_______________|
	 
	ex. mode = 1
	 _______________
	|     Level     |             
	|  - 3.14 rad   |
	|_______________|

	To print values, we have to convert the degrees or radians to a char[], so 
	that we may display it on the LCD. The if-statements handle that logic, as 
	well as the string concatenation to display in the correct format.
*/
void updateLCD(){
    char displayValue[10] = "";
	char num[5] = "";
	char radDeg[4] = "";


	if (mode == 0){
		if(nDeg < 0){
			nDeg = nDeg * -1;
			strcat(displayValue, "- ");
		}
		else
			strcat(displayValue, "+ ");
    	
    	sprintf(num, "%d", nDeg);
    	strcat(displayValue, num);
    	strcat(radDeg, " deg");
	}
	else{
		if(nRad < 0){
			nRad = nRad * -1;
			strcat(displayValue, "- ");
		}
		else
			strcat(displayValue, "+ ");
		
		sprintf(num, "%0.2f", nRad);
		strcat(displayValue, num);
		strcat(radDeg, " rad");
	}


	strcat(displayValue, radDeg);
	
	// Clear the lcd, and then update the value.
    lcdPosition(fd2, 0, 1);
    lcdPrintf(fd2, "%s", "                ");
    lcdPosition(fd2, 2, 1);
    lcdPrintf(fd2, "%s", displayValue);

    delay(100);
}

short int axis_sample(int axis,int fd)
{
	short int data = 0;
	short int data2 = 0;

	usleep(10000);
	data  =  wiringPiI2CReadReg8(fd,axis);
	data2 =  wiringPiI2CReadReg8(fd,axis+1); 
	
	return ( (data2<<8)|data );
}

short int axis_sample_average(int axis, int fd)
{
	int c = 10;
	int value = 0;

	while(c--){
		value += axis_sample(axis, fd);
	}

	return ( value/10 );
}


/* 
	This sets up the wiringPi pins, as well as the accelerometer,
	the LCD, and the button attached to the raspberry pi.
*/
int configure(){
	
	// ***** Sets up wiring pi and the lcd *****
	wiringPiSetup();
	
    if(wiringPiSetup() == -1){
        printf("setup wiringPi failed !\n");
        return -1;
    }
	
	fd2 = lcdInit(2,16,4, 5,4, 0,1,2,3,4,5,6,7);
    //lcdClear(fd);
    if (fd2 == -1){
        printf ("lcdInit failed\n") ;
        return 1;
    }


	// ***** Configures the accelerometer for use. *****

	fd1 = wiringPiI2CSetup(0x53);

	datasimple = wiringPiI2CReadReg8(fd1,0x31);
	wiringPiI2CWriteReg8(fd1,0x31,datasimple|0xb);

	wiringPiI2CWriteReg8(fd1,0x2d,0x08); //POWER_CTL	
	usleep(11000);
	// erase offset bits
	wiringPiI2CWriteReg8(fd1,0x1e,0);
	wiringPiI2CWriteReg8(fd1,0x1f,0);
	wiringPiI2CWriteReg8(fd1,0x20,0);
	usleep(11000);
	// calibrate X axis
	data = axis_sample_average(X_REG,fd1);
	wiringPiI2CWriteReg8(fd1,0x1e,-(data / 4));
	// calibrate Y axis
	data = axis_sample_average(Y_REG,fd1);
	wiringPiI2CWriteReg8(fd1,0x1f,-(data / 4));
	// calibrate Z axis
	data = axis_sample_average(Z_REG,fd1);
	wiringPiI2CWriteReg8(fd1,0x20,-((data - 256 ) / 4));


	// configures the  button
	pinMode(BUTTON, INPUT);

	// use pull-up resistors for button
	pullUpDnControl(BUTTON, PUD_UP);

	// configure ISR for button press
	wiringPiISR(BUTTON, INT_EDGE_FALLING, ButtonISR);


	usleep(100000);
}