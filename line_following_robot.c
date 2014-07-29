/*
 * line_following_robot.c - Main source file for the controlling software of my line following robot
 *
 *  Created on: Jul 1, 2014
 *      Author: NDTC
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

#include "delay.h"
#include "lcd.h"
#include "buttons.h"
#include "motors.h"
#include "sensors.h"

/*
 * Some other values
 */
#define SYSTICK_FREQ			50
#define SYSTICK_PERIOD  		20 // ms
#define CENTER 					3
#define BUTTON_POLL_DIVIDER_5	5
#define BUTTON_POLL_DIVIDER_10 	10

/*
 * Configuring options
 */
#define NUM_CONFIG 		5
#define CONFIG_P 		0x00
#define CONFIG_I 		0x01
#define CONFIG_D 		0x02
#define CONFIG_SPEED 	0x03
#define CONFIG_SEN  	0x04

volatile bool bRunning = 0;
volatile uint8_t ui8Config;
/*
 * Each of these variables has value range from 1 to 255
 */
volatile int32_t i32KP = 4, i32KI = 0, i32KD = 1;
volatile uint32_t ui32MotorMinSpeed = 105;

/*
 * Helper functions
 */
char* ret;
char* numTo3Char(int32_t x) {
	if (!ret) ret = malloc(3 * sizeof(char));
	ret[3] = '\0';
	ret[2] = '0' + x % 10; x /= 10;
	ret[1] = '0' + x % 10; x /= 10;
	ret[0] = '0' + x % 10; x /= 10;
	return ret;
}

int32_t decInRange(int32_t x, int32_t lower, int32_t step) {
	if (x > lower + step) x -= step; else x = lower;
	return x;
}

int32_t incInRange(int32_t x, int32_t upper, int32_t step) {
	if (x < upper - step) x += step; else x = upper;
	return x;
}

int32_t ensureValue(int32_t x, int32_t lower, int32_t upper) {
	if (x < lower) x = lower;
	if (x > upper) x = upper;
	return x;
}

int32_t i32ErrSum, i32LastErr;
int32_t i32PPart, i32IPart, i32DPart;

int32_t computePID(int32_t i32CurErr) {

	i32PPart = i32KP * i32CurErr;
	i32IPart += i32KI * i32CurErr;
	i32DPart = i32KD * (i32CurErr - i32LastErr);

	int32_t i32Ret = i32PPart + i32IPart + i32DPart;

	return i32Ret;
}

void toggleMotors(bool bRun) {
	bRunning = bRun;
	if (bRunning) {
		motorsASetState(MOTOR_RUN);
		motorsBSetState(MOTOR_RUN);
	}
	else {
		motorsASetState(MOTOR_STOP);
		motorsBSetState(MOTOR_STOP);
	}
}

/*
 * Components handler functions
 */
uint32_t ui32ButtonsCounter = 0;
uint8_t ui8LastButtons;

void buttonsHandler() {
	uint8_t ui8Buttons;
	ui8Buttons = buttonsPoll(0, 0);

	ui32ButtonsCounter++;

	switch (ui8Buttons & ALL_BUTTONS) {
	case CONFIG_BUTTON:
		if (ui32ButtonsCounter % BUTTON_POLL_DIVIDER_10 == 1) {
			if (!bRunning) {
				ui8Config = (ui8Config + 1) % NUM_CONFIG;
			}
		}
		break;
	case DEC_BUTTON:
		if (!bRunning && (ui32ButtonsCounter % BUTTON_POLL_DIVIDER_5 == 1)) {
			switch (ui8Config) {
			case CONFIG_P: i32KP = decInRange(i32KP, 0, 1); break;
			case CONFIG_I: i32KI = decInRange(i32KI, 0, 1); break;
			case CONFIG_D: i32KD = decInRange(i32KD, 0, 1); break;
			case CONFIG_SPEED: ui32MotorMinSpeed = decInRange(ui32MotorMinSpeed, 95, 5); break;
			}
		}
		break;
	case INC_BUTTON:
		if (!bRunning && (ui32ButtonsCounter % BUTTON_POLL_DIVIDER_5 == 1)) {
			switch (ui8Config) {
			case CONFIG_P: i32KP = incInRange(i32KP, 255, 1); break;
			case CONFIG_I: i32KI = incInRange(i32KI, 255, 1); break;
			case CONFIG_D: i32KD = incInRange(i32KD, 255, 1); break;
			case CONFIG_SPEED: ui32MotorMinSpeed = incInRange(ui32MotorMinSpeed, 255, 5); break;
			}
		}
		break;
	case RUN_BUTTON:
		if (ui8LastButtons != ui8Buttons) {
			toggleMotors(!bRunning);
		}
	default:
		ui32ButtonsCounter = 0;
	}

	ui8LastButtons = ui8Buttons;
}


/*
 * Systick control
 */

void systickInit() {
    SysTickPeriodSet(SysCtlClockGet() / SYSTICK_FREQ);
    SysTickEnable();
    SysTickIntEnable();
}

