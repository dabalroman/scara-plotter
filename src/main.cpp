#include <Arduino.h>
#include <LiquidCrystal.h>

#include "RemoteDevelopmentService/LoggerHelper.h"
#include "RemoteDevelopmentService/RemoteDevelopmentService.h"

// LCD: RS, E, D4–D7
LiquidCrystal lcd(32, 33, 25, 26, 27, 14);
LcdDisplay lcdDisplay(&lcd);

RemoteDevelopmentService *gRemoteDevelopmentService = nullptr;
PreferencesManager preferencesManager;

void initHardware() {
    Serial.begin(115200);
    lcd.begin(16, 2);

    lcd.clear();
    lcd.print(String(FW_VERSION));
    Serial.println(String(FW_VERSION));
}

void setup() {
    initHardware();
    preferencesManager.read();

    static RemoteDevelopmentService remoteDev;
    remoteDev.init(preferencesManager, lcdDisplay);
    gRemoteDevelopmentService = &remoteDev;

    printLn("ESP-32 ready. FW version: %s, %s %s\n", FW_VERSION, __DATE__, __TIME__);
    printLn("Read from config:");
    printLn("  enableAp: %d", preferencesManager.settings.enableAp);
    printLn("  enableWifi: %d", preferencesManager.settings.enableWifi);
    printLn("  wifiSSID: %s", preferencesManager.settings.wifiSSID);
    printLn("  wifiPassword: %s", preferencesManager.settings.wifiPassword);
}

void loop() {
    gRemoteDevelopmentService->loop();
}
