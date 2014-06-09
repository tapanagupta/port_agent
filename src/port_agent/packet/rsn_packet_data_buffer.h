#ifndef __RSN_PACKET_DATA_BUFFER_H_
#define __RSN_PACKET_DATA_BUFFER_H_

#include "common/circular_buffer.h"
#include "rsn_packet.h"
#include "raw_header.h"
#include "raw_packet.h"

#include <stddef.h>

using namespace std;

// TODO: Make buffer generic for all packet types?  Will have to modify the base packet class?

namespace packet {

class RsnPacketDataBuffer : public CircularBuffer {
        /********************
         *      METHODS     *
         ********************/

    public:

        // TODO: Throws new() exception?
        RsnPacketDataBuffer(size_t bufferCapacity, size_t maxPacketSize, size_t maxInvalidDataSize);

        // Will return pointer to data, make a smart pointer?
        Packet* getNextPacket();

    private:

        RsnPacketDataBuffer();

        // arg invalidSync - buffer starts with an invalid sync word  TODO: need to check size first to make sure enough space
        size_t getLeadingInvalidData(char* data, bool invalidSync = false);

        const size_t peekHeader(char* data);

        const size_t peekPacket(char* data, size_t packetSize);

        bool validatePacket(Packet* packet, RawPacket* rawPacket);

        size_t maxPacketSize_;

        size_t maxInvalidDataSize_;

    };
}

#endif // __RSN_PACKET_DATA_BUFFER_H_
