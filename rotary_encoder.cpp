#include "garage_door_system.h"
#include "garage_door_config.h"
#include <hardware/gpio.h>
#include <iostream>

using namespace std;

static RotaryEncoder* encoderInstance = nullptr;
volatile bool encoder_a, encoder_b, prev_state_a, prev_state_b;

RotaryEncoder::RotaryEncoder(int pA, int pB)
    : pinA(pA), pinB(pB), position(0), rotating(false) {
    encoderInstance = this;
}

void RotaryEncoder::setup() const {
    gpio_init(pinA);
    gpio_init(pinB);
    gpio_set_dir(pinA, GPIO_IN);
    gpio_set_dir(pinB, GPIO_IN);
    
    // gpio_pull_up(pinA);
    // gpio_pull_up(pinB);

    gpio_set_irq_enabled_with_callback(pinA, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &RotaryEncoder::IRQ_callback);
    gpio_set_irq_enabled_with_callback(pinB, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &RotaryEncoder::IRQ_callback);
    // gpio_set_irq_enabled(pinB, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}

int RotaryEncoder::getPosition() {
    return encoderInstance->position;
}

bool RotaryEncoder::isRotating() {
    return encoderInstance->rotating;
}

void RotaryEncoder::IRQ_callback(unsigned int gpio, uint32_t events) {
    prev_state_a = encoder_a;
    prev_state_b = encoder_b;

    encoder_a = !gpio_get(encoderInstance->pinA);
    encoder_b = !gpio_get(encoderInstance->pinB);

    if (encoder_a && prev_state_a && encoder_b && !prev_state_b) {
        encoderInstance->position++;
        encoderInstance->rotating = true;
        // cout << "Going clockwise | " << "Position: " << encoderInstance->position << endl;
    } else if (encoder_a && !prev_state_a && encoder_b && prev_state_b) {
        encoderInstance->position--;
        encoderInstance->rotating = true;
        // cout << "Going counter-clockwise | " << "Position: " << encoderInstance->position << endl;
    } else if (!encoder_a && !encoder_b) {
        encoderInstance->rotating = false;   
    }
}