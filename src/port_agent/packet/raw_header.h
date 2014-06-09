#ifndef __RAW_HEADER_H_
#define __RAW_HEADER_H_

#include "packet.h"

#include <stdint.h>

namespace packet {

class RawHeader {

public:
    const uint32_t getSync() const {
        return sync_;
    }

    const PacketType getPacketType() const;

    const uint16_t getChecksum() const {
        return checksum_;
    }

    const uint16_t getPacketSize() const {
        return packetSize_;
    }

    const Timestamp getTimestamp() const;

    const bool validate(const size_t maxPacketSize) const;

private:

    uint32_t sync_ :24;
    uint8_t packetType_ :8;
    uint16_t packetSize_ :16;
    uint16_t checksum_ :16;
    uint64_t timestamp_ :64;

};
}

#endif // __RAW_HEADER_H_
