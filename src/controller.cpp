#include "controller.hpp"
#include <cstring>

PSController::PSController() { hid_init(); }
PSController::~PSController() { if(handle) hid_close(handle); hid_exit(); }

bool PSController::connect() {
    // Шукаємо DualShock 4 (VID 054c, PID 09cc або 05c4)
    handle = hid_open(0x054c, 0x09cc, NULL);
    if (!handle) handle = hid_open(0x054c, 0x05c4, NULL);
    
    if (handle) {
        // Проста перевірка на BT: у USB репорті зазвичай доступні фічі
        unsigned char buf[64];
        connection = (hid_get_feature_report(handle, buf, sizeof(buf)) > 0) ? ConnType::USB : ConnType::BT;
        return true;
    }
    return false;
}

void PSController::update(int r, int g, int b, uint8_t m1, uint8_t m2) {
    if (!handle) return;

    if (connection == ConnType::USB) {
        uint8_t buf[32] = {0x05, 0xFF, 0x00, 0x00};
        buf[4] = m1; buf[5] = m2; // Вібрація
        buf[6] = r;  buf[7] = g;  buf[8] = b; // RGB
        hid_write(handle, buf, 32);
    } else {
        uint8_t buf[78] = {0x11, 0xC0, 0x20, 0x05, 0xFF, 0x00, 0x00};
        buf[6] = m1; buf[7] = m2;
        buf[8] = r;  buf[9] = g;  buf[10] = b;
        // Тут має бути розрахунок CRC32 для BT, якщо контролер ігнорує пакет
        hid_write(handle, buf, 78);
    }
}
