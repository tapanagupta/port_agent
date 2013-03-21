#include "common/logger.h"
#include "common/util.h"
#include "port_agent/packet/packet.h"
#include "gtest/gtest.h"
#include "publisher_test.h"
#include "telnet_sniffer_publisher.h"
#include "network/tcp_comm_socket.h"
#include "network/tcp_comm_listener.h"


#include <sstream>
#include <string>
#include <string.h>

using namespace std;
using namespace packet;
using namespace logger;
using namespace publisher;
using namespace network;

#define DATAFILE "/tmp/data.log"

class TelnetSnifferPublisherTest : public FilePointerPublisherTest {
    
    protected:
		string m_prefix;
		string m_suffix;
		
        virtual void SetUp() {
            Logger::SetLogFile("/tmp/gtest.log");
            Logger::SetLogLevel("MESG");
            
            LOG(INFO) << "************************************************";
            LOG(INFO) << "   TelnetSnifferPublisherTest Test Start Up";
            LOG(INFO) << "************************************************";

            datafile = DATAFILE;
        }
		
        // For the instrument data we just write raw data, not packets.
	    size_t expectedAsciiPacket(char *buffer, const PacketType &type) {
			string result = m_prefix + "data" + m_suffix;
	        strcpy(buffer, result.c_str());
	        return result.length();
	    }

	    size_t expectedBinaryPacket(char *buffer, const PacketType &type) {
	        return expectedAsciiPacket(buffer, type);
	    }
};

/* Test Basic Creation and ASCII out */
TEST_F(TelnetSnifferPublisherTest, SingleAsciiOut) {
	TelnetSnifferPublisher publisher;
	
	// First test, only output instrument data because the prefix or suffix isn't set
	EXPECT_TRUE(testPublish(publisher, DATA_FROM_INSTRUMENT, true));
	EXPECT_TRUE(testNoPublish(publisher, DATA_FROM_DRIVER));
	EXPECT_TRUE(testNoPublish(publisher, PORT_AGENT_COMMAND));
	EXPECT_TRUE(testNoPublish(publisher, PORT_AGENT_STATUS));
	EXPECT_TRUE(testNoPublish(publisher, PORT_AGENT_FAULT));
	EXPECT_TRUE(testNoPublish(publisher, INSTRUMENT_COMMAND));
	
	// Set the prefix and suffix, now we should see data from the driver
	m_prefix = "<<";
	m_suffix = ">>";
	publisher.setPrefix(m_prefix);
	publisher.setSuffix(m_suffix);
	EXPECT_TRUE(testPublish(publisher, DATA_FROM_DRIVER, true));
}

/* Test Single binary packet out out */
TEST_F(TelnetSnifferPublisherTest, SingleBinaryOut) {
	TelnetSnifferPublisher publisher;

	// It shouldn't matter if the binary flag is set, but let's test anyway.
	EXPECT_TRUE(testPublish(publisher, DATA_FROM_INSTRUMENT, false));
	EXPECT_TRUE(testNoPublish(publisher, DATA_FROM_DRIVER));
	EXPECT_TRUE(testNoPublish(publisher, PORT_AGENT_COMMAND));
	EXPECT_TRUE(testNoPublish(publisher, PORT_AGENT_STATUS));
	EXPECT_TRUE(testNoPublish(publisher, PORT_AGENT_FAULT));
	EXPECT_TRUE(testNoPublish(publisher, INSTRUMENT_COMMAND));
}

