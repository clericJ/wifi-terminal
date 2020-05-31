#include "utils.hpp"

std::map<String, String> deserializeKeyValue(const String &url,
                                             const String elementSeparator,
                                             const String lineSeparator)
{
    std::map<String, String> result;
    char *cStrURL = strdup(url.c_str());
    char *token;

    token = strtok(cStrURL, lineSeparator.c_str());
    logger->println("parse parameters:");
    while (token != NULL)
    {
        String s(token);
        size_t pos = s.indexOf(elementSeparator);

        logger->print("key=" + s.substring(0, pos));
        logger->println("\tval=" + s.substring(pos + 1, std::string::npos));
        result[s.substring(0, pos)] = s.substring(pos + 1, std::string::npos);
        token = strtok(NULL, "&");
    }
    free(cStrURL);
    return result;
}
