// Created by: Emil Blumenthal

#include "garage_door_system.h"
#include <iostream>
#include "pico/stdlib.h"

using namespace std;

int main() {
    // Initialize the Raspberry Pi Pico SDK
    stdio_init_all();

    cout << "Garage Door System" << endl;

    // Create an instance of the GarageDoorController
    GarageDoorController controller;

    // Setup the controller
    controller.setup();

    cout << "Running controller loop" << endl;

    // Main loop
    while (true) {
        controller.run();
        
        sleep_ms(10);
    }

    return 0;
}