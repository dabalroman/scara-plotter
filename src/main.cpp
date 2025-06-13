#include <Arduino.h>
#include <LiquidCrystal.h>

#include "AccelStepper.h"
#include "Input/InputManager.h"
#include "RemoteDevelopmentService/LoggerHelper.h"
#include "RemoteDevelopmentService/RemoteDevelopmentService.h"
#include "StepperMotor/StepperMotor.h"
#include "StepperMotor/StepperMotorCoordinator.h"

// Rotary encoder
constexpr int GPIO_ENCODER_CLK = 4;
constexpr int GPIO_ENCODER_DT = 16;
constexpr int GPIO_ENCODER_SW = 17;

// Stepper motors
constexpr int GPIO_MOTOR_A_DIR = 18;
constexpr int GPIO_MOTOR_A_STEP = 19;
constexpr int GPIO_MOTOR_B_DIR = 21;
constexpr int GPIO_MOTOR_B_STEP = 22;

// Servo
constexpr int GPIO_SERVO = 23;

// Limit switches
constexpr int GPIO_LIMIT_SWITCH_A = 34;
constexpr int GPIO_LIMIT_SWITCH_B = 35;

// LCD (HD44780, 4-bit mode)
constexpr int GPIO_LCD_RS = 32;
constexpr int GPIO_LCD_E = 33;
constexpr int GPIO_LCD_D4 = 25;
constexpr int GPIO_LCD_D5 = 26;
constexpr int GPIO_LCD_D6 = 27;
constexpr int GPIO_LCD_D7 = 14;

// LCD
LiquidCrystal lcd(GPIO_LCD_RS, GPIO_LCD_E, GPIO_LCD_D4, GPIO_LCD_D5, GPIO_LCD_D6, GPIO_LCD_D7);
LcdDisplay lcdDisplay(&lcd);

// Input
volatile uint8_t interruptTriggeredGpio = 0;
void IRAM_ATTR onRemoteReceiverInterrupt_limitSwitchA() { interruptTriggeredGpio = GPIO_LIMIT_SWITCH_A; }
void IRAM_ATTR onRemoteReceiverInterrupt_limitSwitchB() { interruptTriggeredGpio = GPIO_LIMIT_SWITCH_B; }
void IRAM_ATTR onRemoteReceiverInterrupt_encoderSwitch() { interruptTriggeredGpio = GPIO_ENCODER_SW; }

InputManager inputManager(
    GPIO_LIMIT_SWITCH_A,
    GPIO_LIMIT_SWITCH_B,
    GPIO_ENCODER_SW
);

// Motors
AccelStepper accelStepperA(AccelStepper::DRIVER, GPIO_MOTOR_A_STEP, GPIO_MOTOR_A_DIR);
AccelStepper accelStepperB(AccelStepper::DRIVER, GPIO_MOTOR_B_STEP, GPIO_MOTOR_B_DIR);
StepperMotor stepperA(accelStepperA);
StepperMotor stepperB(accelStepperB);
StepperMotorCoordinator stepperCoordinator(stepperA, stepperB, inputManager);

unsigned long lastUpdate = 0;

bool editingA = true;
int lastEncoderClk = HIGH;

// Wi-Fi and OTA
RemoteDevelopmentService *gRemoteDevelopmentService = nullptr;

// Settings
PreferencesManager preferencesManager;

void initHardware() {
    Serial.begin(115200);

    pinMode(GPIO_ENCODER_CLK, INPUT);
    pinMode(GPIO_ENCODER_DT, INPUT);
    pinMode(GPIO_ENCODER_SW, INPUT);

    pinMode(GPIO_MOTOR_A_DIR, OUTPUT);
    pinMode(GPIO_MOTOR_A_STEP, OUTPUT);
    pinMode(GPIO_MOTOR_B_DIR, OUTPUT);
    pinMode(GPIO_MOTOR_B_STEP, OUTPUT);

    pinMode(GPIO_SERVO, OUTPUT);

    pinMode(GPIO_LIMIT_SWITCH_A, INPUT);
    pinMode(GPIO_LIMIT_SWITCH_B, INPUT);

    attachInterrupt(digitalPinToInterrupt(GPIO_LIMIT_SWITCH_A), onRemoteReceiverInterrupt_limitSwitchA, RISING);
    attachInterrupt(digitalPinToInterrupt(GPIO_LIMIT_SWITCH_B), onRemoteReceiverInterrupt_limitSwitchB, RISING);
    attachInterrupt(digitalPinToInterrupt(GPIO_ENCODER_SW), onRemoteReceiverInterrupt_encoderSwitch, RISING);

    lcd.begin(16, 2);
    lcd.clear();
    lcd.print(String(FW_VERSION));
    Serial.println(String(FW_VERSION));
}

void updateValueDisplay() {
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print(editingA ? ">" : " ");
    lcd.print("A: ");
    lcd.print(stepperA.getPosition());

    lcd.setCursor(0, 1);
    lcd.print(editingA ? " " : ">");
    lcd.print("B: ");
    lcd.print(stepperB.getPosition());
}


void setup() {
    initHardware();
    preferencesManager.read();

    static RemoteDevelopmentService remoteDev;
    // remoteDev.init(preferencesManager, lcdDisplay);
    gRemoteDevelopmentService = &remoteDev;

    stepperCoordinator.home();

    printLn("ESP-32 ready. FW version: %s, %s %s\n", FW_VERSION, __DATE__, __TIME__);
    printLn("Read from config:");
    printLn("  enableAp: %d", preferencesManager.settings.enableAp);
    printLn("  enableWifi: %d", preferencesManager.settings.enableWifi);
    printLn("  wifiSSID: %s", preferencesManager.settings.wifiSSID);
    printLn("  wifiPassword: %s", preferencesManager.settings.wifiPassword);
}

void loop() {
    // gRemoteDevelopmentService->loop();
    inputManager.handleInput(interruptTriggeredGpio);
    stepperCoordinator.run();

    // --- Encoder rotation ---
    const int currentClk = digitalRead(GPIO_ENCODER_CLK);
    if (currentClk != lastEncoderClk && currentClk == LOW && stepperCoordinator.isHomed()) {
        const int dt = digitalRead(GPIO_ENCODER_DT);
        const int dir = (dt != currentClk) ? -1 : 1;

        if (editingA) {
            stepperA.moveOffset(dir * 32);
        } else {
            stepperB.moveOffset(dir * 32);
        }
        updateValueDisplay();
    }
    lastEncoderClk = currentClk;

    // --- Encoder button press ---
    if (inputManager.encoderButton.takeActionIfPossible()) {
        editingA = !editingA;
        updateValueDisplay();
    }

    // At most 2 fps, for now
    if (lastUpdate + 500 > millis()) {
        return;
    }

    updateValueDisplay();
    lastUpdate = millis();
    printLn("A %d M %d X %d, B %d M %d X %d", stepperA.getPosition(), stepperA.getMinPosition(),
            stepperA.getMaxPosition(), stepperB.getPosition(), stepperB.getMinPosition(), stepperB.getMaxPosition());
}
