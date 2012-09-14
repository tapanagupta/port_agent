/*******************************************************************************
 * Filename: port_agent_test.h
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Unit tests for the port agent class.  Integration tests are in a different
 * file.
 *
 ******************************************************************************/

#include "common/exception.h"
#include "common/logger.h"
#include "common/spawn_process.h"
#include "port_agent/port_agent.h"
#include "gtest/gtest.h"
#include "common/util.h"

using namespace logger;
using namespace std;
using namespace port_agent;

const char* TEST_OB_CMD_PORT = "4001";
const char* TEST_OB_DATA_PORT = "4002";
const char* TEST_IN_DATA_PORT = "4003";

const char* RESPONSE_FILE="/tmp/gtest.rsp";
const char* CONFIG_FILE="/tmp/gtest.cfg";
const char* TEST_LOG="/tmp/gtest.log";
const char* FILE_LOG="/tmp/gtest.out";
const char* SERVER_LOG="/tmp/gtest.srv";
const char* PORT_AGENT_LOGBASE="/var/ooi/port_agent/port_agent";

class PortAgentUnitTest : public testing::Test {
    protected:
        virtual void SetUp() {
            Logger::SetLogFile(TEST_LOG);
            Logger::SetLogLevel("DEBUG3");
            
            LOG(INFO) << "************************************************";
            LOG(INFO) << "         Port Agent Unit Test Start Up";
            LOG(INFO) << "************************************************";
            
            LOG(INFO) << "Port agent log file: " << portAgentLog();
            
            remove_file(FILE_LOG);
            
            stopPortAgent();
        }
        
        const string portAgentLog() {
            stringstream filename;
            filename << PORT_AGENT_LOGBASE << "_" << TEST_OB_CMD_PORT << ".log";
            return filename.str();
        }
        
        void stopPortAgent() {
            try {
                stringstream cmd;
                cmd << "../port_agent";
                stringstream portStr;
                portStr << TEST_OB_CMD_PORT;

                SpawnProcess process(cmd.str(), 3, "-p", portStr.str().c_str(), "-k");
                process.run();
            }
            catch(exception &e) {
                string err = e.what();
                LOG(ERROR) << "Exception: " << err;
                EXPECT_FALSE(true);
            }
        }
            
        void startPortAgent() {
            try {
                stringstream cmd;
                cmd << "../port_agent";
                stringstream portStr;
                portStr << TEST_OB_CMD_PORT;

                SpawnProcess process(cmd.str(), 8, "-v", "-v", "-v", "-v", "-v", "-v", "-p",
                portStr.str().c_str()); 

                LOG(INFO) << "Start TCP Echo Server: " << process.cmd_as_string();

                process.run();
                sleep(1);
            }
            catch(exception &e) {
                string err = e.what();
                LOG(ERROR) << "Exception: " << err;
                EXPECT_FALSE(true);
            }
        }
    
        void configurePortAgent(const string &config = "") {
            stringstream cmd;
            stringstream shell;
            
            if(config.length())
                cmd << config;
            else 
                cmd << "instrument_type tcp" << endl
                    << "instrument_data_port " << TEST_IN_DATA_PORT << endl
                    << "instrument_addr localhost" << endl
                    << "data_port " << TEST_OB_DATA_PORT << endl;
                    
            LOG(DEBUG) << "Port agent config: " << cmd.str();
            
            create_file(CONFIG_FILE, cmd.str().c_str());
            shell << TOOLSDIR << "/tcp_client_write.py";
            
            LOG(DEBUG) << "Run process: " << shell.str();
            
            SpawnProcess process(shell.str(), 4, "-p", TEST_OB_CMD_PORT,
                                 "-f", CONFIG_FILE);
            process.set_output_file(FILE_LOG);
            process.run();
            
            while(process.is_running()) {
        		LOG(DEBUG) << "Waiting for client process die.";
		        sleep(1);
	        }
        }
        
