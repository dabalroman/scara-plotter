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
                stepperMotorA.setMinPosition(stepperMotorA.getPosition());
                stepperMotorA.setMaxPosition(stepperMotorA.getPosition() + stepperRange);

                homingSequence = homingB;
                printLn("Hit A limit on %d", stepperMotorA.getPosition());
            }

            stepperMotorA.moveOffset(-4);
            stepperMotorB.moveOffset(-4);
        } else if (homingSequence == homingB) {
            if (inputManager.limitSwitchB.takeActionIfPossible()) {
                stepperMotorB.triggerMaxPositionLimitSwitch();
                stepperMotorB.setMinPosition(stepperMotorB.getPosition() - stepperRange);
                stepperMotorB.setMaxPosition(stepperMotorB.getPosition());

                homingSequence = finished;
                printLn("Hit B limit on %d", stepperMotorB.getPosition());
            }

            stepperMotorA.moveOffset(4);
            stepperMotorB.moveOffset(4);
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
    };

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
