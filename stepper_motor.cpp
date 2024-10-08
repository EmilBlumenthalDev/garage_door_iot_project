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
// if step() is called directly, include a delay in the next line
void StepperMotor::step(bool direction) {
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
        sleep_us(850); // modify this to change the speed of the motor
    }
}

int StepperMotor::rotate_till_collision(bool direction, RotaryEncoder& encoder) {
    cout << "MOTOR_STEPS_PER_ITERATION: " << MOTOR_STEPS_PER_ITERATION << endl;
    
    encoder.resetPosition();
    int last_encoder_position = encoder.getPosition();
    uint32_t last_movement_time = to_ms_since_boot(get_absolute_time());
    int step_count = 0;

    while (true) {
        // Rotate the motor by multiple steps in the specified direction
        int steps = direction ? MOTOR_STEPS_PER_ITERATION : -MOTOR_STEPS_PER_ITERATION;
        rotate_steps(steps);
        
        // Get current time and encoder position
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        int current_encoder_position = encoder.getPosition();
        
        // Update steps
        if (direction) {
            step_count += MOTOR_STEPS_PER_ITERATION;
        } else {
            step_count -= MOTOR_STEPS_PER_ITERATION;
        }
        
        // Check if we've seen movement
        int position_change = abs(current_encoder_position - last_encoder_position);
        if (position_change >= MOVEMENT_THRESHOLD) {
            // Movement detected, update our tracking
            last_encoder_position = current_encoder_position;
            last_movement_time = current_time;
        }

        // If no movement for COLLISION_TIMEOUT_MS, we've hit a collision
        else if (current_time - last_movement_time >= COLLISION_TIMEOUT_MS) {
            cout << "Moved: " << position_change << " steps" << endl;
            cout << "Collision detected! No movement for " << COLLISION_TIMEOUT_MS << "ms" << endl;
            stop();
            break;
        }
        
        // Small delay to prevent overwhelming the system
        sleep_ms(MOVEMENT_CHECK_INTERVAL_MS);
    }

    return step_count;
}

void StepperMotor::stop() {
    // Turn off all coils to stop the motor
    gpio_put(STEP_PIN1, false);
    gpio_put(STEP_PIN2, false);
    gpio_put(STEP_PIN3, false);
    gpio_put(STEP_PIN4, false);
}