/*******************************************************************************
 * Class: TelnetSnifferPublisher
 * Filename: telnet_sniffer_publisher.h
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * Publish data to the telnet sniffer.  Dumps raw instrument output to the port.
 *    
 ******************************************************************************/

#ifndef __TELNET_SNIFFER_PUBLISHER_H_
#define __TELNET_SNIFFER_PUBLISHER_H_

#include "tcp_publisher.h"
#include "common/log_file.h"

using namespace std;
using namespace logger;

namespace publisher {
    class TelnetSnifferPublisher : public TCPPublisher {
        /********************
         *      METHODS     *
         ********************/
        
        public:
           TelnetSnifferPublisher();
           TelnetSnifferPublisher(CommBase *socket) : TCPPublisher(socket) {}

	       const PublisherType publisherType() { return PUBLISHER_TELNET_SNIFFER; }
		   
		   bool publishDataFromInstrument(Packet *packet); 
		   bool publishDataFromObservatory(Packet *packet);
		   
		   void setPrefix(const string &param) { m_prefix = param; }
		   void setSuffix(const string &param) { m_suffix = param; }
	   
        protected:
            virtual bool handleInstrumentData(Packet *packet)     { publishDataFromInstrument(packet); }
            virtual bool handleDriverData(Packet *packet)         { publishDataFromObservatory(packet); }
            virtual bool handleInstrumentCommand(Packet *packet)  { return true; }
            virtual bool handleCommand(Packet *packet)            { return true; }
            virtual bool handleStatus(Packet *packet)             { return true; }
            virtual bool handleFault(Packet *packet)              { return true; }
            virtual bool handleDriverCommand(Packet *packet)      { return true; }
            virtual bool handleHearteat(Packet *packet)           { return true; }

        private:
        
        /********************
         *      MEMBERS     *
         ********************/
        
        protected:
            
        private:
			string m_prefix;
			string m_suffix;

    };
}

#endif //__DRIVER_COMMAND_PUBLISHER_H_
