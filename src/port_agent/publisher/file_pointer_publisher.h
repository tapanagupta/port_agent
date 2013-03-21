/*******************************************************************************
 * Class: FilePointerPublisher
 * Filename: file_pointer_publisher.h
 * Author: Bill French (wfrench@ucsd.edu)
 * License: Apache 2.0
 *
 * This is a base packet for publishing packets to a c-style FILE*.  This
 * should work for tcp, udp, and serial connections.
 * 
 * We will default to all handlers writting all packet data to the file pointer.
 * The specialized classes can disable handlers that they don't want.
 *    
 ******************************************************************************/

#ifndef __FILE_POINTER_PUBLISHER_H_
#define __FILE_POINTER_PUBLISHER_H_

#include "publisher.h"
#include "network/comm_base.h"
#include "common/log_file.h"

using namespace std;
using namespace logger;
using namespace network;

namespace publisher {
    class FilePointerPublisher : public Publisher {
        /********************
         *      METHODS     *
         ********************/
        
        public:

           FilePointerPublisher();
	       FilePointerPublisher(const FilePointerPublisher &rhs);
	       FilePointerPublisher(CommBase *socket);
	       
		   virtual ~FilePointerPublisher();

           virtual FilePointerPublisher & operator=(const FilePointerPublisher &rhs);
           virtual bool operator==(FilePointerPublisher &rhs);
           virtual bool compare(Publisher *rhs);
			
           void setCommObject(CommBase *socket);
	   
           // Explicitly set the output file
           void setFilePointer(FILE *fd);
		   
		   CommBase *commSocket() { return m_pCommSocket; }

        protected:

            virtual bool handleInstrumentData(Packet *packet)    { return logPacket(packet); }
            virtual bool handleDriverData(Packet *packet)        { return logPacket(packet); }
            virtual bool handleCommand(Packet *packet)           { return logPacket(packet); }
            virtual bool handleStatus(Packet *packet)            { return logPacket(packet); }
            virtual bool handleFault(Packet *packet)             { return logPacket(packet); }
            virtual bool handleInstrumentCommand(Packet *packet) { return logPacket(packet); }
            virtual bool handleHeartbeat(Packet *packet)         { return logPacket(packet); }

            bool logPacket(Packet *packet);
            virtual bool write(const char *buffer, uint32_t size);

        private:
			bool compareCommSocket(CommBase *rhs);
        

        /********************
         *      MEMBERS     *
         ********************/
        
        protected:
	    CommBase* m_pCommSocket;
            
        private:
            FILE* m_pFilePointer;
	    
    };
}

#endif //__FILE_POINTER_PUBLISHER_H_
