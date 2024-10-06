#include "garage_door_system.h"
#include <hardware/gpio.h>
#include "pico/stdlib.h"
#include <iostream>

using namespace std;

static RotaryEncoder* encoderInstance = nullptr;
volatile bool RotaryEncoder::rotating = false;
volatile bool encoder_a, encoder_b, prev_state_a, prev_state_b;
static absolute_time_t last_movement_time, last_check_time;
// static const uint32_t ROTATION_TIMEOUT_MS = 100; // Timeout to consider rotation stopped
static const uint32_t STABLE_TIME_REQUIRED_US = 15000; // 20 ms

RotaryEncoder::RotaryEncoder(int pA, int pB)
    : pinA(pA), pinB(pB), position(0), revolutions(0) {
    encoderInstance = this;
    last_movement_time = get_absolute_time();
    last_check_time = get_absolute_time();
}

void RotaryEncoder::setup() const {
    gpio_init(pinA);
    gpio_init(pinB);
    gpio_set_dir(pinA, GPIO_IN);
    gpio_set_dir(pinB, GPIO_IN);
    
    gpio_pull_up(pinA);
    gpio_pull_up(pinB);

    gpio_set_irq_enabled_with_callback(pinA, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &RotaryEncoder::IRQ_wrapper);
    gpio_set_irq_enabled(pinB, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}

int RotaryEncoder::getPosition() {
    return encoderInstance->position;
}

void RotaryEncoder::resetPosition() {
    encoderInstance->position = 0;
}

bool RotaryEncoder::isRotating() {
    absolute_time_t current_time = get_absolute_time();
    return (absolute_time_diff_us(last_movement_time, current_time) < STABLE_TIME_REQUIRED_US);
}

void RotaryEncoder::IRQ_wrapper(uint gpio, uint32_t events) {
    if (encoderInstance) {
        RotaryEncoder::IRQ_callback(gpio, events);
    }
}

void RotaryEncoder::IRQ_callback(uint gpio, uint32_t events) {
    prev_state_a = encoder_a;
    prev_state_b = encoder_b;
    
    encoder_a = !gpio_get(encoderInstance->pinA);
    encoder_b = !gpio_get(encoderInstance->pinB);
    
    // Detect movement and update position
    if (encoder_a != prev_state_a || encoder_b != prev_state_b) {
        // Update the last movement time
        last_movement_time = get_absolute_time();
        rotating = true;

        if (encoder_a) {
            encoderInstance->position++;
        }
        
        // Determine direction and update position
        // if (encoder_a && !prev_state_a && encoder_b) {
        //     encoderInstance->position++;
        // } else if (encoder_a && prev_state_a && !encoder_b) {
        //     encoderInstance->position--;
        // }
    }
}