#include <stdio.h>
#include <cstdlib>
#include <vector>
#include <cstring>

#include "mctp.hpp"
#include "i2c/i2c.h"

extern "C" {
#include "libmctp.h"
#include "core-internal.h"
#include "libmctp-i2c.h"
#include "i2c-internal.h"
}

namespace MCTP {
    static const uint8_t I2C_ADDR = 0x21;
    static const uint8_t EID = 0x50;
    struct mctp *mctp;
    struct mctp_binding_i2c *i2c;
    static void rx(uint8_t src_eid, bool tag_owner, uint8_t msg_tag, void *ctx,
        void *msg, size_t len);
    static int tx(const void *buf, size_t len, void *ctx);
    void recv(uint8_t *buf, size_t len) {
        mctp_i2c_rx(i2c, buf, len);
    }
    ;
    void send(uint8_t eid, const uint8_t *buf, size_t len) {
        eid = 0x51;
        mctp_message_tx(mctp, eid, true, 2, buf, len);
        while (!mctp_is_tx_ready(mctp, eid)) {
            mctp_i2c_tx_poll(i2c);
        }
    }
}

void MCTP::init() {
    mctp_set_log_stdio(MCTP_LOG_DEBUG);
    mctp = mctp_init();
    assert(mctp);
    i2c = reinterpret_cast<struct mctp_binding_i2c *>(malloc(
        sizeof(struct mctp_binding_i2c)));
    assert(i2c);

    mctp_i2c_setup(i2c, I2C_ADDR, tx, NULL);
    mctp_register_bus(mctp, mctp_binding_i2c_core(i2c), EID);
    mctp_set_rx_all(mctp, rx, NULL);
    mctp_i2c_set_neighbour(i2c, 0x51, 0x22);
}

static void MCTP::rx(uint8_t src_eid, bool tag_owner, uint8_t msg_tag,
    void *ctx, void *msg, size_t len) {
    printf("I2C RX\n");
}

static int MCTP::tx(const void *buf, size_t len, void *ctx) {
    auto data = reinterpret_cast<const uint8_t *>(buf);

    int bus;
    if ((bus = i2c_open("/dev/i2c-1")) == -1) {

        printf("libi2c error\n");
    }

    printf("antes   ");
    for (size_t i = 0; i < len; i++) {
        printf("%d ", data[i]);
    }
    std::vector<uint8_t> miau(len);
    memcpy(miau.data(), data, len);
    auto hdr = reinterpret_cast<struct mctp_i2c_hdr *>(miau.data());
    hdr->dest = i2c->own_addr << 1;
    hdr->source = ((i2c->own_addr + 1) << 1) + 1;
    miau.data()[5] = 80;
    miau.data()[6] = 81;

    printf("\ndespues ");
    for (size_t i = 0; i < len; i++) {
        printf("%d ", miau.data()[i]);
    }
    printf(" tx\n");

    mctp_i2c_rx(i2c, miau.data(), len);

    return 0;
}