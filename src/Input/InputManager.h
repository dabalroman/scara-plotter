#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "Input.h"

class InputManager {
public:
    Input limitSwitchA;
    Input limitSwitchB;
    Input encoderButton;

    InputManager(const uint8_t limitSwitchAGpio, const uint8_t limitSwitchBGpio, const uint8_t encoderButtonGpio)
        : limitSwitchA(limitSwitchAGpio), limitSwitchB(limitSwitchBGpio), encoderButton(encoderButtonGpio) {
    }

    void handleInput(volatile uint8_t &triggeredGpio) {
        if (triggeredGpio == 0) {
            return;
        }

        if (triggeredGpio == limitSwitchA.getGPIO()) {
            limitSwitchA.trigger();
        } else if (triggeredGpio == limitSwitchB.getGPIO()) {
            limitSwitchB.trigger();
        } else if (triggeredGpio == encoderButton.getGPIO()) {
            encoderButton.trigger();
        }

        triggeredGpio = 0;
    }

    void preventTriggerForMs(const ulong delayMs = 1000) {
        encoderButton.preventTriggerForMs(delayMs);
    }
};

#endif // INPUT_MANAGER_H
