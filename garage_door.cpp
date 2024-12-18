// Created by: Emil Blumenthal

#include "garage_door_system.h"
#include "garage_door_config.h"
#include <iostream>
#include "pico/stdlib.h"

using namespace std;

GarageDoor::GarageDoor(int p1, int p2, int p3, int p4, ButtonController& btnCtrl)
    :   currentState(State::CLOSED),
        lastState(State::CLOSED),
        errorState(ErrorState::NORMAL),
        calibrationState(CalibrationState::NOT_CALIBRATED),
        position(0),
        maxPosition(0),
        encoder(ENCODER_PIN_A, ENCODER_PIN_B),
        motor(p1, p2, p3, p4),
        buttonController(btnCtrl)
{
    encoder.setup();
}

ostream& operator<<(ostream& os, const GarageDoor::State& state) {
    switch(state) {
        case GarageDoor::State::OPEN:
            os << "OPEN";
            break;
        case GarageDoor::State::CLOSED:
            os << "CLOSED";
            break;
        case GarageDoor::State::OPENING:
            os << "OPENING";
            break;
        case GarageDoor::State::CLOSING:
            os << "CLOSING";
            break;
        case GarageDoor::State::STOPPED:
            os << "STOPPED";
            break;
    }
    return os;
}

void GarageDoor::calibrate() {
    bool calibration_success = false;
    int step_count[3] = {0};

    // While calibration is not successful, keep trying
    while (!calibration_success) {
        auto check_stuck = [this](bool direction) -> bool {
            const int TEST_STEPS = 400; // Number of steps to try moving
            
            int initial_position = encoder.getPosition();
            
            // Try to move in the specified direction
            StepperMotor::rotate_steps(direction ? TEST_STEPS : -TEST_STEPS);
            sleep_ms(100);
            
            int final_position = encoder.getPosition();
            int movement = abs(final_position - initial_position);
            
            // If we could move, go back to initial position
            if (movement >= MOVEMENT_THRESHOLD) {
                cout << "Moved " << (direction ? "CLOCKWISE" : "COUNTER CLOCKWISE") << ": " << movement << endl;
                StepperMotor::rotate_steps(direction ? -TEST_STEPS : TEST_STEPS);
                sleep_ms(100);
            }
            
            // Return true if stuck (movement less than threshold)
            return movement < MOVEMENT_THRESHOLD;
        };

        StepperMotor::rotate_till_collision(CLOCKWISE, encoder);

        bool cannot_move_to_closed = check_stuck(true);
        bool cannot_move_to_open = check_stuck(false);

        // Verify we're at bottom by checking movement
        if (cannot_move_to_closed && cannot_move_to_open) {
            cout << "Error: Cannot move at all" << endl;
            return;
        } else if (cannot_move_to_closed) {
            while (!encoder.isRotating()) {
                StepperMotor::rotate_steps(-100);
                sleep_us(850); // might not be necessary, to be tested
            }
            sleep_ms(500);
            cout << "Not stuck at closing end anymore" << endl;
        } else if (cannot_move_to_open) {
            while (!encoder.isRotating()) {
                StepperMotor::rotate_steps(100);
                sleep_us(850); // might not be necessary, to be tested
            }
            sleep_ms(500);
            cout << "Not stuck at opening end anymore" << endl;
        }

        // Move to open position
        encoder.resetPosition();
        step_count[0] = StepperMotor::rotate_till_collision(COUNTER_CLOCKWISE, encoder);
        cout << "Rotary encoder position at open: " << encoder.getPosition() << endl;

        cannot_move_to_closed = check_stuck(true); 
        cannot_move_to_open = check_stuck(false);

        cout << "Cannot move to closed: " << cannot_move_to_closed << endl;
        cout << "Cannot move to open: " << cannot_move_to_open << endl;

        step_count[0] += StepperMotor::rotate_till_collision(COUNTER_CLOCKWISE, encoder);

        sleep_ms(500); // Wait before moving back

        encoder.resetPosition();
        step_count[1] = StepperMotor::rotate_till_collision(CLOCKWISE, encoder);
        cout << "Rotary encoder position at close: " << encoder.getPosition() << endl;

        // Stop touching the wall
        StepperMotor::rotate_steps(-300);

        int offset = 740; // Adjust the offset as needed so the door doesn't hit the wall.
        step_count[2] = step_count[0] - (step_count[0] + step_count[1]) + offset;

        cout << "Step count 0: " << step_count[0] << endl;
        cout << "Step count 1: " << step_count[1] << endl;
        cout << "Adjusted steps: " << step_count[2] << endl;
        cout << "Final rotary encoder position: " << encoder.getPosition() << endl;

        sleep_ms(100);
        calibration_success = true;
    }

    encoder.resetPosition();
    currentState = GarageDoor::State::CLOSED;
    lastState = GarageDoor::State::CLOSED;
    calibrationState = CalibrationState::CALIBRATED;
    maxPosition = step_count[2];
    position = 0;
    cout << "Calibration complete. Max position: " << step_count[2] << endl;
}

