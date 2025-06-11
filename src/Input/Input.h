#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>
constexpr uint8_t minimumTriggerDelayMs = 50;

class Input {
    uint8_t gpio;

    bool canTakeAction = false;

    ulong triggeredAtMs = 0;
    ulong canBeTriggerAtMs = 0;

    void takeAction() {
        canTakeAction = false;
        canBeTriggerAtMs = millis() + minimumTriggerDelayMs;
    }

public:
    explicit Input(const uint8_t gpio) : gpio(gpio) {
    }

    uint8_t getGPIO() const {
        return gpio;
    }

    void trigger() {
        canTakeAction = true;
        triggeredAtMs = millis();
    }

    bool takeActionIfPossible() {
        if (canTakeAction && canBeTriggerAtMs <= millis()) {
            takeAction();
            return true;
        }

        return false;
    }

    void preventTriggerForMs(const ulong delayMs = 500) {
        canBeTriggerAtMs = millis() + delayMs;
    }
};

#endif //INPUT_H
