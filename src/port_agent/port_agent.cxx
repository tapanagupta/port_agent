/*******************************************************************************
 * Class: PortAgent
 * Filename: port_agent.cpp
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Main controlling class for the port agent
 ******************************************************************************/
#include "version.h"
#include "port_agent.h"
#include "config/port_agent_config.h"
#include "connection/observatory_connection.h"
#include "connection/observatory_multi_connection.h"
#include "connection/instrument_tcp_connection.h"
#include "connection/instrument_botpt_connection.h"
#include "connection/instrument_serial_connection.h"
#include "packet/packet.h"
#include "packet/buffered_single_char.h"

#include "publisher/log_publisher.h"
#include "publisher/driver_command_publisher.h"
#include "publisher/driver_data_publisher.h"
#include "publisher/instrument_command_publisher.h"
#include "publisher/instrument_data_publisher.h"
#include "publisher/telnet_sniffer_publisher.h"
#include "publisher/udp_publisher.h"
#include "publisher/tcp_publisher.h"

#include <iostream>
#include <sstream>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;
using namespace packet;
using namespace network;
using namespace port_agent;

/******************************************************************************
 *   PUBLIC METHODS
 ******************************************************************************/

/******************************************************************************
 * Method: Default Constructor
 ******************************************************************************/
PortAgent::PortAgent() {
    m_pObservatoryConnection = NULL;
    m_pInstrumentConnection = NULL;
    
    m_pConfig = NULL;
    m_oState = STATE_UNKNOWN;
}

/******************************************************************************
 * Method: Constructor
 * Description: Construct a configuration object from command line parameters
 *              passed in from the command line using (argv).
 ******************************************************************************/
PortAgent::PortAgent(int argc, char *argv[]) {
    // Setup the log file if we are running as a daemon
    LOG(DEBUG) << "Initialize port agent with args";
    
    m_pConfig = new PortAgentConfig(argc, argv);
    setState(STATE_STARTUP);
    
    m_pInstrumentConnection = NULL;
    m_pObservatoryConnection = NULL;
    m_pTelnetSnifferConnection = NULL;
}

/******************************************************************************
 * Method: Destructor
 * Description: Clear dynamic memory
 ******************************************************************************/
PortAgent::~PortAgent() {
    if(m_pObservatoryConnection)
        delete m_pObservatoryConnection;
        
    if(m_pInstrumentConnection)
        delete m_pInstrumentConnection;
        
    if(m_pTelnetSnifferConnection)
        delete m_pTelnetSnifferConnection;
        
    if(m_pConfig)
        delete m_pConfig;
        
    m_pConfig = NULL;
}

/******************************************************************************
 * Method: daemon_command
 * Description: Overloaded virtual method from the Daemon Process class.
 * Defines the command used to launch the daemon process.
 ******************************************************************************/
const string PortAgent::daemon_command() {
    throw NotImplemented();
}

/******************************************************************************
 * Method: no_daemon
 * Description: Tell the parent class if we should daemonize or not.
 * Returns true if we want to run in signle thread mode
 ******************************************************************************/
bool PortAgent::no_daemon() {
    return m_pConfig ? m_pConfig->noDetatch() : true;
}

/******************************************************************************
 * Method: ppid
 * Description: Tell the parent class the parent process id for a poison pill
 * Returns a pid to monitor.
 ******************************************************************************/
uint32_t PortAgent::ppid() {
    return m_pConfig ? m_pConfig->ppid() : 0;
}

/******************************************************************************
 * Method: pid_file
 * Description: Overloaded virtual method from the Daemon Process class.
 * Returns the path to the daemon process PID file.
 ******************************************************************************/
const string PortAgent::pid_file() {
    return m_pConfig ? m_pConfig->pidfile() : "";
}


/******************************************************************************
 * Method: start
 * Description: Overloaded the base class method to check for different
 * commands, specifically kill and help.
 ******************************************************************************/
bool PortAgent::start() {
    
    if(m_pConfig->help())
        cout << "USAGE: " << PortAgentConfig::Usage() << endl;
        
    else if(m_pConfig->kill())
        kill_process();
    else if(m_pConfig->version())
        displayVersion();
    else {
        return DaemonProcess::start();
    }
    
    return true;
}

/******************************************************************************
 * Method: display_version
 * Description: print the binary version to stdout
 ******************************************************************************/
void PortAgent::displayVersion() {
    cout << PORT_AGENT_VERSION << endl;
}

/******************************************************************************
 * Method: initializeObservatoryDataConnection
 * Description: Initialize the observatory data connection depending upon the
 * configured type.
 ******************************************************************************/
void PortAgent::initializeObservatoryDataConnection() {
    if (m_pConfig->observatoryConnectionType() == OBS_TYPE_STANDARD) {
        initializeObservatoryStandardDataConnection();
    }
    else if (m_pConfig->observatoryConnectionType() == OBS_TYPE_MULTI) {
        initializeObservatoryMultiDataConnection();
    }
     else {
         LOG(ERROR) << "Observatory Connection Type Unknown!";
     }
}

/******************************************************************************
 * Method: initialize
 * Description: Bring the process up from an unknown state to unconfigured.
 ******************************************************************************/
void PortAgent::initializeObservatoryStandardDataConnection() {
    int port;
    ObservatoryConnection *connection = (ObservatoryConnection*)m_pObservatoryConnection;
    TCPCommListener *listener = NULL;
    
    LOG(INFO) << "Initialize observatory data connection";
    
    if(connection) {
        listener = (TCPCommListener *)(connection->dataConnectionObject());
    
        // If we are initialized already ensure the ports match.  If not, reinit
        if(listener) {
            if( listener->getListenPort() != m_pConfig->observatoryDataPort()) 
                listener->disconnect();
        }
    }
    
    // Create the connection object
    if(! m_pObservatoryConnection) {
        m_pObservatoryConnection = new ObservatoryConnection();
        connection = (ObservatoryConnection*)m_pObservatoryConnection;
    }
    
    // Initialize!
    connection->setDataPort(m_pConfig->observatoryDataPort());
    
    if (!connection->dataInitialized())
        connection->initializeDataSocket();
    else 
        LOG(DEBUG) << " - already initialized, all done";
}

/******************************************************************************
 * Method: initializeObservatoryMultiDataConnection
 * Description: Initialize an observatory connection that has multiple data
 * sockets.
 ******************************************************************************/
void PortAgent::initializeObservatoryMultiDataConnection() {
    int port;
    ObservatoryMultiConnection *pConnection = 0;

    pConnection = static_cast<ObservatoryMultiConnection*>(m_pObservatoryConnection);

    LOG(INFO) << "Initialize observatory data connection";

    if (!m_pObservatoryConnection) {
        m_pObservatoryConnection = new ObservatoryMultiConnection();
        pConnection = (ObservatoryMultiConnection*) m_pObservatoryConnection;
    }

    // Iterate through the configured data ports and
    // add TCPCommListener objects for each port
    port = ObservatoryDataPorts::instance()->getFirstPort();
    while (port) {
        LOG(DEBUG) << "initializeObservatoryMultiDataConnection: adding listener for port: " << port;
        pConnection->addListener(port);

        port = ObservatoryDataPorts::instance()->getNextPort();
    }

    if (!pConnection->isDataInitialized()) {
        pConnection->initializeDataSocket();
    }
    else
        LOG(DEBUG) << " - already initialized, all done";
}

/******************************************************************************
 * Method: initialize
 * Description: Bring the process up from an unknown state to unconfigured.
 ******************************************************************************/