volatile uint8_t sensorsData, motorASpeed = 180, motorBSpeed = 180;
volatile int32_t error;

void SysTickIntHandler() {
	sensorsData = sensorsRead();
	if (sensorsData == 0xFF) toggleMotors(false);
	if (bRunning) {
		//
		// Motor A will be set at a static speed while we vary motor B's speed
		// to control the direction
		//
		uint8_t i, sum = 0, cnt = 0;
		for (i = 0; i < 8; i++) {
			uint8_t bit = (sensorsData >> i) & 1;
			sum += i * bit;
			cnt += bit;
		}

		error = sum/cnt - CENTER;
		uint32_t add = computePID(error), offset;
		offset = (ui32MotorMinSpeed + MOTOR_MAX_SPEED) / 2;
		motorASpeed = ensureValue(offset - add, ui32MotorMinSpeed, MOTOR_MAX_SPEED);
		motorBSpeed = ensureValue(offset + add, ui32MotorMinSpeed, MOTOR_MAX_SPEED);
		motorsASetSpeed(motorASpeed);
		motorsBSetSpeed(motorBSpeed);
	}
}

void lcdHandler() {
	if (bRunning) {
		lcdClearAndWriteString("Running...");
		lcdGoToXY(0, 1);
		lcdWriteString("A: ");
		lcdWriteString(numTo3Char(motorASpeed));
		lcdWriteChar(' ');
		lcdWriteString("B ");
		lcdWriteString(numTo3Char(motorBSpeed));

	} else {
		switch (ui8Config) {
		case CONFIG_P:
			lcdClearAndWriteString("Proportional:");
			lcdGoToXY(6, 1);
			lcdWriteString(numTo3Char(i32KP));
			break;
		case CONFIG_I:
			lcdClearAndWriteString("Integral:");
			lcdGoToXY(6, 1);
			lcdWriteString(numTo3Char(i32KI));
			break;
		case CONFIG_D:
			lcdClearAndWriteString("Derivative:");
			lcdGoToXY(6, 1);
			lcdWriteString(numTo3Char(i32KD));
			break;
		case CONFIG_SPEED:
			lcdClearAndWriteString("Min. speed: ");
			lcdGoToXY(6, 1);
			lcdWriteString(numTo3Char(ui32MotorMinSpeed));
			break;
		case CONFIG_SEN:
			lcdClearAndWriteString("Sensors calib.:");
			lcdGoToXY(0, 1);
			lcdWriteString("    ");
			int i;
			for(i = 0; i < 8; i++)
				if ((sensorsData >> i) & 1) {
					lcdWriteChar('1');
				}
				else {
					lcdWriteChar('0');
				}
			break;
		}
	}
}

/*
 * Primary main
 */

