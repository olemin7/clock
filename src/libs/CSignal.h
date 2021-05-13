/*
 * CObserver.h
 *
 *  Created on: 13 may  2021
 *      Author: ominenko
 */

#pragma once

#include <stdint.h>
#include <vector>
#include <functional>
using namespace std;
/***
 *
 */

template<class T>
class Signal {
private:
    std::vector<std::function<void(const T&)> > m_onSignal;
    std::vector<std::function<void(const T&)> > m_onChange;
    bool m_firstTime = true; //for first mine init;
    T m_preVal;
    protected:
    void notify(const T &value) {
        for (auto &iterator : m_onSignal) {
            iterator(value);
        }
        if (m_firstTime || isChanged(value, m_preVal)) {
            m_firstTime = false;
            m_preVal = value;
            for (auto &iterator : m_onChange) {
                iterator(value);
            }
        }
    }
    virtual bool isChanged(const T &lhs, const T &rhs) {
        return lhs != rhs;
    }
public:
    void onSignal(std::function<void(const T&)> _onSignal) {
        m_onSignal.push_back(_onSignal);
    }
    void onChange(std::function<void(const T&)> _onSignal) {
        m_onChange.push_back(_onSignal);
    }
};

template<class T>
class SignalLoop: public Signal<T> {
public:
    virtual void begin() =0;
    virtual void loop()=0;
    virtual ~SignalLoop() {
    }
};

template<class T>
class SignalChange: public SignalLoop<T> {
private:
    T value;
    private:
    virtual T getValue() =0;

public:
    void begin() override {
        this->notify(getValue());
    }
    void loop() override {
        const T tmp = getValue();
        if (tmp != value) {
            value = tmp;
            this->notify(value);
        }
    }
    virtual ~SignalChange() {
    }
    T getSavedValue() const {
        return value;
    }
};

