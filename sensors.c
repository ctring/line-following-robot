/*
 * sensors.c - Reading sensors value functions
 *
 *  Created on: Jul 8, 2014
 *      Author: Cuong T. Nguyen
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

#include "sensors.h"

void sensorsInit() {
	SysCtlPeripheralEnable(SENSORS_GPIO_PERIPH);
	GPIOPinTypeGPIOInput(SENSORS_GPIO_BASE, ALL_SENSORS);
}

uint8_t sensorsRead() {
	return ~GPIOPinRead(SENSORS_GPIO_BASE, ALL_SENSORS);
}

