#pragma once
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
};
