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
        motor(p1, p2, p3, p4) {}

void GarageDoor::calibrate(RotaryEncoder& encoder) {
    // TODO: Implement calibration logic here
    // cout << "Calibrating garage door..." << endl;

    bool calibration_success = false;

    while (!calibration_success) {
        int step_count[3] = {0};
        int steps = 100; // 20 steps seems to be a reliable number of steps that the rotary encoder can detect movement from the motor
        // the encoder can detect the stopping of the motor from 20 steps quite reliably

        // when rotating the motor, a positive value rotates the motor CLOCKWISE
        // and a negative value rotates the motor counter-CLOCKWISE
        // this should rotate the motor CLOCKWISE until there is a collision
        // detecting that the motor has moved with the rotary encoder is faster by moving the motor and checking if the rotary encoder is moving

        // move back and forth the track, turning the other way whenever the rotary encoder stops moving
        // first, see if we cannot move to closed position (because we are already there)
        // if so, move to open
        // else, move to closed

        bool closed = false;
        bool stuck = false;
        bool stuck_open = false;
        bool stuck_closed = false;

        bool moving = false;
        int steps_to_initial_position;
        
        // Lambda function to check if the door is stuck
        auto check_stuck = [](int& step_count, int direction, const char* position) -> bool {
            bool stuck = true;
            cout << "Checking if stuck in " << position << " position" << endl;
            int pos1 = RotaryEncoder::getPosition();
            int attempts = 0;
            const int max_attempts = 300; // Adjust this value as needed
            while (attempts < max_attempts) {
                StepperMotor::step(direction);
                sleep_us(850);
                step_count++;
                int pos2 = RotaryEncoder::getPosition();
                if (pos1 != pos2) {
                    stuck = false;
                    cout << "Not stuck at " << position << " end" << endl;
                    break;
                }
                attempts++;
            }
            if (attempts >= max_attempts) {
                cout << "Giving up after " << max_attempts << " attempts" << endl;
                stuck = true;
            }
            cout << "Moved " << (direction == CLOCKWISE ? "CLOCKWISE" : "COUNTER CLOCKWISE") << ": " << step_count << endl;
            sleep_ms(500);
            return stuck;
        };

        stuck_closed = check_stuck(step_count[0], CLOCKWISE, "closed");
        sleep_ms(100);
        stuck_open = check_stuck(step_count[1], COUNTER_CLOCKWISE, "open");

        steps_to_initial_position = step_count[0] - step_count[1];
        cout << "Steps from initial starting point: " << steps_to_initial_position << endl;
        cout << "Stuck at closed: " << stuck_closed << endl;
        cout << "Stuck at open: " << stuck_open << endl;
        sleep_ms(500);

        if (stuck_closed) {
            cout << "Stuck at closing end" << endl;
            while (!RotaryEncoder::isRotating()) {
                StepperMotor::rotate_steps(-100);
                sleep_us(850); // might not be necessary, to be tested
            }
            sleep_ms(500);
            // StepperMotor::rotate_steps(step_count[1]);
            cout << "Not stuck at closing end anymore" << endl;
        } else if (stuck_open) {
            cout << "Stuck at opening end" << endl;
            while (!RotaryEncoder::isRotating()) {
                StepperMotor::rotate_steps(100);
                sleep_us(850); // might not be necessary, to be tested
            }
            sleep_ms(500);
            // StepperMotor::rotate_steps(-step_count[0]);
            cout << "Not stuck at opening end anymore" << endl;
        } else {
            cout << "Not stuck at either end" << endl;
        }

        cout << "Not stuck" << endl;
        // cout << "Moving to initial position with " << -steps_to_initial_position << " steps" << endl;
        // StepperMotor::rotate_steps(-steps_to_initial_position);
        

        cout << "Moving to x position" << endl;
        sleep_ms(500);


        // Move to closed position

        int pos1 = RotaryEncoder::getPosition();
        while (true) {
            int pos2 = RotaryEncoder::getPosition();
            StepperMotor::rotate_steps(50);
            sleep_us(850);
            step_count[2] += 50;
            int pos3 = RotaryEncoder::getPosition();
            if (pos1 != pos2 && pos2 == pos3) {
                cout << "Rotary encoder is not rotating" << endl;
                cout << "Position 1: " << pos1 << endl;
                cout << "Position 2: " << pos2 << endl;
                cout << "Position 3: " << pos3 << endl;
                cout << "Step count: " << step_count[2] << endl;

                sleep_ms(500);

                // Rotate the motor until the rotary encoder detects movement
                while (!RotaryEncoder::isRotating()) {
                    StepperMotor::rotate_steps(-100);
                    sleep_us(850); // might not be necessary, to be tested
                }

                // Rotate the motor until the rotary encoder stops detecting movement
                while (RotaryEncoder::isRotating()) {
                    StepperMotor::rotate_steps(-100);
                    sleep_us(850); // might not be necessary, to be tested
                }

                // move a bit to open
                break;
            }
        }

        sleep_ms(100);
        cout << "from closed to open: " << step_count[2] << endl;

        calibration_success = true;
    }

    currentState = GarageDoor::State::CLOSED;
    calibrationState = CalibrationState::CALIBRATED;
    maxPosition = 2;
    cout << "Calibration complete. Max position: " << maxPosition << endl;

    // cout << "Calibration complete." << endl;
}

void GarageDoor::open() {
    if (calibrationState == CalibrationState::CALIBRATED) {
        cout << "Opening garage door..." << endl;
        while (position < maxPosition) {
            StepperMotor::rotate_steps(1);
            position++;
            currentState = State::IN_BETWEEN;
        }
        currentState = State::OPEN;
    } else {
        cout << "Cannot open: Door not calibrated." << endl;
    }
}

void GarageDoor::close() {
    if (calibrationState == CalibrationState::CALIBRATED) {
        cout << "Closing garage door..." << endl;
        while (position > 0) {
            StepperMotor::rotate_steps(-1);
            position--;
            currentState = State::IN_BETWEEN;
        }
        currentState = State::CLOSED;
    } else {
        cout << "Cannot close: Door not calibrated." << endl;
    }
}

void GarageDoor::stop() {
    cout << "Stopping garage door..." << endl;
    // TODO: Logic to stop the door would go here
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

void GarageDoor::setStuck() {
    cout << "Door is stuck!" << endl;
    errorState = ErrorState::STUCK;
    calibrationState = CalibrationState::NOT_CALIBRATED;
    StepperMotor::stop();
}

void GarageDoor::updatePosition(int newPosition) {
    // if (newPosition != position) {
    //     cout << "Updating position from " << position << " to " << newPosition << endl;
    //     int steps = newPosition - position;
    //     StepperMotor::rotate_steps(steps);
    //     position = newPosition;
        
    //     if (position <= 0) {
    //         currentState = State::CLOSED;
    //     } else if (position >= maxPosition) {
    //         currentState = State::OPEN;
    //     } else {
    //         currentState = State::IN_BETWEEN;
    //     }
    // }

    position = newPosition;
    if (position <= 0) {
        currentState = State::CLOSED;
    } else if (position >= maxPosition) {
        currentState = State::OPEN;
    } else {
        currentState = State::IN_BETWEEN;
    }
}