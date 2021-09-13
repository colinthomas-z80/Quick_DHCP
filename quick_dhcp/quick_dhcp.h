#include <Winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <Winsock.h>
#include <iphlpapi.h>
#include <stdio.h>

#define DHCP_MAGIC_COOKIE "99.130.83.99"
#define DHCP_PORT_RX "67"
#define OFFSET_DHCP_MSG_TYPE 2
#define DHCP_MSG_TYPE_REQUEST 3

int send_dhcp_offer(SOCKET *s_ptr);
int init_tx_socket(SOCKET *s_ptr);
int init_rx_socket(SOCKET *s_ptr);
int discover_client(SOCKET *s_ptr);
int client_request(SOCKET *s_ptr);
int client_ack(SOCKET *s_ptr);

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
        char pad[200];
        ULONG32 magic;
        char option_ptr[1];
    } dhcp_payload;

// udp header struct
typedef struct {
        USHORT src_port;
        USHORT dest_port;
        USHORT len;
        USHORT checksum;
        dhcp_payload payload;
    } udp_packet;
