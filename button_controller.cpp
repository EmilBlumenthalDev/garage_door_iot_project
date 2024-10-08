#include "garage_door_system.h"
#include "pico/stdlib.h"
#include <hardware/gpio.h>

using namespace std;

bool ButtonController::buttonPressed = false;

ButtonController::ButtonController(int b1, int b2, int b3)
    : calibrationButton1(b1), calibrationButton2(b2), operationButton(b3) {}

void ButtonController::setup() const {
    gpio_init(calibrationButton1);
    gpio_init(calibrationButton2);
    gpio_init(operationButton);
    
    gpio_set_dir(calibrationButton1, GPIO_IN);
    gpio_set_dir(calibrationButton2, GPIO_IN);
    gpio_set_dir(operationButton, GPIO_IN);
    
    gpio_pull_up(calibrationButton1);
    gpio_pull_up(calibrationButton2);
    gpio_pull_up(operationButton);

    gpio_set_irq_enabled_with_callback(operationButton, GPIO_IRQ_EDGE_RISE, true,  &RotaryEncoder::IRQ_wrapper);   
}

void ButtonController::setOperationButtonState(bool state) {
    buttonPressed = state;
}

bool ButtonController::isCalibrationPressed() const {
    // Both calibration buttons must be pressed simultaneously
    return !gpio_get(calibrationButton1) && !gpio_get(calibrationButton2);
}

bool ButtonController::isOperationPressed() const {
    return buttonPressed;
}