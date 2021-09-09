#include <Winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <Winsock.h>
#include <iphlpapi.h>
#include <stdio.h>

int udp_test();

// dhcp payload struct
typedef struct {
        UCHAR op;
        UCHAR htype;
        UCHAR hlen;
        UCHAR hops;
        ULONG32 xid;
        USHORT segs;
        USHORT flags;
        ULONG32 ciaddr;
        ULONG32 yiaddr;
        ULONG32 siaddr;
        ULONG32 giaddr;
        USHORT chaddr_first;
        USHORT chaddr_second;
        USHORT chaddr_third;
        ULONG32 magic;
    } dhcp_payload;

// udp header struct
typedef struct {
        USHORT src_port;
        USHORT dest_port;
        USHORT len;
        USHORT checksum;
        dhcp_payload payload;
    } udp_packet;
