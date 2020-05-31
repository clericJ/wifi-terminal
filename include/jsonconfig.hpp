#ifndef WIRELESS_TERMINAL_JSON_CONFIG_H_INCLUDED
#define WIRELESS_TERMINAL_JSON_CONFIG_H_INCLUDED
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <map>

#define DEFAULT_AP_CHANNEL 4
#define DEFAULT_BAUD_SERIAL 9600
#define MIN_APSSID_LEN 3
#define MAX_APSSID_LEN 32
#define MIN_PASS_LEN 8
#define MAX_PASS_LEN 64
#define AP_MAX_CHANNEL 13
const unsigned int DEFAULT_SERIAL_CONFIG = SERIAL_8N1;
static const char DEFAULT_AP_SSID[] PROGMEM = "WirelessTerminal";
static const char DEFAULT_AP_PASS[] PROGMEM = "123456789";
static const char HTML_ID_SBAUD[] PROGMEM = "baud";
static const char HTML_ID_SCONFIG[] PROGMEM = "config";
static const char HTML_ID_APSSID[] PROGMEM = "SSID";
static const char HTML_ID_APPASS[] PROGMEM = "password";
static const char HTML_ID_APCHANNEL[] PROGMEM = "channel";
static const char HTML_ID_APADDRESS[] PROGMEM = "address";

class Configuration
{
public:
    unsigned long serialBaud = DEFAULT_BAUD_SERIAL;
    unsigned int serialConfig = DEFAULT_SERIAL_CONFIG;

    String APSSID = FPSTR(DEFAULT_AP_SSID);
    String APPassword = FPSTR(DEFAULT_AP_PASS);
    unsigned int APchannel = DEFAULT_AP_CHANNEL;
    IPAddress APaddress = IPAddress(192, 168, 1, 1);

    void serialize(DynamicJsonDocument &document);
    void deserialize(DynamicJsonDocument &document);

    String toUrlString();
    void fromUrlString(const String &url);
    void fromMapping(const std::map<String, String> &mapping);
};

class JSONConfig
{
public:
    static bool read(const String &configFileName, Configuration &config, size_t size);
    static bool write(const String &configFileName, Configuration &config, size_t size);

protected:
    static bool save(Configuration &data, File &configFile, size_t size);
    static bool load(Configuration &data, File &configFile, size_t size);
};
#endif