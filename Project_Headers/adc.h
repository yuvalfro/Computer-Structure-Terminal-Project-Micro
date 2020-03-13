/*
 * adc.h
 *
 *  Created on: Nov 25, 2014
 *      Author: Manuel Alejandro
 */

#ifndef ADC_H_
#define ADC_H_

#include "derivative.h"

/* Prototypes */
void adc_init(void);
int adc_cal(void);
unsigned short adc_read(unsigned char ch);

#endif /* ADC_H_ */
