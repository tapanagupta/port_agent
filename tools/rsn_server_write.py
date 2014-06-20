#!/usr/bin/env python
#
# TODO:
#
# Options:
# 
#  -h, --help - Display the program help screen
# TODO: remove option: -c, --continuous - Do not close the connection after each read
#  -s, --single - Accept one client connection then kill the server.
#  -p PORT, --port PORT - Indicate what port to listen on.  Default: 40000
#  -t TIMEOUT, --timeout TIMEOUT - How long should we wait for a client connection?
#                                  set to 0 for no timeout, default 30
#  -d DELAY, --delay DELAY - sleep X number of seconds (fractions ok) before reading
#                            the net byte of data.  This is useful for testing 
#                            blocking vs. non-blocking reads.
#  -o, --stdout - enable this option if the server should output the buffer read to 
#                 stdout after it is read.
#

import socket
import fcntl
import errno 
import sys
import argparse
import time
import os
import binascii
from struct import Struct

DEFAULT_TIMEOUT = 30
DEFAULT_PORT    = 40000
DEFAULT_DELAY   = 0

"""
Port Agent Packet Types
"""
UNKNOWN = 0
DATA_FROM_INSTRUMENT = 1
DATA_FROM_DRIVER = 2
PORT_AGENT_COMMAND = 3
PORT_AGENT_STATUS = 4
PORT_AGENT_FAULT = 5
INSTRUMENT_COMMAND = 6
HEARTBEAT = 7
PICKLED_DATA_FROM_INSTRUMENT = 8
PICKLED_DATA_FROM_DRIVER = 9

def parseArgs():
    parser = argparse.ArgumentParser(description="TCP Echo Server")
    parser.add_argument("-s", '--single', dest='single', action="store_true",
        help="only accept one connection then quit" )
    parser.add_argument("-c", '--continuous', dest='continuous', action="store_true",
        help="do not close between reads" )
    parser.add_argument('-p', '--port', dest='port', type=int, default=40000, help='specify the INET port to bind')
    parser.add_argument('-t', '--timeout', dest='timeout', type=int, default=DEFAULT_TIMEOUT, help='listen timeout')
    parser.add_argument('-d', '--delay', dest='delay', type=float, default=DEFAULT_DELAY, help='delay between reading characters from the stream')
    parser.add_argument('-o', '--stdout', dest='stdout', action="store_true", help='write bytes read to stdout')

    return parser.parse_args()

def makepacket(msgtype, timestamp, data):

    SYNC = (0xA3, 0x9D, 0x7A)
    HEADER_FORMAT = "!BBBBHHd"
    header_struct = Struct(HEADER_FORMAT)
    HEADER_SIZE = header_struct.size

    def calculateChecksum(data, seed=0):
        n = seed
        for datum in data:
            n ^= datum
        return n

    def pack_header(buf, msgtype, pktsize, checksum, timestamp):
        sync1, sync2, sync3 = SYNC
        header_struct.pack_into(buf, 0, sync1, sync2, sync3, msgtype, pktsize,
                                checksum, timestamp)

    pktsize = HEADER_SIZE + len(data)
    pkt = bytearray(pktsize)
    pack_header(pkt, msgtype, pktsize, 0, timestamp)
    pkt[HEADER_SIZE:] = data
    checksum = calculateChecksum(pkt)
    pack_header(pkt, msgtype, pktsize, checksum, timestamp)
    return pkt

# Make a packet
# data = "A" * (2**16 - HEADER_SIZE - 1)
# txpkt = makepacket(PortAgentPacket.DATA_FROM_INSTRUMENT, 0.0, data)

def run_server(serv, opts):
    conn,addr = serv.accept() #accept the connection
    print("client connected")
    
    try:
        while True:
            data = read_tcp(conn, opts.delay)
            if(data):
                if(opts.stdout): print "Read: " + binascii.hexlify(data)
                time.sleep(1)
                pkt = makepacket(msgtype=DATA_FROM_INSTRUMENT, timestamp=time.time(), data=data)
                send_tcp(conn, pkt)

            if(not opts.continuous): break

        print "closing connection"
        conn.close()
        
    except Exception, e:
        raise e

def send_tcp(conn, buffer):
    ## TODO: Send in pieces
    print("Sending data: " + binascii.hexlify(buffer))
    result = conn.send(buffer)
    print("result: %d" % result)
    

def read_tcp(conn, read_delay):
    result = ""

    conn.settimeout(0.1)

    while True:
        try:
            data = conn.recv(1)
            if(read_delay): time.sleep(read_delay)
            if(data):
                print("READ: " + data)
                result = result + data
            else:
                break
        except socket.timeout:
            break
        
        except socket.error as e:
            # [Errno 35] Resource temporarily unavailable.
            if e.errno == errno.EAGAIN:
                print("eagain")
                pass
            
            # [Errno 54] Connection reset by peer.  Because we are reading a single
            # byte at a time we read until we fail.
            elif e.errno == errno.ECONNRESET:
                print("connection reset")
                pass
            else:
                print("read/write error (errno: %d)" % e.errno)
                raise e

    return result

opts = parseArgs()

##let's set up some constants
hostname = ''    #we are the host
portnum = opts.port    #arbitrary port not currently in use
address = (hostname,portnum)    #we need a tuple for the address
buffer_size = 1    #reasonably sized buffer for data
timeout = opts.timeout         # Socket timeout

## now we create a new socket object (serv)
## see the python docs for more information on the socket types/flags
serv = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
serv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

##bind our socket to the address
serv.bind((address))    #the double parens are to create a tuple with one element
print("listening")
serv.listen(5)
print("ready for connections")

if(timeout):
    serv.settimeout(timeout)

while(1):
    run_server(serv, opts)
    if opts.single:
        break
