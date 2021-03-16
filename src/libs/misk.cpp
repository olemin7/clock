/*
 * misk.cpp
 *
 *  upd 30 dec-2020
 *      Author: ominenko
 */
#include "misk.h"
#include "logs.h"
#include "LittleFS.h"
#include <stdio.h>
#include <sstream>
#include <cmath>
#include "user_interface.h"
using namespace std;

ostream& operator<<(ostream &os, const String &str) {
    os << str.c_str();
    return os;
}

void toJson(std::ostream &os, const float &var) {
    if (isnan(var)) {
        os << "null";
    } else {
        os << var;
    }
}

ostream& operator<<(ostream &os, const timeStatus_t &state) {
    switch (state) {
        case timeNotSet:
            os << "timeNotSet";
            break;
        case timeNeedsSync:
            os << "timeNeedsSync";
            break;
        case timeSet:
            os << "timeSet";
            break;
        default:
            os << "unknown";
            break;
    }
    return os;
}

void toDate(std::ostream &os, const time_t &time) {
    char tt[11];
    sprintf(tt, "%d-%.02d-%02d", year(time), month(time), day(time));
    os << tt;
}
void toTime(std::ostream &os, const time_t &time) {
    char tt[9];
    sprintf(tt, "%d:%.02d:%.02d", hour(time), minute(time), second(time)); //hh:mm:ss
    os << tt;
}

void LED_ON()
{
    digitalWrite(LED_BUILTIN, LOW);
}

void LED_OFF()
{
    digitalWrite(LED_BUILTIN, HIGH);
}

void blink()
{
    LED_ON();
    delay(100);
    LED_OFF();
    delay(400);
}

String getMimeType(String path)
        {
// ------------------------
    if (path.endsWith(".html"))
        return "text/html";
    else if (path.endsWith(".htm"))
        return "text/html";
    else if (path.endsWith(".css"))
        return "text/css";
    else if (path.endsWith(".txt"))
        return "text/plain";
    else if (path.endsWith(".js"))
        return "application/javascript";
    else if (path.endsWith(".json"))
        return "application/json";
    else if (path.endsWith(".png"))
        return "image/png";
    else if (path.endsWith(".gif"))
        return "image/gif";
    else if (path.endsWith(".jpg"))
        return "image/jpeg";
    else if (path.endsWith(".ico"))
        return "image/x-icon";
    else if (path.endsWith(".svg"))
        return "image/svg+xml";
    else if (path.endsWith(".ttf"))
        return "application/x-font-ttf";
    else if (path.endsWith(".otf"))
        return "application/x-font-opentype";
    else if (path.endsWith(".woff"))
        return "application/font-woff";
    else if (path.endsWith(".woff2"))
        return "application/font-woff2";
    else if (path.endsWith(".eot"))
        return "application/vnd.ms-fontobject";
    else if (path.endsWith(".sfnt"))
        return "application/font-sfnt";
    else if (path.endsWith(".xml"))
        return "text/xml";
    else if (path.endsWith(".pdf"))
        return "application/pdf";
    else if (path.endsWith(".zip"))
        return "application/zip";
    else if (path.endsWith(".gz"))
        return "application/x-gzip";
    else if (path.endsWith(".appcache"))
        return "text/cache-manifest";
    return "application/octet-stream";
}

void hw_info(std::ostream &out)
        {
    out << "Compiled " << __DATE__ << " " << __TIME__;
    out << "hw_info ->" << endl;
    out << "CpuFreqMHz " << ESP.getCpuFreqMHz() << endl;
    out << "getFreeHeap " << ESP.getFreeHeap() << endl;
    const auto realSize = ESP.getFlashChipRealSize();
    const auto ideSize = ESP.getFlashChipSize();
    const auto ideMode = ESP.getFlashChipMode();
    out << "Flash info" << endl;
    out << "id:" << ESP.getFlashChipId() << endl;
    out << "size:" << realSize << endl;
    if (ideSize != realSize)
            {
        out << "!!Different size\nFlash IDE size:" << ideSize << endl;
    }
    out << "ide speed:" << ESP.getFlashChipSpeed() << endl;
    out << "ide mode: ";
    switch (ideMode)
    {
        case FM_QIO:
            out << "QIO" << endl;
            break;
        case FM_QOUT:
            out << "QOUT" << endl;
            break;
        case FM_DIO:
            out << "DIO" << endl;
            break;
        case FM_DOUT:
            out << "DOUT" << endl;
            break;
        default:
            out << ideMode;
            out << " UNKNOWN" << endl;
    }
    out << "<- Flash info";
}