void PortAgent::initializeObservatoryCommandConnection() {
    int port;
    //ObservatoryConnection* pConnection = (ObservatoryConnection*) m_pObservatoryConnection;
    TCPCommListener *listener = NULL;

    
    //if (pConnection) {
    if (m_pObservatoryConnection) {
        //listener = (TCPCommListener *)(pConnection->commandConnectionObject());
        listener = (TCPCommListener *)(m_pObservatoryConnection->commandConnectionObject());
    
        // If we are initialized already ensure the ports match.  If not, reinit
        if(listener) {
            if( listener->getListenPort() != m_pConfig->observatoryCommandPort()) 
                listener->disconnect();
        }

        // If the configured connection type is not the same as the existing, delete
        // the object because we need to instantiate a new one.
        PortAgentConnectionType connectionType = m_pObservatoryConnection->connectionType();
        if ((OBS_TYPE_STANDARD == m_pConfig->observatoryConnectionType() &&
                PACONN_OBSERVATORY_STANDARD != connectionType) ||
            (OBS_TYPE_MULTI == m_pConfig->observatoryConnectionType() &&
                    PACONN_OBSERVATORY_MULTI != connectionType)) {
            LOG(DEBUG) << "Observatory connection type changed: deleting existing connection object.";
            delete m_pObservatoryConnection;
            m_pObservatoryConnection = 0;
        }
    }

    // Create the connection object
    if (!m_pObservatoryConnection) {
        if (m_pConfig->observatoryConnectionType() == OBS_TYPE_STANDARD) {
            LOG(DEBUG2) << "creating new observatory standard connection object";
            m_pObservatoryConnection = new ObservatoryConnection();
            ObservatoryConnection* pConnection = (ObservatoryConnection*) m_pObservatoryConnection;
            pConnection->setCommandPort(m_pConfig->observatoryCommandPort());

            if (!pConnection->commandInitialized())
                m_pObservatoryConnection->initializeCommandSocket();
        }
        else if (m_pConfig->observatoryConnectionType() == OBS_TYPE_MULTI) {
            LOG(DEBUG2) << "creating new observatory multi connection object";
            m_pObservatoryConnection = new ObservatoryMultiConnection();
            ObservatoryMultiConnection* pConnection = (ObservatoryMultiConnection*) m_pObservatoryConnection;
            pConnection->setCommandPort(m_pConfig->observatoryCommandPort());

            if (!pConnection->commandInitialized())
                m_pObservatoryConnection->initializeCommandSocket();
        }
        else {
            LOG(ERROR) << "initializeObservatoryCommandConnection: Configured observatory type unknown!";
        }
    }
    
}

/******************************************************************************
 * Method: initializeInstrumentConnection
 * Description: Attempt to connect to the instrument.  This class will attempt
 * to connect to the data port and command port if needed.  If both are not
 * successful then we enter a disconnected state, otherwise we transition to
 * connected.
 *
 * Currently the only connection type we are supporting is TCP.  Eventually
 * this method will need to support more types.
 ******************************************************************************/
void PortAgent::initializeInstrumentConnection() {
    if (m_pConfig->instrumentConnectionType() == TYPE_TCP) {
        initializeTCPInstrumentConnection();
    }
    else if (m_pConfig->instrumentConnectionType() == TYPE_BOTPT) {
        initialize_BOTPT_InstrumentConnection();
    }
    else if (m_pConfig->instrumentConnectionType() == TYPE_SERIAL) {
        initializeSerialInstrumentConnection();
    }
    else {
        LOG(ERROR) << "Instrument connection type not recognized.";
   }

}

/******************************************************************************
 * Method: initializeTCPInstrumentConnection
 * Description: Connect to a TCP type instrument.  There is no command port
 * for this bad boy.
 *
 * State Transitions:
 *  Connected - if we can connect to an instrument
 *  Disconnected - if we fail to connect to an instrument
 ******************************************************************************/
void PortAgent::initializeTCPInstrumentConnection() {
    InstrumentTCPConnection *connection = (InstrumentTCPConnection *)m_pInstrumentConnection;
    
    // Clear if we have already initialized the wrong type
    if(connection && connection->connectionType() != PACONN_INSTRUMENT_TCP) {
        LOG(INFO) << "Detected connection type change.  rebuilding connection.";
        delete connection;
        connection = NULL;
    }
    
    // Create the connection object
    if(!connection)
        m_pInstrumentConnection = connection = new InstrumentTCPConnection();

    // If we have changed out configuration the set the new values and try to connect
    if (connection->dataHost() != m_pConfig->instrumentAddr() ||
       connection->dataPort() != m_pConfig->instrumentDataPort() ) {
        LOG(INFO) << "Detected connection configuration change.  reconfiguring.";

        connection->disconnect();

        connection->setDataHost(m_pConfig->instrumentAddr());
        connection->setDataPort(m_pConfig->instrumentDataPort());
    }

    if (!connection->connected()) {
        LOG(DEBUG) << "Instrument not connected, attempting to reconnect";
        LOG(DEBUG2) << "host: " << connection->dataHost() << " port: " << connection->dataPort();

        setState(STATE_DISCONNECTED);

        try {
            connection->initialize();
        }
        catch(SocketConnectFailure &e) {
            connection->disconnect();
            string msg = e.what();
            LOG(ERROR) << msg;
        };

        // Let everything connect
        sleep(SELECT_SLEEP_TIME);
    }


    if(connection->connected())
        setState(STATE_CONNECTED);
}

/******************************************************************************
 * Method: initialize_BOTPT_InstrumentConnection
 * Description: Connect to a BOTPT TCP type instrument with  two connections (one
 * for send and one for receive).  There is no command port for this bad boy.
 *
 * State Transitions:
 *  Connected - if we can connect to an instrument
 *  Disconnected - if we fail to connect to an instrument
 ******************************************************************************/
void PortAgent::initialize_BOTPT_InstrumentConnection() {
    InstrumentBOTPTConnection *connection = (InstrumentBOTPTConnection *)m_pInstrumentConnection;

    // Clear if we have already initialized the wrong type
    if(connection && connection->connectionType() != PACONN_INSTRUMENT_BOTPT) {
        LOG(INFO) << "Detected connection type change.  rebuilding connection.";
        delete connection;
        connection = NULL;
    }

    // Create the connection object
    // Here we need to possibly initialize two connections, one send and one
    // receive.
    if (!connection)
        m_pInstrumentConnection = connection = new InstrumentBOTPTConnection();

    // If we have changed out configuration the set the new values and try to connect
    if (connection->dataHost() != m_pConfig->instrumentAddr() ||
       connection->dataTxPort() != m_pConfig->instrumentDataTxPort() ||
       connection->dataRxPort() != m_pConfig->instrumentDataRxPort() ) {
        LOG(INFO) << "Detected connection configuration change.  reconfiguring.";
        
        connection->disconnect();
        
        connection->setDataHost(m_pConfig->instrumentAddr());
        connection->setDataTxPort(m_pConfig->instrumentDataTxPort());
        connection->setDataRxPort(m_pConfig->instrumentDataRxPort());
    }
    
    if (!connection->connected()) {
        LOG(DEBUG) << "Instrument not connected, attempting to reconnect";
        LOG(DEBUG2) << "host: " << connection->dataHost() << " port: " << connection->dataTxPort();
        LOG(DEBUG2) << "host: " << connection->dataHost() << " port: " << connection->dataRxPort();
        
        setState(STATE_DISCONNECTED);
        
        try {
            connection->initialize();
        }
        catch(SocketConnectFailure &e) {
            connection->disconnect();
            string msg = e.what();
            LOG(ERROR) << msg;
        };
        
        // Let everything connect
        sleep(SELECT_SLEEP_TIME);
    }
    
    
    if(connection->connected())
        setState(STATE_CONNECTED);
}

/******************************************************************************
 * Method: initializeSerialInstrumentConnection
 * Description: Connect to a Serial type instrument.
 *
 * State Transitions:
 *  Connected - if we can connect to an instrument
 *  Disconnected - if we fail to connect to an instrument
 ******************************************************************************/
void PortAgent::initializeSerialInstrumentConnection() {
    InstrumentSerialConnection *connection = (InstrumentSerialConnection *) m_pInstrumentConnection;

    // Clear if we have already initialized the wrong type
    if (connection && connection->connectionType() != PACONN_INSTRUMENT_SERIAL) {
        LOG(INFO) << "Detected connection type change.  rebuilding connection.";
        delete connection;
        connection = NULL;
    }

    // Create the connection object
    if (!connection) {
        m_pInstrumentConnection = connection = new InstrumentSerialConnection();
        connection->setDevicePath(m_pConfig->devicePath());
    }

    if (m_pConfig->devicePathChanged() || !connection->connected()) {
        LOG(INFO) << "Detected device path change or not opened.  closing and reopening.";
        m_pInstrumentConnection->initialize();
        m_pConfig->clearDevicePathChanged();

        // If the devicePath has changed, we need to initialize the serial settings
        // regardless of whether they have changed.
        if (initializeSerialSettings()) {
            m_pConfig->clearSerialSettingsChanged();  // clear so we don't re-initialize
        }
    }

    // If any of the serial settings have changed, reinitialize them.
    if (m_pConfig->serialSettingsChanged() && connection->connected()) {
        LOG(INFO) << "Detected connection configuration change.  reconfiguring.";
        initializeSerialSettings();
        m_pConfig->clearSerialSettingsChanged();
    }

    if (connection->connected()) {
        setState(STATE_CONNECTED);
    }
    else {
        setState(STATE_DISCONNECTED);
    }
}


