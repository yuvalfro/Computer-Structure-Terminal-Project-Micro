/*
 * pit.h
 *
 *  Created on: Jun 6, 2019
 *      Author: Yuval Froman
 */

#ifndef PIT_H_
#define PIT_H_

#include "derivative.h"

#define ADC_CHANNEL 8 // Channel 8 (PTB0)
#define LED_RED  18 // PTB18
#define LED_GREEN  19 // PTB19

void InitPIT(void);
void PIT_IRQHandler(void);
void enable_irq (int irq);




#endif /* PIT_H_ */