void GarageDoor::open() {
    if (calibrationState != CalibrationState::CALIBRATED) {
        cout << "Cannot open: Door not calibrated." << endl;
        return;
    }

    if (currentState == State::OPEN) {
        cout << "Door is already open." << endl;
        return;
    }

    encoder.resetPosition();
    int last_encoder_position = encoder.getPosition();
    uint32_t last_movement_time = to_ms_since_boot(get_absolute_time());

    // go from current position to 0
    while (position >= maxPosition) {
        StepperMotor::rotate_steps(-MOTOR_STEPS_PER_ITERATION);

        // Get current time and encoder position
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        int current_encoder_position = encoder.getPosition();

        // Update steps
        position -= MOTOR_STEPS_PER_ITERATION;

        // Check if we've seen movement
        int position_change = abs(current_encoder_position - last_encoder_position);
        if (position_change >= MOVEMENT_THRESHOLD) {
            // Movement detected, update our tracking
            last_encoder_position = current_encoder_position;
            last_movement_time = current_time;
        }
        // If no movement for COLLISION_TIMEOUT_MS, we've hit a collision
        else if (current_time - last_movement_time >= COLLISION_TIMEOUT_MS) {
            stop();
            errorState = ErrorState::STUCK;
            calibrationState = CalibrationState::NOT_CALIBRATED;
            break;
        }

        // Check if the operation button was pressed
        if (buttonController.isOperationPressed()) {
            currentState = State::STOPPED;
            stop();
            buttonController.setOperationButtonState(false); // Reset the button
            return;
        }
    }

    currentState = State::OPEN;
}

void GarageDoor::close() {
    if (calibrationState != CalibrationState::CALIBRATED) {
        cout << "Cannot close: Door not calibrated." << endl;
        return;
    }

    if (currentState == State::CLOSED) {
        cout << "Door is already closed." << endl;
        return;
    }

    encoder.resetPosition();
    int last_encoder_position = encoder.getPosition();
    uint32_t last_movement_time = to_ms_since_boot(get_absolute_time());

    // Go from current position to 0
    while (position <= 0) {
        StepperMotor::rotate_steps(MOTOR_STEPS_PER_ITERATION);

        // Get current time and encoder position
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        int current_encoder_position = encoder.getPosition();

        // Update steps
        position += MOTOR_STEPS_PER_ITERATION;

        // Check if we've seen movement
        int position_change = abs(current_encoder_position - last_encoder_position);
        if (position_change >= MOVEMENT_THRESHOLD) {
            // Movement detected, update our tracking
            last_encoder_position = current_encoder_position;
            last_movement_time = current_time;
        }

        // If no movement for COLLISION_TIMEOUT_MS, we've hit a collision
        else if (current_time - last_movement_time >= COLLISION_TIMEOUT_MS) {
            stop();
            errorState = ErrorState::STUCK;
            calibrationState = CalibrationState::NOT_CALIBRATED;
            break;
        }

        // Check if the operation button was pressed
        if (buttonController.isOperationPressed()) {
            currentState = State::STOPPED;
            stop();
            buttonController.setOperationButtonState(false); // Reset the button
            return; // Exit the loop and stop
        }
    }

    currentState = State::CLOSED;
}

void GarageDoor::stop() {
    StepperMotor::stop();
}

void GarageDoor::toggleMovement() {
    if (calibrationState == CalibrationState::NOT_CALIBRATED) {
        return;
    }

    buttonController.setOperationButtonState(false);

    cout << "Current state: " << currentState << endl;
    
    switch(currentState) {
        case State::OPEN:
            cout << "Close the door" << endl;
            currentState = State::CLOSING;
            lastState = State::OPEN;
            close();
            break;
        case State::OPENING:
            cout << "Stop" << endl;
            currentState = State::STOPPED;
            stop();
            break;
        case State::CLOSED:
            cout << "Open the door" << endl;
            currentState = State::OPENING;
            lastState = State::CLOSED;
            open();
            break;
        case State::CLOSING:
            cout << "Stop" << endl;
            currentState = State::STOPPED;
            stop();
            break;
        case State::STOPPED:
            cout << "Starting to open the garage door..." << endl;
            if (lastState == State::OPEN) {
                currentState = State::OPENING;
                open();
            } else {
                currentState = State::CLOSING;
                close();
            }
            break;
    }
}

GarageDoor::State GarageDoor::getState() const {
    return currentState;
}

GarageDoor::ErrorState GarageDoor::getErrorState() const {
    return errorState;
}