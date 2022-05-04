#include <msp430.h>
#include <stdint.h>

#include "DFRobot_BMX160.h"
#include "serial.h"
#include "swi2c_master.h"
#include "timer.h"

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    DCOCTL = 0;                 // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_8MHZ;      // Set DCO
    DCOCTL = CALDCO_8MHZ;
    delay(2000);                // Wait for IMUs to power up

    /* Software I2C pins */
    SWI2C_SCL = BIT1;
    SWI2C_SDA = BIT0;

    /* Initialization */
    uint8_t data[23*4] = {0};
    uart_init();
    DFRobot_BMX160 devices[] = {
        DFRobot_BMX160(0x68, false),
        DFRobot_BMX160(0x69, false),
        DFRobot_BMX160(0x68, true),
        DFRobot_BMX160(0x69, true),
    };
    for (auto device : devices) {
        device.begin();
        delay(8);
    }
    delay(80);

    /* Repeatedly pull data from IMUs and send data to PC */
    while (1) {
        for (uint8_t i = 4; i > 0; i--) {
            devices[i - 1].getAllData(data + (i - 1) * 23);
        }
        uart_send(data, 23 * 4);
    }
}