/******************************************************************************
 * Method: initializeSerialSettings
 * Description: initialize serial settings; can be done independently of opening
 * the device driver (i.e., we can change serial settings without closing the
 * device driver and re-opening).
 ******************************************************************************/
bool PortAgent::initializeSerialSettings() {
    InstrumentSerialConnection *pConnection = (InstrumentSerialConnection *) m_pInstrumentConnection;

    pConnection->setBaud(m_pConfig->baud());
    pConnection->setFlowControl(m_pConfig->flow());
    pConnection->setStopBits(m_pConfig->stopbits());
    pConnection->setDataBits(m_pConfig->databits());
    pConnection->setParity(m_pConfig->parity());
    return pConnection->initializeSerialSettings();

}

/******************************************************************************
 * Method: initializePulishers
 * Description: setup all publishers
 ******************************************************************************/
void PortAgent::initializePublishers() {
    LOG(INFO) << "Initialize Publishers";
    initializePublisherFile();    
    initializePublisherObservatoryData();    
    initializePublisherObservatoryCommand();    
    initializePublisherInstrumentData();    
    initializePublisherInstrumentCommand();    
    initializePublisherTCP();    
    initializePublisherUDP();    
    initializePublisherTelnetSniffer();    
}

/******************************************************************************
 * Method: initializePulisherFile
 * Description: setup the file publisher
 ******************************************************************************/
void PortAgent::initializePublisherFile() {
    LOG(INFO) << "Initialize File Publisher";
    
    if(!m_pConfig || !m_pConfig->datafile().length()) {
        LOG(ERROR) << "PA not configured, not initializing datalog";
        return;
    }
    
    LOG(DEBUG) << "Setup data log initial file: " << m_pConfig->datafile();
    
    LogPublisher publisher;
    publisher.setFilebase(m_pConfig->datafile(), "data");
    publisher.setAsciiMode(false);
    
    m_oPublishers.add(&publisher);
}

/******************************************************************************
 * Method: initializePublisherObservatoryData
 * Description: Depending upon the observatory connection type, setup the
 * observatory data publisher(s)
 ******************************************************************************/
void PortAgent::initializePublisherObservatoryData() {
    CommBase *connection;
    
    LOG(INFO) << "Initialize Observatory Data Publisher";
    if( ! m_pObservatoryConnection ) {
        LOG(ERROR) << "Observatory connection does not exist. " 
                   << "Not setting up data publisher!";
        return;
    }
    
    if (m_pObservatoryConnection->connectionType() == PACONN_OBSERVATORY_STANDARD) {
        initializePublisherObservatoryStandardData();
    }
    else if (m_pObservatoryConnection->connectionType() == PACONN_OBSERVATORY_MULTI) {
        initializePublisherObservatoryMultiData();
    }
    else {
        LOG(ERROR) << "initializePublisherObservatoryData: observatory connection type unknown!";
    }
}

/******************************************************************************
 * Method: initializePublisherObservatoryStandardData
 * Description: setup the observatory data publisher
 ******************************************************************************/
void PortAgent::initializePublisherObservatoryStandardData() {
    CommBase *connection;

    LOG(INFO) << "Initialize Observatory Standard Data Publisher";
    if( ! m_pObservatoryConnection ) {
        LOG(ERROR) << "Observatory connection does not exist. "
                   << "Not setting up data publisher!";
        return;
    }

    connection = m_pObservatoryConnection->dataConnectionObject();
    if( ! connection ) {
        LOG(INFO) << "Observatory data connection not set. "
                   << "Not setting up data publisher!";
        return;
    }

    LOG(DEBUG) << "Create new publisher";
    DriverDataPublisher publisher(connection);

    m_oPublishers.add(&publisher);
}

/******************************************************************************
 * Method: initializePublisherObservatoryMultiData
 * Description: setup the observatory data publisher
 ******************************************************************************/
void PortAgent::initializePublisherObservatoryMultiData() {
    CommBase *pConnection = 0;
    //TCPCommListener* pListener = 0;

    LOG(INFO) << "Initialize Observatory Multi Data Publisher";
    if( ! m_pObservatoryConnection ) {
        LOG(ERROR) << "Observatory connection does not exist. "
                   << "Not setting up data publisher!";
        return;
    }

    // Iterate through the listeners adding the clientFDs
    pConnection = ObservatoryDataSockets::instance()->getFirstSocket();
    while (pConnection) {
        LOG(DEBUG) << "Create new publisher";
        DriverDataPublisher publisher(pConnection);
        m_oPublishers.add(&publisher);

        pConnection = ObservatoryDataSockets::instance()->getNextSocket();
    }
}

/******************************************************************************
 * Method: initializePublisherObservatoryCommand
 * Description: setup the observatory command publisher
 ******************************************************************************/
void PortAgent::initializePublisherObservatoryCommand() {
    CommBase *connection;
    
    LOG(INFO) << "Initialize Observatory Command Publisher";
    if( ! m_pObservatoryConnection ) {
        LOG(ERROR) << "Observatory connection does not exist. " 
                   << "Not setting up command publisher!";
        return;
    }
    
    connection = m_pObservatoryConnection->commandConnectionObject();
    if( ! connection ) {
        LOG(INFO) << "Observatory command connection not set. " 
                   << "Not setting up command publisher!";
        return;
    }
    
    LOG(DEBUG) << "Create new publisher";
    DriverCommandPublisher publisher(connection);
    
    m_oPublishers.add(&publisher);
}

/******************************************************************************
 * Method: initializePublisherInstrumentData
 * Description: setup the instrument data publisher
 ******************************************************************************/
void PortAgent::initializePublisherInstrumentData() {
    CommBase *connection;
    
    LOG(INFO) << "Initialize Instrument Data Publisher";
    if( ! m_pInstrumentConnection ) {
        LOG(ERROR) << "Instrument connection does not exist. "
                   << "Not setting up data publisher!";
        return;
    }
    
    if (m_pInstrumentConnection->connectionType() == PACONN_INSTRUMENT_BOTPT) {
        connection = ((InstrumentBOTPTConnection*) m_pInstrumentConnection)->dataTxConnectionObject();
    }
    else {
        connection = m_pInstrumentConnection->dataConnectionObject();
    }

    if( ! connection ) {
        LOG(INFO) << "Instrument data connection not set. " 
                   << "Not setting up data publisher!";
        return;
    }
    
    LOG(DEBUG) << "Create new publisher";
    InstrumentDataPublisher publisher(connection);
    
    m_oPublishers.add(&publisher);
}

/******************************************************************************
 * Method: initializePublisherInstrumentCommand
 * Description: setup the instrument command publisher
 ******************************************************************************/
void PortAgent::initializePublisherInstrumentCommand() {
    CommBase *connection;
    
    LOG(INFO) << "Initialize Instrument Command Publisher";
    if( ! m_pInstrumentConnection ) {
        LOG(ERROR) << "Instrument connection does not exist. "
                   << "Not setting up command publisher!";
        return;
    }
    
    connection = m_pInstrumentConnection->commandConnectionObject();
    if( ! connection ) {
        LOG(INFO) << "Instrument command connection not set. " 
                   << "Not setting up command publisher!";
        return;
    }
    
    LOG(DEBUG) << "Create new publisher";
    InstrumentCommandPublisher publisher(connection);
    
    m_oPublishers.add(&publisher);
}

/******************************************************************************
 * Method: initializePublisherTelnetSniffer
 * Description: setup the telnet sniffer publisher
 ******************************************************************************/
