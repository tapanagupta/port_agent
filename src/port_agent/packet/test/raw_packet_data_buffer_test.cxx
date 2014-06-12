#include "raw_packet_data_buffer.h"
#include "common/logger.h"
#include "gtest/gtest.h"
#include "common/util.h"
#include "port_agent/config/port_agent_config.h"

#include <sys/types.h>
#include <unistd.h>
#include <string>

using namespace logger;
using namespace std;
using namespace packet;

class PacketDataBuffer : public testing::Test {

    protected:
        virtual void SetUp() {
            Logger::SetLogFile("/tmp/gtest.log");
            Logger::SetLogLevel("DEBUG");

            LOG(INFO) << "************************************************";
            LOG(INFO) << "            Packet Data Buffer Test Start Up";
            LOG(INFO) << "************************************************";
        }

        virtual void TearDown() {
            LOG(INFO) << "PacketDataBuffer TearDown";
            stringstream out;
        }

        ~PacketDataBuffer() {
            LOG(INFO) << "PacketDataBuffer dtor";
        }

        void const printRawBytes(stringstream& out, const char* buffer, const size_t numBytes) {
            for (size_t ii = 0; ii < numBytes;ii++)
                out << setfill('0') << setw(2) << hex << uppercase << byteToUnsignedInt(buffer[ii]);
        }

};

/* Test NOOP */
TEST_F(PacketDataBuffer, CTR) {
    char rawData[] = {          0xA3, 0x9D, 0x7A, // SYNC
                                0x01, // Message Type
                                0x00, 0x19,  // Message Size
                                0x00, 0x5D,  // Checksum
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Time Stamp
                                0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09   // Payload
                              };

    stringstream out;

    printRawBytes(out, rawData, 0x19);
    LOG(DEBUG) << out.str();

    RawPacket* rawPacket = reinterpret_cast<RawPacket*>(rawData);

    LOG(DEBUG) << "checksum = " << rawPacket->calculateChecksum(rawPacket);

    RawPacketDataBuffer dataBuffer(65536, MAX_PACKET_SIZE, MAX_PACKET_SIZE);

    dataBuffer.writeRawData(rawData, 0x19);

    Packet *packet = dataBuffer.getNextPacket();

    if (packet == NULL)
        out << "No packets";

    out.str("");
    LOG(DEBUG) << out.str();
}
