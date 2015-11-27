/*
   SSDP, an implementation of the Simple Service Discovery Protocol
   Copyright 2015 Stephen Pickett

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#ifndef ssdp_h
#define ssdp_h

#include "application.h"

// Configuration ---------------------------------------------------------------
#define SSDP_DEFAULT_WEBSERVER_PORT         (80)
#define SSDP_DEFAULT_NOTIFY_KEEPALIVE_TIME  (10000)     // in [ms]
#define SSDP_RX_PACKET_MAX_SIZE             (64)        // Must be at least 64 bytes. Larger results in more debugging information

// DEBUG level for serial port printout
// 0 - off (no serial debugging)
// 1 - status/processing information
// 2 - trace (including packets)
#define SSDP_SERIAL_DEBUG_LEVEL 0


// Interface -------------------------------------------------------------------
// Creates an SSDP connection object that will:
//  - Provide 'NOTIFY' broadcasts at a regular interval
//  - Respond to 'SEARCH' broadcasts
//  - Provide an XML description
class SSDP
{
public:
    // Initializes the object
    // The 'webServerPort' will be used to listen for incoming TCP web requests
    // and the Ssdp object will spawn its own (very basic) webserver
    SSDP(int webServerPort = SSDP_DEFAULT_WEBSERVER_PORT);

    // Initializes the object
    // The 'xmlFilePath' will be used in responses to SSDP queries to indicate
    // how to obtain the XML data from the webserver that will be running on the
    // port specified by 'webServerPort'.
    // It is the users responsiblity to setup a webserver outside the SSDP framework
    // to respond to XML file requests.
    SSDP(int webServerPort, const char* xmlFilePath);

    // This function must be called on a regular basis (ie, in the loop function)
    void processConnection();

private:
    // Initializes the class if it hasn't been already
    void initialize();
    // Will send a 'NOTIFY' broadcast at a specified interval
    void notifyKeepAlive();
    // Parses SSDP queries
    void ssdpParse();
    // Provides a response to the M-SEARCH requests
    void response();
    // Parses HTTP webserver queries
    void httpParse();
    // Provides a response to the XML description requests
    void xmlDescription();

private:
    // HTTP Web Server
    int _httpServerPort = SSDP_DEFAULT_WEBSERVER_PORT;
    TCPServer _httpServer;
    TCPClient _httpClient;
    const char* _xmlFilePath;

    // UDP Broadcast Client/Server
    UDP _udp;

    // Packet buffer
    char _rxPacketBuffer[SSDP_RX_PACKET_MAX_SIZE] = { 0 };

    // Remembers if the object has been initialized
    bool _initialized = false;
    // Indicates if a webserver needs to be spawned
    bool _spawnWebServer = true;
    // Indicates if the notify is ready to be sent
    bool _notify_timer_sent = false;
};



#endif
