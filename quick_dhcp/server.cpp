#include "quick_dhcp.h"

#define DHCP_PORT "67"

int get_client_discover();

int main(){
    WSADATA wsa;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsa);
    if(iResult != 0)
    {
        printf("WSAStartup Failed : %d\n", iResult);
        return 1;
    }

    // Listen for a DHCP Discover 
    iResult = get_client_discover();
    if(iResult != 0)
    {
        printf("Error Listening for DHCP Discover : %d\n", iResult);
        WSACleanup();
        return 1;
    }
    
    // Create a RAW socket for server responses
    // iResult = udp_test();
    // if(iResult != 0)
    // {
    //     printf("Error Sending UDP Response : %d\n", iResult);
    //     WSACleanup();
    //     return 1;
    // }
    
    return 0;
}

int get_client_discover()
{
    SOCKET s;
    int iResult;

    // Resolve address information for socket.
    // We need a socket that can listen for broadcasts
    struct addrinfo *result = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM; 
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;

   
    // get address profile of our host 
    iResult = getaddrinfo("192.168.1.150", DHCP_PORT, &hints, &result);
    if(iResult != 0)
    {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return -1;
    }

    // create socket
    s = INVALID_SOCKET;
    s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(s == INVALID_SOCKET)
    {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return -1;
    }
    printf("UDP Socket Created\n");

    // bind the socket to our network interface
    iResult = bind(s, result->ai_addr, (int)result->ai_addrlen);
    if(iResult == SOCKET_ERROR)
    {
        printf("Bind failed with error %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(s);
        return -1;
    }
    freeaddrinfo(result); 

    // set sock opt to receive broadcast
    bool optval = TRUE;
    iResult = setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(BOOL));
    if(iResult != 0)
    {
        printf("Error setting sockopt for receive : %d\n", iResult);
        return -1;
    }

    // Create the address profile of the client 
    sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_port = htons(67);
    client.sin_addr.s_addr = inet_addr("255.255.255.255");
    
    // Listen on socket
    printf("Waiting for DHCP Discovery.....\n");
    
    char buf[512];
    int clsize = sizeof(client);
    iResult = recvfrom(s, buf, sizeof(buf), 0, (sockaddr *)&client, &clsize);
    if(iResult > 0)
    {
        printf("Bytes Received : %d\n", iResult);
    }else if( iResult < 0) {
        printf("Recv Error : %d\n", iResult);
        return -1;
    }

    // interpret the discover packet
    // udp header is not in buffer because the socket is already udp type, not raw.
    dhcp_payload *received = (dhcp_payload*)buf;

    ULONG64 lownib = (long long)ntohs(received->chaddr_first);
    ULONG64 midnib = (long long)ntohs(received->chaddr_second);
    ULONG64 highnib = (long long)ntohs(received->chaddr_third);

    ULONG64 mac = 0xFFFFFFFFFFFFFFFF;
    mac = mac & ((lownib << 32) | (midnib << 16) | highnib);

    printf("DHCP Packet Contents:\n\n");
    printf("Message Type : %X\n", received->op);
    printf("Hardware Type : %X\n", received->htype);
    printf("Address Length : %X\n", received->hlen);
    printf("Hops : %X\n", received->hops);
    printf("Transaction ID : %X\n", ntohl(received->xid));
    printf("Seconds : %X\n", received->segs);
    printf("Flags : %X\n", received->flags);
    printf("Sender Address : %X\n", ntohl(received->ciaddr));
    printf("Receiver Address : %X\n", ntohl(received->yiaddr));
    printf("Server Address : %X\n", ntohl(received->siaddr));
    printf("Gateway Address : %X\n", ntohl(received->giaddr));
    printf("Hardware Address lo : %X\n", lownib);
    printf("Hardware Address mid : %X\n", midnib);
    printf("Hardware Address hi : %X\n", highnib);
    printf("Hardware Address : %llX\n", mac);
    printf("Magic Cookie : %X\n", received->magic);
    
    printf("done.\n");
    return 0;
}