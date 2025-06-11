#ifndef STEPPERMOTOR_H
#define STEPPERMOTOR_H
#include "AccelStepper.h"
#include "RemoteDevelopmentService/LoggerHelper.h"


class StepperMotor {
    AccelStepper &stepper;

    long minPosition = -10;
    long maxPosition = 10;

public:
    explicit StepperMotor(AccelStepper &stepper) : stepper(stepper) {
        stepper.setMaxSpeed(200); // Steps/sec
        stepper.setAcceleration(50); // Steps/sec^2
    }

    static long clamp(const long min, const long value, const long max) {
        return value < min ? min : value > max ? max : value;
    }

    void triggerMinPositionLimitSwitch() {
        minPosition = stepper.currentPosition();

        if (stepper.targetPosition() < minPosition) {
            stepper.moveTo(minPosition);
        }

        stepper.stop();
    }

    void triggerMaxPositionLimitSwitch() {
        maxPosition = stepper.currentPosition();

        if (stepper.targetPosition() > maxPosition) {
            stepper.moveTo(maxPosition);
        }

        stepper.setCurrentPosition(minPosition);

        stepper.stop();
    }

    void setMinPosition(const long _minPosition) {
        minPosition = _minPosition;
    }

    void setMaxPosition(const long _maxPosition) {
        maxPosition = _maxPosition;
    }

    void moveToPosition(const long position) const {
        const long clampedPosition = clamp(minPosition, position, maxPosition);

        if (clampedPosition != position) {
            printLn("Tried to move beyond limit - target %d, clamped %d", position, clampedPosition);
        }

        stepper.moveTo(clampedPosition);
    }

    /** Dangerous, use only for homing */
    void moveOffset(const long offset) const {
        stepper.move(offset);
    }

    long getPosition() const {
        return stepper.currentPosition();
    }

    long getDirection() const {
        const long targetPosition = stepper.targetPosition();
        const long currentPosition = stepper.currentPosition();

        return clamp(-1, targetPosition - currentPosition, 1);
    }

    void run() const {
        stepper.run();
    }
};


#endif //STEPPERMOTOR_H
