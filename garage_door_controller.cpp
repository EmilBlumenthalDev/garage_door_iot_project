#include "garage_door_system.h"
#include "garage_door_config.h"
#include "pico/stdlib.h"
#include <iostream>

using namespace std;

GarageDoorController::GarageDoorController()
    :   statusLed(STATUS_LED_PIN),
        errorLed(ERROR_LED_PIN),
        buttons(CALIBRATION_BUTTON1, CALIBRATION_BUTTON2, OPERATION_BUTTON),
        door(STEP_PIN1, STEP_PIN2, STEP_PIN3, STEP_PIN4, buttons)
{

    cout << "Initializing Garage Door Controller..." << endl;

    // Set up the buttons
    // buttons.setup();

    // Initial LED states
    statusLed.turnOff(); // Assume door starts closed
    errorLed.turnOff(); // Assume no initial errors

    cout << "Garage Door Controller initialization complete." << endl;
}

void GarageDoorController::setup() {
    buttons.setup();
}

void GarageDoorController::run() {
    handleLocalOperation();
    updateStatus();
}

void GarageDoorController::handleLocalOperation() {
    if (buttons.isCalibrationPressed()) {
        cout << "Calibrating garage door..." << endl;
        door.calibrate();
    }

    if (buttons.isOperationPressed()) {
        cout << "Operation button pressed" << endl;
        door.toggleMovement();
        buttons.setOperationButtonState(false);
    }
}

void GarageDoorController::updateStatus() {
    if (door.getErrorState() == GarageDoor::ErrorState::STUCK) {
        errorLed.blink();
    } else {
        errorLed.turnOff();
    }

    // cout << "State: " << (door.getState() == GarageDoor::State::OPEN ? "OPEN" : (door.getState() == GarageDoor::State::CLOSED ? "CLOSED" : "IN_BETWEEN")) << endl;
    
    switch (door.getState()) {
        case GarageDoor::State::CLOSED:
            statusLed.turnOff();
            break;
        case GarageDoor::State::OPEN:
            statusLed.turnOn();
            break;
        case GarageDoor::State::STOPPED:
            statusLed.blink();
            break;
    }
}