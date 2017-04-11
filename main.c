/*
 * MSP430 Timer Tutorial Example Code 3
 * Anthony Scranney
 * www.Coder-Tronics.com
 * August 2014
 *
 * PWM example using both TimerA_0 and TimerA_1 to increase and decrease
 * the duty cycle, which in turn alters the brightness of the on board green LED
 */

#include <msp430g2253.h>

#define ZENER BIT4
#define TRANSISTOR BIT5

/*** Global Variable ***/
int IncDec_PWM = 1;
int reverseCount = 0;

void main(void) {

	/*** Watchdog timer and clock Set-Up ***/
	WDTCTL = WDTPW + WDTHOLD;		// Stop watchdog timer
	DCOCTL = 0;             		// Select lowest DCOx and MODx
	BCSCTL1 = CALBC1_1MHZ;  		// Set range
	DCOCTL = CALDCO_1MHZ;   		// Set DCO step + modulation

	/*** GPIO Set-Up ***/
	P1DIR &= ~ZENER;                //The zener diode pin needs to be an input
    P1DIR |= BIT6 | TRANSISTOR;		// P1.6 set as output (Green LED), also transistor pin
    P1SEL |= BIT6;					// P1.6 selected Timer0_A Out1 output
    P1IE |= ZENER;   //Enable interrupts on the zener Diode Pin
    P1IES &= ~ZENER; //Low to High interrupt
    P1IFG &= ~ZENER; //Start with no interrupt pending

	/*** Timer0_A Set-Up ***/
    //This timer actually runs the servo
    TA0CCR0 |= 1000;				// PWM period
    TA0CCR1 |= 1;					// TA0CCR1 PWM duty cycle
    TA0CCTL1 |= OUTMOD_7;			// TA0CCR1 output mode = reset/set
    TA0CTL |= TASSEL_2 + MC_1;		// SMCLK, Up Mode (Counts to TA0CCR0)

	/*** Timer1_A Set-Up ***/
    //I think the point of this timer is to change the speed of the servo
    //So add another timer at CCR1 to control the time that we are to spend discharging
    //Possibly able to use outmod for that, or continuous ticking
    TA1CCR0 |= 4000;				// Counter value
    TA1CCTL0 |= CCIE;				// Enable Timer1_A interrupts
    TA1CTL |= TASSEL_2 + MC_1;		// SMCLK, Use up modev

    _BIS_SR(LPM0_bits + GIE);		// Enter Low power mode 0 with interrupts enabled
}

#pragma vector=TIMER1_A0_VECTOR     // Timer1 A0 interrupt service routine
__interrupt void Timer1_A0 (void) {

	if (P1OUT & TRANSISTOR) {
		TA0CCR1 += IncDec_PWM*2;			// Increase or decrease duty cycle

		if( TA0CCR1 > 998 || TA0CCR1 < 2 ) {	// Reverse direction if it falls within values
			IncDec_PWM = -IncDec_PWM;
			/* Do 2 full 180 movements of the servo before shutting off transistor */

			reverseCount++;
			if (reverseCount == 4) {
				P1OUT &= ~TRANSISTOR;
				reverseCount = 0;
			}
		}
	}
}

/*
#pragma vector=TIMER1_A1_VECTOR
__interrupt void Timer1_A1 (void) {


}
*/

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
	if (P1IFG & ZENER) {
		P1OUT |= TRANSISTOR;
		P1IFG &= ~ZENER;
		//Set a timer for another timer interrupt to occur
	}
}
