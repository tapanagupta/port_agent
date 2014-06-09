#ifndef __RAW_PACKET_
#define __RAW_PACKET_

#include "raw_header.h"

namespace packet {

class RawPacket: public RawHeader {
public:
    char* getPayload() {
        return rawPayload;
    }

private:

    char* rawPayload;

};
}

#endif // __RAW_PACKET_
