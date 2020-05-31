#include "jsonconfig.hpp"
#include "logger.hpp"
#include "utils.hpp"

bool validateParameter_sconfig(long sconfig)
{
    logger->print("validate sbaund: ");
    bool is_valid = false;
    switch (sconfig)
    {
    case SerialConfig::SERIAL_5N1:
    case SerialConfig::SERIAL_6N1:
    case SerialConfig::SERIAL_7N1:
    case SerialConfig::SERIAL_8N1:
    case SerialConfig::SERIAL_5N2:
    case SerialConfig::SERIAL_6N2:
    case SerialConfig::SERIAL_7N2:
    case SerialConfig::SERIAL_8N2:
    case SerialConfig::SERIAL_5E1:
    case SerialConfig::SERIAL_6E1:
    case SerialConfig::SERIAL_7E1:
    case SerialConfig::SERIAL_8E1:
    case SerialConfig::SERIAL_5E2:
    case SerialConfig::SERIAL_6E2:
    case SerialConfig::SERIAL_7E2:
    case SerialConfig::SERIAL_8E2:
    case SerialConfig::SERIAL_5O1:
    case SerialConfig::SERIAL_6O1:
    case SerialConfig::SERIAL_7O1:
    case SerialConfig::SERIAL_8O1:
    case SerialConfig::SERIAL_5O2:
    case SerialConfig::SERIAL_6O2:
    case SerialConfig::SERIAL_7O2:
    case SerialConfig::SERIAL_8O2:
        is_valid = true;
    }
    logger->println(is_valid ? "passed" : "failed");
    return is_valid;
}
bool validateParameter_sbaud(long sbaud)
{
    logger->print("validate sbaud: ");
    bool is_valid = false;
    switch (sbaud)
    {
    case 1200:
    case 2400:
    case 4800:
    case 9600:
    case 19200:
    case 38400:
    case 57600:
    case 74880:
    case 115200:
        is_valid = true;
    }
    logger->println(is_valid ? "passed" : "failed");
    return is_valid;
}
bool validateStringParameter(const String &param, unsigned min, unsigned max)
{
    logger->print("validate string parameter: '" + param);
    if (param.length() >= min && param.length() <= max)
    {
        for (size_t i = 0; i < param.length(); i++)
        {
            if (isPrintable(param[i]))
                continue;

            logger->println("' failed");
            return false;
        }
    }
    logger->println("' passed");
    return true;
}
void Configuration::serialize(DynamicJsonDocument &document)
{
    logger->println("serialize");
    document[FPSTR(HTML_ID_SBAUD)] = serialBaud;
    document[FPSTR(HTML_ID_SCONFIG)] = serialConfig;
    document[FPSTR(HTML_ID_APSSID)] = APSSID;
    document[FPSTR(HTML_ID_APPASS)] = APPassword;
    document[FPSTR(HTML_ID_APCHANNEL)] = APchannel;
    document[FPSTR(HTML_ID_APADDRESS)] = APaddress.toString();
}
void Configuration::deserialize(DynamicJsonDocument &document)
{
    logger->println("deserialize");
    serialBaud = document[FPSTR(HTML_ID_SBAUD)] | DEFAULT_BAUD_SERIAL;
    serialConfig = document[FPSTR(HTML_ID_SCONFIG)] | DEFAULT_SERIAL_CONFIG;
    APSSID = document[FPSTR(HTML_ID_APSSID)].as<String>();
    APSSID = APSSID.isEmpty() ? FPSTR(DEFAULT_AP_SSID) : APSSID;
    APPassword = document[FPSTR(HTML_ID_APPASS)].as<String>();
    APPassword = APPassword.isEmpty() ? FPSTR(DEFAULT_AP_PASS) : APPassword;
    APchannel = document[FPSTR(HTML_ID_APCHANNEL)] | DEFAULT_AP_CHANNEL;
    APaddress.fromString(document[FPSTR(HTML_ID_APADDRESS)].as<String>());
}
String Configuration::toUrlString()
{
    String result;
    result += String(FPSTR(HTML_ID_SBAUD)) + "=" + String(serialBaud);
    result += "&" + String(FPSTR(HTML_ID_SCONFIG)) + "=" + String(serialConfig);
    result += "&" + String(FPSTR(HTML_ID_APSSID)) + "=" + APSSID;
    result += "&" + String(FPSTR(HTML_ID_APPASS)) + "=" + APPassword;
    result += "&" + String(FPSTR(HTML_ID_APCHANNEL)) + "=" + String(APchannel);
    result += "&" + String(FPSTR(HTML_ID_APADDRESS)) + "=" + APaddress.toString();

    return result;
}
void Configuration::fromUrlString(const String &parameters)
{
    std::map<String, String> dict = deserializeKeyValue(parameters, "=", "&");
    fromMapping(dict);
}
void Configuration::fromMapping(const std::map<String, String> &mapping)
{
    if (mapping.find(String(FPSTR(HTML_ID_SBAUD))) != mapping.end())
    {
        long sbaud = mapping.at(FPSTR(HTML_ID_SBAUD)).toInt();
        if (validateParameter_sbaud(sbaud))
            this->serialBaud = sbaud;
    }
    if (mapping.find(String(FPSTR(HTML_ID_SCONFIG))) != mapping.end())
    {
        long sconfig = mapping.at(FPSTR(HTML_ID_SCONFIG)).toInt();
        if (validateParameter_sconfig(sconfig))
            this->serialConfig = sconfig;
    }
    if (mapping.find(String(FPSTR(HTML_ID_APSSID))) != mapping.end())
    {
        String ssid = mapping.at(FPSTR(HTML_ID_APSSID));
        if (validateStringParameter(ssid, MIN_APSSID_LEN, MAX_APSSID_LEN))
            APSSID = ssid;
    }
    if (mapping.find(String(FPSTR(HTML_ID_APPASS))) != mapping.end())
    {
        String pass = mapping.at(FPSTR(HTML_ID_APPASS));
        if (validateStringParameter(pass, MIN_PASS_LEN, MAX_PASS_LEN))
            APPassword = pass;
    }
    if (mapping.find(String(FPSTR(HTML_ID_APCHANNEL))) != mapping.end())
    {
        int channel = mapping.at(FPSTR(HTML_ID_APCHANNEL)).toInt();
        if (channel > 0 && channel <= AP_MAX_CHANNEL)
            APchannel = channel;
    }
    if (mapping.find(String(FPSTR(HTML_ID_APADDRESS))) != mapping.end())
        APaddress.fromString(mapping.at(FPSTR(HTML_ID_APADDRESS)));
}
bool JSONConfig::save(Configuration &data, File &configFile, size_t size)
{
    bool success = false;

    logger->println("saving configuration...");
    DynamicJsonDocument doc(size);

    data.serialize(doc);
    if (configFile)
    {
        serializeJson(doc, configFile);
        serializeJson(doc, *logger);

        success = true;
        logger->println("\nconfiguration saved");
        configFile.close();
    }
    else
    {
        logger->println("failed to open config file for writting");
    }
    return success;
}
bool JSONConfig::load(Configuration &data, File &configFile, size_t size)
{
    logger->println("loading configuration...");
    bool success = false;
    if (configFile)
    {
        DynamicJsonDocument doc(size);
        DeserializationError err = deserializeJson(doc, configFile);
        if (err)
        {
            logger->print("deserializeJson() failed with code ");
            logger->println(err.c_str());
        }
        else
        {
            data.deserialize(doc);
            serializeJson(doc, *logger);
            logger->println("\nconfiguration loaded");
            success = true;
        }
        configFile.close();
    }
    return success;
}
bool JSONConfig::read(const String &configFileName, Configuration &config, size_t size)
{
    bool success = false;
    if (!LittleFS.exists(configFileName))
    {
        logger->println("creating default configuration file");
        File configFile = LittleFS.open(configFileName, "w");
        success = JSONConfig::save(config, configFile, size);
        configFile.close();
    }
    else
    {
        success = true;
    }
    if (success)
    {
        File configFile = LittleFS.open(configFileName, "r");
        success = JSONConfig::load(config, configFile, size);
        configFile.close();
    }
    return success;
}
bool JSONConfig::write(const String &configFileName, Configuration &config, size_t size)
{
    bool success = false;
    File configFile = LittleFS.open(configFileName, "w");
    success = JSONConfig::save(config, configFile, size);
    configFile.close();

    return success;
}