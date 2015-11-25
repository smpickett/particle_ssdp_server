#include <stdint.h>
#include <stdio.h>
#include "application.h"
#include "ssdp.h"
using namespace std;

// Configuration ---------------------------------------------------------------
const char* SSDP_DEFAULT_XML_FILENAME = "ssdp/desc.xml";
const unsigned int SSDP_PORT_REMOTE = 1900;
const unsigned int SSDP_PORT_LOCAL = 1900;
const IPAddress SSDP_IP(239, 255, 255, 250);

const char *SSDP_XML_DESCRIPTION = "HTTP/1.1 200 OK\r\n"
  "Content-Type: text/xml\r\n\r\n"
  "<?xml version='1.0'?>\r\n"
  "<root xmlns='urn:schemas-upnp-org:device-1-0'>"
  "<device>"
  "<deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>"
  "<friendlyName>Particle Photon</friendlyName>"
  "<manufacturer>Stephen Pickett</manufacturer>"
  "<manufacturerURL>https://github.com/smpickett/particle_ssdp_server</manufacturerURL>"
  "<modelName>Photon</modelName>"
  "<modelNumber>Photon Core</modelNumber>"
  "<modelURL>https://www.particle.io/</modelURL>"
  "<presentationURL>/</presentationURL>"
  "<serialNumber>1</serialNumber>"
  "<UDN>uuid:abcdefgh-7dec-11d0-a765-7499692d3040</UDN>"
  "</device>"
  "</root>\r\n";

// SSDP Implementation ---------------------------------------------------------
Ssdp::Ssdp(int webServerPort)
    :_httpServer(_httpServerPort)
{
    _spawnWebServer = true;
    _xmlFilePath = SSDP_DEFAULT_XML_FILENAME;
    _httpServerPort = webServerPort;
}

Ssdp::Ssdp(int webServerPort, const char* xmlFilePath)
    :_httpServer(0)
{
    _spawnWebServer = false;
    _xmlFilePath = xmlFilePath;
    _httpServerPort = webServerPort;
}

void Ssdp::ProcessConnection()
{
    // Initialize, if required
    Initialize();

    // Send a keep alive at a regular interval
    NotifyKeepAlive();

    // Monitor for SSDP requests
    SsdpParse();

    // Monitor for HTTP requests
    HttpParse();
}

void Ssdp::Initialize()
{
    if (_initialized)
        return;

    // First, ensure we can ping the gateway.
    // This ensures that the multicast broadcast is working properly
    // See various issues related to the UDP multicast on particle.io forums
    IPAddress gateway = WiFi.gatewayIP();
    if (!WiFi.ping(gateway))
    {
        #if SSDP_SERIAL_DEBUG_LEVEL > 0
        Serial.print("SSDP: Ping to gateway ");
        Serial.print(gateway);
        Serial.println(" was not responsive. SSDP therefore not yet configured.");
        #endif
        return;
    }

    // Start UDP
    _udp.begin(SSDP_PORT_LOCAL);
    _udp.joinMulticast(SSDP_IP);

    // Start TCP server
    if (_spawnWebServer)
    {
        _httpServer.begin();
        #if SSDP_SERIAL_DEBUG_LEVEL > 0
        Serial.printf("SSDP: Configuration of Webserver on port %d\r\n", _httpServerPort);
        #endif
    }

    _initialized = true;

    #if SSDP_SERIAL_DEBUG_LEVEL > 0
    Serial.println("SSDP: Configured");
    #endif
}

void Ssdp::HttpParse()
{
    if (!_spawnWebServer)
        return;

    if (_httpClient.connected())
    {
        int len = _httpClient.available();
        if (len > 0)
        {
            // Identify who queried us
            IPAddress remoteIP = _httpClient.remoteIP();
            #if SSDP_SERIAL_DEBUG_LEVEL > 0
            Serial.printf("SSDP: Received TCP packet (len: %d) from %d.%d.%d.%d\r\n", len, remoteIP[0], remoteIP[1], remoteIP[2], remoteIP[3]);
            #endif

            // Read the query packet
            int i = 0;
            while (len-- > 0 && i < SSDP_RX_PACKET_MAX_SIZE && _httpClient.available())
            {
                _rxPacketBuffer[i++] = _httpClient.read();
            }

            #if SSDP_SERIAL_DEBUG_LEVEL > 1
            Serial.println("SSDP: /-------------------------------");
            Serial.println(_rxPacketBuffer);
            Serial.println("SSDP: -------------------------------/");
            #endif

            if (strncmp("GET /", _rxPacketBuffer, 5) == 0)
            {
              #if SSDP_SERIAL_DEBUG_LEVEL > 0
              Serial.println("SSDP: Processing GET /");
              #endif
                if (strncmp(_xmlFilePath, _rxPacketBuffer + 5, strlen(_xmlFilePath)) == 0)
                {
                    #if SSDP_SERIAL_DEBUG_LEVEL > 0
                    Serial.printf("SSDP: Processing GET /%s\r\n", SSDP_DEFAULT_XML_FILENAME);
                    #endif
                    XmlDescription();
                }
            }
        }
    }
    else
    {
        _httpClient = _httpServer.available();
    }
}

