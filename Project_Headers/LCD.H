/*
 * LCD.H
 *
 *  Created on: May 28, 2018
 *      Author: itzhakv
 */

#ifndef LCD_H_
#define LCD_H_

void LCDconf();
void LCDstrobeRoutine ();
void delay1mn(int n);
void LCDcommand (int command);
void dataByte();
void nop();

#endif /* LCD_H_ */