void PortAgent::initializePublisherTelnetSniffer() {
    LOG(INFO) << "Initialize Telnet Sniffer Publisher";
    
    int port = m_pConfig->telnetSnifferPort();
    if(port <= 0) {
        LOG(INFO) << "telnet sniffer not configured.  Not starting.";
        return;
    }
    
    LOG(DEBUG) << "Establish TCP Listener for Telnet Sniffer";
    if(m_pTelnetSnifferConnection)
        delete m_pTelnetSnifferConnection;
    
    m_pTelnetSnifferConnection = new TCPCommListener();
    m_pTelnetSnifferConnection->setPort(port);
    
    try {
        m_pTelnetSnifferConnection->initialize();
    }
    catch(SocketConnectFailure &e) {
        if(m_pTelnetSnifferConnection)
            delete m_pTelnetSnifferConnection;
        m_pTelnetSnifferConnection = NULL;
        LOG(ERROR) << "Failed to estabilsh telnet sniffer: ";
        return;
    };
    
    TelnetSnifferPublisher publisher(m_pTelnetSnifferConnection);
    
    if(m_pConfig->telnetSnifferPrefix().length())
        publisher.setPrefix(m_pConfig->telnetSnifferPrefix());
    
    if(m_pConfig->telnetSnifferSuffix().length())
        publisher.setSuffix(m_pConfig->telnetSnifferSuffix());
    
    m_oPublishers.add(&publisher);
}

/******************************************************************************
 * Method: initializePublisherTCP
 * Description: setup the tcp publisher
 ******************************************************************************/
void PortAgent::initializePublisherTCP() {
    LOG(INFO) << "Initialize TCP Publisher";
}

/******************************************************************************
 * Method: initializePublisherUDP
 * Description: setup the udp publisher
 ******************************************************************************/
void PortAgent::initializePublisherUDP() {
    LOG(INFO) << "Initialize UDP Publisher";
}


/******************************************************************************
 * Method: handlePortAgentCommand
 * Description: This method is called outside of the normal packet publishing
 * process to affect change on the port agent from port agnet commands passed
 * in via the observatory command port.
 *
 * This method can accept multiple commands at once delimeted by newlines
 * Parameter:
 *   commands - newline delimeted string of port agent commands.
 ******************************************************************************/
void PortAgent::handlePortAgentCommand(const char * commands) {
    PortAgentCommand cmd;
    LOG(DEBUG2) << "COMMAND DATA: " << commands;
    
    if(!m_pConfig)
        return;
    
    // Clear the command buffer
    while(cmd = m_pConfig->getCommand()) {
    }
    
    m_pConfig->parse(commands);
    
    processPortAgentCommands();
    // TODO: Add code for commands. i.e. Configuration Update, shutdown, etc...

    // TODO: This messes things up; when a command like BREAK comes in, we don't
    // want to go to STATE_CONFIGURED.
    //if(m_pConfig->isConfigured())
    //    setState(STATE_CONFIGURED);
}

/******************************************************************************
 * Method: processPortAgentCommands
 * Description: Loop through all the queued commands in the port agent
 * configuration object and handle each command.
 ******************************************************************************/
void PortAgent::processPortAgentCommands() {
    PortAgentCommand cmd;
    ostringstream msg;

    while(cmd = m_pConfig->getCommand()) {
        switch (cmd) {
            case CMD_COMM_CONFIG_UPDATE:
                LOG(DEBUG) << "communication config update command";
                setState(STATE_UNCONFIGURED);
                break;
            case CMD_PUBLISHER_CONFIG_UPDATE:
                LOG(DEBUG) << "publisher config update command";
                break;
            case CMD_PATH_CONFIG_UPDATE:
                LOG(DEBUG) << "path config update command";
                break;
            case CMD_SAVE_CONFIG:
                LOG(DEBUG) << "same config command";
                publishFault("not implemented");
                break;
            case CMD_GET_CONFIG:
                LOG(DEBUG) << "get config command";
                publishFault("not implemented");
                break;
            case CMD_GET_STATE:
                LOG(DEBUG) << "get state command";
                publishStatus(getCurrentStateAsString());
                break;
            case CMD_PING:
                msg << "pong. version: " << PORT_AGENT_VERSION;
                LOG(DEBUG) << "ping command. logger version: " << PORT_AGENT_VERSION;
                publishStatus(msg.str());
                break;
            case CMD_BREAK:
                LOG(DEBUG) << "break command";
                m_pInstrumentConnection->sendBreak(m_pConfig->breakDuration());
                break;
            case CMD_ROTATION_INTERVAL:
                LOG(DEBUG) << "set rotation interval";
                setRotationInterval();
                break;
            case CMD_SHUTDOWN:
                LOG(DEBUG) << "shutdown command";
                shutdown();
                break;
        };
    }
}


/******************************************************************************
 * Method: handleTCPConnect
 * Description: handler for processing a client trying to connect to a TCP
 * listener.
 * Parameter:
 *   listener - TCP listener object for managing the tcp connection.
 ******************************************************************************/
void PortAgent::handleTCPConnect(TCPCommListener &listener) {
    listener.acceptClient();
    LOG(DEBUG) << "new client FD: " << listener.clientFD();
    
    if(! listener.connected()) {
        throw SocketConnectFailure("tcp client connect");
    }
}

/******************************************************************************
 * Method: handleStateUnconfigured
 * Description: handler for the unconfigured state
 ******************************************************************************/
void PortAgent::handleStateUnconfigured(const fd_set &readFDs) {
    LOG(DEBUG) << "start state unconfigured handler";
    
    handleObservatoryCommandAccept(readFDs);
    handleObservatoryCommandRead(readFDs);
    handleObservatoryDataRead(readFDs);
    
    if(m_pConfig->isConfigured())
        setState(STATE_CONFIGURED);
}

/******************************************************************************
 * Method: handleStateConfigured
 * Description: handler for the configured state.  All we need to do in this
 * state is either go into connected (if we can connect to the instrument) or
 * disconnected.
 ******************************************************************************/
void PortAgent::handleStateConfigured(const fd_set &readFDs) {
    LOG(DEBUG) << "start state configured handler";
    
    initializeObservatoryCommandConnection();
    initializeObservatoryDataConnection();
    initializeInstrumentConnection();
    initializePublishers();
}

/******************************************************************************
 * Method: handleStateConnected
 * Description: handler for the connected state
 ******************************************************************************/
void PortAgent::handleStateConnected(const fd_set &readFDs) {
    LOG(DEBUG) << "start state connected handler";
    
    // Accept any new connections.
    handleObservatoryCommandAccept(readFDs);
    handleObservatoryDataAccept(readFDs);
    
    // Read data
    handleObservatoryCommandRead(readFDs);
    handleObservatoryDataRead(readFDs);
    handleInstrumentDataRead(readFDs);
}

/******************************************************************************
 * Method: handleStateDisconnected
 * Description: handler for the disconnected state
 ******************************************************************************/
void PortAgent::handleStateDisconnected(const fd_set &readFDs) {
    LOG(DEBUG) << "start state disconnected handler";
    
    // Accept any new connections.
    handleObservatoryCommandAccept(readFDs);
    handleObservatoryDataAccept(readFDs);
    
    // Read data
    handleObservatoryCommandRead(readFDs);
    handleObservatoryDataRead(readFDs);
    handleInstrumentDataRead(readFDs);
}

/******************************************************************************
 * Method: handleStateUnknown
 * Description: handler for the unconfigured state.  We should never get here
 * so just EXPLODE!
 ******************************************************************************/
void PortAgent::handleStateUnknown() {
    LOG(DEBUG) << "start state unknown handler";
    throw UnknownState();
}


/******************************************************************************
 * Method: handleStateStartup
 * Description: handler for the startup state.  Initialize the command
 * connection.
 ******************************************************************************/
void PortAgent::handleStateStartup() {
    // Setup logging
    Logger::SetLogFile(m_pConfig->logfile());
        
    LOG(DEBUG) << "start up state handler";
    
    initializeObservatoryCommandConnection();
    setState(STATE_UNCONFIGURED);
}

/******************************************************************************
 * Method: handleCommon
 * Description: common tasks to be handled regardless of state.
 ******************************************************************************/
