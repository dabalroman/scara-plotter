#ifndef REMOTE_DEVELOPMENT_SERVICE_H
#define REMOTE_DEVELOPMENT_SERVICE_H

#include <deque>
#include <WebServer.h>

#include "LiquidCrystal.h"
#include "../PreferencesManager.h"
#include "Display/LcdDisplay.h"

class RemoteDevelopmentService {
    WebServer *OTAServer = nullptr;
    WiFiServer *telnetServer = nullptr;
    WiFiClient telnetClient;
    PreferencesManager *preferencesManager = nullptr;
    LcdDisplay *lcdDisplay = nullptr;

    bool isAPActive = false;
    bool isWifiActive = false;
    bool isTelnetActive = false;
    bool isOTAActive = false;
    bool isNTPActive = false;

    std::deque<String> logBuffer;
    const size_t MAX_LOGS = 20;

    void setupOTA();

    void setupTelnet();

    void handleTelnet();

public:
    void enableAP();

    void disableAP();

    void init(PreferencesManager &_preferencesManager, LcdDisplay &_lcdDisplay);

    void loop();

    void remotePrintLn(const char *format, ...);

    void telnetFlushLogBuffer();

    bool isAnyNetworkingActive() const {
        return isAPActive || isWifiActive;
    }
};


#endif //REMOTE_DEVELOPMENT_SERVICE_H
