#include "publisher_test.h"
#include "common/logger.h"
#include "common/util.h"
#include "port_agent/packet/packet.h"
#include "port_agent/publisher/publisher_list.h"
#include "port_agent/publisher/driver_command_publisher.h"
#include "port_agent/publisher/instrument_command_publisher.h"
#include "port_agent/publisher/tcp_publisher.h"
#include "port_agent/publisher/udp_publisher.h"
#include "port_agent/publisher/log_publisher.h"

#include "network/udp_comm_socket.h";
#include "network/tcp_comm_socket.h";
#include "network/tcp_comm_listener.h";

#include "gtest/gtest.h"

#include <sstream>
#include <string.h>
#include <stdio.h>

using namespace std;
using namespace logger;
using namespace publisher;
using namespace packet;

class PublisherListTest : public testing::Test {
    protected:
        virtual void SetUp() {
            Logger::SetLogFile("/tmp/gtest.log");
            Logger::SetLogLevel("MESG");
            
            LOG(INFO) << "************************************************";
            LOG(INFO) << "   PublisherList Test Start Up";
            LOG(INFO) << "************************************************";
        }
};

TEST_F(PublisherListTest, CTOR) {
	PublisherList list;
    
	TCPCommSocket socketA;
    socketA.setHostname("localhost");
    socketA.setPort(4001);
	
	TCPCommSocket socketB;
    socketB.setHostname("localhost");
    socketB.setPort(4002);
	
    InstrumentCommandPublisher publisherA(&socketA);
    InstrumentCommandPublisher publisherB(&socketB);
	TCPCommSocket *comm;
	InstrumentCommandPublisher *pub;
	
	list.add(&publisherA);
	pub = ((InstrumentCommandPublisher *)(list.front()));
	comm = (TCPCommSocket *)(pub->commSocket());
	
	EXPECT_EQ(list.size(), 1);
	EXPECT_EQ(comm->port(), 4001);
	
	list.add(&publisherB);
	pub = ((InstrumentCommandPublisher *)(list.front()));
	comm = (TCPCommSocket *)(pub->commSocket());
	EXPECT_EQ(list.size(), 1);
	EXPECT_EQ(comm->port(), 4002);
}

TEST_F(PublisherListTest, CommListener) {
	try {
		
	PublisherList list;
    
    TCPCommListener socketA;
	socketA.setPort(4001);
	EXPECT_EQ(socketA.port(), 4001);
	socketA.initialize();
	EXPECT_EQ(socketA.getListenPort(), 4001);
	
	TCPCommListener socketB;
    socketB.setPort(4002);
	socketB.initialize();
	
    DriverCommandPublisher publisherA(&socketA);
    DriverCommandPublisher publisherB(&socketB);
	TCPCommListener *comm;
	DriverCommandPublisher *pub;
	
	list.add(&publisherA);
	pub = ((DriverCommandPublisher *)(list.front()));
	comm = (TCPCommListener *)(pub->commSocket());
	
	EXPECT_EQ(list.size(), 1);
	EXPECT_EQ(comm->getListenPort(), 4001);
	
	list.add(&publisherB);
	pub = ((DriverCommandPublisher *)(list.front()));
	comm = (TCPCommListener *)(pub->commSocket());
	EXPECT_EQ(list.size(), 1);
	EXPECT_EQ(comm->getListenPort(), 4002);
	}
	catch(OOIException &e) {
		string msg = e.what();
		LOG(ERROR) << "Exception: " << msg;
		ASSERT_FALSE(true);
	};
}


TEST_F(PublisherListTest, UDPSocket) {
	PublisherList list;
    
	UDPCommSocket socketA;
    socketA.setHostname("localhost");
    socketA.setPort(4001);
	
	UDPCommSocket socketB;
    socketB.setHostname("localhost");
    socketB.setPort(4002);
	
    UDPPublisher publisherA(&socketA);
    UDPPublisher publisherB(&socketB);
	UDPCommSocket *comm;
	UDPPublisher *pub;
	
	list.add(&publisherA);
	pub = ((UDPPublisher *)(list.back()));
	comm = (UDPCommSocket *)(pub->commSocket());
	
	EXPECT_EQ(list.size(), 1);
	EXPECT_EQ(comm->port(), 4001);
	
	list.add(&publisherB);
	pub = ((UDPPublisher *)(list.back()));
	comm = (UDPCommSocket *)(pub->commSocket());
	EXPECT_EQ(list.size(), 2);
	EXPECT_EQ(comm->port(), 4002);
}

TEST_F(PublisherListTest, TCPSocket) {
	PublisherList list;
    
	TCPCommSocket socketA;
    socketA.setHostname("localhost");
    socketA.setPort(4001);
	
	TCPCommSocket socketB;
    socketB.setHostname("localhost");
    socketB.setPort(4002);
	
    TCPPublisher publisherA(&socketA);
    TCPPublisher publisherB(&socketB);
	TCPCommSocket *comm;
	TCPPublisher *pub;
	
	list.add(&publisherA);
	pub = ((TCPPublisher *)(list.back()));
	comm = (TCPCommSocket *)(pub->commSocket());
	
	EXPECT_EQ(list.size(), 1);
	EXPECT_EQ(comm->port(), 4001);
	
	list.add(&publisherB);
	pub = ((TCPPublisher *)(list.back()));
	comm = (TCPCommSocket *)(pub->commSocket());
	EXPECT_EQ(list.size(), 2);
	EXPECT_EQ(comm->port(), 4002);
}

TEST_F(PublisherListTest, File) {
    LogPublisher publisher;
	PublisherList list;
    
	list.add(&publisher);
	EXPECT_EQ(list.size(), 1);
}

TEST_F(PublisherListTest, PublishTest) {
	PublisherList list;
	LogPublisher publisher;
	TCPCommSocket socketA;
    socketA.setHostname("localhost");
    socketA.setPort(4001);
    TCPPublisher publisherA(&socketA);
            
	Timestamp ts(1, 0x80000000);
    Packet packet(DATA_FROM_DRIVER, ts, "data", 4);
	
	list.add(&publisher);
	EXPECT_EQ(list.size(), 1);
	
	list.add(&publisherA);
	EXPECT_EQ(list.size(), 2);
	
	list.publish(&packet);
}

TEST_F(PublisherListTest, DuplicateAddTest) {
	PublisherList list;
	LogPublisher publisher;
	
	TCPCommSocket socketA;
    socketA.setHostname("localhost");
    socketA.setPort(4001);
    TCPPublisher publisherA(&socketA);
            
	TCPCommSocket socketB;
    socketB.setHostname("localhost");
    socketB.setPort(4001);
    TCPPublisher publisherB(&socketB);
            
	list.add(&publisher);
	EXPECT_EQ(list.size(), 1);
	
	list.add(&publisherA);
	EXPECT_EQ(list.size(), 2);
	
	list.add(&publisherB);
	EXPECT_EQ(list.size(), 2);
}

