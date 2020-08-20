/*
 * Setup:
		+------------+
		|            | -- NC
		|       P2.4 | -- Push Button
		|            |
		|            |
		|       P1.5 | -- Encoder B
 		|       P1.4 | -- Encoder A
		+------------+
 */

#include <msp430.h> 
#include <string.h>
#include "nokia5110.h"
#include "rotary_encoder.h"

unsigned char isPrompting = 1;		// initially set to YES
unsigned char alarmTriggered = 0;
unsigned short desiredRate;

unsigned char rotButIFG = 0;		// rotary encoder button pressed


unsigned char rotKnobIFG;

/*
 * main.c
 */
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    Clock_Init_1MHz();			// used for LCD
    SPI_Init();						// for LCD screen connection
	_delay_cycles(50000);

	RotEnc_Init();				// sets on-board LED to output for debugging

	LCD_Init();
	clearLCD();

	__bis_SR_register(GIE);						// General interrupts enable

	
	while (1) {
		// initial screen shows "Desired Flow Rate" prompt
		// Flow rate value should be displayed and the rotary encoder motion action should reflect on the screen

		if (isPrompting && !rotButIFG) {
			char rate[6];
			int2str(desiredRate, rate);

			// LCD screen display
			setCursor(0, 0);
			prints("Desired");		// 7 characters				Desired
			setCursor(0, 1);		//							flow rate:
			prints("flow rate:");	// 10 characters

			setCursor(30, 5);
			prints("   ");
			setCursor(30, 5);
			prints(rate);

			setCursor(60, 5);
			prints("mL/h");
		} else if (rotButIFG) {
			isPrompting = 0;
			rotButIFG = 0;
			clearLCD();
		} else { // display flow rate
			setCursor(36, 3);
			prints("XX mL/h");
			setCursor(36, 4);
			prints("detected");
		}
	}
}


/* references:
 *		https://www.43oh.com/wp-content/uploads/2010/10/msp430_rotary_encoder_switch.c
 *		https://github.com/sneznaovca/Launchpad-rotary-encoder/blob/master/rotary-encoder.c
 *		(arduino) https://www.best-microcontroller-projects.com/rotary-encoder.html
 */
