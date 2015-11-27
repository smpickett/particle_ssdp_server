#include "ssdp.h"
#include "WebServer.h"

Ssdp ssdpServer(80, "description.xml");
WebServer webServer("", 80);

void xmlDescriptionCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete);
void rootCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete);

void setup(void)
{
  /* webserver setup */
  webServer.setDefaultCommand(&rootCmd);
  webServer.addCommand("description.xml", &xmlDescriptionCmd);
  webServer.begin();
}

void loop(void)
{
  char buff[64];
  int len = 64;

  webServer.processConnection(buff, &len);
  ssdpServer.processConnection();
}

P(root) = "<html><title>SSDP and Webduino Example</title><body><h2>Welcome to the core!</h2></body></html>";

P(xmlSsdp) = "HTTP/1.1 200 OK\r\n"
  "Content-Type: text/xml\r\n\r\n"
  "<?xml version='1.0'?>\r\n"
  "<root xmlns='urn:schemas-upnp-org:device-1-0'>"
  "<device>"
  "<deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>"
  "<friendlyName>Particle Photon with Webduino!</friendlyName>"
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

void rootCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  server.httpSuccess();

  if (type == WebServer::HEAD)
    return;

  server.printP(root);
}

void xmlDescriptionCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  server.printP(xmlSsdp);
}
