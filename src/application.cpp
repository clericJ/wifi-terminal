#include "application.hpp"

void changeBuilinLedState()
{
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

Application::Application()
{
    _settings = new Configuration();
    _terminalServer = new WiFiServer(DEFAULT_TERMINAL_SERVER_PORT);
    _WebServer = new ESP8266WebServer(WEB_SERVER_PORT);
    _FTPServer = new FtpServer();
    _blinker = new Ticker();
}
bool Application::startAP()
{
    bool result = false;
    WiFi.mode(WIFI_AP);

    if (!WiFi.softAPConfig(_settings->APaddress, DEFAULT_AP_GATEWAY, DEFAULT_AP_MASK))
    {
        logger->println("AP Config Failed");
    }
    else if (WiFi.softAP(_settings->APSSID, _settings->APPassword, _settings->APchannel))
    {
        logger->println("\nnetwork " + _settings->APSSID + " running");
        logger->println("AP IP address: " + WiFi.softAPIP().toString());
        result = true;
    }
    else
    {
        logger->println("starting AP failed");
    }
    return result;
}
void Application::halt()
{
    _blinker->attach(BLINK_SPEED_FAST, changeBuilinLedState);
    logger->println("fatal error - rebooting...");
    delay(REBOOT_DELAY);

    ESP.reset();
}
void Application::handleSettingsSave()
{
    logger->println("handle settings save");
    std::map<String, String> settingsMap;
    for (int i = 0; i < _WebServer->args(); i++)
    {
        settingsMap[_WebServer->argName(i)] = _WebServer->arg(i);
    }
    _settings->fromMapping(settingsMap);
    JSONConfig::write(FPSTR(CONFIG_FILENAME), *_settings, CONFIG_SIZE);
    _WebServer->send(HTTP_SERVER_OK_, FPSTR(HTTP_TEXT_PLAIN), FPSTR(AFTER_SAVING_MSG));

    delay(REBOOT_DELAY);
    ESP.reset();
}
void Application::handleGetSettings()
{
    logger->println("handle get settings");
    String serializedData;
    DynamicJsonDocument doc(CONFIG_SIZE);

    _settings->serialize(doc);
    serializeJson(doc, serializedData);
    _WebServer->send(HTTP_SERVER_OK_, FPSTR(HTTP_TEXT_PLAIN), serializedData);
}
bool Application::handleRoot()
{
    logger->println("handle root");
    auto success = false;
    if (LittleFS.exists(FPSTR(INDEX_PAGE_FILENAME)))
    {
        File file = LittleFS.open(FPSTR(INDEX_PAGE_FILENAME), "r");
        _WebServer->streamFile(file, FPSTR(HTTP_TEXT_HTML));
        success = true;
        file.close();
    }
    return success;
}
void Application::handleTerminalClient()
{
    if (_terminalServer->hasClient())
    {
        if (_terminalClient.connected())
        {
            _terminalClient.println("new client connected, current connection aborted");
            _terminalClient.stop();
        }
        _terminalClient = _terminalServer->available();
        logger->println("Client connected to telnet server");
    }
    while (_terminalClient.available() && Serial.availableForWrite() > 0)
        Serial.write(_terminalClient.read());

    size_t maxToTcp = 0;
    if (_terminalClient)
    {
        size_t bytesCount = _terminalClient.availableForWrite();
        if (bytesCount)
        {
            if (!maxToTcp)
                maxToTcp = bytesCount;
            else
                maxToTcp = std::min(maxToTcp, bytesCount);
        }
        else
            logger->println("client is congested");
    }
    size_t bufferLen = std::min((size_t)Serial.available(), maxToTcp);
    bufferLen = std::min(bufferLen, STACK_MAX_SIZE);
    if (bufferLen)
    {
        uint8_t buffer[bufferLen];
        size_t serialGotBytesCount = Serial.readBytes(buffer, bufferLen);
        if (_terminalClient.availableForWrite() >= serialGotBytesCount)
        {
            size_t sended = _terminalClient.write(buffer, serialGotBytesCount);
            if (sended != bufferLen)
            {
                logger->printf("len mismatch: available:%zd serial-read:%zd tcp-write:%zd\n",
                               bufferLen, serialGotBytesCount, sended);
            }
        }
    }
}
void Application::initialize()
{
    pinMode(LED_BUILTIN, OUTPUT);
    logger->begin(DEFAULT_BAUD_LOGGER);
    logger->println(FPSTR(WELCOME_STRING));

    if (!LittleFS.begin())
    {
        logger->println("failed to mount FS");
        halt();
    }
    JSONConfig::read(FPSTR(CONFIG_FILENAME), *_settings, CONFIG_SIZE);
    Serial.begin(_settings->serialBaud);
    Serial.swap();
    Serial.flush();
    Serial.setRxBufferSize(RX_BUFFER_SIZE);

    if (startAP())
        _blinker->attach(BLINK_SPEED_MIDDLE, changeBuilinLedState);
    else
        halt();

    _WebServer->on(FPSTR(HTTP_ROOT_LINK), [&]() mutable { this->handleRoot(); });
    _WebServer->on(FPSTR(HTTP_SAVE_LINK), [&]() mutable { this->handleSettingsSave(); });
    _WebServer->on(FPSTR(HTTP_CONF_LINK), [&]() mutable { this->handleGetSettings(); });

    _WebServer->onNotFound([&]() mutable {
        _WebServer->send(HTTP_SERVER_NOT_FOUND_, FPSTR(HTTP_TEXT_PLAIN), FPSTR(HTTP_NOT_FOUND_TEXT));
    });
    _WebServer->begin();

    _terminalServer->begin();
    _terminalServer->setNoDelay(true);
    _FTPServer->begin(FPSTR(FTP_LOGIN_), FPSTR(FTP_PASSWORD_));
    _blinker->detach();
}
void Application::mainloop()
{
    _FTPServer->handleFTP();
    _WebServer->handleClient();
    this->handleTerminalClient();
}