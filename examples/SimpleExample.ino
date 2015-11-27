#include "ssdp.h"

Ssdp ssdpServer(80);

void setup(void)
{ }

void loop(void)
{
  ssdpServer.processConnection();
}
