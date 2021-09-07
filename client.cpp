#include <Winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iphlpapi.h>
#include <stdio.h>

#define DHCP_PORT "67"

// Have an example client to simulate a dhcp requesting device