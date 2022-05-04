/*
 * timer.cpp
 *
 *  Created on: April 18, 2022
 *      Author: Yumin Su
 *
 *  Helper function: delay
 */

#include <msp430.h>

#include "timer.h"

void delay(unsigned int time) {
    unsigned int count = 0;
    WDTCTL = WDT_MDLY_8;            // WDT interrupt per 1ms (CLK at 8MHz)
    IE1 |= WDTIE;                   // Enable WDT interrupt
    while (count < time) {
        __bis_SR_register(LPM0_bits + GIE);    // Enter LPM0 w/ interrupt
        count++;
    }
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(WDT_VECTOR))) watchdog_timer (void)
#else
#error Compiler not supported!
#endif
{
    LPM0_EXIT;
}


// Timer A0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A (void)
#else
#error Compiler not supported!
#endif
{
    __bic_SR_register_on_exit(LPM0_bits);
}
