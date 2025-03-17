#pragma once

#include <cstdint>

namespace MCTP {
    void init();

    void recv(uint8_t *buf, size_t len);
    void send(uint8_t eid, const uint8_t *buf, size_t len);
}