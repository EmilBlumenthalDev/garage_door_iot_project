#include "garage_door_system.h"
#include "garage_door_config.h"
#include "pico/stdlib.h"
#include <iostream>

using namespace std;

GarageDoorController::GarageDoorController()
    :   door(STEP_PIN1, STEP_PIN2, STEP_PIN3, STEP_PIN4),
        encoder(ENCODER_PIN_A, ENCODER_PIN_B),
        statusLed(STATUS_LED_PIN),
        errorLed(ERROR_LED_PIN),
        buttons(CALIBRATION_BUTTON1, CALIBRATION_BUTTON2, OPERATION_BUTTON)
{

    cout << "Initializing Garage Door Controller..." << endl;

    // Set up the rotary encoder
    encoder.setup();

    // Set up the buttons
    buttons.setup();

    // Initial LED states
    statusLed.turnOff();  // Assume door starts closed
    errorLed.turnOff();   // Assume no initial errors

    cout << "Garage Door Controller initialization complete." << endl;
}

void GarageDoorController::setup() {
    encoder.setup();
    buttons.setup();
}

void GarageDoorController::run() {
    handleLocalOperation();
    updateStatus();
}

void GarageDoorController::handleLocalOperation() {
    if (buttons.isCalibrationPressed()) {
        cout << "Calibrating garage door..." << endl;
        door.calibrate(encoder);
    }
    
    if (buttons.isOperationPressed()) {
        switch (door.getState()) {
            case GarageDoor::State::CLOSED:
                door.open();
                break;
            case GarageDoor::State::OPEN:
                door.close();
                break;
            case GarageDoor::State::IN_BETWEEN:
                GarageDoor::stop();
                break;
        }
    }
}

void GarageDoorController::updateStatus() {
    int newPosition = RotaryEncoder::getPosition();
    door.updatePosition(newPosition);
    
    if (door.getErrorState() == GarageDoor::ErrorState::STUCK) {
        errorLed.blink();
    } else {
        errorLed.turnOff();
    }
    
    switch (door.getState()) {
        case GarageDoor::State::CLOSED:
            statusLed.turnOff();
            break;
        case GarageDoor::State::OPEN:
            statusLed.turnOn();
            break;
        case GarageDoor::State::IN_BETWEEN:
            statusLed.blink();
            break;
    }
}