/*******************************************************************************
 * Class: SerialCommSocket
 * Filename: udp_comm_socket.cxx
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Manage connecting to serial devices
 *
 ******************************************************************************/

#include "serial_comm_socket.h"
#include "common/util.h"
#include "common/logger.h"
#include "common/exception.h"


using namespace std;
using namespace logger;
using namespace network;
    
/******************************************************************************
 *   PUBLIC METHODS
 ******************************************************************************/

/******************************************************************************
 * Method: Constructor
 * Description: Default constructor.
 ******************************************************************************/
SerialCommSocket::SerialCommSocket() {

}


/******************************************************************************
 * Method: Copy Constructor
 * Description: Copy constructor.
 ******************************************************************************/
SerialCommSocket::SerialCommSocket(const SerialCommSocket &rhs) {
}


/******************************************************************************
 * Method: Destructor
 * Description: destructor.
 ******************************************************************************/
SerialCommSocket::~SerialCommSocket() {
}

/******************************************************************************
 * Method: copy
 * Description: return a new object deep copied.
 ******************************************************************************/
CommBase * SerialCommSocket::copy() {
	return new SerialCommSocket(*this);
}

/******************************************************************************
 * Method: Equality operator
 * Description: overloading!
 ******************************************************************************/
bool SerialCommSocket::operator==(SerialCommSocket &rhs) {
		return compare(&rhs);
}

/******************************************************************************
 * Method: compare
 * Description: compare objects
 ******************************************************************************/
bool SerialCommSocket::compare(CommBase *rhs) {
		return false;
}

/******************************************************************************
 * Method: assignment operator
 * Description: overloaded assignment operator.
 ******************************************************************************/
SerialCommSocket & SerialCommSocket::operator=(const SerialCommSocket &rhs) {
	return *this;
}

/******************************************************************************
 * Method: initialize
 * Description: Perform required initialization.
 ******************************************************************************/
bool SerialCommSocket::initialize() {
	bool bReturnCode = true;

	bIsConfigured = true;
	return bReturnCode;
}

/******************************************************************************
 * Method: isInitialized
 * Description: Has this object been configured?
 ******************************************************************************/
bool SerialCommSocket::isConfigured() {
	return bIsConfigured;
}


