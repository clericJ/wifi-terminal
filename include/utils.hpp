#ifndef WIRELESS_TERMINAL_UTILS_H_INCLUDED
#define WIRELESS_TERMINAL_UTILS_H_INCLUDED
#pragma once

#include <Arduino.h>
#include <map>
#include "logger.hpp"

std::map<String, String> deserializeKeyValue(const String &url,
                                             const String elementSeparator,
                                             const String lineSeparator);
#endif