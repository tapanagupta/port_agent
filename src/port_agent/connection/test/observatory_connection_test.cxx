#include "common/exception.h"
#include "common/logger.h"
#include "common/util.h"
#include "network/comm_base.h"
#include "common/spawn_process.h"
#include "network/tcp_comm_listener.h"
#include "port_agent/connection/observatory_connection.h"
#include "gtest/gtest.h"

#include <sstream>
#include <string>
#include <string.h>

using namespace std;
using namespace logger;
using namespace port_agent;

#define TEST_DATA_PORT "6001"
#define TEST_COMMAND_PORT "6000"
#define FILE_LOG "/tmp/gtest.out"

class ObservatoryConnectionTest : public testing::Test {
    
    protected:
        virtual void SetUp() {
            Logger::SetLogFile("/tmp/gtest.log");
            Logger::SetLogLevel("MESG");
            
            LOG(INFO) << "************************************************";
            LOG(INFO) << "     Observatory Connection Test Start Up";
            LOG(INFO) << "************************************************";
        }
		
        void TearDown() {
	        LOG(INFO) << "Tear down test";
	    
	        while(m_oProcess.is_running()) {
    		    LOG(DEBUG) << "Waiting for client to die.";
		        sleep(1);
	        }
		}
		        
    void startTCPEchoClient(uint16_t port, char* testData) {
		int connectDelay = 1;
		int writeDelay = 0;
		int readDelay = 0;
        stringstream cmd;
        LOG(DEBUG) << "Start echo client";
        LOG(DEBUG2) << "Tools dir: " << TOOLSDIR;
 
        cmd << TOOLSDIR << "/tcp_client_echo.py";
        stringstream portStr;
        portStr << port;
        
        stringstream delayStr;
        delayStr << connectDelay;

        stringstream writeDelayStr;
        writeDelayStr << connectDelay;

        stringstream readDelayStr;
        readDelayStr << connectDelay;

        // Start the echo client with a 1 second connection delay
        SpawnProcess process(cmd.str(), 10,
             "-p", portStr.str().c_str(), 
             "-t", delayStr.str().c_str(),
             "-d", testData,
             "-w", writeDelayStr.str().c_str(),
             "-r", readDelayStr.str().c_str() ); 

        LOG(INFO) << "Start TCP Echo Client: " << process.cmd_as_string();
        process.set_output_file(FILE_LOG);

        bool result = process.run();
    
	    m_oProcess = process;
	}
	
	void zeroBuffer(char *buf, int size) {
	    for(int i = 0; i < size; i++) {
		buf[i] = 0;
	    }
	}
	
	protected:
	    SpawnProcess m_oProcess;
};

/* Test Normal Observatory Connection */
TEST_F(ObservatoryConnectionTest, NormalConnection) {
	CommBase *dataConnection;
	CommBase *commandConnection;
	
    try {
        ObservatoryConnection connection;
        Connection *pConnection = &connection;
    
        EXPECT_EQ(pConnection->connectionType(), PACONN_OBSERVATORY_STANDARD);
    
        connection.setCommandPort(6000);
    
        connection.initialize();
    
        EXPECT_TRUE(connection.dataInitialized());
        EXPECT_TRUE(connection.commandInitialized());
    
        EXPECT_FALSE(connection.dataConnected());
        EXPECT_FALSE(connection.commandConnected());
    
        ASSERT_TRUE(connection.dataConnectionObject());
        ASSERT_TRUE(connection.commandConnectionObject());
        
        EXPECT_EQ(connection.commandConnectionObject()->getListenPort(), atoi(TEST_COMMAND_PORT));
        
        EXPECT_NE(connection.dataConnectionObject()->getListenPort(), atoi(TEST_DATA_PORT));
        connection.setDataPort(atoi(TEST_DATA_PORT));
        EXPECT_EQ(connection.dataConnectionObject()->getListenPort(), atoi(TEST_DATA_PORT));
		
		// Now that we are all configiured let's initialize the connection
		connection.dataInitialized();
		connection.commandInitialized();
		
		ASSERT_TRUE(connection.dataInitialized());
		ASSERT_TRUE(connection.commandInitialized());
		
		dataConnection = connection.dataConnectionObject();
		commandConnection = connection.commandConnectionObject();
        EXPECT_TRUE(((TCPCommListener*)dataConnection)->serverFD());
        EXPECT_TRUE(((TCPCommListener*)commandConnection)->serverFD());
		
		LOG(DEBUG) << "Listening";
		
		for(int i = 0; i < 60; i < 60) {
		    startTCPEchoClient(atoi(TEST_COMMAND_PORT), "test");
            LOG(DEBUG) << "DATA FD: " << ((TCPCommListener*)dataConnection)->serverFD();
            LOG(DEBUG) << "CMD  FD: " << ((TCPCommListener*)commandConnection)->serverFD();
			
			sleep(1);
		}
		
    }
    catch(OOIException &e) {
		string err = e.what();
		LOG(ERROR) << "EXCEPTION: " << err;
		ASSERT_FALSE(true);
	}
}

