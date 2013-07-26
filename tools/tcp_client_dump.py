#!/usr/bin/python
#
# This tool it used to test TCP listeners.  It connects to a tcp port.
# bytes are read, then written to an output file.
#
# Options:
# 
#  -h, --help - Display the program help screen
#  -f, --datafile - File to dump output too
#  -h, --host - Host to connect too
#  -p PORT, --port PORT - Port to connect too
#  -t TIMEOUT, --timeout TIMEOUT - Timeout duration 
#

import socket
import fcntl
import errno 
import sys
import argparse
import time
import os

DEFAULT_TIMEOUT = 30
DEFAULT_PORT    = 40000
DEFAULT_DELAY   = 0
BUFSIZE = 1024

def parseArgs():
    parser = argparse.ArgumentParser(description="TCP Echo Server")
    parser.add_argument("-f", '--datafile', required=True, dest='datafile', help="where do dump output" )
    parser.add_argument("-n", '--hostname', required=True, dest='hostname', action="store", help="hostname to connect too" )
    parser.add_argument('-p', '--port', required=True, dest='port', type=int, default=40000, help='specify the INET port to connect')
    parser.add_argument('-t', '--timeout', dest='timeout', type=int, default=DEFAULT_TIMEOUT, help='listen timeout')

    return parser.parse_args()

def write_data(data):
    FILE = open(opts.datafile, "w");
    FILE.write(data);
    print "Write: " + data;
    FILE.close();

def read_tcp(conn):
    print("read data from client")
    while True:
        try:
            data = conn.recv(BUFSIZE)
            return data;
        except socket.error as e:
            # [Errno 35] Resource temporarily unavailable.
            if e.errno == errno.EAGAIN:
                pass
            else:
                raise e


##### MAIN #####
opts = parseArgs()

##let's set up some constants
hostname = opts.hostname    #we are the host
portnum = opts.port    #arbitrary port not currently in use
address = (hostname,portnum)    #we need a tuple for the address
buffer_size = 1    #reasonably sized buffer for data
timeout = opts.timeout         # Socket timeout

## now we create a new socket object (serv)
## see the python docs for more information on the socket types/flags
conn = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
conn.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

print("connecting to %s:%s" % (hostname, portnum))
conn.connect(address)    #the double parens are to create a tuple with one element

if(timeout):
    conn.settimeout(timeout)
        
while(1):
    data = read_tcp(conn);
    if(data):
        write_data(data);
