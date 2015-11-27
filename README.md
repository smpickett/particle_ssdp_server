SSDP Server for Particle and Spark Cores
====================
This library provides support for implementing  SSDP (Simple Service Discovery Protocol) for the Particle.IO Photon and Spark cores.

More about SSDP can be found here: https://en.wikipedia.org/wiki/Simple_Service_Discovery_Protocol

SSDP is a great way of broadcasting the functionality and location of your device to other devices on the network

![Network Discovery in Windows](/readme_example_network.png)

## Basic Example
#### in application.ino
```
#include "ssdp.h"

Ssdp ssdpServer(80);

void setup(void)
{ }

void loop(void)
{
  ssdpServer.ProcessConnection();
}
```

## Example combined with a webserver (such as 'webduino')
#### in application.ino
```
#include "ssdp.h"
#include "webserver.h"

Ssdp ssdpServer(80, "description.xml");
Webserver webServer("", 80);

void xmlDescription(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  /* Process and respond to XML request */
  /* standard webduino response */
  /* ... */
}

void setup(void)
{
  /* webserver setup */
  webserver.addCommand("description.xml", &xmlDescription);
  webserver.begin();
}

void loop(void)
{
  char buff[64];
  int len = 64;

  webserver.processConnection(buff, &len);
  ssdpServer.ProcessConnection();
}
```

