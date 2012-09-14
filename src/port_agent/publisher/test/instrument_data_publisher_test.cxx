#include "common/logger.h"
#include "common/util.h"
#include "port_agent/packet/packet.h"
#include "gtest/gtest.h"
#include "publisher_test.h"
#include "instrument_data_publisher.h"
#include "network/tcp_comm_socket.h"
#include "network/tcp_comm_listener.h"


#include <sstream>
#include <string>
#include <string.h>

using namespace std;
using namespace packet;
using namespace logger;
using namespace publisher;

#define DATAFILE "/tmp/data.log"

class InstrumentDataPublisherTest : public FilePointerPublisherTest {
    
    protected:
        virtual void SetUp() {
            Logger::SetLogFile("/tmp/gtest.log");
            Logger::SetLogLevel("MESG");
            
            LOG(INFO) << "************************************************";
            LOG(INFO) << "   InstrumentDataPublisherTest Test Start Up";
            LOG(INFO) << "************************************************";

            datafile = DATAFILE;
        }


        // For the instrument data we just write raw data, not packets.
	    size_t expectedAsciiPacket(char *buffer, const PacketType &type) {
	        strcpy(buffer, "data");
	        return 4;
	    }

	    size_t expectedBinaryPacket(char *buffer, const PacketType &type) {
	        return expectedAsciiPacket(buffer, type);
	    }
};

/* Test Basic Creation and ASCII out */
TEST_F(InstrumentDataPublisherTest, SingleAsciiOut) {
	InstrumentDataPublisher publisher;
	EXPECT_TRUE(testPublish(publisher, DATA_FROM_DRIVER, true));
	EXPECT_TRUE(testNoPublish(publisher, DATA_FROM_INSTRUMENT));
	EXPECT_TRUE(testNoPublish(publisher, PORT_AGENT_COMMAND));
	EXPECT_TRUE(testNoPublish(publisher, PORT_AGENT_STATUS));
	EXPECT_TRUE(testNoPublish(publisher, PORT_AGENT_FAULT));
	EXPECT_TRUE(testNoPublish(publisher, INSTRUMENT_COMMAND));
}

/* Test Single binary packet out out */
TEST_F(InstrumentDataPublisherTest, SingleBinaryOut) {
	InstrumentDataPublisher publisher;

	// It shouldn't matter if the binary flag is set, but let's test anyway.
	EXPECT_TRUE(testPublish(publisher, DATA_FROM_DRIVER, false));
	EXPECT_TRUE(testNoPublish(publisher, DATA_FROM_INSTRUMENT));
	EXPECT_TRUE(testNoPublish(publisher, PORT_AGENT_COMMAND));
	EXPECT_TRUE(testNoPublish(publisher, PORT_AGENT_STATUS));
	EXPECT_TRUE(testNoPublish(publisher, PORT_AGENT_FAULT));
	EXPECT_TRUE(testNoPublish(publisher, INSTRUMENT_COMMAND));
}

/* Test publication failures */
TEST_F(InstrumentDataPublisherTest, FailureNoFile) {
	InstrumentDataPublisher publisher;
	EXPECT_TRUE(testPublishFailure(publisher, DATA_FROM_DRIVER));
}


// Test equality operator
TEST_F(InstrumentDataPublisherTest, TCPCommSocketEqualityOperator) {
	try {
    	InstrumentDataPublisher leftPublisher, rightPublisher;
    	TCPCommSocket leftSocket, rightSocket;
    	
    	EXPECT_TRUE(leftPublisher == leftPublisher);
    	EXPECT_TRUE(leftPublisher == rightPublisher);
    	
	    // Test the base equality tests
	    leftPublisher.setAsciiMode(false);
	    rightPublisher.setAsciiMode(true);
	    EXPECT_FALSE(leftPublisher == rightPublisher);
	    rightPublisher.setAsciiMode(false);
	    EXPECT_TRUE(leftPublisher == rightPublisher);
	        
        
    	// Test with sockets
    	leftSocket.setHostname("localhost");
        leftSocket.setPort(4001);
    	
	    rightSocket.setHostname("localhost");
        rightSocket.setPort(4001);
    	    
    	EXPECT_TRUE(leftSocket == rightSocket);
    	leftPublisher.setCommObject(&leftSocket);
    	EXPECT_TRUE(leftPublisher.commSocket());
    	
	    EXPECT_FALSE(leftPublisher == rightPublisher);
	
	    rightPublisher.setCommObject(&rightSocket);
	    EXPECT_TRUE(rightPublisher.commSocket());
	    EXPECT_TRUE(leftPublisher == rightPublisher);
        
		rightSocket.setPort(4002);
	    rightPublisher.setCommObject(&rightSocket);
	    EXPECT_FALSE(leftPublisher == rightPublisher);
	}
	catch(OOIException &e) {
		string err = e.what();
		LOG(ERROR) << "EXCEPTION: " << err;
		ASSERT_FALSE(true);
	}
}

// Test equality operator
TEST_F(InstrumentDataPublisherTest, TCPListenerEqualityOperator) {
	try {
    	InstrumentDataPublisher leftPublisher, rightPublisher;
    	TCPCommListener leftSocket;
		TCPCommSocket rightSocket;
    	
    	EXPECT_TRUE(leftPublisher == leftPublisher);
    	EXPECT_TRUE(leftPublisher == rightPublisher);
    	
    	leftPublisher.setCommObject(&leftSocket);
	    rightPublisher.setCommObject(&rightSocket);
    	
	    EXPECT_FALSE(leftPublisher == rightPublisher);
	}
	catch(OOIException &e) {
		string err = e.what();
		LOG(ERROR) << "EXCEPTION: " << err;
		ASSERT_FALSE(true);
	}
}