void PortAgent::handleCommon(const fd_set &readFDs) {
    handleTelnetSnifferAccept(readFDs);
    handleTelnetSnifferRead(readFDs);
}

/******************************************************************************
 * Method: poll
 * Description: main program loop.  Looping structure is in base class
 ******************************************************************************/
void PortAgent::poll() {
    fd_set readFDs;
    struct timeval tv;
    int readyCount;
    int maxFD = buildFDSet(readFDs);
    
    tv.tv_sec = SELECT_SLEEP_TIME;
    tv.tv_usec = 0;
    
    // Main select to see if any incoming pipes have data.
    LOG(DEBUG) << "Start select process";
    readyCount = select(maxFD+1, &readFDs, NULL, NULL, &tv);
    if(readyCount < 0) {
        if (errno != EINTR) 
            LOG(ERROR) << "Socket select error: " << strerror(errno);
        else
            LOG(DEBUG) << "Socket select error: " << strerror(errno) << " IGNORED";
        
        return;
    }

    LOG(DEBUG) << "On select: ready to read on " << readyCount << " connections";
    
    LOG(DEBUG) << "Port Agent Version: " << PORT_AGENT_VERSION;
    LOG(DEBUG) << "CURRENT STATE: " << getCurrentStateAsString();
    
    try {
        // We don't use else if here so that the work in one state handler
        // can change the state can call a subsiquent handler without having
        // to iterate.
        if(getCurrentState() == STATE_UNCONFIGURED)
            handleStateUnconfigured(readFDs);
        
        if(getCurrentState() == STATE_CONFIGURED)
            handleStateConfigured(readFDs);
        
        if(getCurrentState() == STATE_CONNECTED)
            handleStateConnected(readFDs);
        
        if(getCurrentState() == STATE_DISCONNECTED)
            handleStateDisconnected(readFDs);
        
        if(getCurrentState() == STATE_STARTUP)
            handleStateStartup();
        
        if(getCurrentState() == STATE_UNKNOWN)
            handleStateUnknown();
            
        handleCommon(readFDs);
            
        publishHeartbeat();
    }
    catch(UnknownState &e) {
        //re-throw the exception
        throw e;
    }
    catch(OOIException &e) {
        string msg = e.what();
        LOG(ERROR) << msg;
        // TODO: publish fault packet
    }
}

/******************************************************************************
 * Method: buildFDSet
 * Description: Build a fd_set of file descriptors of all of our read lines and
 * return the max file descriptor for use in a select statement.
 *
 * We need to read from several descriptors:
 *  * Observatory Command Connection (Listener)
 *  * Observatory Connection Connection (Client)
 *  * Observatory Data Connection (Listener)
 *  * Observatory Data Connection (Client)
 *  * Instrument Data Connection (Client)
 *  * Telnet Sniffer Connection (Listener)
 * 
 * Return:
 *  the maximum file descriptor value.
 *  readFDs populated with all current FDs available for reading
 ******************************************************************************/
int PortAgent::buildFDSet(fd_set &readFDs) {
    int maxFD = 0;
    
    FD_ZERO(&readFDs);
    
    addObservatoryCommandListenerFD(maxFD, readFDs);
    addObservatoryCommandClientFD(maxFD, readFDs);
    addObservatoryDataListenerFD(maxFD, readFDs);
    addObservatoryDataClientFD(maxFD, readFDs);
    addInstrumentDataClientFD(maxFD, readFDs);
    addTelnetSnifferListenerFD(maxFD, readFDs);
    addTelnetSnifferClientFD(maxFD, readFDs);
    
    return maxFD;
}

/******************************************************************************
 * Method: addTelnetSnifferListenerFD
 * Description: Add the telnet sniffer fd to the fd_set.  Also update
 * the max file descriptor.
 *
 * If the connection isn't initialized then do nothing.
 ******************************************************************************/
void PortAgent::addTelnetSnifferListenerFD(int &maxFD, fd_set &readFDs) {
    
    if(m_pTelnetSnifferConnection) {
        int fd = 0;
    
        if(m_pTelnetSnifferConnection->listening())
            fd = getTelnetSnifferListenerFD();
    
        if(m_pTelnetSnifferConnection->listening() && fd) {
            LOG(DEBUG2) << "add telnet sniffer listener FD";
            maxFD = fd > maxFD ? fd : maxFD;
            FD_SET(fd, &readFDs);
        }
        else {
            LOG(DEBUG) << "telnet sniffer not initialized";
        }
    }
}

/******************************************************************************
 * Method: addTelnetSnifferClientFD
 * Description: Add the sniffer client fd to the fd_set.  Also update
 * the max file descriptor.
 *
 * If the connection isn't initialized then do nothing.
 ******************************************************************************/
void PortAgent::addTelnetSnifferClientFD(int &maxFD, fd_set &readFDs) {
    if(m_pTelnetSnifferConnection) {
        int fd = m_pTelnetSnifferConnection->clientFD();
        
        if(fd) {
            LOG(DEBUG) << "add telnet sniffer client FD";
            maxFD = fd > maxFD ? fd : maxFD;
            FD_SET(fd, &readFDs);
        }
        else {
            LOG(DEBUG) << "telnet sniffer client not initialized";
        }    
    }
}

/******************************************************************************
 * Method: addObservatoryCommandListenerFD
 * Description: Add the observatory connection fd to the fd_set.  Also update
 * the max file descriptor.
 *
 * If the connection isn't initialized then do nothing.
 ******************************************************************************/
void PortAgent::addObservatoryCommandListenerFD(int &maxFD, fd_set &readFDs) {
    CommBase *pConnection;
    
    if(m_pObservatoryConnection) {
        pConnection = m_pObservatoryConnection->commandConnectionObject();
        int fd = 0;
    
        if(m_pObservatoryConnection->commandInitialized())
            fd = getObservatoryCommandListenerFD();
    
        if(m_pObservatoryConnection->commandInitialized() && fd) {
            LOG(DEBUG2) << "add observatory command listener FD";
            maxFD = fd > maxFD ? fd : maxFD;
            FD_SET(fd, &readFDs);
        }
        else {
            LOG(DEBUG2) << "Observatory command listener not initialized";
        }
    }
}

/******************************************************************************
 * Method: addObservatoryCommandClientFD
 * Description: Add the observatory connection fd to the fd_set.  Also update
 * the max file descriptor.
 *
 * If the connection isn't initialized then do nothing.
 ******************************************************************************/
void PortAgent::addObservatoryCommandClientFD(int &maxFD, fd_set &readFDs) {
    CommBase *pConnection;
    
    if(m_pObservatoryConnection) {
        pConnection = m_pObservatoryConnection->commandConnectionObject();
        int fd = 0;
        
        if(m_pObservatoryConnection->commandConnected())
            fd = getObservatoryCommandClientFD();
        
        if(m_pObservatoryConnection->commandConnected() && fd) {
            LOG(DEBUG2) << "add observatory command client FD";
            maxFD = fd > maxFD ? fd : maxFD;
            FD_SET(fd, &readFDs);
        }
        else {
            LOG(DEBUG2) << "Observatory data client not initialized";
        }    
    }
}

/******************************************************************************
 * Method: addObservatoryDataListenerFD
 * Description: Depending upon the type of observatory connection, invoke the
 * appropriate method to add the FD(s) to the select fd set.
 *
 ******************************************************************************/
void PortAgent::addObservatoryDataListenerFD(int &maxFD, fd_set &readFDs) {
    PortAgentConnectionType connectionType;

    if (m_pObservatoryConnection) {
        connectionType = m_pObservatoryConnection->connectionType();

        if (PACONN_OBSERVATORY_STANDARD == connectionType) {
            addObservatoryStandardDataListenerFD(maxFD, readFDs);
        }
        else if (PACONN_OBSERVATORY_MULTI == connectionType) {
            addObservatoryMultiDataListenerFDs(maxFD, readFDs);
        }
        else {
            LOG(ERROR) << "PortAgent::addObservatoryDataListenerFD: unknown observatory type: " << connectionType;
        }
    }
}

/******************************************************************************
 * Method: addObservatoryStandardDataListenerFD
 * Description: Add the observatory data fd to the fd_set.  Also update
 * the max file descriptor.
 *
 * If the connection isn't initialized then do nothing.
 ******************************************************************************/
