/*
 * serial.h
 *
 *  Created on: April 18, 2022
 *      Author: Yumin Su
 *
 *  Serial interface
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdint.h>

/* Initialize I2C communication with device at <addr> */
void i2c_init(uint8_t addr);

/* Send len bytes of data from buffer (data) */
void i2c_send(uint8_t *data, uint8_t len);

/* Receive len bytes of data to buffer (data) */
void i2c_get(uint8_t *data, uint8_t len);

/* Send I2C stop signal */
void i2c_stop(void);

/* Initialize UART communication */
void uart_init(void);

/* Send len (+4 bytes for EOF) bytes of data */
void uart_send(uint8_t *data, uint8_t len);

#endif /* SERIAL_H_ */
