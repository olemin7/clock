/*
 * logs.cpp
 *
 *  Created on: Jan 15, 2021
 *      Author: ominenko
 */
#include "logs.h"
#include <vector>
unsigned int Cdbg_scope::level = 0;

std::string log_buffer;

void logs_begin() {
    log_buffer.reserve(DEBUG_BUFFER);
    log_stream << "\n\n<logs_begin>\n\n";
}

class CLogStreamBuf: public std::streambuf
{
public:
    int overflow(int c) override {
        if (log_buffer.length() >= DEBUG_BUFFER) {
            log_buffer = "\n<<OWF>>";
        }
        log_buffer.push_back(static_cast<char>(c));
        Serial.print(static_cast<char>(c));
        return c;
    }
};

CLogStreamBuf log_stream_buf;
std::ostream log_stream(&log_stream_buf);
