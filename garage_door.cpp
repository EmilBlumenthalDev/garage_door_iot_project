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

    int pos1 = encoder.getPosition();

    for (int i = 0; i < 1000; i++) {
        StepperMotor::rotate_steps(-1);
    }

    int pos2 = encoder.getPosition();

    cout << "pos1: " << pos1 << endl;
    cout << "pos2: " << pos2 << endl;

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
    cout << "Calibration complete. Max position: " << maxPosition << endl;

    // cout << "Calibration complete." << endl;
}

void GarageDoor::open() {
    if (calibrationState == CalibrationState::CALIBRATED) {
        cout << "Opening garage door..." << endl;
        while (position < maxPosition) {
            StepperMotor::step(1);
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
            StepperMotor::step(-1);
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
}

void GarageDoor::updatePosition(int newPosition) {
    if (newPosition != position) {
        int steps = newPosition - position;
        StepperMotor::step(steps);
        position = newPosition;
        
        if (position <= 0) {
            currentState = State::CLOSED;
        } else if (position >= maxPosition) {
            currentState = State::OPEN;
        } else {
            currentState = State::IN_BETWEEN;
        }
    }
}