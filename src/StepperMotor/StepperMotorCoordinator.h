#ifndef STEPPERMOTORCOORDINATOR_H
#define STEPPERMOTORCOORDINATOR_H
#include "gcode.h"
#include "StepperMotor.h"
#include "Input/InputManager.h"

enum HomingSequence {
    homingA,
    offsettingA,
    homingB,
    offsettingB,
    finished,
    drawingPath
};

class StepperMotorCoordinator {
    StepperMotor &stepperMotorA;
    StepperMotor &stepperMotorB;
    ServoPWM &penServo;
    InputManager &inputManager;

    const long stepperRange = 1000;
    const long minimumStepperDistanceFromEachOther = 200;

    const long homingStepLength = 32;
    const long homingSequenceOffset = 200;
    const long armRange = 2900;

    size_t drawIndex = 0;
    bool penReadyToMove = false;
    bool inMotion = false;

    HomingSequence homingSequence = finished;

    void runHoming() {
        if (homingSequence == homingA) {
            if (inputManager.limitSwitchA.takeActionIfPossible()) {
                stepperMotorA.triggerMinPositionLimitSwitch();
                stepperMotorA.setZeroPosition();
                stepperMotorA.setMinPosition(armRange / -2);
                stepperMotorA.setMaxPosition(armRange / 2);

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
                stepperMotorA.setMinPosition(armRange / -2);
                stepperMotorA.setMaxPosition(armRange / 2);

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

                homingSequence = drawingPath;
                drawIndex = 0;
                printLn("Offset B done; starting path draw");
            }

            stepperMotorB.moveOffset(homingStepLength * -1);
        } else if (homingSequence == drawingPath) {
            if (drawIndex < pathLength) {
                const long rawA = pathSteps[drawIndex][1];
                const long rawB = pathSteps[drawIndex][0];

                if (rawA >= 4096 && rawB >= 4096) {
                    penServo.up();
                    penReadyToMove = false;
                    drawIndex++;

                    if (drawIndex < pathLength) {
                        stepperMotorA.moveToPosition(pathSteps[drawIndex][1]);
                        stepperMotorB.moveToPosition(pathSteps[drawIndex][0]);
                    }
                    return;
                }

                const long currentA = stepperMotorA.getPosition();
                const long targetA = stepperMotorA.getTargetPosition();
                const long currentB = stepperMotorB.getPosition();
                const long targetB = stepperMotorB.getTargetPosition();
                const bool atTarget = abs(currentA - targetA) < 5 && abs(currentB - targetB) < 5;

                if (atTarget && !penReadyToMove) {
                    penServo.down();
                    penReadyToMove = true;
                    return;
                }

                if (atTarget) {
                    stepperMotorA.moveToPosition(rawA);
                    stepperMotorB.moveToPosition(rawB);
                    ++drawIndex;
                }
            } else {
                homingSequence = finished;
                stepperMotorB.moveToPosition(0);
                stepperMotorA.moveToPosition(0);
                penServo.up();
            }
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
    StepperMotorCoordinator(StepperMotor &_stepperMotorA, StepperMotor &_stepperMotorB, ServoPWM &penServo,
                            InputManager &_inputManager)
        : stepperMotorA(_stepperMotorA), stepperMotorB(_stepperMotorB), penServo(penServo),
          inputManager(_inputManager) {
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
