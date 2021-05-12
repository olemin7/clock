/*
 * CLightSensor.h
 *
 *  Created on: 11 ???. 2017 ?.
 *      Author: User
 */

#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <array>
#include "CSignal.h"
/*
 PhotoR     10K
 +3    o---/\/\/--.--/\/\/---o GND
 |
 Pin 0 o-----------
 */

class CLightDetectResistor: public SignalChange<int> {
    std::array<int, 10> m_filter;
    uint8_t m_count = 0; //average count
    static constexpr int m_refreshPeriod = 100; // ms
    unsigned long m_nextRead = 0;
    const int m_Pin = A0;
    const int m_Tolerance = 10;
    public:
    void setup();
    int getValue();
};

