
#include "rsn_packet_data_buffer.h"

using namespace packet;
using namespace std;

RsnPacketDataBuffer::RsnPacketDataBuffer(size_t bufferCapacity, size_t maxPacketSize, size_t maxInvalidDataSize):
    CircularBuffer(bufferCapacity),
    maxPacketSize_(maxPacketSize),
    maxInvalidDataSize_(maxInvalidDataSize){
}

Packet* RsnPacketDataBuffer::getNextPacket() {   // TODO: Add data timeout?  Would need separate thread.

    char data[maxPacketSize_];
    Packet* packet = NULL;
    size_t sentinelSize = 3;  // TODO: get from parameter or add constant to packet.h
    size_t numberInvalidBytes = 0;

    // Buffer is empty
    if (size() > 0)
        return packet;

    numberInvalidBytes = getLeadingInvalidData(data);  // Get invalid data until a sync appears
    if ((numberInvalidBytes == 0) && (size() > HEADER_SIZE)) {
        // TODO: Should be at sync word
        // TODO: Need to peek_header and check return value
        const size_t headerSize = peekHeader(data);
        RawHeader* header = reinterpret_cast<RawHeader*>(data);
        if (header->validate(maxPacketSize_)) {
            numberInvalidBytes = getLeadingInvalidData(data, true);  // Invalid sync, see if there is more invalid data
        } else if (header->getPacketSize() < size()) {  // Possibly a truncated packet, will have to wait for more data
            // TODO: log
            packet = NULL;
        } else {  // validate data for a packet
            const size_t packetSize = peekPacket(data, header->getPacketSize());
            RawPacket* rawPacket = reinterpret_cast<RawPacket*>(data);
            packet = new Packet(DATA_FROM_INSTRUMENT, rawPacket->getTimestamp(), rawPacket->getPayload(), rawPacket->getPacketSize());
            if (validatePacket(packet, rawPacket)) {
                // TODO: log
                discard(rawPacket->getPacketSize());
            } else {  // Assume invalid sync
                // TODO: log checksum failure
                delete packet;
                packet = NULL;
                numberInvalidBytes = getLeadingInvalidData(data, true);  // See if there is more invalid data
            }
        }
    }

    if ((numberInvalidBytes > 0) && (packet != NULL))
    {
        // TODO: Throw an internal exception
    }

    if (numberInvalidBytes > 0) {
        packet = new Packet(PORT_AGENT_FAULT, Timestamp(), data, numberInvalidBytes);  // TODO: new packet type?
    } else if (packet == NULL) {
        // TODO: buffer is empty or not a complete packet
    }

    return packet;
}

size_t  RsnPacketDataBuffer::getLeadingInvalidData(char* data, bool invalidSync) {

    size_t numberInvalidBytes = 0;
    char next_byte;
    size_t sync_index = 0;
    size_t sync_index_max = 3;

    if (invalidSync && size() >= 3) {  // TODO: make sentinel size a param or something
        numberInvalidBytes = read(data, 3);  // Add sync to invalid data, TODO: check return value
    }

    while(peek_next_byte(next_byte))
    {
        if (next_byte == *(&SYNC + sync_index))
        {
            sync_index++;
            if (sync_index == sync_index_max)
                break;
        }
        else
        {
            sync_index = 0;
            numberInvalidBytes++;
            if (numberInvalidBytes > maxInvalidDataSize_) break;
        }
    }
    if (sync_index > 0)
        //  Sync could have been truncated between reads?
    reset_peek();

    if (numberInvalidBytes > 0)
        read(data, numberInvalidBytes);  // TODO: check return value

    return numberInvalidBytes;
}

const size_t RsnPacketDataBuffer::peekHeader(char* data) {
    // TODO: Check size to make sure enough room for header?
    size_t bytesRead = peek(data, HEADER_SIZE);
    reset_peek();
    return bytesRead;
}

const size_t RsnPacketDataBuffer::peekPacket(char* data, size_t packetSize) {
    size_t rawPacketSize = peek(data, packetSize);  // TODO: Check return value?
    reset_peek();
    return rawPacketSize;
}

bool RsnPacketDataBuffer::validatePacket(Packet* packet, RawPacket* rawPacket) {
    bool valid = false;
    uint16_t rawPacketChecksum = rawPacket->getChecksum();
    uint16_t packetChecksum = packet->checksum();
    if (rawPacketChecksum == packetChecksum)
        valid = true;
    return true;
}
