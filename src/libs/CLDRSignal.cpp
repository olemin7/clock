/*
 * CLDRSignal.cpp
 *
 *  Created on: Apr 28, 2021
 *      Author: ominenko
 */

#include <libs/CLDRSignal.h>

uint8_t CLDRSignal::getValue() {
    const auto val = ldr.get();

    if (val < m_min) {
        return m_intensity_min;
    } else if (val >= m_max) {
        return m_intensity_max;
    }
    return m_intensity_min + static_cast<uint8_t>((val - m_min) / (m_max - m_min) * (m_intensity_max - m_intensity_min));
}

