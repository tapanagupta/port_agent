/*******************************************************************************
 * Class: DriverCommandPublisher
 * Filename: driver_command_publisher.cxx
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Publish data to the driver command port.
 *
 ******************************************************************************/

#include "driver_command_publisher.h"
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
DriverCommandPublisher::DriverCommandPublisher() : DriverPublisher() { }

/******************************************************************************
 * Method: write
 * Description: Overload the base implementation of write to only write if 
 * connected.  This will reduce error/warnings in the logs when no client is
 * connected to the command port.
 *
 * Parameter:
 *    char* - the buffer that we are writting.
 *    size - how many bytes?
 *
 * Exceptions:
 *    FileDescriptorNULL
 *    PacketPublishFailure
 ******************************************************************************/
bool DriverCommandPublisher::write(const char *buffer, uint32_t size) {
    if(m_pCommSocket && m_pCommSocket->connected()) 
        DriverPublisher::write(buffer, size);
    else
        LOG(DEBUG) << "Command port not connected, not writing packets";
}