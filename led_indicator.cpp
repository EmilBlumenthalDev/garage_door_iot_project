#include "garage_door_system.h"
#include "pico/stdlib.h"
#include <hardware/gpio.h>

LEDIndicator::LEDIndicator(int pin) : ledPin(pin) {
    gpio_init(ledPin);
    gpio_set_dir(ledPin, GPIO_OUT);
}

void LEDIndicator::turnOn() const {
    gpio_put(ledPin, true);
}

void LEDIndicator::turnOff() const {
    gpio_put(ledPin, false);
}

void LEDIndicator::blink() const {
    // This is a non-blocking blink. For a real implementation,
    // you might want to use a timer or a separate thread.
    static uint32_t last_blink_time = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    if (current_time - last_blink_time >= 500) {  // Blink every 500ms
        gpio_put(ledPin, !gpio_get(ledPin));  // Toggle LED
        last_blink_time = current_time;
    }
}