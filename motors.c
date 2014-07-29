/*
 * motor.c - Drive the direction and speed, with PWM, of two motors
 *
 *  Created on: Jul 8, 2014
 *      Author: NDTC
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"

#include "motors.h"

uint32_t ui32Load;
uint32_t ui32PWMClock;

void motorsInit() {

	SysCtlPWMClockSet(SYSCTL_PWMDIV_4);

	SysCtlPeripheralEnable(MOTORS_GPIO_PERIPH);
	SysCtlPeripheralEnable(MOTORS_PWM_PERIPH);

	GPIOPinTypeGPIOOutput(MOTORS_GPIO_BASE, MOTOR_A_1|MOTOR_A_2|MOTOR_B_1|MOTOR_B_2);

	GPIOPinTypePWM(MOTORS_GPIO_BASE, MOTOR_A_E|MOTOR_B_E);
	GPIOPinConfigure(MOTOR_A_E_PWM_PIN_MAP);
	GPIOPinConfigure(MOTOR_B_E_PWM_PIN_MAP);

	ui32PWMClock = SysCtlClockGet() / 4;
	ui32Load = (ui32PWMClock / MOTORS_PWM_FREQUENCY) - 1;
	PWMGenConfigure(MOTORS_PWM_BASE, MOTORS_PWM_GEN, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(MOTORS_PWM_BASE, MOTORS_PWM_GEN, ui32Load);

	PWMPulseWidthSet(MOTORS_PWM_BASE, MOTOR_A_E_PWM_OUT, 0);
	PWMPulseWidthSet(MOTORS_PWM_BASE, MOTOR_B_E_PWM_OUT, 0);

	PWMGenEnable(MOTORS_PWM_BASE, MOTORS_PWM_GEN);
	PWMOutputState(MOTORS_PWM_BASE, MOTOR_A_E_PWM_BIT, false);
	PWMOutputState(MOTORS_PWM_BASE, MOTOR_B_E_PWM_BIT, false);

}

void motorsASetDirection(uint8_t ui8Dir) {
	switch (ui8Dir) {
	case MOTOR_DIR_CW:
		GPIOPinWrite(MOTORS_GPIO_BASE, MOTOR_A_1, ~MOTOR_A_1);
		GPIOPinWrite(MOTORS_GPIO_BASE, MOTOR_A_2, MOTOR_A_2);
		break;
	case MOTOR_DIR_CCW:
		GPIOPinWrite(MOTORS_GPIO_BASE, MOTOR_A_1, MOTOR_A_1);
		GPIOPinWrite(MOTORS_GPIO_BASE, MOTOR_A_2, ~MOTOR_A_2);
		break;
	case MOTOR_DIR_FAST_STOP:
		GPIOPinWrite(MOTORS_GPIO_BASE, MOTOR_A_1, MOTOR_A_1);
		GPIOPinWrite(MOTORS_GPIO_BASE, MOTOR_A_2, MOTOR_A_2);
		break;
	}
}

void motorsBSetDirection(uint8_t ui8Dir) {
	switch (ui8Dir) {
	case MOTOR_DIR_CW:
		GPIOPinWrite(MOTORS_GPIO_BASE, MOTOR_B_1, ~MOTOR_B_1);
		GPIOPinWrite(MOTORS_GPIO_BASE, MOTOR_B_2, MOTOR_B_2);
		break;
	case MOTOR_DIR_CCW:
		GPIOPinWrite(MOTORS_GPIO_BASE, MOTOR_B_1, MOTOR_B_1);
		GPIOPinWrite(MOTORS_GPIO_BASE, MOTOR_B_2, ~MOTOR_B_2);
		break;
	case MOTOR_DIR_FAST_STOP:
		GPIOPinWrite(MOTORS_GPIO_BASE, MOTOR_B_1, MOTOR_B_1);
		GPIOPinWrite(MOTORS_GPIO_BASE, MOTOR_B_2, MOTOR_B_2);
		break;
	}
}

void motorsASetState(bool bRunning) {
	PWMOutputState(MOTORS_PWM_BASE, MOTOR_A_E_PWM_BIT, bRunning);
}

void motorsBSetState(bool bRunning) {
	PWMOutputState(MOTORS_PWM_BASE, MOTOR_B_E_PWM_BIT, bRunning);
}

void motorsASetSpeed(uint32_t ui32Speed) {
	PWMPulseWidthSet(MOTORS_PWM_BASE, MOTOR_A_E_PWM_OUT, ui32Speed * ui32Load / MOTOR_MAX_SPEED);
}

void motorsBSetSpeed(uint32_t ui32Speed) {
	PWMPulseWidthSet(MOTORS_PWM_BASE, MOTOR_B_E_PWM_OUT, ui32Speed * ui32Load / MOTOR_MAX_SPEED);
}