        string commandPortAgent(const string &cmd = "") {
            stringstream shell;
            string response;
            
            LOG(DEBUG) << "Issue port agent command: " << cmd;
            remove_file(RESPONSE_FILE);
            
            create_file(CONFIG_FILE, cmd.c_str());
            shell << TOOLSDIR << "/tcp_client_write.py";
            
            LOG(DEBUG) << "Run process: " << shell.str();
            
            SpawnProcess process(shell.str(), 8, "-p", TEST_OB_CMD_PORT,
                                 "-f", CONFIG_FILE, "-r", RESPONSE_FILE,
                                 "-t", "3" );
            
            process.set_output_file(FILE_LOG);
            process.run();
            
            while(process.is_running()) {
        		LOG(DEBUG) << "Waiting for client process die.";
		        sleep(1);
	        }
            
            LOG(DEBUG2) << "process response: " << response;
            response = read_file(RESPONSE_FILE);
            return response;
        }
        
        void sendDriverData(const string &cmd) {
            stringstream shell;
            string response;
            
            remove_file(CONFIG_FILE);
            
            LOG(DEBUG) << "Send driver data on port " << TEST_OB_DATA_PORT << ": " << cmd;
            
            create_file(CONFIG_FILE, cmd.c_str());
            shell << TOOLSDIR << "/tcp_client_write.py";
            
            LOG(DEBUG) << "Run process: " << shell.str();
            
            SpawnProcess process(shell.str(), 4, "-p", TEST_OB_DATA_PORT,
                                 "-f", CONFIG_FILE);
            
            process.set_output_file(FILE_LOG);
            process.run();
            
            while(process.is_running()) {
        		LOG(DEBUG) << "Waiting for client process die.";
		        sleep(1);
	        }
        }
        
        virtual void TearDown() {
            LOG(INFO) << "Tear down test";
	    
            while(m_oProcess.is_running()) {
        		LOG(DEBUG) << "Waiting for client to die.";
		        sleep(1);
	        }
	    
	        LOG(DEBUG) << "echo client process complete.";
            stopPortAgent();
        }
    
        ~PortAgentUnitTest() {
            LOG(INFO) << "CommonTest dtor";
        }

        void startTCPEchoServer() {
            stringstream cmd;
            cmd << TOOLSDIR << "/tcp_server_echo.py";
            stringstream portStr;
            portStr << TEST_IN_DATA_PORT;

            SpawnProcess process(cmd.str(), 5, "-s", "-p",
				 portStr.str().c_str(), "-t",
				 "5" ); 

            LOG(INFO) << "Start TCP Echo Server: " << process.cmd_as_string();
	        process.set_output_file(SERVER_LOG);
	
            bool result = process.run();
            sleep(1);
        
	    m_oProcess = process;
	}
            
        void startTCPClientDump(uint16_t port, const string &hostname, const string &file, uint16_t timeout = 10) {
            stringstream cmd;
            
            cmd << TOOLSDIR << "/tcp_client_dump.py";
            stringstream portStr;
            portStr << TEST_IN_DATA_PORT;

            stringstream timeoutStr;
            portStr << timeout;

            SpawnProcess process(cmd.str(), 8, "-n", hostname.c_str(), "-p",
                 "-f", file.c_str(),
				 portStr.str().c_str(), "-t", timeoutStr.str().c_str() ); 

            LOG(INFO) << "Start TCP Echo Server: " << process.cmd_as_string();
	        process.set_output_file(SERVER_LOG);
	
            bool result = process.run();
        }
	
	
        protected:
	    SpawnProcess m_oProcess;
};


/////////////////////////////////////////////////////////////////////////////////
// Integration Tests
/////////////////////////////////////////////////////////////////////////////////

/* Test Startup */
TEST_F(PortAgentUnitTest, StartUp) {
    try {
        string response;
        
        startTCPEchoServer();
        startPortAgent();
        configurePortAgent();
        response = commandPortAgent("get status");
        
        remove_file("/tmp/out");
        //startTCPClientDump(atoi(TEST_OB_CMD_PORT), "localhost", "/tmp/out");
        
        sendDriverData("foo");
        
    }
    catch(exception &e) {
        string err = e.what();
        LOG(ERROR) << "Exception: " << err;
        EXPECT_TRUE(false);
    }
}

/* Test startup sequence and failures */
// Successful start should end in the unconfigured state

/* Test TCP port agent */
/* Walk through port agent states */
// Make sure we try disonnect and make config changes to transition to unconfigured

