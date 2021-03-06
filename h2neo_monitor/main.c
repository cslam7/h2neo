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
#include "test.h"
#include "convertNprint.h"

#define MEMSIZE			10								// size of memory buffer used for flow rate calculations
#define GTT_FACTOR		20								// factor specified in tubing packaging
#define GTT_FACTOR_STR  "20"							// ^ in string format... not sure if it'll work lol
#define SIGNAL_LENGTH	40								// 2 * 20ms

// tic - number of times the Timer ISR is entered after x clock cycles
//			tic will be programmed to be 1ms long
// sec - seconds (tic * clock cycles)
// min - minutes (sec / 60)
unsigned short int dropStopwatch = SIGNAL_LENGTH + 1;	// Length of time between each drop
														// used to check if 1 drop has occurred ( > 20ms)
														// value primed to enter if() the first time
unsigned long int tic = 0;  							// (data type short can only go up to 65,535 ms which is only ~1m5sec)
unsigned short int msec = 0, sec = 0, min = 0;
unsigned short int oMsec = 0, oSec = 0, oMin = 0; 		// old sec; old min
unsigned char dropFLG = 0;  							// presence of a drop

// save last 5 time interval values and average to find more accurate flow rate
unsigned long int ticMem[MEMSIZE];  					// global var auto initialized to 0
unsigned short int index = 0;
char str[6]; 											// used to convert each integer to string

float flowRate;

unsigned char isPrompting = 1;							// initially set to YES
unsigned char alarmTriggered = 0;
unsigned short desiredRate = 0;

// interrupt flags
char rotKnobIFG = 0;									// rotary encoder knob turned
char rotButIFG = 0;										// rotary encoder button pressed
char s2IFG = 0;											// on-board P1.1 (S2) pressed

short i = 0, yCursor = 1;  // yCursor = 0 is taken by the stopwatch display

char refRate[6];

/********************************************************************************
 * main.c
 ********************************************************************************/
int main(void) {
	WDTCTL = WDTPW + WDTHOLD;				// stop watchdog timer
	P4DIR |= BIT7;					// Configure P4.7 as output (for blinking debugging)

	//Setup Buttons (REMOVE ONCE REPLACED BY SIGNAL)
	P1DIR &= ~BIT1;					// P1.1 input
	P1REN |= BIT1; 					// Enable pullup resistor of P1.1 (default: GND)
	P1OUT |= BIT1;					// Set pullup resistor to active (+3.3V) mode

	Clock_Init_1MHz();				// used for TimerA and LCD

	Timer0_A5_Init();				// Initialize Timer A0

	ADC12_0_Init();					// for analog sensor signal

	SPI_Init();						// for LCD screen connection
	_delay_cycles(50000);

	RotEnc_Init();				// sets on-board LED to output for debugging

	LCD_Init();
	clearLCD();
//	setCursor(0, 0);
//	prints("time:");
//	setCursor(36, 0);  // each character is 6wide 8tall
//	prints("00:00:00");



//	// set up display of memory buffer
//	for (i = 0; i < 5; i++) {
//		int2str(ticMem[i], str);
//		setCursor(0, yCursor++);
//		prints(str);
//	}
	yCursor = 1;

	P1IE |= BIT1;					// P1.1 interrupt enabled
	P1IFG &= ~BIT1;					// P1.1 interrupt flag cleared

	ADC12CTL0 &= ~ADC12SC;      				// Clear the start bit (precautionary)
	ADC12CTL0 |= ADC12SC;						// Start conversion

	__bis_SR_register(GIE);						// General interrupts enable

	while (1) {

		if (isPrompting && !rotButIFG) {
			int2str(desiredRate, refRate);

			// LCD screen display
			setCursor(0, 0);
			prints("Desired");		// 7 characters				Desired
			setCursor(0, 1);		//							flow rate:
			prints("flow rate:");	// 10 characters

			setCursor(30, 5);
			prints("   ");
			setCursor(30, 5);
			prints(refRate);

			setCursor(60, 5);
			prints("mL/h");
		} else if (rotButIFG) {
			if (isPrompting) {
				isPrompting = 0;
			} else {
				isPrompting = 1;
			}
			rotButIFG = 0;
			clearLCD();
		} else { // display flow rate
			active_monitor();
		}
	}
 }

void active_monitor(void)
{
	//Poll Buttons here. Control the Timer. Update LCD Display.
	if (dropFLG && (dropStopwatch > SIGNAL_LENGTH)) {
		if (!TA0CCR0) { // TIMER IS OFF if !; else not 0, aka timer is ON
			startTimer0_A5();
			dropStopwatch = 0;
		} else {
			stopTimer0_A5();

			ticMem[index] = tic;		// save measured time to ticMem buffer

			// print to screen (for debugging)
			int2str(ticMem[index++], str);
			setCursor(0, yCursor);
			prints("      ");  // 6 blank to clear screen
			setCursor(0, yCursor++);
			prints(str);
			if (index > 4) {  // memsize - 1 (when memsize = 5)
				index = 0;  // index wraparound
				yCursor = 1;
			}

			startTimer0_A5();
		}
		dropFLG = 0;
	}

	// display desired flow rate
	setCursor(0, 0);
	prints("ref: ");
	prints(refRate);
	prints(" mL/h");

	// display GTT factor
	setCursor(42, 1);
	prints("GTT:");
	setCursor(72, 1);
	prints(GTT_FACTOR_STR);

	msec = tic;
	sec = tic / 1000;
	min = tic / 60000;

	if (msec != oMsec) {  // if different
		char str[2];
		msec = msec % 100;
		int2strXX(msec, str);
		setCursor(72, 0);
		prints(str);
	}
	oMsec = msec;

	if (sec != oSec) {  // if different
		char str[2];
		int2strXX(sec%60, str);
		setCursor(54, 0);
		prints(str);
	}
	oSec = sec;

	if (min != oMin) {
		char str[2];
		int2strXX(min, str);
		setCursor(36, 0);
		prints(str);
	}
	oMin = min;

	if (ticMem[0]) {  // not zero
		// this might be being repeated too many times...
		short int count = 0, avgTime_ms = 0;
		long int sum = 0;
		for (i = 0; i < MEMSIZE; i++) {
			if (ticMem[i] > 500) { // assuming that drops will not be < 500ms apart
				sum += ticMem[i];
				count++;
			}
		}
		avgTime_ms = (float) sum / count;  // yields average msec
		float gtt = GTT_FACTOR;
		float temp = gtt * avgTime_ms;
		flowRate = 3600000.0 / temp;

		// change the flowRate to string
		char buf[80];
		displayFlowRate(&flowRate, buf);
		setCursor(36, 3);
		prints(buf);
		setCursor(60, 3);
		prints(" mLh");
	} else {
		setCursor(36, 3);
		prints("no drops");
		setCursor(36, 4);
		prints("detected");
	}
}
