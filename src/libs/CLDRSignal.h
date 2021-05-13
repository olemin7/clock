/*
 * CLDRSignal.h
 *
 *  Created on: Apr 28, 2021
 *      Author: ominenko
 */

#ifndef SRC_LIBS_CLDRSIGNAL_H_
#define SRC_LIBS_CLDRSIGNAL_H_
#include <libs/CADC_filter.h>
#include "CADC_filter.h"

class CLDRSignal: public Signal<uint8_t> {
    CADC_filter ldr;
    int16_t m_min = 250;
    int16_t m_max = 1000;
    static constexpr auto m_intensity_min = uint8_t(0);
    static constexpr auto m_intensity_max = uint8_t(15);
    uint8_t m_level;
    void on_change(const int &val);
    public:
    uint8_t getValue() {
        return m_level;
    }
    void loop() {
        ldr.loop();
    }
    void setup();
    void begin() {
        ldr.begin();
    }
    void setRange(int16_t min, int16_t max) {
        m_min = min;
        m_max = max;
    }
    int16_t getRAW() {
        return ldr.getSavedValue();
    }
};

#endif /* SRC_LIBS_CLDRSIGNAL_H_ */
