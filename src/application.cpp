#include "application.hpp"

void changeBuilinLedState()
{
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

Application::Application()
{
    _settings = new Configuration();
    _terminalServer = new WiFiServer(DEFAULT_TERMINAL_SERVER_PORT);

    _webSockServer = new WebSocketsServer(WEBSOCKET_PORT_);

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
String Application::getContentType(const String &filename)
{
    if (filename.endsWith(".html"))
        return "text/html";

    else if (filename.endsWith(".css"))
        return "text/css";

    else if (filename.endsWith(".js"))
        return "application/javascript";

    else if (filename.endsWith(".ico"))
        return "image/x-icon";

    return HTTP_TEXT_PLAIN;
}
void Application::handleNotFound()
{
    if (!handleFileRead(_WebServer->uri()))
    {
        _WebServer->send(HTTP_SERVER_NOT_FOUND_, FPSTR(HTTP_TEXT_PLAIN), FPSTR(HTTP_NOT_FOUND_TEXT));
    }
}
bool Application::handleFileRead(String path)
{
    bool success = false;
    logger->println("handle file read: " + path);
    if (path.endsWith("/"))
        path += "index.html";

    String contentType = getContentType(path);
    if (LittleFS.exists(path))
    {
        File file = LittleFS.open(path, "r");
        _WebServer->streamFile(file, contentType);
        file.close();

        logger->println(String("Sent file: ") + path);
        success = true;
    }
    else
        logger->println(String("File Not Found: ") + path);

    return success;
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
void Application::handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    if (type == WStype_CONNECTED)
    {
        logger->printf("ws client #%u connected from:", num);
        logger->println(_webSockServer->remoteIP(num));
    }
    else if (type == WStype_DISCONNECTED)
    {
        logger->printf("ws client #%u disconnected\n", num);
    }
    else if (type == WStype_TEXT)
    {
        logger->write(payload, length);
        Serial.write(payload, length);
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

    _WebServer->on(FPSTR(HTTP_SAVE_LINK), [&]() mutable { this->handleSettingsSave(); });
    _WebServer->on(FPSTR(HTTP_CONF_LINK), [&]() mutable { this->handleGetSettings(); });
    _WebServer->onNotFound([&]() mutable { handleNotFound();});
    _WebServer->begin();

    _terminalServer->begin();
    _terminalServer->setNoDelay(true);
    _FTPServer->begin(FPSTR(FTP_LOGIN_), FPSTR(FTP_PASSWORD_));
    _blinker->detach();

    _webSockServer->begin();
    _webSockServer->onEvent([&](uint8_t num, WStype_t type, uint8_t *payload, size_t length) mutable {
         this->handleWebSocketEvent(num, type, payload, length); });
}
void Application::mainloop()
{
    _webSockServer->loop();
    _FTPServer->handleFTP();
    _WebServer->handleClient();
    this->handleTerminalClient();
    this->handleWebConsole();
}
void Application::handleWebConsole()
{
    if(_terminalClient.connected())
         return;

    // протестировать
    // static char buffer[LINE_MAX];
    // static size_t index = 0;
    // while(Serial.available())
    // {
    //     char chr = Serial.read();
    //     if((chr == '\n') || (index == LINE_MAX))
    //     {
    //         _webSockServer->broadcastTXT(buffer, index);
    //         index = 0;
    //     }
    //     if(chr != '\n')
    //         buffer[index++] = chr;
    // }

    static String line;
    while(Serial.available())
    {
        char chr = Serial.read();
        if((chr == '\n') || (line.length() >= LINE_MAX))
        {
            _webSockServer->broadcastTXT(line);
            line.clear();
        }
        if (chr != '\n')
            line += chr;
        
    }
}