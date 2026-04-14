#include "controller.hpp"
#include <cstring>

PSController::PSController() { hid_init(); }
PSController::~PSController() { if(handle) hid_close(handle); hid_exit(); }

bool PSController::connect() {
    // Шукаємо DS4 або DualSense
    handle = hid_open(0x054c, 0x09cc, NULL); 
    if (!handle) handle = hid_open(0x054c, 0x0ce6, NULL);
    
    if (handle) {
        unsigned char buf[64];
        int res = hid_get_feature_report(handle, buf, sizeof(buf));
        connection = (res > 0) ? ConnType::USB : ConnType::BT;
        return true;
    }
    return false;
}

void PSController::update(int r, int g, int b, uint8_t m1, uint8_t m2) {
    if (!handle) return;
    if (connection == ConnType::USB) {
        uint8_t buf[32] = {0x05, 0xFF, 0x00, 0x00, m1, m2, (uint8_t)r, (uint8_t)g, (uint8_t)b};
        hid_write(handle, buf, 32);
    } else {
        uint8_t buf[78] = {0x11, 0xC0, 0x20, 0x05, 0xFF, 0x00, 0x00, m1, m2, (uint8_t)r, (uint8_t)g, (uint8_t)b};
        hid_write(handle, buf, 78);
    }
}
