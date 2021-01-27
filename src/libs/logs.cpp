/*
 * logs.cpp
 *
 *  Created on: Jan 15, 2021
 *      Author: ominenko
 */
#include "logs.h"
unsigned int Cdbg_scope::level = 0;

class CLogBuffer: public std::streambuf
{
public:
    int overflow(int c) override {
        Serial.print(static_cast<char>(c));
        return c;
    }
};

CLogBuffer log_buffer;
std::ostream log_stream(&log_buffer);
