#include "garage_door_system.h"
#include "garage_door_config.h"
#include <iostream>
#include "pico/stdlib.h"

using namespace std;

GarageDoor::GarageDoor(int p1, int p2, int p3, int p4)
    :   currentState(State::CLOSED),
        errorState(ErrorState::NORMAL),
        calibrationState(CalibrationState::NOT_CALIBRATED),
        position(0),
        maxPosition(0),
        encoder(ENCODER_PIN_A, ENCODER_PIN_B),
        motor(p1, p2, p3, p4) 
{
    // Set up the rotary encoder
    encoder.setup();
}

void GarageDoor::calibrate() {
    bool calibration_success = false;
    int step_count[3] = {0};

    // While calibration is not successful, keep trying
    while (!calibration_success) {
        // 20 steps seems to be a reliable number of steps that the rotary encoder can detect movement from the motor using isRotating();
        // the encoder can detect the stopping of the motor from 20 steps quite reliably using isRotating();

        // when rotating the motor, a positive value rotates the motor CLOCKWISE
        // and a negative value rotates the motor counter-CLOCKWISE
        // this should rotate the motor CLOCKWISE until there is a collision
        // detecting that the motor has moved with the rotary encoder is faster by moving the motor and checking if the rotary encoder is moving

        auto check_stuck = [this](bool direction) -> bool {
            const int TEST_STEPS = 400; // Number of steps to try moving
            const int MOVEMENT_THRESHOLD = 2; // Minimum encoder movement to consider not stuck
            
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

        cout << "Cannot move to closed: " << cannot_move_to_closed << endl;
        cout << "Cannot move to open: " << cannot_move_to_open << endl;

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

        cannot_move_to_closed = check_stuck(true); 
        cannot_move_to_open = check_stuck(false);

        cout << "Cannot move to closed: " << cannot_move_to_closed << endl;
        cout << "Cannot move to open: " << cannot_move_to_open << endl;

        if (!cannot_move_to_closed && !cannot_move_to_open) {
            // We ain't there yet
            cout << "Door is not properly opened yet" << endl;
            step_count[0] += StepperMotor::rotate_till_collision(COUNTER_CLOCKWISE, encoder);
        }

        step_count[1] = StepperMotor::rotate_till_collision(CLOCKWISE, encoder);

        // Stop touching the wall
        StepperMotor::rotate_steps(-300);

        int offset = 600; // Adjust the offset as needed so the door doesn't hit the wall
        step_count[2] = step_count[0] - (step_count[0] + step_count[1]) + offset;

        cout << "Step count 0: " << step_count[0] << endl;
        cout << "Step count 1: " << step_count[1] << endl;
        cout << "Adjusted steps: " << step_count[2] << endl;
        cout << "Final rotary encoder position: " << encoder.getPosition() << endl;

        sleep_ms(100);
        calibration_success = true;
    }

    currentState = GarageDoor::State::CLOSED;
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

    cout << "Closing garage door..." << endl;
    currentState = State::IN_BETWEEN;

    const int STEPS_PER_MOVE = 60;

    // go from current position to 0
    while (position >= maxPosition) {
        cout << "Position: " << position << endl;
        StepperMotor::rotate_steps(-STEPS_PER_MOVE);
        position -= STEPS_PER_MOVE;
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

    // eg: pos = -20580, max = -20580
    cout << "Closing garage door..." << endl;
    currentState = State::IN_BETWEEN;

    const int STEPS_PER_MOVE = 60;

    // go from current position to 0
    while (position <= 0) {
        cout << "Position: " << position << endl;
        StepperMotor::rotate_steps(STEPS_PER_MOVE);
        position += STEPS_PER_MOVE;
    }
    
    currentState = State::CLOSED;
}

void GarageDoor::stop() {
    StepperMotor::stop();
}

GarageDoor::State GarageDoor::getState() const {
    return currentState;
}

GarageDoor::ErrorState GarageDoor::getErrorState() const {
    return errorState;
}

GarageDoor::CalibrationState GarageDoor::getCalibrationState() const {
    return calibrationState;
}

int GarageDoor::getPosition() const {
    return position;
}

void GarageDoor::setStuck() {
    cout << "Door is stuck!" << endl;
    errorState = ErrorState::STUCK;
    calibrationState = CalibrationState::NOT_CALIBRATED;
    StepperMotor::stop();
}

void GarageDoor::updatePosition(int newPosition) {
    position = newPosition;
    if (position <= 0) {
        currentState = State::OPEN;
    } else if (position >= maxPosition) {
        currentState = State::CLOSED;
    } else {
        currentState = State::IN_BETWEEN;
    }
}