#include "raw_header.h"

using namespace packet;

const PacketType RawHeader::getPacketType() const {
    const PacketType packetType =
            static_cast<const PacketType>(packetType_);
    Packet::typeToString(packetType);
    return packetType;
}
;

const Timestamp RawHeader::getTimestamp() const {
    uint32_t seconds = timestamp_ >> 32;
    uint32_t fraction = timestamp_;
    const Timestamp timestamp(seconds, fraction);
    return timestamp;
}
;

const bool RawHeader::validate(const size_t maxPacketSize) const {

    bool valid = true;

    if (getSync() != SYNC) {
        valid = false;
    }

    if (getPacketType() != DATA_FROM_INSTRUMENT) {
        valid = false;
    }

    if ((getPacketSize() < HEADER_SIZE)
            || (getPacketSize() > maxPacketSize)) {
        valid = false;
    }

    return valid;
}
