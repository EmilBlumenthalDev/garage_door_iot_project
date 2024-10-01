#include "garage_door_system.h"
#include "garage_door_config.h"
#include <iostream>

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

    // int steps = 0;
    // int pos1 = encoder.getPosition();

    // int pos2 = encoder.getPosition();

    // cout << "pos1: " << pos1 << endl;
    // cout << "pos2: " << pos2 << endl;
    // cout << "steps: " << steps << endl;

    // basic calibration goes as follows:
    // if position is positive, the door is open
    // if position is negative, the door is closed
    // 1. get pos1, open or close the door until pos2 is one unit apart from pos1
    // 2. move the door to one direction, until pos1 and pos2 are one unit apart
    // 3. move the door to the other direction, until pos1 and pos2 are one unit apart

    int pos1 = encoder.getPosition();
    int pos2;
    int step_count;
    int steps = -400;

    for (int i = 0; i < 3; i++) {
        motor.rotate_steps(steps);
        step_count += steps;
    }

    pos2 = encoder.getPosition();

    cout << "Position 1: " << pos1 << endl;
    cout << "Position 2: " << pos2 << endl;
    cout << "Step count: " << step_count << endl;

    if (pos2 == pos1) {
        cout << "Encoder is not rotating." << endl;
        steps = -steps;
        for (int i = 0; i < 3; i++) {
            motor.rotate_steps(steps);
            step_count += steps;
        }
    } else {
        cout << "Encoder is rotating." << endl;
    }

    // determine how many steps it takes for the encoder to detect a change
    // for (int i = 0; i < 3; i++) {
    //     pos1 = encoder.getPosition();
    //     while (true) {
    //         motor.rotate_steps(i % 2 == 0 ? 1 : -1);
    //         steps[i]++;
    //         pos2 = encoder.getPosition();
    //         if (pos2 != pos1) break;
    //     }
    // }

    // cout << steps[0] << " " << steps[1] << " " << steps[2] << endl;
    // int average = (steps[0] + steps[1] + steps[2]) / 3;

    // // open the door until it gets stuck
    // while (true) {
    //     motor.rotate_steps(1);
    //     pos2 = encoder.getPosition();
    //     if (pos2 - pos1 >= average) break;
    // }

    // move until encoder stops
    // while (true) {
    //     motor.rotate_steps(-1);
    //     pos2 = encoder.getPosition();
    //     if (pos2 == pos1) break;
    // }

    // 1. get pos1, open or close the door until pos2 is one unit apart from pos1
    // while (pos1 == encoder.getPosition()) {
    //     // open the door 159 steps and check if the encoder value has changed
    //     StepperMotor::rotate_steps(159);
    //     steps += 159;
    //     pos2 = encoder.getPosition();
    //     if (pos2 != pos1) break;

    //     // close the door 159 steps and check if the encoder value has changed
    //     StepperMotor::rotate_steps(-159);
    //     steps += 159;
    //     pos2 = encoder.getPosition();
    //     if (pos2 != pos1) break;
    // }

    // for (int i = 0; i < 159; i++) {
    //     StepperMotor::rotate_steps(1);
    //     steps++;
    //     pos2 = encoder.getPosition();
    //     if (pos2 - pos1 >= 1) break;
    // }

    // run the motor until the encoder repeats the same value
    // while (true) {
    //     StepperMotor::rotate_steps(-1);
    //     steps++;
    //     pos2 = encoder.getPosition();
    //     if (pos2 == pos1) break;
    // }

    // StepperMotor::rotate_steps(-1000);

    // Move door to fully closed position
    // while (position > 0) {
    //     StepperMotor::rotate_steps(-1000);
    //     position--;
    // }
    
    // // Move door to fully open position and count steps
    // maxPosition = 0;
    // while (true) {
    //     StepperMotor::rotate_steps(1000);
    //     maxPosition++;

    //     // TODO: change this to check for rotary encoder here
    //     if (maxPosition >= 1000) break; // Arbitrary limit for this example
    // }
    
    // // Move door back to closed position
    // while (position < maxPosition) {
    //     StepperMotor::rotate_steps(-1000);
    //     position--;
    // }
    
    currentState = State::CLOSED;
    calibrationState = CalibrationState::CALIBRATED;
    maxPosition = pos2;
    cout << "Calibration complete. Max position: " << maxPosition << endl;

    // cout << "Calibration complete." << endl;
}

void GarageDoor::open() {
    if (calibrationState == CalibrationState::CALIBRATED) {
        cout << "Opening garage door..." << endl;
        while (position < maxPosition) {
            motor.rotate_steps(1);
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
            motor.rotate_steps(-1);
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
    motor.stop();
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