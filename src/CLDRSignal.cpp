/*
 * CLDRSignal.cpp
 *
 *  Created on: Apr 28, 2021
 *      Author: ominenko
 */

#include "CLDRSignal.h"
#include "logs.h"
void CLDRSignal::on_change(const int &val) {
    uint8_t output = 0;
    if (val < m_input_min) {
        output = m_out_min;
    } else if (val >= m_input_max) {
        output = m_out_max;
    } else {
        output = m_out_min + static_cast<uint8_t>((val - m_input_min) * (m_out_max - m_out_min) / (m_input_max - m_input_min));
    }
    DBG_OUT << "CLDRSignal input= " << val << ",output= " << static_cast<unsigned>(output) << endl;
    notify(output);
}

void CLDRSignal::setup() {
    ldr.setup();
    ldr.onChange([this](const int &val) {
        this->on_change(val);
    });
}
