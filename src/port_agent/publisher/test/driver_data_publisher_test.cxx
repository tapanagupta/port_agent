#include "common/logger.h"
#include "common/util.h"
#include "port_agent/packet/packet.h"
#include "gtest/gtest.h"
#include "publisher_test.h"
#include "driver_data_publisher.h"
#include "network/udp_comm_socket.h"
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

class DriverDataPublisherTest : public FilePointerPublisherTest {
    
    protected:
        virtual void SetUp() {
            Logger::SetLogFile("/tmp/gtest.log");
            Logger::SetLogLevel("MESG");
            
            LOG(INFO) << "************************************************";
            LOG(INFO) << "   DriverDataPublisherTest Test Start Up";
            LOG(INFO) << "************************************************";

            datafile = DATAFILE;
        }
};

/* Test Basic Creation and ASCII out */
TEST_F(DriverDataPublisherTest, SingleAsciiOut) {
	DriverDataPublisher publisher;
	EXPECT_TRUE(testPublish(publisher, DATA_FROM_INSTRUMENT, true));
	LOG(INFO) << "Next test";
	LOG(INFO) << "Next test";
	LOG(INFO) << "Next test";
	LOG(INFO) << "Next test";
	LOG(INFO) << "Next test";
	LOG(INFO) << "Next test";
	LOG(INFO) << "Next test";
	LOG(INFO) << "Next test";
	LOG(INFO) << "Next test";
	LOG(INFO) << "Next test";
	LOG(INFO) << "Next test";
	EXPECT_TRUE(testPublish(publisher, PORT_AGENT_STATUS, true));
	LOG(INFO) << "Next test";
	EXPECT_TRUE(testPublish(publisher, PORT_AGENT_FAULT, true));
	LOG(INFO) << "Next test";
	EXPECT_TRUE(testNoPublish(publisher, DATA_FROM_DRIVER));
	LOG(INFO) << "Next test";
	EXPECT_TRUE(testNoPublish(publisher, PORT_AGENT_COMMAND));
	LOG(INFO) << "Next test";
	EXPECT_TRUE(testNoPublish(publisher, INSTRUMENT_COMMAND));
	LOG(INFO) << "Next test";
	LOG(INFO) << "Next test";
	LOG(INFO) << "Next test";
	LOG(INFO) << "Next test";
	LOG(INFO) << "Next test";
	LOG(INFO) << "Next test";
	LOG(INFO) << "Next test";
	LOG(INFO) << "Next test";
}

/* Test Single binary packet out out */
TEST_F(DriverDataPublisherTest, SingleBinaryOut) {
	DriverDataPublisher publisher;

	// It shouldn't matter if the binary flag is set, but let's test anyway.
	EXPECT_TRUE(testPublish(publisher, DATA_FROM_INSTRUMENT, false));
	EXPECT_TRUE(testPublish(publisher, PORT_AGENT_STATUS, false));
	EXPECT_TRUE(testPublish(publisher, PORT_AGENT_FAULT, false));
	EXPECT_TRUE(testNoPublish(publisher, DATA_FROM_DRIVER));
	EXPECT_TRUE(testNoPublish(publisher, PORT_AGENT_COMMAND));
	EXPECT_TRUE(testNoPublish(publisher, INSTRUMENT_COMMAND));
}

/* Test publication failures */
TEST_F(DriverDataPublisherTest, FailureNoFile) {
	DriverDataPublisher publisher;
	EXPECT_TRUE(testPublishFailure(publisher, DATA_FROM_INSTRUMENT));
}

// Test equality operator
TEST_F(DriverDataPublisherTest, UDPCommSocketEqualityOperator) {
	try {
    	DriverDataPublisher leftPublisher, rightPublisher;
    	UDPCommSocket leftSocket, rightSocket;
    	
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
TEST_F(DriverDataPublisherTest, TCPListenerEqualityOperator) {
	try {
    	DriverDataPublisher leftPublisher, rightPublisher;
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