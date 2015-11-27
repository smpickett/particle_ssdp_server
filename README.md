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
#include "WebServer.h"

Ssdp ssdpServer(80, "description.xml");
WebServer webServer("", 80);

/* Webserver functions prototypes */

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
  ssdpServer.ProcessConnection();
}

/* Webserver functions & html/xml messages */
```