void LittleFS_info(std::ostream &out)
        {
    out << "LittleFS_info" << endl;
    FSInfo info;
    LittleFS.info(info);

    out << "Total:" << info.totalBytes << endl;
    out << "Used:" << info.usedBytes << endl;
    out << "nBlock:" << info.blockSize << " Page:" << info.pageSize << endl;
    out << "nMax open files:" << info.maxOpenFiles << endl;
    out << "maxPathLength:" << info.maxPathLength << endl;
//
    out << "files:" << endl;
    auto dir = LittleFS.openDir("/");
    auto index = 0;
    while (dir.next())
    {
        out << index++ << "." << dir.fileName() << " SZ:" << dir.fileSize() << endl;
    }
    out << "<--LittleFS_info" << endl;
}

bool isExtMach(const std::string &name, const std::string &ext)
        {
    const auto pos = name.find_last_of('.');
    if (std::string::npos == pos)
            {
        return false; //no extention
    }
    const auto fext = name.substr(pos + 1);
    if (fext.length() != ext.length())
            {
        return false;
    }
    auto n = fext.length();
    while (n--)
    {
        if (std::toupper(fext.at(n)) != std::toupper(ext.at(n)))
                {
            return false;
        }
    }
    return true;
}
string getResetInfo() {
    ostringstream info;
    const auto rst_info = system_get_rst_info();
    info << "rst_info " << rst_info->reason << ":";
    switch (rst_info->reason) {
        case REASON_DEFAULT_RST:
            info << "REASON_DEFAULT_RST";
            break;
        case REASON_WDT_RST: /* hardware watch dog reset */
            info << "REASON_WDT_RST";
            break;
        case REASON_EXCEPTION_RST: /* exception reset, GPIO status won’t change */
            info << "REASON_EXCEPTION_RST";
            break;
        case REASON_SOFT_WDT_RST: /* software watch dog reset, GPIO status won’t change */
            info << "REASON_SOFT_WDT_RST";
            break;
        case REASON_SOFT_RESTART: /* software restart ,system_restart , GPIO status won’t change */
            info << "REASON_SOFT_RESTART";
            break;
        case REASON_DEEP_SLEEP_AWAKE: /* wake up from deep-sleep */
            info << "REASON_DEEP_SLEEP_AWAKE";
            break;
        case REASON_EXT_SYS_RST:/* external system reset */
            info << "REASON_EXT_SYS_RST";
            break;
        default:
            info << "unknow";
            break;
    }
    if (rst_info->reason == REASON_WDT_RST ||
            rst_info->reason == REASON_EXCEPTION_RST ||
            rst_info->reason == REASON_SOFT_WDT_RST) {
        if (rst_info->reason == REASON_EXCEPTION_RST) {
            info << " exccause " << rst_info->exccause;

        }

        info << " epc1=" << std::hex << rst_info->epc1;
        info << ",epc2=" << std::hex << rst_info->epc2;
        info << ",epc3=" << std::hex << rst_info->epc3;
        info << ",excvaddr=" << std::hex << rst_info->excvaddr;
        info << ",depc=" << std::hex << rst_info->depc;
//The   address of  the last    crash   is  printed,    which   is  used    to debug garbled output.
    }
    info << std::endl;
    return info.str();
}
std::string to_string(uint32_t ul)
        {
    char tt[20];
    snprintf(tt, sizeof(tt) - 1, "%u", ul);
    tt[sizeof(tt) - 1] = 0;
    return std::string(tt);
}

bool isSafeMode(const uint8_t pin, unsigned long timeout) {
    bool result = false;
    pinMode(pin, INPUT_PULLUP);
    delay(100);
    LED_ON();
    if (!digitalRead(pin)) {
        const auto till = millis() + timeout;
        unsigned long blink = 0;
        bool led = true;
        while (1) {
            const auto now = millis();
            if (now > till) {
                break;
            }
            if (now > blink) {
                led = !led;
                if (led) {
                    LED_ON();
                } else {
                    LED_OFF();
                }
                blink = now + 300;
            }
            if (digitalRead(pin)) {
                result = true;
                break;
            }
            yield();
        }
    }
    LED_OFF();
    return result;
}
