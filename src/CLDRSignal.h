/*
 * CLDRSignal.h
 *
 *  Created on: Apr 28, 2021
 *      Author: ominenko
 */

#pragma once
#include <stdint.h>
#include <CADC_filter.h>

class CLDRSignal: public Signal<uint8_t> {
    CADC_filter ldr;
    int16_t m_input_min = 250;
    int16_t m_input_max = 1000;
    uint8_t m_out_min = 0;
    uint8_t m_out_max = 15;
    void on_change(const int &val);
    public:
    void loop() {
        ldr.loop();
    }
    void setup();
    void setRange(int16_t iMin, int16_t iMax, uint8_t oMin, uint8_t oMax) {
        m_input_min = iMin;
        m_input_max = iMax;
        m_out_min = oMin;
        m_out_max = oMax;
    }
    int16_t getLDR() {
        int val;
        ldr.getValue(val);
        return val;
    }
    ~CLDRSignal() {
    }
};
