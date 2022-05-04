/*
 * i2c.c
 *
 *  Created on: April 18, 2022
 *      Author: Yumin Su
 *
 *  I2C interface
 */

#include <msp430.h>

#include "serial.h"

static enum status {null, i2c_transmit, i2c_receive, uart_transmit} state;
static uint8_t buf[100] = {0};
static uint8_t length = 0;

void i2c_init(uint8_t addr) {
    P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
    P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
    UCB0CTL1 |= UCSWRST;                      // Enable SW reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
    UCB0BR0 = 8;                             // fSCL = SMCLK/12 = ~100kHz
    UCB0BR1 = 0;
    UCB0I2CSA = addr;                         // Set slave address
    UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
}

void i2c_send(uint8_t *data, uint8_t len) {
    IE2 = UCB0TXIE;                           // Enable TX interrupt
    state = i2c_transmit;
    for (uint8_t i = len; i > 0; i--)
        buf[len - i] = data[i - 1];
    length = len;
    UCB0CTL1 |= UCTR + UCTXSTT;               // I2C TX, start condition
    __bis_SR_register(CPUOFF + GIE);          // Enter LPM0 w/ interrupts
    state = null;
}

void i2c_get(uint8_t *data, uint8_t len) {
    uint8_t i = 0;

    IE2 = UCB0RXIE;                          // Enable TX interrupt
    state = i2c_receive;
    length = len;
    UCB0CTL1 &= ~UCTR;
    UCB0CTL1 |= UCTXSTT;                      // I2C TX, start condition
    __bis_SR_register(CPUOFF + GIE);          // Enter LPM0 w/ interrupts
    state = null;
    for (i = len; i > 0; i--) {
        data[len - i] = buf[i - 1];
    }
    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
}

void i2c_stop(void) {
    UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
}

void uart_init(void) {
    P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2;
    UCA0CTL1 |= UCSSEL_2 + UCSWRST;                     // SMCLK
    UCA0BR0 = 13;                              // 1MHz 115200
    UCA0BR1 = 0;                              // 1MHz 115200
    UCA0MCTL = UCBRS2 + UCBRS0;               // Modulation UCBRSx = 5
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
}

void uart_send(uint8_t *data, uint8_t len) {
    for (uint8_t i = len; i > 0; i--) {
        buf[len - i + 4] = data[i - 1];
    }
    for (uint8_t i = 4; i > 0; i--)
        buf[i - 1] = i - 1;
    length = len + 4;
    state = uart_transmit;
    IE2 = UCA0TXIE;                          // Enable USCI_A0 TX interrupt
    __bis_SR_register(CPUOFF + GIE);          // Enter LPM0 w/ interrupts
    // state = null;
}

// USCI_B0 Data ISR
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0TX_VECTOR))) USCIAB0TX_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch (state) {
    case i2c_transmit:
        if (length) {
            UCB0TXBUF = buf[--length];
        } else {
            UCB0CTL1 |= UCTXSTP;
            while (UCB0CTL1 & UCTXSTP);              // Ensure stop condition got sent
            IFG2 &= ~(UCB0TXIFG);                     // Clear USCI_B0 TX int flag
            __bic_SR_register_on_exit(CPUOFF);        // Exit LPM0
        }
        break;
    case i2c_receive:
        length--;
        if (length) {
            buf[length] = UCB0RXBUF;
            if (length == 1) {
                UCB0CTL1 |= UCTXSTP;
            }
        } else {
            buf[length] = UCB0RXBUF;
            __bic_SR_register_on_exit(CPUOFF);        // Exit LPM0
        }
        IFG2 &= ~(UCB0RXIFG);                     // Clear USCI_B0 TX int flag
        break;
    case uart_transmit:
        if (length) {
            UCA0TXBUF = buf[--length];
        } else {
            IE2 &= ~UCA0TXIE;
            __bic_SR_register_on_exit(CPUOFF);        // Exit LPM0
        }
        break;
    case null:
        IFG2 &= ~(UCB0RXIFG | UCB0TXIFG);
        break;
    }
}

