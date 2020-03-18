/*
 * log.h
 *
 *  Created on: 30 זמגע. 2019 נ.
 *      Author: ominenko
 */

#ifndef LIBS_LOGS_H_
#define LIBS_LOGS_H_
#ifndef UNIT_TEST
#include <sdios.h>
#include <Stream.h>
#endif
// debug does not work with marlin
#define DEBUG_STREAM Serial

#define ERR_LOG(...){\
DBG_PRINT(";ERR ");DBG_PRINT(__FILE__);DBG_PRINT(":");DBG_PRINT(static_cast<unsigned>(__LINE__));\
DBG_PRINT(" ");DBG_PRINTLN(__VA_ARGS__);}

#define DBG_FUNK_LINE()      {DBG_PRINT(__FUNCTION__); DBG_PRINT(":"); DBG_PRINTLN(__LINE__);}

#ifdef DEBUG_STREAM
#ifdef UNIT_TEST
#define DBG_PRINT(...)       { std::cout <<__VA_ARGS__; }
#define DBG_PRINTLN(...)     { std::cout <<__VA_ARGS__;}
#else
#define DBG_PRINT(...)       { Serial.print(__VA_ARGS__); }
#define DBG_PRINTLN(...)     { Serial.println(__VA_ARGS__); }
#endif

#else
#define DBG_PRINT(...)       {}
#define DBG_PRINTLN(...)  {}
#endif
#endif /* LIBS_LOGS_H_ */
