#include <CAN.h>
#include "logamaticCan.h"
#include "common.h"

static const long bitrates[] = {
    (long)1000E3,
    (long)500E3,
    (long)250E3,
    (long)200E3,
    (long)125E3,
    (long)100E3,
     (long)80E3,
     (long)50E3,
     (long)40E3,
     (long)20E3,
     (long)10E3,
      (long)5E3, 
 };

static char buf[42];

void logamaticCan::setup() {
    // SPI.pins(); //use the default HSPI pins
    // SPI.setHwCs(true);  // seems to collide with CAN library's CS
    CAN.setPins(D8, D1);
    CAN.setClockFrequency(8E6);
    int rc = CAN.begin(bitrates[_bitrateIdx]);
    Serial.print("CAN.begin(");
    Serial.print(bitrates[_bitrateIdx]);
    Serial.print(")-> ");
    Serial.println(rc);
    // CAN.loopback();
    if (rc == 1) {
        // CAN.onReceive(onReceive);
    }
}

void logamaticCan::mqttRecv(char* topic, byte* payload, unsigned int length) {
    if (strcmp(send_topic, topic) == 0 && length > 0) {
        handleSend(payload, length);
    }
}

void logamaticCan::handleSend(byte* payload, unsigned int length) {
    unsigned int rtr;
    unsigned long id;
    unsigned int b[8];  // %hhx is not reliably supported, parse into unsigned int
    uint8_t data[8];
    unsigned l = length < sizeof(buf) - 1 ? length : sizeof(buf) - 1;
    memcpy(buf, payload, l);
    buf[l] = '\0';
    int r = sscanf(buf, "%x;%lx; %x %x %x %x %x %x %x %x", 
            &rtr, &id, &b[0], &b[1], &b[2], &b[3], 
            &b[4], &b[5], &b[6], &b[7]);
    if (r != 10) {
        mqttPublish(TOPIC_PREFIX "can/err", "MQTT payload");
        return;
    }
    for (int i = 0; i < 8; i++) {
        data[i] = b[i] & 0xff;
    }
    bool ok = CAN.beginPacket(id, 8, rtr) &&
    CAN.write(data, 8) == 8 &&
    CAN.endPacket();

    if (!ok) {
        mqttPublish(TOPIC_PREFIX "can/error/write", "CAN write");
    } else {
        mqttPublish(TOPIC_PREFIX "can/dbg", "CAN write success");
    }
}

void logamaticCan::handleRecv() {
    int d;
    int sz;
    while((sz = CAN.parsePacket()) > 0) {
        _lastrecv = millis();
        int bi = snprintf(buf, sizeof(buf), "%x;%lx;", CAN.packetRtr(), CAN.packetId());
        int n = 0;
        while (n < 8 && (d = CAN.read()) >= 0) {
            bi += snprintf(buf + bi, sizeof(buf) - bi, "%02x ", d);
            n++;
        }
        if (bi >= (int)sizeof(buf)) {
            bi = sizeof(buf) - 1;  // snprintf returns the untruncated length
        }
        if (!mqttPublishRaw(TOPIC_PREFIX "can/raw/recv/", (uint8_t*)buf, bi, false)) {
            break;  // connection dropped, stop publishing until reconnect
        }
    }
}

void logamaticCan::checkErrors() {
    static unsigned long last_check = 0;
    unsigned long now = millis();
    if (now - last_check < 5000) {
        return;
    }
    last_check = now;
    int tec = CAN.readTEC();
    int rec = CAN.readREC();
    int eflg = CAN.readEFLG();
    const int recv_ov_bit0 = 0x40;
    const int recv_ov_bit1 = 0x80;
    const int recv_ov_bits = recv_ov_bit0 | recv_ov_bit1;

    if (eflg & recv_ov_bits) {
        _recv_overflows++;
        CAN.clearEFLG();
    }
    if (tec != _tec || rec != _rec || eflg != _eflg) {
        char buf[48];
        int l = snprintf(buf, sizeof(buf), "EFLG:%x TEC:%x REC:%x ROF:%u", eflg, tec, rec, _recv_overflows);
        if (l >= (int)sizeof(buf)) {
            l = sizeof(buf) - 1;
        }
        mqttPublishRaw(TOPIC_PREFIX "can/error/regs", (uint8_t*)buf, l, true);
    }
    _tec = tec;
    _rec = rec;
    _eflg = eflg;
}

void logamaticCan::loop() {
    handleRecv();
    checkErrors();
}

logamaticCan LogamaticCAN;
