#ifndef ARDUINO_MOCK_H
#define ARDUINO_MOCK_H

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

//#define FRAMAC
#define BINARY

#ifdef BINARY
#include<Arduino.h>
#include<i2c_t3.h>

uint64_t mock_millis();

uint64_t mock_micros();

void mock_interrupts();

void mock_noInterrupts();

void mock_analogWrite(int pin, int value);

void mock_digitalWrite(int pin, int state);

int mock_digitalRead(int pin);

void mock_delay(int millis);

void mock_delayMicroseconds(uint64_t micros);

void mock_pinMode(int pin, int mode);

void mock_attachInterrupt(int pin, void f(), int mode);

int wire_requestFrom(int devAddr, int size);

void wire_beginTransmission(int device);

void wire_write(int address);

int wire_read();

int wire_endTransmission();
int wire_endTransmission_nostop();

int wire_available();

void wire_begin(i2c_mode mode, uint8_t address, i2c_pins pins, i2c_pullup pullup, uint32_t rate, i2c_op_mode opMode);

#endif // BINARY

#ifdef FRAMAC
#include "./wdog_registers.h"

//implements: GLOBAL_timestamp_type
uint64_t milliseconds = 1;
//implements: GLOBAL_timestamp_type
uint64_t microseconds = 1;

#define INTERRUPTS_OFF 0
#define INTERRUPTS_ON  1

// Interrupts are on at boot
//@ ghost int ghost_interrupt_status = INTERRUPTS_ON;

#define I2C_MASTER 0
#define I2C_PINS_16_17 0
#define I2C_PULLUP_EXT 0
#define I2C_RATE_400 0
#define I2C_NOSTOP 0

#define HIGH 0
#define LOW 1

#define OUTPUT 1
#define INPUT 0

uint64_t mock_millis();

uint64_t mock_micros();

void mock_interrupts();

void mock_noInterrupts();

void mock_analogWrite(int pin, int value);

void mock_digitalWrite(int pin, int state);

int mock_digitalRead(int pin);

void mock_delay(int millis);

void mock_delayMicroseconds(uint64_t micros);

void mock_pinMode(int pin, int mode);

// delay is allowed at boot
//implements: GLOBAL_no_delay_in_loop
//@ ghost int ghost_delay_allowed = 1;
//@ ghost int ghost_delay_happened = 1;
void mock_delay(int millis);
void mock_delayMicroseconds(uint64_t micros);

void mock_pinMode(int pin, int mode);

#define CHANGE 0
#define RISING 1
#define FALLING 2
void mock_attachInterrupt(int pin, void f(), int mode);

//implements: MAIN_no_alloc
void *mock_malloc(size_t s);

int wire_requestFrom(int devAddr, int size);

void wire_beginTransmission(int device);

void wire_write(int address);

int wire_read();

int wire_endTransmission();
int wire_endTransmission_nostop();

int wire_available();
void wire_begin(int, int, int, int, int);
#endif // FRAMAC
#endif // ARDUINO_MOCK_H