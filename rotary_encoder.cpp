#include "garage_door_system.h"
#include "garage_door_config.h"
#include <hardware/gpio.h>
#include <iostream>

using namespace std;

static RotaryEncoder* encoderInstance = nullptr;
volatile bool encoder_a, encoder_b, prev_a, prev_b;

RotaryEncoder::RotaryEncoder(int pA, int pB)
    : pinA(pA), pinB(pB), position(0) {
    encoderInstance = this;
}

void RotaryEncoder::setup() const {
    gpio_init(pinA);
    gpio_init(pinB);
    gpio_set_dir(pinA, GPIO_IN);
    gpio_set_dir(pinB, GPIO_IN);
    
    // gpio_pull_up(pinA);
    // gpio_pull_up(pinB);

    gpio_set_irq_enabled_with_callback(pinA, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &RotaryEncoder::ISR_callback);
    // gpio_set_irq_enabled(pinB, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}

int RotaryEncoder::getPosition() const {
    return position;
}

void RotaryEncoder::ISR_callback(unsigned int gpio, uint32_t events) {
    if (encoderInstance) {
        static int8_t lookup_table[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
        static uint8_t encoder_state = 0;

        // read the state of the two input pins
        encoder_state = ((encoder_state & 0b0011) << 2) | (gpio_get(encoderInstance->pinB) << 1) | gpio_get(encoderInstance->pinA);

        // update the position
        encoderInstance->position += lookup_table[encoder_state & 0b1111];     
    }

    // uint32_t gpio_state = 0;

    // // read the state of the two input pins
    // gpio_state = gpio_get(ENCODER_PIN_A) << 1;

	// static bool ccw_fall = false;
	// static bool cw_fall = false;
	
	// uint8_t enc_value = 0;
	// enc_value = (gpio_state & 0x03);
}