void Ssdp::SsdpParse()
{
    int len = _udp.parsePacket();
    if (len > 0)
    {
        // Identify who queried us
        IPAddress remoteIP = _udp.remoteIP();
        #if SSDP_SERIAL_DEBUG_LEVEL > 0
        Serial.printf("SSDP: Received UDP packet (len: %d) from %d.%d.%d.%d:%d\r\n", len, remoteIP[0], remoteIP[1], remoteIP[2], remoteIP[3], _udp.remotePort());
        #endif

        // Read the query packet
        _udp.read(_rxPacketBuffer, SSDP_RX_PACKET_MAX_SIZE);
        #if SSDP_SERIAL_DEBUG_LEVEL > 1
        Serial.println("SSDP: /-------------------------------");
        Serial.println(rxPacketBuffer);
        Serial.println("SSDP: -------------------------------/");
        #endif

        if (strncmp("M-SEARCH", _rxPacketBuffer, 8) == 0)
        {
          #if SSDP_SERIAL_DEBUG_LEVEL > 0
          Serial.println("SSDP: Processing M-SEARCH");
          #endif
          Response();
        }
    }
}

void Ssdp::Response()
{
    IPAddress localip = WiFi.localIP();
  char ip[] = "255.255.255.255:65535/";
  sprintf(ip, "%d.%d.%d.%d:%d/", localip[0], localip[1], localip[2], localip[3], _httpServerPort);

  _udp.beginPacket(_udp.remoteIP(), _udp.remotePort());
  _udp.write("HTTP/1.1 200 OK\r\nSERVER:Cream/3.1,UPnP/1.0,UPnP/1.0\r\nCACHE-CONTROL:max-age=360\r\n");
  _udp.write("LOCATION:http://");
  _udp.write(ip);
  _udp.write(_xmlFilePath);
  _udp.write("\r\nST:upnp:rootdevice\r\nUSN:uuid:b629a14a-d342-3c5c-bc8c-f7417arduino::upnp:rootdevice\r\nEXT:\r\n\r\n");
  _udp.endPacket();
}

void Ssdp::XmlDescription()
{
  _httpClient.println(SSDP_XML_DESCRIPTION);
  _httpClient.stop();
}

void Ssdp::NotifyKeepAlive()
{
    // Periodically send another notify
    if ((millis() % SSDP_DEFAULT_NOTIFY_KEEPALIVE_TIME == 0) && !_notify_timer_sent)
    {
        IPAddress localip = WiFi.localIP();
        char ip[] = "255.255.255.255:65535/";
        sprintf(ip, "%d.%d.%d.%d:%d/", localip[0], localip[1], localip[2], localip[3], _httpServerPort);

        // Send the NOTIFY packet
        _udp.beginPacket(SSDP_IP, SSDP_PORT_REMOTE);
        _udp.write("NOTIFY * HTTP/1.1\r\n");
        _udp.write("HOST: 239.255.255.250:1900\r\n");
        _udp.write("CACHE-CONTROL: max-age=900\r\n");
        _udp.write("NT: upnp:rootdevice\r\n");
        _udp.write("USN: uuid:abcdefga-7dec-11d0-a765-7499692d3040::upnp:rootdevice\r\n");
        _udp.write("NTS: ssdp:alive\r\n");
        _udp.write("SERVER: Arduino UPnP/1.0\r\n");
        _udp.write("LOCATION: http://");
        _udp.write(ip);
        _udp.write(_xmlFilePath);
        _udp.endPacket();
        _notify_timer_sent = true;

        #if SSDP_SERIAL_DEBUG_LEVEL > 0
        Serial.println("SSDP: Sent Notify");
        #endif
    }
    else
    {
        _notify_timer_sent = false;
    }
}
