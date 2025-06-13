#ifndef STEPPERMOTORCOORDINATOR_H
#define STEPPERMOTORCOORDINATOR_H
#include "StepperMotor.h"
#include "Input/InputManager.h"

enum HomingSequence {
    homingA,
    offsettingA,
    homingB,
    offsettingB,
    finished
};

class StepperMotorCoordinator {
    StepperMotor &stepperMotorA;
    StepperMotor &stepperMotorB;
    InputManager &inputManager;

    const long stepperRange = 1000;
    const long minimumStepperDistanceFromEachOther = 200;

    const long homingStepLength = 32;
    const long homingSequenceOffset = 200;
    const long armRange = 2900;

    HomingSequence homingSequence = finished;

    void runHoming() {
        if (homingSequence == homingA) {
            if (inputManager.limitSwitchA.takeActionIfPossible()) {
                stepperMotorA.triggerMinPositionLimitSwitch();
                stepperMotorA.setZeroPosition();
                stepperMotorA.setMinPosition(stepperMotorA.getPosition());
                stepperMotorA.setMaxPosition(0);

                homingSequence = offsettingA;

                // Reset limit switch state if it was clicked at the very start
                inputManager.limitSwitchB.takeActionIfPossible();

                printLn("Hit A limit on %d", stepperMotorA.getPosition());
            }

            stepperMotorA.moveOffset(homingStepLength * -1);
            stepperMotorB.moveOffset(homingStepLength * -1);
        } else if (homingSequence == offsettingA) {
            if (stepperMotorA.getPosition() > homingSequenceOffset) {
                homingSequence = homingB;
                printLn("Offset A done");
            }

            stepperMotorA.moveOffset(homingStepLength);
            stepperMotorB.moveOffset(homingStepLength);
        } else if (homingSequence == homingB) {
            if (inputManager.limitSwitchB.takeActionIfPossible()) {
                stepperMotorB.triggerMaxPositionLimitSwitch();
                stepperMotorB.setZeroPosition();
                stepperMotorB.setMinPosition(0);
                stepperMotorB.setMaxPosition(stepperMotorB.getPosition());

                homingSequence = offsettingB;
                printLn("Hit B limit on %d", stepperMotorB.getPosition());
            }

            stepperMotorB.moveOffset(homingStepLength);
        } else if (homingSequence == offsettingB) {
            if (stepperMotorB.getPosition() < homingSequenceOffset * -1) {
                const long halfOfRange = armRange / 2;

                // Set "0" in both to the robot middle.
                // Stepper A position is positive, due to CCW rotation.
                stepperMotorA.setZeroPosition(halfOfRange * -1 + stepperMotorA.getPosition());
                // Stepper B position is negative due to CW rotation.
                stepperMotorB.setZeroPosition(halfOfRange + stepperMotorB.getPosition());

                homingSequence = finished;
                printLn("Offset B done");
            }

            stepperMotorB.moveOffset(homingStepLength * -1);
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

    bool isHomed() const {
        return homingSequence == finished;
    }

    void home() {
        homingSequence = homingA;
    }

    void run() {
        if (homingSequence != finished) {
            runHoming();
        } else {
            runStandard();
        }

        stepperMotorA.run();
        stepperMotorB.run();
    }
};

#endif //STEPPERMOTORCOORDINATOR_H
