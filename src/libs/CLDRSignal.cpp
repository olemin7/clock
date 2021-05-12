/*
 * CLDRSignal.cpp
 *
 *  Created on: Apr 28, 2021
 *      Author: ominenko
 */

#include "CLDRSignal.h"
#include "logs.h"
void CLDRSignal::on_change(const int &val) {
    if (val < m_min) {
        m_level = m_intensity_min;
    } else if (val >= m_max) {
        m_level = m_intensity_max;
    } else {
        m_level = m_intensity_min + static_cast<uint8_t>((val - m_min) * (m_intensity_max - m_intensity_min) / (m_max - m_min));
    }
    // DBG_OUT << "CLDRSignal val= " << val << ",lev= " << static_cast<unsigned>(m_level) << endl;
    notify(m_level);
}

void CLDRSignal::setup() {
    ldr.setup();
    ldr.onSignal([this](const int &val) {
        this->on_change(val);
    });
}
