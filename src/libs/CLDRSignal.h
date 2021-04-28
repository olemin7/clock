/*
 * CLDRSignal.h
 *
 *  Created on: Apr 28, 2021
 *      Author: ominenko
 */

#ifndef SRC_LIBS_CLDRSIGNAL_H_
#define SRC_LIBS_CLDRSIGNAL_H_
#include "CSignal.h"
#include "CLightDetectResistor.h"

class CLDRSignal: public SignalChange<uint8_t> {
    CLightDetectResistor ldr;
    int16_t m_min = 250;
    int16_t m_max = 1000;
    static constexpr auto m_intensity_min = uint8_t(0);
    static constexpr auto m_intensity_max = uint8_t(15);
    uint8_t getValue();
    public:

    void setRange(int16_t min, int16_t max) {
        m_min = min;
        m_max = max;
    }
    int16_t get() {
        return ldr.get();
    }
};

#endif /* SRC_LIBS_CLDRSIGNAL_H_ */