void PortAgent::addObservatoryStandardDataListenerFD(int &maxFD, fd_set &readFDs) {
    CommBase *pConnection;
    
    if (m_pObservatoryConnection) {
        pConnection = m_pObservatoryConnection->dataConnectionObject();
        int fd = 0;
    
        if(m_pObservatoryConnection->dataInitialized())
            fd = getObservatoryDataListenerFD();
    
        if(m_pObservatoryConnection->dataInitialized() && fd) {
            LOG(DEBUG2) << "add observatory data listener FD";
            maxFD = fd > maxFD ? fd : maxFD;
            FD_SET(fd, &readFDs);
        }
        else {
            LOG(DEBUG2) << "Observatory data listener not initialized";
        }
    }
}

/******************************************************************************
 * Method: addObservatoryMultiDataListenerFDs
 * Description: Iterate through the listeners, adding their fds to the fd_set.
 * Also update the max file descriptor.
 *
 * If the connection isn't initialized then do nothing.
 ******************************************************************************/
void PortAgent::addObservatoryMultiDataListenerFDs(int &maxFD, fd_set &readFDs) {
    TCPCommListener* pListener = 0;

    if (m_pObservatoryConnection) {
        pListener = ObservatoryDataSockets::instance()->getFirstSocket();
        while (pListener) {
            if (pListener->listening()) {
                int fd = pListener->serverFD();
                if (fd) {
                    LOG(DEBUG2) << "adding observatory multi data listener FD: " << fd;
                    maxFD = fd > maxFD ? fd : maxFD;
                    FD_SET(fd, &readFDs);
                }
            }
            pListener = ObservatoryDataSockets::instance()->getNextSocket();
        }
    }
}

/******************************************************************************
 * Method: addObservatoryDataClientFD
 * Description: Depending upon the observatory connection type, add the data
 * FD(s) to the select fd_set.
 *
 * If the connection isn't initialized then do nothing.
 ******************************************************************************/
void PortAgent::addObservatoryDataClientFD(int &maxFD, fd_set &readFDs) {
    PortAgentConnectionType connectionType;

    if (m_pObservatoryConnection) {
        connectionType = m_pObservatoryConnection->connectionType();

        if (PACONN_OBSERVATORY_STANDARD == connectionType) {
            addObservatoryStandardDataClientFD(maxFD, readFDs);
        }
        else if (PACONN_OBSERVATORY_MULTI == connectionType) {
            addObservatoryMultiDataClientFDs(maxFD, readFDs);
        }
        else {
            LOG(ERROR) << "PortAgent::addObservatoryDataClientFD: unknown observatory type: " << connectionType;
        }
    }
}

/******************************************************************************
 * Method: addObservatoryStandardDataClientFD
 * Description: Add the observatory data fd to the fd_set.  Also update
 * the max file descriptor.
 *
 * If the connection isn't initialized then do nothing.
 ******************************************************************************/
void PortAgent::addObservatoryStandardDataClientFD(int &maxFD, fd_set &readFDs) {
    CommBase *pConnection;

    if(m_pObservatoryConnection) {
        pConnection = m_pObservatoryConnection->dataConnectionObject();
        int fd = 0;

        fd = getObservatoryDataClientFD();

        if(fd) {
            LOG(DEBUG2) << "add observatory data client FD";
            maxFD = fd > maxFD ? fd : maxFD;
            FD_SET(fd, &readFDs);
        }
        else {
            LOG(DEBUG2) << "Observatory data client not initialized";
        }
    }
}

/******************************************************************************
 * Method: addObservatoryMultiDataClientFD
 * Description: Iterate through the connections, adding their fds to the fd_set.
 * Also update the max file descriptor.
 *
 * If the connection isn't initialized then do nothing.
 ******************************************************************************/
void PortAgent::addObservatoryMultiDataClientFDs(int &maxFD, fd_set &readFDs) {
    TCPCommListener* pListener = 0;
    
    if (m_pObservatoryConnection) {
        pListener = ObservatoryDataSockets::instance()->getFirstSocket();
        while (pListener) {
            if (pListener->connected()) {
                int fd = pListener->clientFD();
                if (fd) {
                    LOG(DEBUG2) << "adding observatory multi data client FD: " << fd;
                    maxFD = fd > maxFD ? fd : maxFD;
                    FD_SET(fd, &readFDs);
                }
            }
            pListener = ObservatoryDataSockets::instance()->getNextSocket();
        }
    }
}

/******************************************************************************
 * Method: addInstrumentDataClientFD
 * Description: Add the instrument client fd to the fd_set.  Also update
 * the max file descriptor.
 *
 * If the connection isn't initialized then do nothing.
 ******************************************************************************/
void PortAgent::addInstrumentDataClientFD(int &maxFD, fd_set &readFDs) {
    CommBase *pConnection;
    

    if(m_pInstrumentConnection) {
        if (m_pInstrumentConnection->connectionType() == PACONN_INSTRUMENT_BOTPT) {
            pConnection = ((InstrumentBOTPTConnection*) m_pInstrumentConnection)->dataTxConnectionObject();
        }
        else {
            pConnection = m_pInstrumentConnection->dataConnectionObject();
        }
        int fd = 0;
        
        fd = getInstrumentDataRxClientFD();
        
        if (fd) {
            LOG(DEBUG2) << "add instrument data client FD";
            maxFD = fd > maxFD ? fd : maxFD;
            FD_SET(fd, &readFDs);
        }
        else {
            LOG(DEBUG2) << "Observatory data client not initialized";
        }    
    }
}

/******************************************************************************
 * Method: getTelnetSnifferListenerFD
 * Description: Get the file descriptor
 ******************************************************************************/
int PortAgent::getTelnetSnifferListenerFD() {
    if(m_pTelnetSnifferConnection) {
        return m_pTelnetSnifferConnection->serverFD();
    }
    
    return 0;
}

/******************************************************************************
 * Method: getObservatoryCommandListenerFD
 * Description: Get the file descriptor
 ******************************************************************************/
int PortAgent::getObservatoryCommandListenerFD() {
    CommBase *pConnection = m_pObservatoryConnection->commandConnectionObject();
    if(m_pObservatoryConnection->commandInitialized()) {
        return ((TCPCommListener*)pConnection)->serverFD();
    }
    
    return 0;
}

/******************************************************************************
 * Method: getObservatoryCommandClientFD
 * Description: Get the file descriptor
 ******************************************************************************/
int PortAgent::getObservatoryCommandClientFD() {
    CommBase *pConnection = m_pObservatoryConnection->commandConnectionObject();
    if(m_pObservatoryConnection->commandConnected()) {
        return ((TCPCommListener*)pConnection)->clientFD();
    }
    
    return 0;
}

/******************************************************************************
 * Method: getObservatoryDataListenerFD
 * Description: Get the file descriptor
 ******************************************************************************/
int PortAgent::getObservatoryDataListenerFD() {
    CommBase *pConnection = m_pObservatoryConnection->dataConnectionObject();
    if(m_pObservatoryConnection->dataInitialized()) {
        return ((TCPCommListener*)pConnection)->serverFD();
    }
    
    return 0;  
}



/******************************************************************************
 * Method: getObservatoryDataClientFD
 * Description: Get the file descriptor
 ******************************************************************************/
int PortAgent::getObservatoryDataClientFD() {
    CommBase *pConnection = m_pObservatoryConnection->dataConnectionObject();
    if (pConnection) {
        if (m_pObservatoryConnection->dataConnected()) {
            return ((TCPCommListener*)pConnection)->clientFD();
        }
    }
    else {
        LOG(ERROR) << "getObservatoryDataClientFD: No observatory connection object!";
    }
    
    return 0;
}

/******************************************************************************
 * Method: getInstrumentDataTxClientFD
 * Description: Get the Tx file descriptor
 ******************************************************************************/
