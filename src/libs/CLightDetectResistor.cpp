/*
 * CLightSensor.cpp
 *
 *  Created on: 11 ???. 2017 ?.
 *      Author: User
 */
#include <ESP8266WiFi.h>
#include "CLightDetectResistor.h"

void CLightDetectResistor::setup() {
    pinMode(m_Pin, INPUT);
}

int CLightDetectResistor::getValue() {
    const auto now = millis();
    if (now >= m_nextRead) {
        m_nextRead = now + m_refreshPeriod;
        std::rotate(m_filter.rbegin(), m_filter.rbegin() + 1, m_filter.rend());
        auto val = analogRead(m_Pin);
        if (1 < m_Tolerance) {
            val = (val / m_Tolerance) * m_Tolerance; //rounding
        }
        m_filter[0] = val;
        if (m_count < m_filter.size()) {
            m_count++;
        }
    }
    return m_count ? (std::accumulate(m_filter.begin(), m_filter.end(), 0) / m_count) : 0;

}
