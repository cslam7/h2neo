// Written by Jenny Cho
// Modified in May 11th, 2020

// Project h2neo

// The following project is for an electric flow rate monitor for gravity-assisted
// IV therapy equipment. A LCD screen is integrated into a MSP430F5529 Launchpad,
// interfaced via SPI communication. Other user interface features a rotary encoder
// that is used to adjust and input setting by the user.
// The flow rate sensing is done using an optical system consisting of an infrared
// LED and a photodiode.

#include <msp430f5529.h>
#include "nokia5110.h"
#include "rotary_encoder.h"

unsigned short ps;		// Grab BIT4 and BIT5
unsigned short ns;
double value = 0;

/********************************************************************************
 * main.c
 ********************************************************************************/
int main(void) {
	WDTCTL = WDTPW + WDTHOLD;				// stop watchdog timer
	Clock_Init_1MHz();
	SPI_Init();
	_delay_cycles(500000);

	LCD_Init();
	RotEnc_Init();
	clearLCD();

	setCursor(0, 0);

	__bis_SR_register(GIE);					// enable interrupts
	__no_operation();						// For debugger

	while (1) {
		ps = (P1IN>>4) & 0x3;
//		display_value(value);
//		prints("hello");
//		_delay_cycles(200000);
	}
}
