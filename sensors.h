/*
 * sensors.h - Protypes for reading sensors value functions
 *
 *  Created on: Jul 8, 2014
 *      Author: NDTC
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#define SENSORS_GPIO_PERIPH 		SYSCTL_PERIPH_GPIOB
#define SENSORS_GPIO_BASE 			GPIO_PORTB_BASE
#define ALL_SENSORS 				(GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7)

void sensorsInit();
uint8_t sensorsRead();

#endif /* SENSOR_H_ */
