/*
 * log.h
 *
 *  Created on: 15 jan 2021
 *      Author: ominenko
 */

#pragma once
#include <sdios.h>
#include <iostream>
#include <ostream>
#include <Arduino.h>

#define DEBUG_STREAM Serial

#define DEBUG_BUFFER 1024*2

extern std::string log_buffer;
extern std::ostream log_stream;

std::string get_log_stream();

#define DBG_OUT log_stream

class Cdbg_scope
{
    const String m_funcName;
    static unsigned int level;

public:
    Cdbg_scope(const char *file, const char *func) :
            m_funcName(FPSTR(func)) {
        level++;
        echo_level('<');
        DBG_OUT << m_funcName.c_str() << std::endl;
    }
    static void echo_level(char ch) {
        auto i = level;
        while (i--) {
            DBG_OUT << ch;
        }
    }
    ~Cdbg_scope() {
        echo_level('>');
        DBG_OUT << m_funcName.c_str() << std::endl;
        level--;
    }
};
void logs_begin();

#define DBG_FUNK() Cdbg_scope dbg_scope(__FILE__,__PRETTY_FUNCTION__)

