/*******************************************************************************
 * Class: TelnetSnifferPublisher
 * Filename: telnet_sniffer_publisher.cxx
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Publish data to the telnet sniffer.  Dumps raw instrument output to the port.
 *
 ******************************************************************************/

#include "tcp_publisher.h"
#include "telnet_sniffer_publisher.h"
#include "common/logger.h"
#include "common/exception.h"
#include "port_agent/packet/packet.h"

#include <sstream>
#include <string>

#include <stdio.h>

using namespace std;
using namespace packet;
using namespace logger;
using namespace publisher;
    
/******************************************************************************
 *   PUBLIC METHODS
 ******************************************************************************/

/******************************************************************************
 * Method: Constructor
 * Description: default constructor
 ******************************************************************************/
TelnetSnifferPublisher::TelnetSnifferPublisher() : TCPPublisher() {
    m_prefix = "";
    m_suffix = "";
}

		   
/******************************************************************************
 * Method: publishDataFrominstrument
 * Description: Write a packet of data to the internal file pointer.  Raise an
 * error if we don't have a file pointer.
 *
 * Parameter:
 *    Packet* - Pointer to a packet of data we need to write to the FILE*
 ******************************************************************************/
bool TelnetSnifferPublisher::publishDataFromInstrument(Packet *packet) {
    LOG(DEBUG2) << "Publish packet to sniffer: " << packet->payload();
    return write(packet->payload(), packet->payloadSize());
}

/******************************************************************************
 * Method: publishDataFrominstrument
 * Description: Write a packet of data to the internal file pointer.  Raise an
 * error if we don't have a file pointer.
 *
 * Parameter:
 *    Packet* - Pointer to a packet of data we need to write to the FILE*
 ******************************************************************************/
bool TelnetSnifferPublisher::publishDataFromObservatory(Packet *packet) {
    bool result = true;
    if(m_prefix.length() || m_suffix.length()) {
        if(m_prefix.length()) {
            write(m_prefix.c_str(), m_prefix.length());
        }
        result = write(packet->payload(), packet->payloadSize());
        if(m_suffix.length()) {
            write(m_suffix.c_str(), m_suffix.length());
        }
    }
    
    return result;
}