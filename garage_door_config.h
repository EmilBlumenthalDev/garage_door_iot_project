#include <cstdint>
#ifndef GARAGE_DOOR_CONFIG_H
#define GARAGE_DOOR_CONFIG_H

// Stepper motor pins
#define STEP_PIN1 2
#define STEP_PIN2 3
#define STEP_PIN3 6
#define STEP_PIN4 13

// Rotary encoder pins
#define ENCODER_PIN_A 28
#define ENCODER_PIN_B 27

// LED pins
#define STATUS_LED_PIN 20
#define ERROR_LED_PIN 21

// Button pins
#define CALIBRATION_BUTTON1 9
#define CALIBRATION_BUTTON2 7
#define OPERATION_BUTTON 8

// Other constants
#define BLINK_INTERVAL_MS 500
#define STEPPER_WAITING_US 850
#define CLOCKWISE true
#define COUNTER_CLOCKWISE false

// Constants for checking if stuck
#define COLLISION_TIMEOUT_MS 600
#define MOVEMENT_CHECK_INTERVAL_MS 40
#define MOVEMENT_THRESHOLD 2
#define MOTOR_STEPS_PER_ITERATION 60

static const uint8_t step_sequence[8] = {0x09, 0x08, 0x0C, 0x04, 0x06, 0x02, 0x03, 0x01};

#endif