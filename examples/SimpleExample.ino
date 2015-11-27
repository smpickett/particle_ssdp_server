#include "SSDP.h"

SSDP ssdpServer(80);

void setup(void)
{ }

void loop(void)
{
  ssdpServer.processConnection();
}
