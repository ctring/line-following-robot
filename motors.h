/*
 * motor.h - Prototypes functions for driving motors
 *
 *  Created on: Jul 8, 2014
 *      Author: NDTC
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#define MOTORS_GPIO_PERIPH 			SYSCTL_PERIPH_GPIOE
#define MOTORS_PWM_PERIPH 			SYSCTL_PERIPH_PWM0
#define MOTORS_PWM_FREQUENCY 		1000

#define MOTORS_GPIO_BASE 			GPIO_PORTE_BASE
#define MOTORS_PWM_BASE 			PWM0_BASE
#define MOTORS_PWM_GEN 				PWM_GEN_2

/*
 * Right motor
 */
#define MOTOR_A_1 					GPIO_PIN_0
#define MOTOR_A_2 					GPIO_PIN_1
#define MOTOR_A_E 					GPIO_PIN_4
#define MOTOR_A_E_PWM_PIN_MAP 		GPIO_PE4_M0PWM4
#define MOTOR_A_E_PWM_BIT 			PWM_OUT_4_BIT
#define MOTOR_A_E_PWM_OUT			PWM_OUT_4

/*
 * Left motor
 */
#define MOTOR_B_1 					GPIO_PIN_2
#define MOTOR_B_2 					GPIO_PIN_3
#define MOTOR_B_E 					GPIO_PIN_5
#define MOTOR_B_E_PWM_PIN_MAP		GPIO_PE5_M0PWM5
#define MOTOR_B_E_PWM_BIT 			PWM_OUT_5_BIT
#define MOTOR_B_E_PWM_OUT			PWM_OUT_5

#define MOTOR_DIR_CW				0x00
#define MOTOR_DIR_CCW				0x01
#define MOTOR_DIR_FAST_STOP			0x02
#define MOTOR_STOP					0x00
#define MOTOR_RUN	 				0x01
#define MOTOR_MAX_SPEED				255

void motorsInit();

void motorsASetDirection(uint8_t ui8Dir);
void motorsASetState(bool bRunning);
void motorsASetSpeed(uint32_t ui32Speed);

void motorsBSetDirection(uint8_t ui8Dir);
void motorsBSetState(bool bRunning);
void motorsBSetSpeed(uint32_t ui32Speed);

#endif /* MOTOR_H_ */
