/*
 * buttons.h - Prototypes for buttons polling utility (Based on buttons api of TI)
 *
 *  Created on: Jul 8, 2014
 *      Author: Cuong T. Nguyen
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

/*
 *  Defines for the hardware resources used by the pushbuttons.
 *
 *  The switches are on the following ports/pins:
 *
 *  The switches tie the GPIO to ground, so the GPIOs need to be configured
 *  with pull-ups, and a value of 0 means the switch is pressed.
 *
 */
#define BUTTONS_GPIO_PERIPH     SYSCTL_PERIPH_GPIOC
#define BUTTONS_GPIO_BASE       GPIO_PORTC_BASE

#define NUM_BUTTONS             4
#define CONFIG_BUTTON           GPIO_PIN_4
#define DEC_BUTTON              GPIO_PIN_5
#define INC_BUTTON				GPIO_PIN_6
#define RUN_BUTTON				GPIO_PIN_7

#define ALL_BUTTONS             (GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7)

/*
 *  Useful macros for detecting button events.
 */

#define BUTTON_PRESSED(button, buttons, changed)                              \
        (((button) & (changed)) && ((button) & (buttons)))

#define BUTTON_RELEASED(button, buttons, changed)                             \
        (((button) & (changed)) && !((button) & (buttons)))

/*
 * Functions exported from buttons.c
 */
extern void buttonsInit(void);
extern uint8_t buttonsPoll(uint8_t *pui8Delta, uint8_t *pui8Raw);

#endif /* BUTTONS_H_ */