int PortAgent::getInstrumentDataTxClientFD() {
    CommBase *pConnection;;
    TCPCommSocket *socket;

    if (m_pInstrumentConnection->connectionType() == PACONN_INSTRUMENT_BOTPT) {
        pConnection = ((InstrumentBOTPTConnection*) m_pInstrumentConnection)->dataTxConnectionObject();
    }
    else {
        pConnection = m_pInstrumentConnection->dataConnectionObject();
    }

    if (m_pInstrumentConnection->dataConnected()) {
        socket = (TCPCommSocket*)pConnection;
        if(socket && socket->connected())
            return socket->getSocketFD();
    }
    else {
        LOG(ERROR) << "Instrument data client not connected";
    }

    return 0;
}

/******************************************************************************
 * Method: getInstrumentDataRxClientFD
 * Description: Get the Rx file descriptor
 ******************************************************************************/
int PortAgent::getInstrumentDataRxClientFD() {
    CommBase *pConnection;;
    TCPCommSocket *socket;

    if (m_pInstrumentConnection->connectionType() == PACONN_INSTRUMENT_BOTPT) {
        pConnection = ((InstrumentBOTPTConnection*) m_pInstrumentConnection)->dataRxConnectionObject();
    }
    else {
        pConnection = m_pInstrumentConnection->dataConnectionObject();
    }

    if (m_pInstrumentConnection->dataConnected()) {
        socket = (TCPCommSocket*)pConnection;
        if(socket && socket->connected())
            return socket->getSocketFD();
    }
    else {
        LOG(ERROR) << "Instrument data client not connected";
    }
    
    return 0;    
}

/******************************************************************************
 * Method: publishHeartbeat
 * Description: Generate a heartbeat packet if the heartbeat timeout has been
 *              exceeded.
 ******************************************************************************/
void PortAgent::publishHeartbeat() {
    Timestamp ts;
    time_t now = time(NULL);
    
    // if we have specificed a heartbeat interval and we need to send a heartbeat
    if(m_pConfig->heartbeatInterval() && now - m_lLastHeartbeat > m_pConfig->heartbeatInterval() ) {
        
        Packet packet(PORT_AGENT_HEARTBEAT, ts, "", 0);
        LOG(DEBUG) << "Port Agent Heartbeat";
        publishPacket(&packet);
        m_lLastHeartbeat = now;
    }
}

/******************************************************************************
 * Method: publishFault
 * Description: Generate a fault packet and send it to the publishers.
 ******************************************************************************/
void PortAgent::publishFault(const string &msg) {
    Timestamp ts;
    Packet packet(PORT_AGENT_FAULT, ts, (char *)(msg.c_str()), msg.length());

    LOG(ERROR) << "Port Agent Fault: " << msg;
    publishPacket(&packet);
}

/******************************************************************************
 * Method: publishStatus
 * Description: Generate a status packet and send it to the publishers.
 ******************************************************************************/
void PortAgent::publishStatus(const string &msg) {
    Timestamp ts;
    Packet packet(PORT_AGENT_STATUS, ts, (char *)(msg.c_str()), msg.length());

    LOG(ERROR) << "Port Agent Status: " << msg;
    publishPacket(&packet);
}

/******************************************************************************
 * Method: publishPacket
 * Description: Publish a packet. Just iterate over all publisher and call
 * the publish method.  Easy Peasy
 ******************************************************************************/
void PortAgent::publishPacket(Packet *packet) {
    LOG(DEBUG) << "Publish packet.";
    m_oPublishers.publish(packet);
}

/******************************************************************************
 * Method: publishPacket
 * Description: Create a packet and publish it.
 ******************************************************************************/
void PortAgent::publishPacket(char *payload, uint16_t size, PacketType type) {
    Timestamp ts;
    Packet packet(type, ts, payload, size);
    publishPacket(&packet); 
}

/******************************************************************************
 * Method: handleTelnetSnifferAccept
 * Description: Accept connection to the telnet sniffer connection.
 ******************************************************************************/
void PortAgent::handleTelnetSnifferAccept(const fd_set &readFDs) {
    int serverFD = getTelnetSnifferListenerFD();
    
    LOG(DEBUG) << "handleTelnetSnifferAccept - do we need to accept a new connection?";
    LOG(DEBUG2) << "Telnet Sniffer Listener FD: " << serverFD;
        
    // Accept a new client
    if(serverFD && FD_ISSET(serverFD, &readFDs)) {
        LOG(DEBUG) << "Telnet sniffer listener has data";
        handleTCPConnect(*m_pTelnetSnifferConnection);
        LOG(DEBUG) << "telnet sniffer client fd: " << m_pTelnetSnifferConnection->clientFD();
    }
}

/******************************************************************************
 * Method: handleTelnetSnifferRead
 * Description: Read from the telnet sniffer.  All data is ignored, but we need
 * the read to detect disconnects.
 ******************************************************************************/
void PortAgent::handleTelnetSnifferRead(const fd_set &readFDs) {
    int clientFD = 0;
    int bytesRead = 0;
    char buffer[1024];
    
    if(m_pTelnetSnifferConnection)
        clientFD = m_pTelnetSnifferConnection->clientFD();
    
    LOG(DEBUG) << "handleTelnetSnifferAccept - do we need to read from the telnet sniffer";
    LOG(DEBUG) << "Telnet Sniffer Client FD: " << clientFD;
        
    if(clientFD && FD_ISSET(clientFD, &readFDs)) {
        LOG(DEBUG) << "Read data from Telnet Sniffer Client FD: " << clientFD;
        bytesRead = m_pTelnetSnifferConnection->readData(buffer, 1023);
        buffer[bytesRead] = '\0';
        
        if(bytesRead) {
            LOG(DEBUG2) << "Bytes read: " << bytesRead;
            LOG(DEBUG) << "Bytes read from sniffer port are ignored: " << buffer;
        }
    }
}

/******************************************************************************
 * Method: handleObservatoryCommandConnect
 * Description: Accept connection to the TCP command connection.
 ******************************************************************************/
void PortAgent::handleObservatoryCommandAccept(const fd_set &readFDs) {
    CommBase *pConnection = m_pObservatoryConnection->commandConnectionObject();
    int serverFD = getObservatoryCommandListenerFD();
    
    LOG(DEBUG) << "handleObservatoryCommandAccept - do we need to accept a new connection?";
    LOG(DEBUG) << "Observatory Command Listener FD: " << serverFD;
        
    // Accept a new observatory command client
    if(serverFD && FD_ISSET(serverFD, &readFDs)) {
        LOG(DEBUG) << "Observatory command listener has data";
        handleTCPConnect(*((TCPCommListener*)pConnection));
    }
}

/******************************************************************************
 * Method: handleObservatoryCommandRead
 * Description: Read from the observatory command port
 ******************************************************************************/
void PortAgent::handleObservatoryCommandRead(const fd_set &readFDs) {
    CommBase *pConnection = m_pObservatoryConnection->commandConnectionObject();
    int clientFD = getObservatoryCommandClientFD();
    int bytesRead = 0;
    char buffer[1024];
    
    LOG(DEBUG) << "handleObservatoryCommandRead - do we need to read from the observatory command";
    LOG(DEBUG) << "Observatory Command Client FD: " << clientFD;
        
    if(clientFD && FD_ISSET(clientFD, &readFDs)) {
        LOG(DEBUG) << "Read data from Observatory Command Client FD: " << clientFD;
        bytesRead = ((TCPCommListener*)pConnection)->readData(buffer, 1023);
        buffer[bytesRead] = '\0';
        
        if(bytesRead) {
            LOG(DEBUG2) << "Bytes read: " << bytesRead;
            handlePortAgentCommand(buffer);
            publishPacket(buffer, bytesRead, PORT_AGENT_COMMAND);
        }
    }
}

/******************************************************************************
 * Method: handleObservatoryDataAccept
 * Description: Depending upon the observatory connection type, read from the
 * FD(s).
 ******************************************************************************/
void PortAgent::handleObservatoryDataAccept(const fd_set &readFDs) {
    if (m_pObservatoryConnection->connectionType() == PACONN_OBSERVATORY_STANDARD) {
        handleObservatoryStandardDataAccept(readFDs);
    }
    else if (m_pObservatoryConnection->connectionType() == PACONN_OBSERVATORY_MULTI) {
        handleObservatoryMultiDataAccept(readFDs);
    }
    else {
        LOG(ERROR) << "handleObservatoryDataAccept: Observatory connection type unknown!";
    }

}

