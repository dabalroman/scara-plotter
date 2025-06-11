#include <Arduino.h>
#include <LiquidCrystal.h>

#include "Input/InputManager.h"
#include "RemoteDevelopmentService/LoggerHelper.h"
#include "RemoteDevelopmentService/RemoteDevelopmentService.h"

// Rotary encoder
constexpr int GPIO_ENCODER_CLK = 4;
constexpr int GPIO_ENCODER_DT  = 16;
constexpr int GPIO_ENCODER_SW  = 17;

// Stepper motors
constexpr int GPIO_MOTOR_A_DIR  = 18;
constexpr int GPIO_MOTOR_A_STEP = 19;
constexpr int GPIO_MOTOR_B_DIR  = 21;
constexpr int GPIO_MOTOR_B_STEP = 22;

// Servo
constexpr int GPIO_SERVO = 23;

// Limit switches
constexpr int GPIO_LIMIT_SWITCH_A = 12; // 34 on the PCB
constexpr int GPIO_LIMIT_SWITCH_B = 13; // 35 on the PCB

// LCD (HD44780, 4-bit mode)
constexpr int GPIO_LCD_RS = 32;
constexpr int GPIO_LCD_E  = 33;
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

unsigned long lastUpdate = 0;


// Wi-Fi and OTA
RemoteDevelopmentService *gRemoteDevelopmentService = nullptr;\

// Settings
PreferencesManager preferencesManager;

void initHardware() {
    Serial.begin(115200);

    pinMode(GPIO_ENCODER_CLK, INPUT);
    pinMode(GPIO_ENCODER_DT, INPUT);
    pinMode(GPIO_ENCODER_SW, INPUT_PULLUP);

    pinMode(GPIO_MOTOR_A_DIR, OUTPUT);
    pinMode(GPIO_MOTOR_A_STEP, OUTPUT);
    pinMode(GPIO_MOTOR_B_DIR, OUTPUT);
    pinMode(GPIO_MOTOR_B_STEP, OUTPUT);

    pinMode(GPIO_SERVO, OUTPUT);

    pinMode(GPIO_LIMIT_SWITCH_A, INPUT_PULLUP);
    pinMode(GPIO_LIMIT_SWITCH_B, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(GPIO_LIMIT_SWITCH_A), onRemoteReceiverInterrupt_limitSwitchA, RISING);
    attachInterrupt(digitalPinToInterrupt(GPIO_LIMIT_SWITCH_B), onRemoteReceiverInterrupt_limitSwitchB, RISING);
    attachInterrupt(digitalPinToInterrupt(GPIO_ENCODER_SW), onRemoteReceiverInterrupt_encoderSwitch, RISING);

    lcd.begin(16, 2);
    lcd.clear();
    lcd.print(String(FW_VERSION));
    Serial.println(String(FW_VERSION));
}

void setup() {
    initHardware();
    preferencesManager.read();

    static RemoteDevelopmentService remoteDev;
    // remoteDev.init(preferencesManager, lcdDisplay);
    gRemoteDevelopmentService = &remoteDev;

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

    // At most 20 fps, for now
    if (lastUpdate + 50 > millis()) {
        return;
    }

    lastUpdate = millis();

    if (inputManager.limitSwitchA.takeActionIfPossible()) {
        printLn("Limit switch A triggered");
    }

    if (inputManager.limitSwitchB.takeActionIfPossible()) {
        printLn("Limit switch B triggered");
    }

    if (inputManager.encoderButton.takeActionIfPossible()) {
        printLn("Encoder switch A triggered");
    }
}
