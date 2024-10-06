#ifndef GARAGE_DOOR_SYSTEM_H
#define GARAGE_DOOR_SYSTEM_H
#include "garage_door_config.h"

#include <cstdint>
#include <chrono>

class RotaryEncoder {
    public:
        RotaryEncoder(int pA, int pB);
        void setup() const;
        [[nodiscard]] static int getPosition();
        static void IRQ_callback(uint gpio, uint32_t events);
        static void IRQ_wrapper(uint gpio, uint32_t events);
        [[nodiscard]] static bool isRotating();
        static void resetPosition();

    private:
        int pinA, pinB;
        volatile int position;
        volatile int revolutions;
        static volatile bool rotating;
};

class StepperMotor {
    public:
        StepperMotor(int p1, int p2, int p3, int p4);
        static void step(bool direction);
        static void rotate_steps(int steps);
        static void stop() ;
        static int rotate_till_collision(bool direction, RotaryEncoder& encoder);

    private:
        int pin1, pin2, pin3, pin4;
};

class GarageDoor {
    public:
        enum class State { OPEN, CLOSED, IN_BETWEEN };
        enum class ErrorState { NORMAL, STUCK };
        enum class CalibrationState { CALIBRATED, NOT_CALIBRATED };
        
        GarageDoor(int p1, int p2, int p3, int p4);
        void calibrate();
        void open();
        void close();
        static void stop();
        int getPosition() const;
        [[nodiscard]] State getState() const;
        [[nodiscard]] ErrorState getErrorState() const;
        [[nodiscard]] CalibrationState getCalibrationState() const;
        void setStuck();
        void updatePosition(int newPosition);

    private:
        State currentState;
        ErrorState errorState;
        CalibrationState calibrationState;
        int position;
        int maxPosition;
        RotaryEncoder encoder;
        StepperMotor motor;
};

class LEDIndicator {
    public:
        explicit LEDIndicator(int pin);
        void turnOn() const;
        void turnOff() const;
        void blink() const;

    private:
        int ledPin;
};

class ButtonController {
    public:
        ButtonController(int b1, int b2, int b3);
        void setup() const;
        [[nodiscard]] bool isCalibrationPressed() const;
        [[nodiscard]] bool isOperationPressed() const;

    private:
        int calibrationButton1, calibrationButton2, operationButton;
};

class GarageDoorController {
    public:
        GarageDoorController();
        void setup();
        void run();

    private:
        void handleLocalOperation();
        void updateStatus();

        GarageDoor door;
        LEDIndicator statusLed;
        LEDIndicator errorLed;
        ButtonController buttons;
};

#endif