/******************************************************************************
 * Method: handleObservatoryDataAccept
 * Description: Accept connection to the TCP data connection.
 ******************************************************************************/
void PortAgent::handleObservatoryStandardDataAccept(const fd_set &readFDs) {
    CommBase *pConnection = m_pObservatoryConnection->dataConnectionObject();
    int serverFD = getObservatoryDataListenerFD();

    LOG(DEBUG) << "handleObservatoryDataAccept - do we need to accept a new connection?";
    LOG(DEBUG2) << "Observatory Data Listener FD: " << serverFD;

    // Accept a new observatory command client
    if(serverFD && FD_ISSET(serverFD, &readFDs)) {
        LOG(DEBUG) << "Observatory data listener has data";
        handleTCPConnect(*((TCPCommListener*)pConnection));
    }
}

/******************************************************************************
 * Method: handleObservatoryDataAccept
 * Description: Iterate through the observatory data connections accepting
 * those that are ready.
 ******************************************************************************/
void PortAgent::handleObservatoryMultiDataAccept(const fd_set &readFDs) {
    CommBase *pConnection = 0;
    int serverFD = 0;
    
    LOG(DEBUG) << "handleObservatoryMultiDataAccept - checking for new connections";

    // Iterate through the listeners adding the clientFDs
    pConnection = ObservatoryDataSockets::instance()->getFirstSocket();
    while (pConnection) {
        // Accept a new observatory command client
        serverFD = ((TCPCommListener*)pConnection)->serverFD();
        if(serverFD && FD_ISSET(serverFD, &readFDs)) {
            LOG(DEBUG) << "Observatory data listener has new connection request";
            handleTCPConnect(*((TCPCommListener*)pConnection));
        }

        pConnection = ObservatoryDataSockets::instance()->getNextSocket();
    }
}

/******************************************************************************
 * Method: handleObservatoryDataRead
 * Description: Depending upon the observatory connection type, read from the
 * FD(s)
 ******************************************************************************/
void PortAgent::handleObservatoryDataRead(const fd_set &readFDs) {
    if (m_pObservatoryConnection->connectionType() == PACONN_OBSERVATORY_STANDARD) {
        handleObservatoryStandardDataRead(readFDs);
    }
    else if (m_pObservatoryConnection->connectionType() == PACONN_OBSERVATORY_MULTI) {
        handleObservatoryMultiDataRead(readFDs);
    }
    else {
        LOG(ERROR) << "handleObservatoryDataRead: Observatory connection type unknown!";
    }

}

/******************************************************************************
 * Method: handleObservatoryStandardDataRead
 * Description: Read from the observatory data port
 ******************************************************************************/
void PortAgent::handleObservatoryStandardDataRead(const fd_set &readFDs) {
    CommBase *pConnection = m_pObservatoryConnection->dataConnectionObject();
    int clientFD = getObservatoryDataClientFD();
    int bytesRead = 0;
    char buffer[1024];
    
    LOG(DEBUG) << "handleObservatoryDataRead - checking for observatory standard data";
    LOG(DEBUG2) << "Observatory Data Client FD: " << clientFD;

    if(clientFD && FD_ISSET(clientFD, &readFDs)) {
        LOG(DEBUG2) << "Read data from Observatory Data Client FD: " << clientFD;
        bytesRead = ((TCPCommListener*)pConnection)->readData(buffer, 1023);
        buffer[bytesRead] = '\0';

        if(bytesRead) {
            LOG(DEBUG2) << "Bytes read: " << bytesRead;
            publishPacket(buffer, bytesRead, DATA_FROM_DRIVER);
        }
    }
}

/******************************************************************************
 * Method: handleObservatoryMultiDataRead
 * Description: Iterate through the observatory data connections reading
 * those that are ready.
 ******************************************************************************/
void PortAgent::handleObservatoryMultiDataRead(const fd_set &readFDs) {
    CommBase *pConnection = 0;
    int bytesRead = 0;
    char buffer[1024];

    LOG(DEBUG) << "handleObservatoryDataRead - checking for observatory multi data";

    // Iterate through the listeners checking the clientFDs
    pConnection = ObservatoryDataSockets::instance()->getFirstSocket();
    while (pConnection) {
        int clientFD = ((TCPCommListener*) pConnection)->clientFD();
        LOG(DEBUG2) << "Observatory Data Client FD: " << clientFD;

        if (clientFD && FD_ISSET(clientFD, &readFDs)) {
            LOG(DEBUG2) << "Read data from Observatory Data Client FD: " << clientFD;
            bytesRead = ((TCPCommListener*)pConnection)->readData(buffer, 1023);
            buffer[bytesRead] = '\0';

            if(bytesRead) {
                LOG(DEBUG2) << "Bytes read: " << bytesRead;
                publishPacket(buffer, bytesRead, DATA_FROM_DRIVER);
            }
        }
        pConnection = ObservatoryDataSockets::instance()->getNextSocket();
    }
}

/******************************************************************************
 * Method: handleInstrumentDataRead
 * Description: Read from the instrument data port
 ******************************************************************************/
void PortAgent::handleInstrumentDataRead(const fd_set &readFDs) {
    CommBase *pConnection;

    if (m_pInstrumentConnection->connectionType() == PACONN_INSTRUMENT_BOTPT) {
        pConnection = ((InstrumentBOTPTConnection*) m_pInstrumentConnection)->dataRxConnectionObject();
    }
    else {
        pConnection = m_pInstrumentConnection->dataConnectionObject();
    }

    int clientFD = getInstrumentDataRxClientFD();
    int bytesRead = 0;
    char buffer[MAX_PACKET_SIZE];
    unsigned int read_size;
    LOG(DEBUG) << "handleInstrumentDataRead - do we need to read from the instrument data";
    
    if(! pConnection->connected()) {
        LOG(DEBUG2) << "instrument not connected, attempting to re-init the socket";
        initializeInstrumentConnection();
        clientFD = getInstrumentDataRxClientFD();
    }
    
    LOG(DEBUG2) << "Instrument Data Client FD: " << clientFD;
        
    if(clientFD && FD_ISSET(clientFD, &readFDs)) {
        read_size = m_pConfig->maxPacketSize();
        LOG(DEBUG) << "Read data from Instrument Data Client FD: " << clientFD << " max packet size: " << read_size;
        bytesRead = pConnection->readData(buffer, read_size);
        
        if(bytesRead) {
            LOG(DEBUG2) << "Bytes read: " << bytesRead;
            publishPacket(buffer, bytesRead, DATA_FROM_INSTRUMENT);
            //buffer[bytesRead] = '\0';
        }
    }
}

/******************************************************************************
 * Method: getCurrentStateAsString
 * Description: return the current state as a string object
 ******************************************************************************/
const string PortAgent::getCurrentStateAsString() {
    if(getCurrentState() == STATE_UNCONFIGURED)
        return "UNCONFIGURED";
    else if(getCurrentState() == STATE_CONFIGURED)
        return "CONFIGURED";
    else if(getCurrentState() == STATE_CONNECTED)
        return "CONNECTED";
    else if(getCurrentState() == STATE_DISCONNECTED)
        return "DISCONNECTED";
    else if(getCurrentState() == STATE_STARTUP)
        return "STARTUP";
    else
        return "UNKNOWN";
}

/******************************************************************************
 * Method: setState
 * Description: State the port agent intrument connection state
 ******************************************************************************/
void PortAgent::setState(const PortAgentState &state) {
    // Only set the state if it has changed.
    if(state != getCurrentState()) {
        const string previousState = getCurrentStateAsString();
    
        m_oState = state;

        LOG(DEBUG) << "***********************************************";
        LOG(DEBUG) << "State transition from " << previousState << " TO " << getCurrentStateAsString();
        LOG(DEBUG) << "***********************************************";
    }
}

/******************************************************************************
 * Method: setRotationInterval
 * Description: Change the rotation interval for the data log publisher
 ******************************************************************************/
void PortAgent::setRotationInterval() {
    RotationType type = m_pConfig->rotation_interval();
        
    Publisher *found = m_oPublishers.searchByType(PUBLISHER_FILE);
    if(found) {
        LOG(DEBUG) << "Found publisher.  Setting rotation interval";
        ((FilePublisher*)found)->setRotationInterval(type);
    }
}
