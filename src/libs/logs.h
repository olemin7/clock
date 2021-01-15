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

#define DEBUG_STREAM Serial

class NullBuffer: public std::streambuf
{
public:
    int overflow(int c) {
        return c;
    }
};

extern std::ostream null_stream;

#ifdef DEBUG_STREAM
#define DBG_OUT std::cout
#else
    #define DBG_OUT null_stream
#endif

class Cdbg_funk
{
    const char *m_func;
    public:
    Cdbg_funk(const char *file, const char *func) :
            m_func(func) {
        DBG_OUT << ">>enter func:" << FPSTR(m_func) << std::endl;
    }
    ~Cdbg_funk() {
        DBG_OUT << "<<exit func:" << FPSTR(m_func) << std::endl;
    }
};

#ifdef DEBUG_STREAM
#define CDBG_FUNK() Cdbg_funk dbg_funk(__FILE__,__FUNCTION__)
#define DBG_PRINT(...)       { Serial.print(__VA_ARGS__); }
#define DBG_PRINTLN(...)     { Serial.println(__VA_ARGS__); }
#else
    #define CDBG_FUNK()
#define DBG_PRINT(...)       {}
#define DBG_PRINTLN(...)  {}

#endif

#define DBG_FUNK_LINE()  DBG_OUT<<    FPSTR(__FUNCTION__)<<":"<<FPSTR(__LINE__)<<endl

