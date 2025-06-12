#ifndef STEPPERMOTORCOORDINATOR_H
#define STEPPERMOTORCOORDINATOR_H
#include "StepperMotor.h"
#include "Input/InputManager.h"

enum HomingSequence {
    homingA,
    homingB,
    finished
};

class StepperMotorCoordinator {
    StepperMotor &stepperMotorA;
    StepperMotor &stepperMotorB;
    InputManager &inputManager;

    const long stepperRange = 1000;
    const long minimumStepperDistanceFromEachOther = 200;

    HomingSequence homingSequence = finished;

    void runHoming () {
        if (homingSequence == homingA) {
            if (inputManager.limitSwitchA.takeActionIfPossible()) {
                stepperMotorA.triggerMinPositionLimitSwitch();
                stepperMotorA.setZeroPosition();
                stepperMotorA.setMinPosition(stepperMotorA.getPosition());
                stepperMotorA.setMaxPosition(0);

                homingSequence = homingB;

                // Reset limit switch state if it was clicked at the very start
                inputManager.limitSwitchB.takeActionIfPossible();

                printLn("Hit A limit on %d", stepperMotorA.getPosition());
            }

            stepperMotorA.moveOffset(-32);
            // stepperMotorB.moveOffset(-32);
        } else if (homingSequence == homingB) {
            if (inputManager.limitSwitchB.takeActionIfPossible()) {
                stepperMotorB.triggerMaxPositionLimitSwitch();
                stepperMotorB.setZeroPosition();
                stepperMotorB.setMinPosition(0);
                stepperMotorB.setMaxPosition(stepperMotorB.getPosition());

                homingSequence = finished;
                printLn("Hit B limit on %d", stepperMotorB.getPosition());
            }

            stepperMotorA.moveOffset(32);
            stepperMotorB.moveOffset(32);
        }
    }

    void runStandard() const {
        if (inputManager.limitSwitchA.takeActionIfPossible()) {
            stepperMotorA.triggerMinPositionLimitSwitch();
        }

        if (inputManager.limitSwitchB.takeActionIfPossible()) {
            stepperMotorB.triggerMaxPositionLimitSwitch();
        }
    }

public:
    StepperMotorCoordinator(StepperMotor &_stepperMotorA, StepperMotor &_stepperMotorB, InputManager &_inputManager)
        : stepperMotorA(_stepperMotorA), stepperMotorB(_stepperMotorB), inputManager(_inputManager) {
    }

    bool isHomed () const {
        return homingSequence == finished;
    }

    void home() {
        homingSequence = homingA;
    }

    void run () {
        if (homingSequence == homingA || homingSequence == homingB) {
            runHoming();
        } else {
            runStandard();
        }

        stepperMotorA.run();
        stepperMotorB.run();
    }
};

#endif //STEPPERMOTORCOORDINATOR_H