int main(void) {

	SysCtlClockSet(SYSCTL_SYSDIV_10|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	buttonsInit();
	motorsInit();
	sensorsInit();
	lcdInit(FUNCTION_SET_2_LINES, ENTRY_MODE_SET_INCREMENT,	DISPLAY_ON);

	systickInit();
	IntMasterEnable();

	motorsASetDirection(MOTOR_DIR_CW);
	motorsBSetDirection(MOTOR_DIR_CCW);

	while (1) {
		buttonsHandler();
		lcdHandler();
		delayMs(20);
	}
}

/*
 * Testing main: Motors (2)
 */
int main4(void) {
	SysCtlClockSet(SYSCTL_SYSDIV_10|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	lcdInit(FUNCTION_SET_BASE | FUNCTION_SET_2_LINES,
				ENTRY_MODE_SET_BASE | ENTRY_MODE_SET_INCREMENT,
				DISPLAY_BASE | DISPLAY_ON);
	motorsInit();
	buttonsInit();
	uint8_t dirA = 0, dirB = 0, speedA = MOTOR_MAX_SPEED, speedB = MOTOR_MAX_SPEED;
	uint8_t option = 0;
	while (1) {
		uint8_t ui8Buttons;
		ui8Buttons = buttonsPoll(0, 0);

		ui32ButtonsCounter++;

		switch (ui8Buttons & ALL_BUTTONS) {
		case CONFIG_BUTTON:
			if (ui32ButtonsCounter % BUTTON_POLL_DIVIDER_10 == 1) {
				option = (option + 1) % 2;
			}
			break;
		case DEC_BUTTON:
			if (ui32ButtonsCounter % BUTTON_POLL_DIVIDER_5 == 1) {
				if (option == 0)
					speedA = decInRange(speedA, 0, 3);
				else
					speedB = decInRange(speedB, 0, 3);

			}
			break;
		case INC_BUTTON:
			if (ui32ButtonsCounter % BUTTON_POLL_DIVIDER_5 == 1) {
				if (option == 0)
					speedA = incInRange(speedA, 255, 3);
				else
					speedB = incInRange(speedB, 255, 3);
			}
			break;
		case RUN_BUTTON:
			if (ui32ButtonsCounter % BUTTON_POLL_DIVIDER_10 == 1) {
				if (option == 0)
					dirA = (dirA + 1) % 2;
				else
					dirB = (dirB + 1) % 2;

			}
			break;
		default:
			ui32ButtonsCounter = 0;
		}

		lcdClearAndWriteString("Motor ");
		lcdWriteChar(option == 0 ? 'A' : 'B');
		if (option == 0)
			lcdWriteString(dirA == 0 ? " stopped." : " running.");
		else
			lcdWriteString(dirB == 0 ? " stopped." : " running.");

		lcdGoToXY(0, 1);

		lcdWriteString("Speed: ");
		lcdWriteString(numTo3Char(option == 0 ? speedA : speedB));

		motorsASetSpeed(speedA);
		motorsBSetSpeed(speedB);
		motorsASetDirection(MOTOR_DIR_CW);
		motorsBSetDirection(MOTOR_DIR_CCW);
		motorsASetState(dirA);
		motorsBSetState(dirB);

		delayMs(20);
	}
}

/*
 * Testing main: Motors (1)
 */
int main3(void) {
	SysCtlClockSet(SYSCTL_SYSDIV_10|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	lcdInit(FUNCTION_SET_BASE | FUNCTION_SET_2_LINES,
				ENTRY_MODE_SET_BASE | ENTRY_MODE_SET_INCREMENT,
				DISPLAY_BASE | DISPLAY_ON);
	motorsInit();
	buttonsInit();
	uint8_t dirA = 2, dirB = 2, speed = MOTOR_MAX_SPEED;
	while (1) {
		uint8_t ui8Buttons;
		ui8Buttons = buttonsPoll(0, 0);

		ui32ButtonsCounter++;

		switch (ui8Buttons & ALL_BUTTONS) {
		case CONFIG_BUTTON: // Motor B
			if (ui32ButtonsCounter % BUTTON_POLL_DIVIDER_10 == 1) {
				dirB = (dirB + 1) % 3;
			}
			break;
		case DEC_BUTTON:
			if (ui32ButtonsCounter % BUTTON_POLL_DIVIDER_5 == 1) {
				speed = decInRange(speed, 0, 3);
				speed = decInRange(speed, 0, 3);

			}
			break;
		case INC_BUTTON:
			if (ui32ButtonsCounter % BUTTON_POLL_DIVIDER_5 == 1) {
				speed = incInRange(speed, 255, 3);
				speed = incInRange(speed, 255, 3);
			}
			break;
		case RUN_BUTTON: // Motor A
			if (ui32ButtonsCounter % BUTTON_POLL_DIVIDER_10 == 1) {
				dirA = (dirA + 1) % 3;
			}
			break;
		default:
			ui32ButtonsCounter = 0;
		}

		lcdClearAndWriteString("Speed: ");
		lcdWriteString(numTo3Char(speed));

		lcdGoToXY(0, 1);

		char tmp[3];
		lcdWriteString("B: ");
		sprintf(tmp, "%d", dirB);
		lcdWriteString(tmp);
		lcdWriteChar(' ');
		lcdWriteString("A: ");
		sprintf(tmp, "%d", dirA);
		lcdWriteString(tmp);

		motorsASetSpeed(speed);
		motorsBSetSpeed(speed);
		motorsASetDirection(dirA);
		motorsBSetDirection(dirB);
		motorsASetState(dirA != 2);
		motorsBSetState(dirB != 2);

		delayMs(20);
	}
}

/*
 * Testing main: Sensors
 */
int main2(void) {
	SysCtlClockSet(SYSCTL_SYSDIV_10|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	lcdInit(FUNCTION_SET_BASE | FUNCTION_SET_2_LINES,
			ENTRY_MODE_SET_BASE | ENTRY_MODE_SET_INCREMENT,
			DISPLAY_BASE | DISPLAY_ON);
	systickInit();
	sensorsInit();
	IntMasterEnable();
	while(1) {
		lcdClear();
		int i;
		for(i = 0; i < 8; i++)
			if ((sensorsData >> i) & 1) {
				lcdWriteChar('*');
			}
			else {
				lcdWriteChar('0');
			}
		lcdWriteChar(' ');
		lcdWriteString(numTo3Char(error));
		lcdGoToXY(0, 1);
		lcdWriteString("A: ");
		lcdWriteString(numTo3Char(motorASpeed));
		lcdWriteChar(' ');
		lcdWriteString("B: ");
		lcdWriteString(numTo3Char(motorBSpeed));
		delayMs(30);
	}
}


/*
 * Testing main: Buttons and Lcd
 */
int main1(void) {
	SysCtlClockSet(SYSCTL_SYSDIV_10|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	lcdInit(FUNCTION_SET_BASE | FUNCTION_SET_2_LINES,
			ENTRY_MODE_SET_BASE | ENTRY_MODE_SET_INCREMENT,
			DISPLAY_BASE | DISPLAY_ON);
	motorsInit();
	buttonsInit();
	while(1) {
		buttonsHandler();
		lcdHandler();
		delayMs(30);
	}
}
