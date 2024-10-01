#include "garage_door_system.h"
#include <hardware/gpio.h>
#include <iostream>

using namespace std;

static RotaryEncoder* encoderInstance = nullptr;
volatile bool RotaryEncoder::rotating = false;

volatile bool encoder_a, encoder_b, prev_state_a, prev_state_b;

RotaryEncoder::RotaryEncoder(int pA, int pB)
    : pinA(pA), pinB(pB), position(0) {
    encoderInstance = this;
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

    // gpio_set_irq_enabled_with_callback(pinA, GPIO_IRQ_EDGE_FALL, true, &RotaryEncoder::IRQ_callback);
    // gpio_set_irq_enabled_with_callback(pinA, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &RotaryEncoder::IRQ_callback);
    // gpio_set_irq_enabled(pinB, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}

int RotaryEncoder::getPosition() {
    return encoderInstance->position;
}

bool RotaryEncoder::isRotating() {
    return rotating;
}

void RotaryEncoder::IRQ_wrapper(uint gpio, uint32_t events) {
    if (encoderInstance) {
        RotaryEncoder::IRQ_callback(gpio, events);
    }
}

// void RotaryEncoder::IRQ_callback(uint gpio, uint32_t events) {
//     static int8_t lookup_table[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
//     static uint8_t encoder_state = 0;

//     encoder_state = ((encoder_state & 0b0011) << 2) | (gpio_get(encoderInstance->pinB) << 1) | gpio_get(encoderInstance->pinA);
//     encoderInstance->position += lookup_table[encoder_state & 0b1111]; // Modifies the static position
// }

void RotaryEncoder::IRQ_callback(unsigned int gpio, uint32_t events) {
    encoder_a = !gpio_get(encoderInstance->pinA);

    if (encoder_a) {
        encoderInstance->position++;
    }
}

// void RotaryEncoder::IRQ_callback(unsigned int gpio, uint32_t events) {
//     prev_state_a = encoder_a;
//     prev_state_b = encoder_b;

//     encoder_a = !gpio_get(encoderInstance->pinA);
//     encoder_b = !gpio_get(encoderInstance->pinB);

//     if (encoder_a && prev_state_a && encoder_b && !prev_state_b) {
//         encoderInstance->position++;
//         encoderInstance->rotating = true;
//         // cout << "Going clockwise | " << "Position: " << encoderInstance->position << endl;
//     } else if (encoder_a && !prev_state_a && encoder_b && prev_state_b) {
//         encoderInstance->position--;
//         encoderInstance->rotating = true;
//         // cout << "Going counter-clockwise | " << "Position: " << encoderInstance->position << endl;
//     } else if (!encoder_a && !encoder_b) {
//         encoderInstance->rotating = false;   
//     }
// }