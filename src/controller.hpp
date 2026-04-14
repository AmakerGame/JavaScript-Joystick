#pragma once
#include <vector>
#include <cstdint>
#include "hidapi.h"

enum class ConnType { USB, BT };

class PSController {
public:
    PSController();
    ~PSController();
    bool connect();
    void update(int r, int g, int b, uint8_t motor_heavy, uint8_t motor_light);
    
private:
    hid_device* handle = nullptr;
    ConnType connection = ConnType::USB;
    uint32_t calculate_crc32(uint8_t* data, size_t len);
};
