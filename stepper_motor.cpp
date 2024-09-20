#include "garage_door_system.h"
#include "garage_door_config.h"
#include "pico/stdlib.h"
#include <hardware/gpio.h>

#include <cstdlib>
#include <iostream>

using namespace std;

StepperMotor::StepperMotor(int p1, int p2, int p3, int p4)
    : pin1(p1), pin2(p2), pin3(p3), pin4(p4) {
    // Initialize GPIO pins
    gpio_init(pin1);
    gpio_init(pin2);
    gpio_init(pin3);
    gpio_init(pin4);
    
    gpio_set_dir(pin1, GPIO_OUT);
    gpio_set_dir(pin2, GPIO_OUT);
    gpio_set_dir(pin3, GPIO_OUT);
    gpio_set_dir(pin4, GPIO_OUT);
}


// function for going a step
void StepperMotor::step(bool direction) {
    const int step_sequence[8] = {0x09, 0x08, 0x0C, 0x04, 0x06, 0x02, 0x03, 0x01};
    static int8_t step_index = 0;

    // if direction is clockwise
    if (direction) {
        if (++step_index >= 8) step_index = 0;
    } else {
        if (--step_index < 0) step_index = 7;
    }

    gpio_put(STEP_PIN1, (step_sequence[step_index] & 0x08) >> 3);
    gpio_put(STEP_PIN2, (step_sequence[step_index] & 0x04) >> 2);
    gpio_put(STEP_PIN3, (step_sequence[step_index] & 0x02) >> 1);
    gpio_put(STEP_PIN4, (step_sequence[step_index] & 0x01));
}

void StepperMotor::rotate_steps(int steps) {
    // Rotate the motor in the desired direction.
    bool clockwise = steps >= 0;
    int start = clockwise ? 0 : steps;
    int target = clockwise ? steps : 0;

    // Rotate the motor in steps
    for (int s = start; s != target; s++) {
        step(clockwise);
        sleep_ms(2);
    }
}

void StepperMotor::stop() const {
    // Turn off all coils to stop the motor
    gpio_put(STEP_PIN1, false);
    gpio_put(STEP_PIN2, false);
    gpio_put(STEP_PIN3, false);
    gpio_put(STEP_PIN4, false);
}