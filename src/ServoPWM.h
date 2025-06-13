#ifndef SERVO_PWM_H
#define SERVO_PWM_H

#include <Arduino.h>

// LEDC (ESP32 built-in PWM) configuration
static constexpr int SERVO_LEDC_CH = 0;
static constexpr int SERVO_FREQUENCY = 50; // 50 Hz for hobby servos
static constexpr int SERVO_RESOLUTION = 16; // 16-bit resolution
static constexpr int SERVO_PERIOD_US = 1000000 / SERVO_FREQUENCY; // 20 000 µs

// Pulse widths in microseconds
static constexpr int SERVO_CENTER_US = 1500; // 90°
static constexpr int SERVO_DELTA_US = 500; // ±30°

inline uint32_t pulseToDuty(const int us) {
    // duty = us / period_us × max_duty
    constexpr uint32_t maxDuty = (1UL << SERVO_RESOLUTION) - 1;
    return static_cast<uint32_t>(us) * maxDuty / SERVO_PERIOD_US;
}

class ServoPWM {
    uint8_t gpio = 0;
    uint8_t position = 0;

public:
    explicit ServoPWM(const uint8_t gpio): gpio(gpio) {
    }

    void down() {
        if (position == 80) {
            return;
        }

        position = 80;
        writeAngle(position);
    }

    void up() {
        if (position == 120) {
            return;
        }

        position = 120;
        writeAngle(position);
    }

    bool isUp() const {
        return position >= 100;
    }

    void begin() {
        ledcSetup(SERVO_LEDC_CH, SERVO_FREQUENCY, SERVO_RESOLUTION);
        ledcAttachPin(gpio, SERVO_LEDC_CH);
        writeAngle(120);
    }

    void writeAngle(int degrees) {
        degrees = constrain(degrees, 0, 180);
        const int pulse = SERVO_CENTER_US + ((degrees - 90) * SERVO_DELTA_US) / 90;
        ledcWrite(SERVO_LEDC_CH, pulseToDuty(pulse));
    }
};

#endif //SERVO_PWM_H
